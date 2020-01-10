#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <strings.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>

#define SERVER_TCP_PORT 3000
#define BUFLEN		100	/* length of buffer */

int echod(int);
void reaper(int);

int main(int argc, char **argv)
{
	int 	sd, new_sd, client_length, port;
	struct	sockaddr_in server, client;

	switch(argc){
	case 1:
		port = SERVER_TCP_PORT;
		break;
	case 2:
		port = atoi(argv[1]);
		break;
	default:
		fprintf(stderr, "Usage: %d [port]\n", argv[0]);
		exit(1);
	}

	/* Create a stream socket	*/	
	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		fprintf(stderr, "Can't creat a socket\n");
		exit(1);
	}

	/* Bind an address to the socket	*/
	bzero((char *)&server, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(sd, (struct sockaddr *)&server, sizeof(server)) == -1){
		fprintf(stderr, "Can't bind name to socket\n");
		exit(1);
	}

	/* queue up to 5 cocurrent clients  */
	listen(sd, 5);

	(void) signal(SIGCHLD, reaper);

	while(1) {
	  client_length = sizeof(client);
	  new_sd = accept(sd, (struct sockaddr *)&client, &client_length);
	  if(new_sd < 0){
	    fprintf(stderr, "Can't accept client \n");
	    exit(1);
	  }
	  switch (fork()){
	  case 0:		/* child process*/
		(void) close(sd);
		exit(echod(new_sd));
	  default:		/* parent process */
		(void) close(new_sd);
		break;
	  case -1:
		fprintf(stderr, "fork: error\n");
	  }
	}
}

int echod(int sd)
{
	char	*bp, buf[BUFLEN], tempFile[BUFLEN];
	int 	n, bytes_to_read;

	
	int c, fre, lengthgth, length,ffile;

	char errormsg[] = "Error  \n";
	
	struct PDU {
		char type;
		unsigned int length;
		char data[100];

	} tx, rx;
	
	struct stat fstat;

	struct dirent *dp;
	FILE *fp;

while (n = read(sd, &rx, sizeof(rx))){
	
	printf("%s", rx.data); //Prints received information for debugging purposes

 	/*Upload Section*/
	 if (rx.type == 'U'){  
	  
	  tx.type = 'R'; //Header type R
	  write(sd, &tx, sizeof(tx)); //Writes acknowledgmentto client
	  length = rx.length;
	  //printf("%d",length);
	  lengthgth = 0;
		fp = fopen(rx.data,"w+"); 
	 while(n = read(sd, &rx, sizeof(rx))){ //Reads file
	 fprintf(fp,"%s", rx.data);	//Prints to new file
	    //printf("%d",rx.length);
		printf("%s",rx.data);
		if (rx.length <100){
	      fclose(fp);
		  //fflush(sd)
		break;
		}
	  }
		

	}
	/*End Upload Section*/


	/*Download Section*/
	else if (rx.type == 'D'){
			n = read(c, tx.data, 100); //Checks filename

	snprintf(tempFile, rx.length, "%s", rx.data);
	c = open(tempFile, O_RDONLY); 
	if(c == -1){ //file doesn't exist
 

		tx.type = 'E';
		snprintf(tx.data, sizeof(errormsg), "%s", errormsg);
		tx.length = sizeof(errormsg);
		write(sd, &tx, sizeof(tx));
	} else { //File exists
		lstat(tempFile, &fstat);
		lengthgth =0;
		tx.type = 'F'; //Header type F
		while (lengthgth < fstat.st_size){
			n = read(c, tx.data, 100);
			tx.length = n;
			write(sd, &tx, sizeof(tx)); //Write file information to Client
			lengthgth += n;
		}

		close(c);
		
	}

	}
	/*End Download Section*/

	//List Directory Section
	else if (rx.type == 'L'){
	  
	  snprintf(tempFile,sizeof(tx.data),"%s",rx.data);
	  
	  DIR *dfd = opendir(tempFile);

	  if(dfd != NULL){ //Contents in directory exist aka directory exists
	    while((dp = readdir(dfd)) != NULL){//Reads until nothing is left in directory
	      snprintf(tx.data, sizeof(dp->d_name), "%s",dp->d_name);
	      tx.type = 'I'; //Header type I
	      write(sd, &tx, sizeof(tx)); //Outputs contents to client
	    }
	    closedir(dfd);
	    tx.type = 'x';
	    write(sd, &tx, sizeof(tx));
	  }

	}
	//List Directory Section
	

	//Change Direectory Section
	else if (rx.type == 'P'){
	  snprintf(tempFile, sizeof(tx.data),"%s",rx.data);
	  ffile = chdir(tempFile); //See if directory exists
	  if (ffile == 0){ //Success case
	    tx.type = 'R';
	    write(sd, &tx, sizeof(tx));
	  }
	    else{ //Fail case
	      tx.type = 'E';
	      write(sd,&tx,sizeof(tx));
	    }
	    
	}
	//End Change Direectory Section


}
	close(sd);
	close(c);
	return(0);
}

/*	reaper		*/
void	reaper(int sig)
{
	int	status;
	while(wait3(&status, WNOHANG, (struct rusage *)0) >= 0);
}
