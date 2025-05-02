#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stddef.h>

#define PORT 59222
#define MAX_CONNECTIONS 10
#define BUFFER_SIZE 1024
#define HISTORY_SIZE 2048
#define PADDING 8
#define NAME_SIZE 16

// 'id' is the same as its index in 'connection connections[]'
typedef struct
{
	int id;
	char client_name[NAME_SIZE];
	struct sockaddr_in *addr_in;
	int client_fd;
	char active;
	pthread_t thread;
} connection;

pthread_mutex_t history_mutex = PTHREAD_MUTEX_INITIALIZER;
FILE* history_file;
const char* history_location = "./mhist";
int server_fd;
connection connections[MAX_CONNECTIONS] = {0};
pthread_t interface_thread;

void* handle_interface(void* arg)
{
	printf("Running server interface. Press ENTER to close the server\n> ");
	getc(stdin);
	close(server_fd);

	for (int i = 0; i < MAX_CONNECTIONS; i++)
	{
		if (connections[i].active)
		{
			connections[i].active = 0;
			close(connections[i].client_fd);
		}
	}
	exit(0);
}

void* handle_client(void* arg)
{
	printf("Connecting client...\n");
	connection* client_conn = (connection*)arg;
	struct sockaddr_in* client_addr = client_conn->addr_in;

	char message[BUFFER_SIZE];
	char history[HISTORY_SIZE];

	do
	{
		// Send current chat history
		history_file = fopen(history_location, "r");
		fread(history, sizeof(char), HISTORY_SIZE, history_file);
		printf("Sending history: %s\n", history);
		
		// Send to each connection
		for (int i = 0; i < MAX_CONNECTIONS; i++)
		{
			if (connections[i].active)
			{
				send(connections[i].client_fd, history, strlen(history), 0);
			}
		}
		printf("History sent\n");
		fclose(history_file);

		// Process message
		printf("Processing next message...\n");
		// I can't spell recie received recieved??
		ssize_t bytes_rcvd = recv(client_conn->client_fd, message, BUFFER_SIZE - 1, 0);
		// Set the last character to a null terminator jusssst in case
		message[BUFFER_SIZE - 1] = '\0';

		// Handle disconnection
		if (bytes_rcvd == 0)
		{
			connections[client_conn->id].active = 0;
			printf("Disconnection\n");
			// printf("Client \'%s\' at %s%s disconnected\n", client_conn->client_name, inet_ntoa(client_addr->sin_addr), ntohs(client_addr->sin_port));
			close(client_conn->client_fd);
			return 0;
		}
		
		printf("Updating chat history with '%s'...\n", message);
		// Lock history file for appending client's message
		pthread_mutex_lock(&history_mutex);
		history_file = fopen(history_location, "a+");
		fprintf(history_file, message);
		int f_size = ftell(history_file);
		
		if (f_size > HISTORY_SIZE)
			fseek(history_file, -HISTORY_SIZE, SEEK_END);
		else
			fseek(history_file, 0, SEEK_SET);

		fread(history, sizeof(char), HISTORY_SIZE, history_file);

		fclose(history_file);
		pthread_mutex_unlock(&history_mutex);
	} while (1);
}

int get_open_spot()
{
	for (int i = 0; i < MAX_CONNECTIONS; i++)
	{
		printf("Checking connection %d activity status: %d\n", i, connections[i].active);
		if (!connections[i].active)
			return i;
	}
	return -1;
}

int main()
{
	struct sockaddr_in server_addr;

	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("Socket connection failed\n");
		exit(-1);
	}

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(PORT);

	if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
	{
			perror("Socket bind error\n");
			exit(-1);
	}

	if (listen(server_fd, MAX_CONNECTIONS) < 0)
	{
		perror("Listen error\n");
		exit(-1);
	}

	printf("Listening on port %d\n", PORT);
	
	int create_interface_result = pthread_create(&interface_thread, NULL, handle_interface, NULL);

	// Connection loop
	while (1)
	{
		struct sockaddr_in client_addr;
		int len = sizeof(client_addr);
		int client_fd;

		if ((client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &len)) < 0)
		{
			perror("Could not connect to client\n");
			continue;
		}

		// Do stuff
		
		int next_free_spot = get_open_spot();
		
		if (next_free_spot == -1)
		{
			perror("Maximum connections reached, cannot accept additional connection\n");
			continue;
		}

		connections[next_free_spot].id = next_free_spot;
		connections[next_free_spot].active = 1;
		connections[next_free_spot].client_fd = client_fd;
		// TODO: Add name implementation
		connections[next_free_spot].client_name[0] = '\0';
		connections[next_free_spot].addr_in = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));
		*(connections[next_free_spot].addr_in) = client_addr; 
		
		// Spawn new thread. Thread will run indefinitely, handling client connections 
		int create_thread_result = pthread_create(&(connections[next_free_spot].thread), NULL, handle_client, (void*)&connections[next_free_spot]);

		if (create_thread_result != 0)
		{
			connections[next_free_spot].active = 0;
			perror("pthread_create failed\n");
			continue;
		}
		

		// Detach the thread because we don't care about it's result
		pthread_detach(connections[next_free_spot].thread);
	}

	pthread_mutex_destroy(&history_mutex);
	return 0;
}
