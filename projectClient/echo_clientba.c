#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/stat.h>
#include <sys/fcntl.h>



#define SERVER_TCP_PORT 3000	/* well-known port */
#define BUFLEN		1000	/* buffer length */

int main(int argc, char **argv)
{
	int 	n, i, bytes_to_read;
	int 	sd, port;
	struct	hostent		*hp;
	struct	sockaddr_in server;
	char	*host, *bp, rbuf[BUFLEN], sbuf[BUFLEN];

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
		char data[1000];
	} sheader,rx;

	char nfile[1000];
	int length;
	int fd;
	int done;
	struct stat fstat;
	char func;
	FILE *fp;
	
	
	
	printf("What would you like to do?\nd - download file\nu - upload a file\nl - list files in  directory\np - change directory\n");


	while(func = getchar()){	/* get user input */
	if (func == 'd'){ 		/*download mode  */
		printf("Filename of the file you would like to download:\n");
		n=read(0, sbuf, BUFLEN);
	


		sheader.type = 'D';
		sheader.len = n;
		snprintf(sheader.data, n, "%s", sbuf);
		write(sd, &sheader, sizeof(sheader));
		fp = fopen(sheader.data,"w+"); /* creates file to write contents to */
		
		while(n = read(sd, &rx, sizeof(rx))){
		  
		 
		  printf("%s",rx.data);
		fprintf(fp,"%s",rx.data);
		    if(rx.len <1000){
			fclose(fp);
			done = 1;
			break;
			}
		    
		  }
		
	}
	else if (func == 'u'){ 	/* upload mode */
		printf("File to upload:\n");
		while (n = read(0,sbuf,BUFLEN))
		{
			snprintf(nfile,n,"%s",sbuf);
			fd = open(nfile, O_RDONLY);
			if (fd != -1)
			{
				done = 1;
				break;
			}	
			else
			{
				printf("file doesn't exist\n");
				done = 1;
			}
			printf("File to upload:\n");
		}
		
		lstat(nfile, &fstat);
		sheader.type = 'U';
		snprintf(sheader.data, n, "%s", sbuf);
		sheader.len = sizeof(sheader.data);
		write(sd, &sheader, sizeof(sheader));

		
		while (n=read(sd,&rx,sizeof(rx))){
			if (rx.type == 'R'){
				length =0;
				while(length<fstat.st_size){
					n=read(fd,rx.data, 1000);
					write(sd, &rx, sizeof(rx));
					length +=n;
				}
				close(fd);
				printf("file doesn't exist\n");
				break;
			}
			
		
		}
			
	
	}

	else if (func == 'p'){ /* change directory */
	  
	  printf("Directory: \n");
	  n = read(0,sbuf,BUFLEN);
	  sheader.type = 'P';
	  snprintf(sheader.data,n,"%s",sbuf);
	  sheader.len = sizeof(sheader.data);
	  write(sd, &sheader, sizeof(sheader));

	  while(n = read(sd, &rx, sizeof(rx))){
	    if (rx.type == 'R'){
	      printf("Directory Changed\n");
	      break;
	    }
	    if (rx.type == 'E'){
	      printf("Directory not found\n");
	      break;
	    }
	    
	  }
	

	}

	else if (func == 'l'){ /* list files in directory */
	  printf("Directory to check:\n");
	  n = read(0,sbuf,BUFLEN);
	  sheader.type = 'L';
	  snprintf(sheader.data,n,"%s",sbuf);
	  sheader.len = sizeof(sheader.data);
	  write(sd, &sheader, sizeof(sheader));

	  while(n = read(sd, &rx, sizeof(rx))){
	    if (rx.type == 'I'){
	      printf("%s",rx.data);
	      printf("\n");
	    }
	    if (rx.type == 'x'){
	      break;
	    }
	  }
	  
	}
		

	if (done)
	{
		close(sd);
		return(0);
	}	
	else
	{
		printf("What would you like to do?\nd - download file\nu - upload a file\nl - list files in  directory\np - change directory\n");
	}
	

	}


}
