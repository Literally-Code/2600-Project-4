#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stddef.h>

#define HOST "localhost"
#define PORT 59222
#define HISTORY_SIZE 2048
#define BUFFER_SIZE 1024

int main()
{
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

	//message
	char buffer[BUFFER_SIZE];
	char history[HISTORY_SIZE];
	//loop to send and recieve
	while (1) {
		//get current chat and print
		int total = 0;

		ssize_t bytes_rcvd = recv(client_fd, history, HISTORY_SIZE, 0);
		printf("%d%s\n", bytes_rcvd, history);

		if (bytes_rcvd == 0)
		{
			printf("Connection terminated by server.\n");
			break;
		}

		//user input
		printf("You: ");
		fgets(buffer, BUFFER_SIZE, stdin);
		
		//send input message
		send(client_fd, buffer, strlen(buffer), 0);
	}
	
	close(client_fd);
	return 0;
}
