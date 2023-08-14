#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#define MAX_LENGTH 1024

void error(const char *msg) { 
    perror(msg); 
    exit(1); 
}   //function for errors

int main(int argc, char *argv[]) {
    int listenSocketFD, establishedConnectionFD, port, charsRead, sentBytes;
    socklen_t sizeOfSockAddr;
    char buffer[MAX_LENGTH];  
    struct sockaddr_in serverAddress, clientAddress;
    FILE *fp;

    if (argc < 2) { 
        fprintf(stderr, "Usage: %s port\n", argv[0]); 
        exit(1); 
    }  //setting up args to receive port

    memset((char *)&serverAddress, '\0', sizeof(serverAddress)); 
    port = atoi(argv[1]); 
    serverAddress.sin_family = AF_INET; 
    serverAddress.sin_port = htons(port); 
    serverAddress.sin_addr.s_addr = INADDR_ANY; 

    // Setting up the socket
    listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
    if (listenSocketFD < 0) {
        error("ERROR while setting up socket");
    }

    // Enable the socket
    if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0){ // Connect socket to port
        error("ERROR while enabling the socket");
    }
    listen(listenSocketFD, 5); // socket on

    // Accepting connection
    sizeOfSockAddr = sizeof(clientAddress); 
    establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfSockAddr); 
    if (establishedConnectionFD < 0) {
        error("ERROR trying to connect");
    }

    // Receive the ucid from the client
    memset(buffer, '\0', MAX_LENGTH);
    charsRead = recv(establishedConnectionFD, buffer, MAX_LENGTH - 1, 0); 
    if (charsRead < 0) {
        error("ERROR reading UCID");
    }
    printf("Received ucid: %s\n", buffer);
    char ucid[MAX_LENGTH];
    strncpy(ucid, buffer, MAX_LENGTH);
    // Send datetime
    time_t t = time(NULL);
    struct tm timer = *localtime(&t);
    char datetime[MAX_LENGTH];
    sprintf(datetime, "%d-%02d-%02d %02d:%02d:%02d", timer.tm_year + 1900, timer.tm_mon + 1, timer.tm_mday, timer.tm_hour, timer.tm_min, timer.tm_sec);
    charsRead = send(establishedConnectionFD, datetime, strlen(datetime), 0);
    if (charsRead < 0){
        error("ERROR sending date time!!!");
    }
    //generate passcode
    int seconds = timer.tm_sec;
    char password[10];
    char num[10];
    sprintf(password,"%d",seconds);
    strncpy(num,ucid+4,4);
    num[4]='\0';
    strcat(password,num);
    

    //receive passcode
    char passcode[MAX_LENGTH];
    memset(passcode, '\0', MAX_LENGTH);
    int recPass = recv(establishedConnectionFD, passcode, MAX_LENGTH - 1, 0); 
    if (recPass < 0) {
        error("ERROR receiving passcode");
    }
    int password_check=strcmp(password,passcode);

    if(password_check==0){
        printf("Code is a match sending data.txt");
        // Open the file data.txt
        fp = fopen("data.txt", "r");
        if (fp == NULL) {
            error("ERROR opening file");
        }
        // Send the contents of the file to the client
        memset(buffer, '\0', MAX_LENGTH);
        while (fgets(buffer, MAX_LENGTH - 1, fp) != NULL) {
            sentBytes = send(establishedConnectionFD, buffer, strlen(buffer), 0);
            if (sentBytes < 0) {
                error("ERROR sending data.txt");
            }
            memset(buffer, '\0', MAX_LENGTH);
        }
        }else{
            printf("Code does not match!!!");
        }


    // Closing
    close(establishedConnectionFD); 
    close(listenSocketFD);
    return 0; 
}


//references
//1.Codes from tutorial in d2l
//2.https://www.geeksforgeeks.org/socket-programming-cc/
//3.https://man7.org/linux/man-pages/man2/socket.2.html
//4.https://www.ibm.com/docs/en/zos/2.1.0?topic=functions-send-send-data-socket
//5.https://stackoverflow.com/questions/57730441/sockets-programming-sending-and-receiving-different-data-to-different-clients-i
//6.https://www.binarytides.com/server-client-example-c-sockets-linux/
//7.https://www.youtube.com/watch?v=7d7_G81uews
//8.https://www.youtube.com/watch?v=LOjbwO8O2ec
//9.https://www.youtube.com/watch?v=LtXEMwSG5-8
//10.https://www.geeksforgeeks.org/strings-in-c/
//11.https://www.geeksforgeeks.org/c-program-for-char-to-int-conversion/
//12.https://www.geeksforgeeks.org/c-program-for-int-to-char-conversion/
//13.https://idiotdeveloper.com/file-transfer-using-tcp-socket-in-c/
//14.https://stackoverflow.com/questions/11952898/c-send-and-receive-file
//15.https://www.programiz.com/c-programming/c-file-input-output
//16.https://www.youtube.com/watch?v=n3oh76pJ1eA
//17.https://www.tutorialspoint.com/c_standard_library/c_function_localtime.htm#:~:text=The%20C%20library%20function%20struct,in%20the%20local%20time%20zone.
//18.https://www.geeksforgeeks.org/time-h-localtime-function-in-c-with-examples/
//19.https://stackoverflow.com/questions/62952404/sending-date-and-time-as-a-tcp-server-welcome-message