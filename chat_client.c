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


	
	close(client_fd);
	return 0;
}
