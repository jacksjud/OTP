#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#define BUF_SIZE 1024

void read_file(const char *filename, char *buffer, size_t size);
int validate_text(const char *text);

int main(int argc, char *argv[]) {
    // Make sure that we have enough arguments
    if(argc != 4) {
        // inform user of correct usage and exit
        fprintf(stderr, "Usage: %s plaintext key port\n", argv[0]);
        exit(1);
    }

    // Declare variables 
    int sockfd, port;                                           // Socket file descriptor and port
    struct sockaddr_in server_addr;                             // Socket server address
    char plaintext[BUF_SIZE], key[BUF_SIZE], buffer[BUF_SIZE];  // Arrays for plaintext, key, and buffer

    // Read files - get plaintext and key from user inputted file names
    read_file(argv[1], plaintext, BUF_SIZE);                    
    read_file(argv[2], key, BUF_SIZE);


    // Ensure key and plaintext are the same size, key cannot be smaller than plaintext
    if(strlen(key) < strlen(plaintext)) {
        fprintf(stderr, "Error: key '%s' is too short.\n", argv[2]);
        exit(1);
    }

    // Ensure plaintext doesn't have bad characters (outside of alphabet or space)
    if(!validate_text(plaintext)){
        fprintf(stderr, "Error: %s contains bad characters\n", argv[1]);
        exit(1);
    }
    // Ensure plaintext doesn't have bad characters (outside of alphabet or space) 
    if(!validate_text(key)){
        fprintf(stderr, "Error: %s contains bad characters\n", argv[2]);
        exit(1);
    }

    // Get port from user input 
    port = atoi(argv[3]);

    // Create socket 
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1) {
        // if error, print and exit 
        perror("ENC CLIENT: Error with socket");
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
        perror("ENC CLIENT: Error with connect");
        exit(2);
    }

    // Store plaintext and key to buffer, with delimiter
    snprintf(buffer, sizeof(buffer), "%s|%s", plaintext, key);

    if(send(sockfd, buffer, strlen(buffer), 0) == -1) {
        perror("ENC CLIENT: Error with send");
        exit(1);
    }

    int numbytes = recv(sockfd, buffer, BUF_SIZE-1, 0);
    if(numbytes == -1) {
        perror("ENC CLIENT: Error with recv");
        exit(1);
    }

    buffer[numbytes] = '\0';
    printf("%s\n", buffer);

    close(sockfd);
    return 0;
}

void read_file(const char *filename, char *buffer, size_t size) {
    FILE *file = fopen(filename, "r");
    if(!file) {
        perror("ENC CLIENT: Error with fopen");
        exit(1);
    }
    if(!fgets(buffer, size, file)) {
        perror("ENC CLIENT: Error with fgets");
        exit(1);
    }
    buffer[strcspn(buffer, "\n")] = '\0';
    fclose(file);
}

int validate_text(const char *text) {
    for (size_t i = 0; i < strlen(text); i++) {
        if((text[i] < 'A' || text[i] > 'Z') && text[i] != ' ') {
            return 0;
        }
    }
    return 1;
}