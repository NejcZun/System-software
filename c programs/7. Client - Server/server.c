#include <stdio.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <err.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#define MAX 80 
#define PORT 8080
#define SA struct sockaddr 


double evaluate(char *string){
	float num1, num2;
	char exp[2];
    sscanf(string,"%f %s %f",&num1, exp, &num2);
	if(strcmp(exp, "+")==0) return num1+num2;
	if(strcmp(exp, "-")==0) return num1-num2;
	if(strcmp(exp, "/")==0) return num1/num2;
	if(strcmp(exp, "*")==0) return num1*num2;
    return -99999;
} 



// Function designed for chat between client and server. 
void start(int sockfd) { 
	char buff[MAX]; 
	int n; 
	// infinite loop for chat 
	while (1) { 
		bzero(buff, MAX); 

		// read the message from client and copy it in buffer 
		read(sockfd, buff, sizeof(buff)); 
		printf("[Client to Server]: %s", buff);
		// client requests to math
		double res = evaluate(buff);
		printf("[Server to Client]: Result: %f\n\n", res); 
		bzero(buff, MAX); //delete the buff
		
		//copy result into memory of buffer
		sprintf(buff,"%f",res);
		if(res == -99999)sprintf(buff,"%s","Invalid expression use: <num> <expression> <num>");
		// and send that buffer to client 
		write(sockfd, buff, sizeof(buff)); 
	} 
} 

// Driver function 
int main() { 

	int sockfd, connfd, len; 
	struct sockaddr_in servaddr, cli; 

	// socket create and verification 
	if( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) perror("Napaka pri socket!"); 
	else printf("Socket successfully created..\n"); 
	
	bzero(&servaddr, sizeof(servaddr)); 

	// assign IP, PORT 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
	servaddr.sin_port = htons(PORT); 

	// Binding newly created socket to given IP and verification 
	if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) { perror("Socket bind failed.\n");exit(0);} 
	else printf("Socket successfully binded..\n"); 

	// Now server is ready to listen and verification 
	if ((listen(sockfd, 128)) != 0) {perror("Socket listen failed.\n");exit(0);} 
	else printf("Server listening..\n");
	
	len = sizeof(cli); 

	// Accept the data packet from client and verification 
	if ( (connfd = accept(sockfd, (SA*)&cli, &len)) < 0){perror("Accept failed.\n");exit(0);} 
	else printf("server acccept the client...\n"); 

	// Function for chatting between client and server 
	start(connfd); 

	// After chatting close the socket 
	close(sockfd); 
} 
