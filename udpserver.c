/* 
 * udpserver.c - A simple UDP echo server 
 * usage: udpserver <port>
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define BUFSIZE 1024

/*
 * error - wrapper for perror
 */
void error(char *msg) {
  perror(msg);
  exit(1);
}




bool checkForEnd(char word[]){
    char *lowercaseWord = malloc (sizeof(char) * BUFSIZE);
    bool flag = false;
    strcpy(lowercaseWord, word);
    for(int i = 0;i < strlen(lowercaseWord); i++)
        lowercaseWord[i] = tolower(lowercaseWord[i]);

    if(strcmp(lowercaseWord, "end\n") == 0)
      flag = true;

    free(lowercaseWord);
    return flag;
}


int main(int argc, char **argv) {
  int sockfd; /* socket */
  int portno; /* port to listen on */
  int clientlen; /* byte size of client's address */
  struct sockaddr_in serveraddr; /* server's addr */ // defined in Socket API, is a struct used to specify an endpoints address
  struct sockaddr_in clientaddr; /* client addr */
  struct hostent *hostp; /* client host info */
  char buf[BUFSIZE]; /* message buf */
  char *hostaddrp; /* dotted decimal host addr string */
  int optval; /* flag value for setsockopt */
  int n; /* message byte size */
  
  //new define
  struct hostent *myent;
  int len = 0;
  char *buf2;
  struct in_addr  myen;
  long int *add;
  /* 
   * check command line arguments 
   */
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }
  portno = atoi(argv[1]);

  /* 
   * socket: create the parent socket 
   */
  sockfd = socket(AF_INET, SOCK_DGRAM, 0); //af__inet specifies ipv4, SOCK_dgram specifies a udp connection
  if (sockfd < 0) 
    error("ERROR opening socket");

  /* setsockopt: Handy debugging trick that lets 
   * us rerun the server immediately after we kill it; 
   * otherwise we have to wait about 20 secs. 
   * Eliminates "ERROR on binding: Address already in use" error. 
   */
  optval = 1;
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, 
	     (const void *)&optval , sizeof(int));

  /*
   * build the server's Internet address
   */
  bzero((char *) &serveraddr, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET; // the internet protocal used
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY); // htonl host byte order to network byte order, used to aid in confusion of byte ordering? INADDR_ANY is used when we dont know the ip address of our machine
  serveraddr.sin_port = htons((unsigned short)portno); // address port
  printf("Server port number: %hu (%d)\n",(unsigned short)serveraddr.sin_port,portno);

  /* 
   * bind: associate the parent socket with a port 
   */
  if (bind(sockfd, (struct sockaddr *) &serveraddr,  // associates and reserves a port for use by the socket
	   sizeof(serveraddr)) < 0) 
    error("ERROR on binding");

  /* 
   * main loop: wait for a datagram, then echo it
   */
  clientlen = sizeof(clientaddr);
  while (1) {

    /*
     * recvfrom: receive a UDP datagram from a client
     */
    bzero(buf, BUFSIZE);
    n = recvfrom(sockfd, buf, BUFSIZE, 0,
		 (struct sockaddr *) &clientaddr, &clientlen);
    if (n < 0)
      error("ERROR in recvfrom");
    

    /* 
     * gethostbyaddr: determine who sent the datagram
     */

    hostp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr, 
			  sizeof(clientaddr.sin_addr.s_addr), AF_INET);
    

    if (hostp == NULL)
      error("ERROR on gethostbyaddr");
    hostaddrp = inet_ntoa(clientaddr.sin_addr);
    if (hostaddrp == NULL)
      error("ERROR on inet_ntoa\n");
    printf("server received datagram from %s (%s)\n", 
	   hostp->h_name, hostaddrp);
    printf("client port number: %d\n",clientaddr.sin_port);
    printf("server received %d/%d bytes: %s\n", (int)strlen(buf), n, buf);


   
    /* 
     * sendto: echo the input back to the client 
     */
    int i = 0;
    char returnI[50];
    if(checkForEnd(buf)){
      strcpy(buf, "Bye");
      printf("Closing connection to: %s (%s)\n", 
	        hostp->h_name, hostaddrp);
      n = sendto(sockfd, buf, strlen(buf), 0, 
        (struct sockaddr *) &clientaddr, clientlen);
      if (n < 0) 
        error("ERROR in sendto");
    }else{
      char* token;
      token = strtok(buf, " ");
      while(token != NULL){
        token = strtok(NULL, " ");
        i++;
      }
      sprintf(returnI, "%d\n", i);
      n = sendto(sockfd, returnI, strlen(buf), 0, 
        (struct sockaddr *) &clientaddr, clientlen);
      if (n < 0) 
        error("ERROR in sendto");
    }

  }
}
