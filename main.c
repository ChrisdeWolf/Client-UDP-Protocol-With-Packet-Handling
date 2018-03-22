// Client-side UDP Code 
// Written by Sarvesh Kulkarni <sarvesh.kulkarni@villanova.edu>
// Adapted for use from "Beej's Guide to Network Programming" (C) 2017
// Modified by Christopher deWolf
//===================================================================================================
// IMPORTANT: Must open buffer-written file with LibreOffice, gedit produces poem in another language.
//===================================================================================================

#include <stdio.h>		// Std i/o libraries - obviously
#include <stdlib.h>		// Std C library for utility fns & NULL defn
#include <unistd.h>		// System calls, also declares STDOUT_FILENO
#include <errno.h>	    // Pre-defined C error codes (in Linux)
#include <string.h>		// String operations - surely, you know this!
#include <sys/types.h>  // Defns of system data types
#include <sys/socket.h> // Unix Socket libraries for TCP, UDP, etc.
#include <netinet/in.h> // INET constants
#include <arpa/inet.h>  // Conversion of IP addresses, etc.
#include <netdb.h>		// Network database operations, incl. getaddrinfo

// Our constants ..
#define MAXBUF 10000      // 4K max buffer size for i/o over nwk
#define SRVR_PORT "6666"  // the server's port# to which we send data changed to 6666
						  // NOTE: ports 0 -1023 are reserved for superuser!



int main(int argc, char *argv[]) {
	
    int sockfd;			 // Socket file descriptor; much like a file descriptor
    struct addrinfo hints, *servinfo, *p; // Address structure and ptrs to them
    int rv, nbytes, nread;	  // nread is important for later, it equals number of bytes in each packet
	char buf[MAXBUF];    // Size of our network app i/o buffer
	char newbuf[MAXBUF]; // newbuffer

    if (argc != 3) {
        fprintf(stderr,"ERROR! Correct Usage is: ./program_name server userid\n"
		        "Where,\n    server = server_name or ip_address, and\n"
		        "    userid = your LDAP (VU) userid\n");
        exit(1);
    }

	// First, we need to fill out some fields of the 'hints' struct
    memset(&hints, 0, sizeof hints); // fill zeroes in the hints struc
    hints.ai_family = AF_UNSPEC;     // AF_UNSPEC means IPv4 or IPv6; don't care
    hints.ai_socktype = SOCK_DGRAM;  // SOCK_DGRAM means UDP

	// Then, we call getaddrinfo() to fill out other fields of the struct 'hints
	// automagically for us; servinfo will now point to the addrinfo structure
	// of course, if getaddrinfo() fails to execute correctly, it will report an
	// error in the return value (rv). rv=0 implies no error. If we do get an
	// error, then the function gai_strerror() will print it out for us
    if ((rv = getaddrinfo(argv[1], SRVR_PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // We start by pointing p to wherever servinfo is pointing; this could be
	// the very start of a linked list of addrinfo structs. So, try every one of
	// them, and open a socket with the very first one that allows us to
	// Note that if a socket() call fails (i.e. if it returns -1), we continue
	// to try opening a socket by advancing to the next node of the list
	// by means of the stmt: p = p->ai_next (ai_next is the next ptr, defined in
	// struct addrinfo).
	//						SOCKET CALL: pointer p is pointing to address structures
	//									and first node in linked list with valid address.
	//									Does until error or end of list, p->NULL
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("CLIENT: socket");
            continue;
		}
        break;
    }

	// OK, keep calm - if p==NULL, then it means that we cycled through whole
	// linked list, but did not manage to open a socket! We have failed, and
	// with a deep hearted sigh, accept defeat; with our tail between our legs,
	// we terminate our program here with error code 2 (from main).
    if (p == NULL) {
        fprintf(stderr, "CLIENT: failed to create socket\n");
        return 2;
    }

	// If p!=NULL, then things are looking up; the OS has opened a socket for
	// us and given us a socket descriptor. We are cleared to send! Hurray!
	// The sendto() function will report how many bytes (nbytes) it sent; but a
	// negative value (-1) means failure. Sighhh. 
	//						SEND TO socket i just created and addresses I just populated
	//								argv[0] is first char in first argument
    if ((nbytes = sendto(sockfd, argv[2], strlen(argv[2]), 0,
             p->ai_addr, p->ai_addrlen)) == -1) {
        perror("CLIENT: sendto");
        exit(1);
    }

    printf("CLIENT: sent '%s' (%d bytes) to %s\n", argv[2], nbytes, argv[1]); //prints out to terminal number of bytes I sent

	// Recv packet from server. This iterates through each packet from the server, and adds
	// each to a buffer called 'buf'. 'buf' is then written into a file 'fp' until all data is written.
	
	int i = 0;				// counter variable for while loop
	int dum = 0;			// dummy variable currently not used in program but used for potential fwrite command

	

	char path[300];												//dynamically write to a file
	char fileName[300];
	char thisPath[300] = "/home/ubuntu/Desktop/udp_client/data_files/";

	strcpy( path, thisPath);

	nread = recvfrom( sockfd, buf+(i*106), MAXBUF, 0, NULL, NULL);  //Grab the title of the file name in this loop and add it to path to write to
	for( int k = 0; k<300; k++){
		fileName[k] = *(buf+(k)+4);
	}

	strcat( path, fileName);
	FILE *fp = fopen( path, "w");	//reference and open the file to write to

	
	char fileSize[MAXBUF];			//find the total file size to find number of packets for sorting
	for( int j = 0; j < 4; j++){
		
		char numb = *(buf+j);
		fileSize[j] = numb;			
	}
	
	int FILESIZE = atoi( fileSize);				
	printf("\n FILESIZE %i \n\n", FILESIZE);	//debugging statement for total size of file
	
	int numPackets = (FILESIZE / 100);			//divide by packet sizes and round up
	if( FILESIZE % 100 != 0){
		numPackets = numPackets + 1;
	}
	printf("\n numPACKETS %i \n\n", numPackets);
	
	i = 1;
	
	while( i <= numPackets ){							//iterate through with this loop until all packets recieved
	
		nread = recvfrom( sockfd, buf+(i*106), MAXBUF, 0, NULL, NULL);	//returns number of bytes in entire packet recieved
		if (nread<0) {
			perror("CLIENT: Problem in recvfrom");
			exit(1);
		}

		if( write( STDOUT_FILENO, buf+(i*106)+6, nread-6) < 0) {				//compares and writes to buffer (unsorted)
			perror("CLIENT: Problem writing to stdout");
			exit(1);
		}
		
		
	// we recvd our very first packet from sender: <filesize,filename>

	//printf("\nRECIEVED %d BYTES\n\n", nread);
		
	//Output recvd data to stdout; 

		
		char packetNum[3];							//pull out packet number data
		packetNum[0] = *(buf+(i*106));
		packetNum[1] = *(buf+(i*106)+1);
		packetNum[2] = '\0';

		int PACKETNUM = atoi( packetNum);			//turn the ascii data into integer data to use

		printf("\nPacket number %d \n\n", PACKETNUM);	   //prints packet # under each packet in buffer for debugging

		memcpy( newbuf+(PACKETNUM*100), buf+(i*106)+6, 100);			//copy each packet to it's sorted spot using packet # into a new buffer
			
		i++;
	
		dum = dum + nread;	  // adding number bytes to dummy variable each packet for fwrite print(NOT USED)
		
	}

	
	fwrite( newbuf, FILESIZE, 1, fp);	   //write sorted poem to file
	
	fclose(fp);		//close up file after writing to it

	
	// AFTER all packets have been received ....
	// free up the linked-list memory that was allocated for us so graciously
	// getaddrinfo() above; and close the socket as well - otherwise, bad things
	// could happen
	freeaddrinfo(servinfo);
	close(sockfd);
	
	printf("\n\n"); // So that the new terminal prompt starts two lines below
	
    return 0;
}