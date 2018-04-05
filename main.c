// Client-side UDP Code 
// Adapted for use from "Beej's Guide to Network Programming" (C) 2017
// Modified by Christopher deWolf

#include <stdio.h>		
#include <stdlib.h>		
#include <unistd.h>		// System calls, also declares STDOUT_FILENO
#include <errno.h>	    	// Pre-defined C error codes (in Linux)
#include <string.h>		
#include <sys/types.h>  	// Definitions of system data types
#include <sys/socket.h> 	// Unix Socket libraries for TCP, UDP ...
#include <netinet/in.h> 	// INET constants
#include <arpa/inet.h>  	// Conversion of IP addresses ...
#include <netdb.h>		// Network database operations, including getaddrinfo

// constants
#define MAXBUF 10000      //max buffer size
#define MAXPATH 500
#define SRVR_PORT "6666"  // Server port recieving data from

int main(int argc, char *argv[]) {
	
    int sockfd;			 
    struct addrinfo hints, *servinfo, *p; // Address structure and pointers to them
    int rv, nbytes, nread;	  
    char buf[MAXBUF];    
    char newbuf[MAXBUF]; 
	
			// Handles with any incorrect input formats
    if (argc != 3) {								
        fprintf(stderr,"ERROR! Correct Usage is: ./program_name server userid\n"
		        "Where,\n    server = server_name or ip_address, and\n"
		        "    userid = your userid\n");
        exit(1);
    }

	
    memset( &hints, 0, sizeof hints); 
    hints.ai_family = AF_UNSPEC;     // AF_UNSPEC means IPv4 or IPv6, either is fine
    hints.ai_socktype = SOCK_DGRAM;  // SOCK_DGRAM is UDP

	
    if ((rv = getaddrinfo(argv[1], SRVR_PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
  
	// SOCKET CALL: pointer p is pointing to address structures
	//	        and first node in linked list with valid address.
	//		Does this until error or end of list, p->NULL
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("CLIENT: socket");
            continue;
		}
        break;
    }

	// p==NULL, cycled through whole linked list, but did not open a socket
    if (p == NULL) {
        fprintf(stderr, "CLIENT: failed to create socket\n");
        return 2;
    }

    if ((nbytes = sendto(sockfd, argv[2], strlen(argv[2]), 0,
             p->ai_addr, p->ai_addrlen)) == -1) {
        perror("CLIENT: sendto");
        exit(1);
    }

    printf("CLIENT: sent '%s' (%d bytes) to %s\n", argv[2], nbytes, argv[1]); //prints out to terminal number of bytes sent

	// Recv packet from server. This iterates through each packet from the server, and adds
	// each to a buffer called 'buf'. 'buf' is then written into a file 'fp' until all data is written.
	
	int i = 0;				// Counter variable for while loop

	char path[MAXPATH];				// Dynamically write to a file
	char fileName[MAXPATH];
	char thisPath[MAXPATH] = "/home/ubuntu/Desktop/udp_client/data_files/";

	strcpy( path, thisPath);	// Copy "thisPath" to "path"

	nread = recvfrom( sockfd, buf+(i*106), MAXBUF, 0, NULL, NULL);  //Grab the title of the file name in this loop and add it to path to write to
	for( int k = 0; k<MAXPATH; k++){
		fileName[k] = *(buf+(k)+4);
	}

	strcat( path, fileName);
	FILE *fp = fopen( path, "w");	// Reference and open the file to write to

	
	char fileSize[MAXBUF];			// Find the total file size to find number of packets for sorting
	for( int j = 0; j < 4; j++){
		
		char numb = *(buf+j);
		fileSize[j] = numb;			
	}
	
	int FILESIZE = atoi( fileSize);				
	printf("\n FILESIZE %i \n\n", FILESIZE);	// Debugging statement for total size of file
	
	int numPackets = (FILESIZE / 100);		// Divide by packet sizes and round up
	if( FILESIZE % 100 != 0){
		numPackets = numPackets + 1;
	}
	printf("\n numPACKETS %i \n\n", numPackets);
	
	i = 1;
	
	while( i <= numPackets ){		// Iterate through with this loop until all packets recieved
	
		nread = recvfrom( sockfd, buf+(i*106), MAXBUF, 0, NULL, NULL);	// read in next packet data
		if (nread<0) {
			perror("CLIENT: Problem in recvfrom");
			exit(1);
		}

		if( write( STDOUT_FILENO, buf+(i*106)+6, nread-6) < 0) {	// Compares and writes data to buffer (unsorted)
			perror("CLIENT: Problem writing to stdout");
			exit(1);
		}

	//printf("\nRECIEVED %d BYTES\n\n", nread);	// debugging info
		
		char packetNum[3];				//pull out packet number data (first 3)
		packetNum[0] = *(buf+(i*106));
		packetNum[1] = *(buf+(i*106)+1);
		packetNum[2] = '\0';

		int PACKETNUM = atoi( packetNum);		//turn the ascii data into integer data to use

		printf("\nPacket number %d \n\n", PACKETNUM);	   //prints packet # under each packet in buffer for debugging

		memcpy( newbuf+(PACKETNUM*100), buf+(i*106)+6, 100);		//copy each packet to it's sorted spot using packet # into a new buffer
			
		i++;
	}

	fwrite( newbuf, FILESIZE, 1, fp);	   //write sorted buffer to file
	
	fclose(fp);		// close up file after writing to it

	// free up the linked-list memory and close the socket
	freeaddrinfo(servinfo);
	close(sockfd);
	
	printf("\n\n"); 
	
    return 0;
}
