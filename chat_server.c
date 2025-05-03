#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stddef.h>

#define PORT 59222
#define MAX_CONNECTIONS 10
#define BUFFER_SIZE 512
#define HISTORY_SIZE 2048
#define PADDING 8
#define NAME_SIZE 16

// 'id' is the same as its index in 'connection connections[]'
typedef struct
{
	int id;
	int client_fd;
	char client_name[NAME_SIZE];
	struct sockaddr_in *addr_in;
	char active;
	pthread_t thread;
} connection;

pthread_mutex_t history_mutex = PTHREAD_MUTEX_INITIALIZER;
FILE* history_file;
const char* history_location = "./mhist";
int server_fd;
connection connections[MAX_CONNECTIONS];

void* handle_client(void* arg)
{
	connection* client_conn = (connection*)arg;
	struct sockaddr_in* client_addr =  client_conn->addr_in;

	char buffer[BUFFER_SIZE];
	char latest_history[HISTORY_SIZE];
	char updated_buffer[BUFFER_SIZE + HISTORY_SIZE + PADDING];

	while (1)
	{
		// I can't spell recie received recieved??
		// lol
		ssize_t bytes_rcvd = recv(client_conn->client_fd, buffer, BUFFER_SIZE - 1, 0);
		// Set the last character to a null terminator jusssst in case
		buffer[bytes_rcvd] = '0';

		// Handle disconnection
		if (bytes_rcvd == 0)
		{
			connections[client_conn->id].active = 0;
			printf("Client \'%s\' at %s%s disconnected\n", client_conn->client_name, inet_ntoa(client_addr->sin_addr), ntohs(client_addr->sin_port));
			return 0;
		}

		// Lock history file for appending client's message
		pthread_mutex_lock(&history_mutex);
		
		history_file = fopen(history_location, "a+");
		fread(latest_history, sizeof(char), HISTORY_SIZE, history_file);
		sprintf(updated_buffer, "%s%s\n", latest_history, buffer);

		fprintf(history_file, updated_buffer);
		fclose(history_file);

		pthread_mutex_unlock(&history_mutex);
		// Prayer said here
		send(client_conn->client_fd, updated_buffer, strlen(updated_buffer), 0);
	}
}

int get_open_spot()
{
	for (int i = 0; i < MAX_CONNECTIONS; i++)
	{
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

		// Spawn new thread. Thread will run indefinitely, handling client connections 
		connections[next_free_spot].client_fd = client_fd;

		int create_thread_result = pthread_create(&(connections[next_free_spot].thread), NULL, handle_client, (void*)&connections[next_free_spot]);

		if (create_thread_result != 0)
		{
			perror("pthread_create failed\n");
			continue;
		}
		
		connections[next_free_spot].id = next_free_spot;
		connections[next_free_spot].active = 1;
		// TODO: Add name implementation
		connections[next_free_spot].client_name[0] = '\0';

		// Detach the thread because we don't care about it's result
		pthread_detach(connections[next_free_spot].thread);

	}

	pthread_mutex_destroy(&history_mutex);
	return 0;
}
