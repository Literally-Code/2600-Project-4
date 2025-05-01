#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define HOST "localhost"
#define PORT 59222

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
	server_addr.sin_addr.s_addr = ((struct in_addr*)hptr->h_addr_list[0])->s_addr;
	server_addr.sin_port = htons(PORT);
	
	if (connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
	{
		perror("Could not connect to server\n");
		close(client_fd);
	}

	//message
	char buffer[1024];
	//loop to send and recieve
	while (1) {
		
		//user input
		printf("You: ");
		fgets(buffer, sizeof(buffer), stdin);
		
		//send input message
		send(client_fd, buffer, strlen(buffer), 0);
		
		//get replies
		memset(buffer, 0, sizeof(buffer));
		ssize_t bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

		if (bytes_received == 0) {
			printf("Connection closed by peer.");
			break;
		} 
		if (bytes_received < 0) {
			printf("Connection failed.");
			break;
		}
	}
	
	close(client_fd);
	return 0;
}
