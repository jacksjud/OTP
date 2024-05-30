#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#define BUF_SIZE 1024

void read_file(const char *filename, char *buffer, size_t size);

/* 
Function:   main

Desc:   Acts as actual client,
takes inputted files to get ciphertext
and key in order to send to the
decryption server. 

*/
int main(int argc, char *argv[]) {
    if(argc != 4) {
        fprintf(stderr, "Usage: %s ciphertext key port\n", argv[0]);
        exit(1);
    }

    int sockfd, port;                                               // Socket file descriptor and port
    struct sockaddr_in server_addr;                                 // Socket server address
    char ciphertext[BUF_SIZE], key[BUF_SIZE], buffer[BUF_SIZE];     // Arrays for ciphertext, key, and buffer

    // Read files - get ciphertext and key from user inputted file names
    read_file(argv[1], ciphertext, BUF_SIZE);
    read_file(argv[2], key, BUF_SIZE);

    // Ensure key and ciphertext are the same size, key cannot be smalelr
    if(strlen(key) < strlen(ciphertext)) {
        fprintf(stderr, "Error: key '%s' is shorter than plaintext.\n", argv[2]);
        exit(1);
    }

    // Get port from user input
    port = atoi(argv[3]);

    // Create socket 
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1) {
        // if error, print and exit 
        perror("DEC CLIENT: Error with socket");
        exit(2);
    }

    // Clears out the address struct 
    memset((char*) &server_addr, '\0', sizeof(server_addr));

    // Address is network capable 
    server_addr.sin_family = AF_INET;
    // Store port num
    server_addr.sin_port = htons(port);
    // Set to localhost 
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Connect to server
    if(connect(sockfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) {
        // if error, print and exit
        perror("DEC CLIENT: Error with connect");
        exit(2);
    }

    // Store ciphertext and key to buffer, with delimiter
    snprintf(buffer, sizeof(buffer), "%s|%s", ciphertext, key);
    
    // Send buffer
    if(send(sockfd, buffer, strlen(buffer), 0) == -1) {
        // if error, print and exit
        perror("DEC CLIENT: Error with send");
        exit(1);
    }

    // Receive response 
    int numbytes = recv(sockfd, buffer, BUF_SIZE-1, 0);
    if(numbytes == -1) {
        // if error, print and exit
        perror("DEC CLIENT: Error with recv");
        exit(1);
    }

    // Set last to null char
    buffer[numbytes] = '\0';
    printf("%s\n", buffer);

    close(sockfd);
    return 0;
}

/* 
Function:   read_file

Desc:   Takes file name, buffer (where to
put the read text), and the size of the
read. Reads line, removes newline chars,
and closes file.

*/
void read_file(const char *filename, char *buffer, size_t size) {
    // Open file in read mode
    FILE *file = fopen(filename, "r");

    // If file wasn't opened successfully
    if(!file) {
        // if error, print and exit
        perror("DEC CLIENT: Error with fopen");
        exit(1);
    }
    // Read line from file - store to buffer
    if(!fgets(buffer, size, file)) {
        // if error, print and exit
        perror("DEC CLIENT: Error with fgets");
        exit(1);
    }
}