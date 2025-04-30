#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 59222
#define BUFFER_SIZE 1024
#define MAX_CONNECTIONS 10

pthread_mutex_t history_mutex = PTHREAD_MUTEX_INITIALIZER;
FILE* message_history;
const char* history_file = "./mhist";

void* handle_client(void* message)
{
	pthread_mutex_lock(history_mutex);
	
	message_history = fopen(history_file, "a");
	fprintf(message_history, (char*)message);
	fclose(history_file);

	pthread_mutex_unlock(history_mutex);
}

int main()
{
	int server_fd;
	struct sockaddr_in server_addr;
	char buffer[BUFFER_SIZE];
	pthread_t threads[MAX_CONNECTIONS];

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

		close(client_fd);
	}

	return 0;
}
