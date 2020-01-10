#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>


#define SERVER_TCP_PORT 3000
#define BUFLEN		100	/* length of buffer */

int main(int argc, char **argv)
{
	int 	n, i, bytes_to_read;
	int 	sd, port;
	struct	hostent		*hp;
	struct	sockaddr_in server;
	char	*host, *bp, receiveBuff[BUFLEN], sendBuff[BUFLEN];

	switch(argc){
	case 2:
		host = argv[1];
		port = SERVER_TCP_PORT;
		break;
	case 3:
		host = argv[1];
		port = atoi(argv[2]);
		break;
	default:
		fprintf(stderr, "Usage: %s host [port]\n", argv[0]);
		exit(1);
	}

	/* Create a stream socket	*/	
	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		fprintf(stderr, "Can't creat a socket\n");
		exit(1);
	}

	bzero((char *)&server, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	if (hp = gethostbyname(host)) 
	  bcopy(hp->h_addr, (char *)&server.sin_addr, hp->h_length);
	else if ( inet_aton(host, (struct in_addr *) &server.sin_addr) ){
	  fprintf(stderr, "Can't get server's address\n");
	  exit(1);
	}

	/* Connecting to the server */
	if (connect(sd, (struct sockaddr *)&server, sizeof(server)) == -1){
	  fprintf(stderr, "Can't connect \n");
	  exit(1);
	}
	
	struct pdu_header {
		char type;
		unsigned int len;
		char data[100];
	} tx,rx;

	char Tempbuffer[100];
	int length;
	int fd;
	struct stat fstat;
	char input;
	FILE *fp;
	
	
	
	printf("What do you want to do?\nd - download file\nu - upload a file\nl - list files in  directory\np - change directory\n");


	while(input = getchar()){	/* gets user input */
	
	/*Upload Section*/
	else if (input == 'u'){
		printf("File to upload:\n");
		while (n = read(0,sendBuff,BUFLEN))
		{
			snprintf(Tempbuffer,n,"%s",sendBuff);
			fd = open(Tempbuffer, O_RDONLY);
			if (fd != -1)
				break;
			else
				printf("file doesn't exist on client\n");
			printf("File to upload to server:\n");
		}
		
		lstat(Tempbuffer, &fstat);
		tx.type = 'U'; //Header type U
		snprintf(tx.data, n, "%s", sendBuff);
		tx.len = sizeof(tx.data);
		//printf("%d",tx.len);
		write(sd, &tx, sizeof(tx)); //Write to server with header U and filename

		
		while (n=read(sd,&rx,sizeof(rx))){ //Checks server response
			if (rx.type == 'R'){ //Success case
				length =0;
				while(length<fstat.st_size){
					n=read(fd,rx.data, 100);
					rx.len=n;
					write(sd, &rx, sizeof(rx));
					length +=n;
				}
				close(fd);
				break;
			}
			
		
		}
			
	
	}
	/*End Upload Section*/


	/*Download Section*/
	if (input == 'd'){ 		
		printf("Filename of the file  to download:\n");
		n=read(0, sendBuff, BUFLEN);
	


		tx.type = 'D'; //Header type D
		tx.len = n;
		snprintf(tx.data, n, "%s", sendBuff);
		write(sd, &tx, sizeof(tx)); //Writes to server with header and filename
		fp = fopen(tx.data,"w+"); /* creates a new file to write  to */
		
		while(n = read(sd, &rx, sizeof(rx))){ //Reads server resp.
		  
		 
		  printf("%s",rx.data); //Outputs data for debugging
		fprintf(fp,"%s",rx.data); //Prints to file
		    if(rx.len <100){
			fclose(fp);
			break;
			}
		    
		  }
		
	}
	/*End Download Section*/

	/*List File Section*/
	else if (input == 'l'){ 
	  printf("List Directory Location:\n");
	  n = read(0,sendBuff,BUFLEN);
	  tx.type = 'L'; //header L
	  snprintf(tx.data,n,"%s",sendBuff);
	  tx.len = sizeof(tx.data);
	  write(sd, &tx, sizeof(tx));	//Writes to server with header and directory 

	  while(n = read(sd, &rx, sizeof(rx))){  //Reads server resp. to check if success
	    if (rx.type == 'I'){ //Success case returns with list
	      printf("%s",rx.data);
	      printf("\n");
	    }
	    if (rx.type == 'x'){ //Error case
			printf("Directory not located\n"); 
	      break;
	    }
	  }
	  
	}
	/*End List File Section*/

	/*Change Directory Section*/
	else if (input == 'p'){ 
	  
	  printf("Directory: \n");
	  n = read(0,sendBuff,BUFLEN);
	  tx.type = 'P'; //header P
	  snprintf(tx.data,n,"%s",sendBuff);
	  tx.len = sizeof(tx.data);
	  write(sd, &tx, sizeof(tx)); //Writes to server with header and directory

	  while(n = read(sd, &rx, sizeof(rx))){ //Reads server resp. to check  if successfull
	    if (rx.type == 'R'){  //Success case
	      printf("Directory Changed Successfully\n");
	      break;
	    }
	    if (rx.type == 'E'){  //Error case
	      printf("Directory not located successfully\n");
	      break;
	    }
	    
	  }
	

	}
	/*End Change Directory Section*/

		printf("What do you want to do?\nd - download file\nu - upload a file\nl - list files in  directory\np - change directory\n");

	
	}
	close(sd);
	return(0);
}
