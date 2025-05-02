#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stddef.h>
#include <pthread.h>

#define HOST "localhost"
#define PORT 59222
#define HISTORY_SIZE 2048
#define BUFFER_SIZE 1024

char buffer[BUFFER_SIZE];
char history[HISTORY_SIZE];

void* handle_update_history(void* arg)
{
	int client_fd = *((int*)arg);

	do
	{
		ssize_t bytes_rcvd = recv(client_fd, history, BUFFER_SIZE, 0);

		if (bytes_rcvd == 0)
		{
			printf("Connection closed by server.\n");
			exit(-1);
		}

		printf("%s\n", history);
	} 
	while (1);
}

int main()
{
	pthread_t update_thread;
	int client_fd = socket(AF_INET, SOCK_STREAM, 0);

	if (client_fd < 0)
	{
		perror("Could not open socket\n");
		exit(-1);
	}

	struct hostent* host_ptr = gethostbyname(HOST);
	
	if (host_ptr == NULL)
	{
		perror("Could not get host\n");
		exit(-1);
	}

	if (host_ptr->h_addrtype != AF_INET)
	{
		perror("Bad address family\n");
		exit(-1);
	}


	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = ((struct in_addr*)host_ptr->h_addr_list[0])->s_addr;
	server_addr.sin_port = htons(PORT);
	
	if (connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
	{
		perror("Could not connect to server\n");
		close(client_fd);
		exit(-1);
	}
	
	printf("Connection successful, gathering chat history\n");

	// Spawn thread for updating when server sends message data
	pthread_create(&update_thread, NULL, handle_update_history, (void*)&client_fd);

	//message
	//loop to send and recieve
	while (1) {
		//user input
		fgets(buffer, BUFFER_SIZE, stdin);
		
		//send input message
		send(client_fd, buffer, strlen(buffer), 0);
	}
	

	close(client_fd);
	return 0;
}
