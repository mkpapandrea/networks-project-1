/* tcp_ client.c */ 
/* Programmed by Nicholas Zahabiun */
/* 10/23/2019 */     

#include <stdio.h>          /* for standard I/O functions */
#include <stdlib.h>         /* for exit */
#include <string.h>         /* for memset, memcpy, and strlen */
#include <netdb.h>          /* for struct hostent and gethostbyname */
#include <sys/socket.h>     /* for socket, connect, send, and recv */
#include <netinet/in.h>     /* for sockaddr_in */
#include <unistd.h>         /* for close */

#define STRING_SIZE 1024

int main(void) {
	setbuf(stdout, NULL);
	
	/* All the output file vareables */
	FILE *outfptr; // pointer to the output file
	outfptr = fopen("out.txt", "w");
	if(outfptr == NULL)
	{
		printf("Error!");
		exit(1);
	}
	
	/* End output file vareables */
	int sock_client;  /* Socket used by client */

	struct sockaddr_in server_addr;  /* Internet address structure that
										stores server address */
	struct hostent * server_hp;      /* Structure to store server's IP
										address */
	char server_hostname[STRING_SIZE] = "localhost"; /* Server's hostname */
	unsigned short server_port;  /* Port number used by server (remote port) */

	char sentence[STRING_SIZE];  /* send message */
	char modifiedSentence[STRING_SIZE]; /* receive message */
	unsigned int msg_len;  /* length of message */                      
	int bytes_sent; // number of bytes sent
	int bytes_recd = 1;  //or received 
  
	/* open a socket */

	if ((sock_client = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		perror("Client: can't open stream socket");
		exit(1);
	}

	/* Note: there is no need to initialize local client address information 
			unless you want to specify a specific local port
			(in which case, do it the same way as in udpclient.c).
			The local address initialization and binding is done automatically
			when the connect function is called later, if the socket has not
			already been bound. */

	/* initialize server address information */

	//printf("Enter hostname of server: %s \n", server_hostname);
	//scanf("%s", server_hostname);
	//server_hostname = "localhost";
	if ((server_hp = gethostbyname(server_hostname)) == NULL) {
		perror("Client: invalid server hostname");
		close(sock_client);
		exit(1);
	}

	//printf("Enter port number for server: ");
	server_port = 5321;
	//printf("\nServer port number: %d \n", server_port);
	//scanf("%hu", &server_port);

	/* Clear server address structure and initialize with server address */
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	memcpy((char *)&server_addr.sin_addr, server_hp->h_addr,
									server_hp->h_length);
	server_addr.sin_port = htons(server_port);

	/* connect to the server */
 		
	if (connect(sock_client, (struct sockaddr *) &server_addr, 
									sizeof (server_addr)) < 0) {
		perror("Client: can't connect to server");
		close(sock_client);
		exit(1);
	}
   
	/* user interface */
	/* This "for loop" is for taking in multiple files, but it is not used yet */
	for(int i = 0; i < 10; i++){ 
		printf("Please input the name of the file to recive:\n");
		fgets(sentence, STRING_SIZE, stdin); // Take in text file name
		sentence[strcspn(sentence, "\n")] = 0; // Remove new line from text file name
		msg_len = strlen(sentence) + 1; // Take into account the NULL character at the end of the string
	   
		/* send message */
	   
		bytes_sent = send(sock_client, sentence, msg_len, 0); // Send the name of string to server

		/* get response from server */
	  
		//bytes_recd = recv(sock_client, modifiedSentence, STRING_SIZE, 0); 
		int inplay = 1; // This variable is a flag for the end transmition packet
		int totalData = 0; // This is the summation of all packet data recived in bytes
		int numPackets = 0; // This is the number of data packets recived
		while((inplay == 1) && (bytes_recd > 0)){ // This makes sure there are more packets to recive and packets are being recived
		
			// First Recive the header packet
			uint16_t header[2]; // Empty array to be filled with header packet data
			bytes_recd = recv(sock_client, header, 4, 0); // recive header packet
			if (bytes_recd != 4) { // make sure header is correct size
				printf("Header problem");
			}
			uint16_t packetNumber = ntohs(header[1]); // Convert packet number
			uint16_t packetLength = ntohs(header[0]); // Convert packet length
			if (packetLength > 81){ // make sure legth is within allowed size
				printf("packet length error");
			}
			if ( packetLength == 0 ){ // Check if it is end of transmition packet
				printf("End of Transmission Packet with sequence number %d received with %d data bytes\n", packetNumber, bytes_recd);
				fclose(outfptr); // Close out put file
				printf("Number of data packets received: %d\nTotal number of data bytes received: %d\n", (numPackets - 1), totalData);
				inplay = 0; // The file reading is nolonger in play
			}

			// Second recive data packet
			char datapacket[STRING_SIZE];
			if(inplay != 0){ // Make sure the end of transmition packet did not come
				bytes_recd = recv(sock_client, datapacket, packetLength, 0); // read data
				numPackets += 1; // Add to packet itterater
				totalData += bytes_recd; // count total data recived
				if(bytes_recd > 81){ // make sure bytes recived are not more than 80 characters including new line + end string
					printf("packet length error");
				}
				datapacket[bytes_recd] = '\0'; // All NULL terminator to make char[] a string
				fputs(datapacket, outfptr); // Write to output file
				printf("Packet %d received with %d data bytes\n", packetNumber, (bytes_recd - 1));
				if((bytes_recd != packetLength) || (bytes_recd > 81)){ // make sure the length of the packet same as bytes read
					printf("\nBIG ERROR BIG ERROR\nBytes recived: %d\nBytes in header: %d", bytes_recd, packetLength);
				}
			}

		}
		inplay = 1;
		//printf("\nwhile loop exited\n");
		
	}


   /* close the socket */

   close (sock_client);
}
