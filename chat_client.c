#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stddef.h>
#include <pthread.h>
#include <netdb.h>
#include <ctype.h>

#define HOST "localhost"
#define PORT 59223
#define BUFFER_SIZE 512
#define NAME_SIZE 16

#define MIN_USERNAME_LENGTH 3

int client_fd;
char username[NAME_SIZE];

//helper function for the recieve thread
void* receive_messages(void* arg){
	char buffer[BUFFER_SIZE];

	while(1) {
		ssize_t bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
		
		printf("\033[2J\033[H");
		//error checking
		if (bytes_received == 0) {
            		printf("Connection closed by server.\n");
            		exit(-1);
					break;
        	} 
        
		if (bytes_received < 0) {
            		printf("Connection failed.\n");
            		exit(-1);
					break;
       		}

		//print receiving messages
		buffer[bytes_received] = '\0'; //safety
		printf("\n%s", buffer);
		printf("%s: ", username);
		fflush(stdout);
	}
	return NULL;
}

char processCommand(char *str) {
    int count = 0;
    for (int i = 0; str[i]; i++) {
        if (!isspace(str[i])) {
            str[count++] = str[i];
        }
    }
    str[count] = '\0';

	if (!strcmp(str, "!exit"))
	{
		printf("Connection closed by client\n");
		exit(0);
	}
	printf("Command not found\n");
}

int main()
{
	//init client_fd
	client_fd = socket(AF_INET, SOCK_STREAM, 0);
	
	//error checking client_fd
	if (client_fd < 0)
	{
		perror("Could not open socket\n");
		exit(-1);
	}

	//init host_ptr
	struct hostent *host_ptr = gethostbyname(HOST);
	
	//error checking host_pot
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

	//initializing server_addr
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = ((struct in_addr*)host_ptr->h_addr_list[0])->s_addr;
	server_addr.sin_port = htons(PORT);
	
	//error checking server_addr
	if (connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
	{
		perror("Could not connect to server\n");
		close(client_fd);
		exit(-1);
	}
	
	printf("Connection established. Type !exit to exit at any time\n");
	//get username
	char valid_username = 0;
	
	while (!valid_username)
	{
		printf("Enter your username: ");
		fgets(username, sizeof(username), stdin);
		username[strcspn(username, "\n")] = '\0';

		valid_username = 1;
		
		//incase user exits in username
		if (username[0] == '!')
		{
			processCommand(username);
			valid_username = 0;
			continue;
		}

		if (strlen(username) < MIN_USERNAME_LENGTH)
		{
			printf("Username must be at least %d characters long\n", MIN_USERNAME_LENGTH);
			valid_username = 0;
		}
	}

	//make receiving thread
	pthread_t receive_thread;
	if (pthread_create(&receive_thread, NULL, receive_messages, NULL) != 0) {
		perror("Could not create receive thread");
		close(client_fd);
		exit(-1);
	}

	char buffer[BUFFER_SIZE];
	
	//loop to send and receive
	while (1) {
		//user input
		//printf("%s: ", username);
		//fflush(stdout);
			
		if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
			perror("Input error");
			break;
		}

		
		if (buffer[0] == '\n'){
			printf("Type something to send a message\n%s: ", username);
			continue;
		}

		if (buffer[0] == '!')
		{
			processCommand(buffer);
			printf("%s: ", username);
			continue;
		}
		
		//printf("\033[2J\033[H");
		//fflush(stdout);

		char message[BUFFER_SIZE];
		snprintf(message, sizeof(message), "%s: %s", username, buffer);
		
		//send input message
		if(send(client_fd, message, strlen(message), 0) < 0) {
			perror("Message send failed.");
			break;
		}
	}
        
	pthread_join(receive_thread, NULL);
	close(client_fd);
	return 0;
}

