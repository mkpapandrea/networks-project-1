/* tcpserver.c */
/* Programmed by Nicholas Zahabiun */
/* 10/23/2019 */    

#include <ctype.h>          /* for toupper */
#include <stdio.h>          /* for standard I/O functions */
#include <stdlib.h>         /* for exit */
#include <string.h>         /* for memset */
#include <sys/socket.h>     /* for socket, bind, listen, accept */
#include <netinet/in.h>     /* for sockaddr_in */
#include <unistd.h>         /* for close */

#define STRING_SIZE 1024   

/* SERV_TCP_PORT is the port number on which the server listens for
   incoming requests from clients. You should change this to a different
   number to prevent conflicts with others in the class. */

#define SERV_TCP_PORT 5321


int main(void) {
	setbuf(stdout, NULL); // This removes the printf buffer
	
	/* This is all the file handeling vareables */
	
	FILE *filePointer; // Pointer to the file
	char *fileLine = NULL; // Char pointer file line string
	size_t fileLineLength = 0; // Length of file line
	ssize_t read; // Return of read function
	char *cpfileLine; // Copy of file line
	long unsigned int lencp = 0; // Copy of the file line length 
	
	/* End of file handeling vareables */
	
	int sock_server;  /* Socket on which server listens to clients */
	int sock_connection;  /* Socket on which server exchanges data with client */

	struct sockaddr_in server_addr;  /* Internet address structure that
                                        stores server address */
	unsigned int server_addr_len;  /* Length of server address structure */
	unsigned short server_port;  /* Port number used by server (local port) */

	struct sockaddr_in client_addr;  /* Internet address structure that stores client address */
	unsigned int client_addr_len;  /* Length of client address structure */

	char sentence[STRING_SIZE];  /* receive message */
	char modifiedSentence[STRING_SIZE]; /* send message */
	unsigned int msg_len;  /* length of message */
	int bytes_sent, bytes_recd; /* number of bytes sent or received */
	unsigned int i;  /* temporary loop variable */

	/* open a socket */

	if ((sock_server = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		perror("Server: can't open stream socket");
		exit(1);                                                
	}

   /* initialize server address information */
    
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl (INADDR_ANY);  /* This allows choice of
										any host interface, if more than one
										are present */ 
	server_port = SERV_TCP_PORT; /* Server will listen on this port */
	server_addr.sin_port = htons(server_port);

	/* bind the socket to the local server port */

	if (bind(sock_server, (struct sockaddr *) &server_addr,
									sizeof (server_addr)) < 0) {
		perror("Server: can't bind to local address");
		close(sock_server);
		exit(1);
	}                     

	/* listen for incoming requests from clients */

	if (listen(sock_server, 50) < 0) {    /* 50 is the max number of pending */
		perror("Server: error on listen"); /* requests that will be queued */
		close(sock_server);
		exit(1);
	}
	printf("I am here to listen ... on port %hu\n\n", server_port);
  
	client_addr_len = sizeof (client_addr);

	/* wait for incoming connection requests in an indefinite loop */


	sock_connection = accept(sock_server, (struct sockaddr *) &client_addr, 
                                         &client_addr_len);
                     /* The accept function blocks the server until a
                        connection request comes from a client */
	if (sock_connection < 0) {
		perror("Server: accept() error\n"); 
		close(sock_server);
		exit(1);
	}

	for (;;) {
		
		/* receive the name of the text file */
		bytes_recd = recv(sock_connection, sentence, STRING_SIZE, 0);

		if (bytes_recd > 0){ // If the paket recived was not empty
			
			int totalNumberBytes = 0; //This is the total number of bytes transmited
			
			filePointer = fopen(sentence, "r"); // Open the file to the pointer
			if(filePointer == NULL){ // make sure file was opened 
				printf("Can't open the file \n");
			} else { // if the file was opened
				uint16_t packetNum = 0; // packet number starts at zero for the file
				while ((read = getline(&fileLine, &fileLineLength, filePointer)) != -1) { // Read first line from file
					
					// First you send your packet header
					uint16_t header[2]; // Create header feild
					header[1] = htons(packetNum); // Convert packet number
					header[0] = htons((uint16_t)(read)); // Convert packet size
					bytes_sent = send(sock_connection, header, sizeof(header), 0); // send header packet
					
					// Second you send your data packet
					bytes_sent = send(sock_connection, fileLine, read, 0); // Send file's line
					totalNumberBytes += bytes_sent; // keep track of total bytes sent
					printf("Packet %d transmitted with %d data bytes\n",packetNum, (bytes_sent - 1));
					packetNum = packetNum + 1; // Itterate packet number
				}
				// Send end transmition packet
				uint16_t header[2]; // Create header feild
				header[1] = htons(packetNum); // Create header feild
				header[0] = htons(0); // No data packets
				bytes_sent = send(sock_connection, header, sizeof(header), 0);
				printf("End of Transmission Packet with sequence number %d transmitted with %d data bytes", packetNum, bytes_sent);
				printf("\nNumber of data packets transmitted: %d\nTotal number of data bytes transmitted: %d\n", (packetNum-1), totalNumberBytes);
			}
		
		}

	} 
   
	/* close the socket */

	close(sock_connection);
}






