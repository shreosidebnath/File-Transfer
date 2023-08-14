#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <ctype.h>
#define MAX_LENGTH 1024

void error(const char *msg) { 
    perror(msg); 
    exit(1); 
}

int main(int argc, char const *argv[]) {
  if (argc < 2) {
        printf("Usage: %s port\n", argv[0]);
        exit(1);
    }//arguments

    int port = atoi(argv[1]);
    printf("Using port: %d\n", port);
  int sockfd;
  struct sockaddr_in serv_addr;

  char buffer[MAX_LENGTH];

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    error("FAILED TO CREATE SOCKET!!!");
  } //create socket

  memset(&serv_addr, 0, sizeof(serv_addr));

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port);

  // Convert IPv4 and IPv6 addresses from text to binary form
  if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
    error("INVALID ADDRESS!!!");
  } 

  if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
    error("CONNECTION FAILURE!!!");
  } //connect with server

  //take user input for UCID and send it to server
  char ucid[MAX_LENGTH];
  printf("Enter your UCID: ");
  scanf("%s", ucid);
  // Check the length of UCID
  int len = strlen(ucid);
  if (len != 8) {
    printf("ERROR UCID must have 8 numbers!!!");
    return 1;
  }
  // Check if the UCID is right
  for (int i = 0; i < len; i++) {
    if (!isdigit(ucid[i])) {
      printf("ERROR UCID can only contain numbers!!!");
      return 1;
    }
  }
  int echeck_ucid=send(sockfd, ucid, strlen(ucid), 0);
  if (echeck_ucid < 0){
        error("ERROR sending UCID!!!");
    }

  //receive datetime from server
  int n = read(sockfd, buffer, MAX_LENGTH);
  buffer[n] = '\0';
  printf("Datetime from server: %s\n", buffer);
  if (n < 0) {
        error("ERROR receiving date time");
    }
  //extracting second from the time received
  char *p = buffer;
  int seconds = atoi(p + 17);

  //creating passcode
  char passcode[10];
  char num[10];
  sprintf(passcode,"%d",seconds);
  strncpy(num,ucid+4,4);
  num[4]='\0';
  strcat(passcode,num);

  //sending passcode
  int echeck_passcode = send(sockfd, passcode, strlen(passcode), 0);
  if (echeck_passcode < 0){
        error("ERROR sending passcode!!!");
    }
  printf("Sent passcode: %s\n", passcode);


  FILE *fp;
  fp = fopen("data.txt", "w");
  if(fp==NULL){
    error("error opening file");
  }
  int readBytes =0;
  while ((n = read(sockfd, buffer, MAX_LENGTH)) > 0) {
    readBytes+=n;
    buffer[n] = '\0';
    fprintf(fp, "%s", buffer);
  }
  printf("%d bytes Received from the server",readBytes);
  fclose(fp);

  return 0;
}
