#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>

#define BACKLOG 5
#define BUF_SIZE 1024
#define MOD 27

void handle_client(int new_fd);
char encrypt_char(char plain, char key);


/* 
Function:   main

Desc:   Acts as server, encrypts sent
plaintext according to the sent key.
Sends back encrypted text.

*/
int main(int argc, char *argv[]) {
    // Make sure that we have enough arguments
    if(argc != 2) {
        // Inform user of correct usage and exit 
        fprintf(stderr, "Usage: %s port\n", argv[0]);
        exit(1);
    }

    // Declare variables

    int sockfd, new_fd;                             // Socket file descriptors
    struct sockaddr_in server_addr, client_addr;    // Socket addresses (server & client)
    socklen_t sin_size;                             // Size of socket - records socket struct size 
    int port = atoi(argv[1]);                       // Port number from user input  

    // Create socket 
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    // if fail, exit w/ message
    if(sockfd == -1) {
        perror("ENC SERVER: Error with socket");
        exit(1);
    }

    // Clears out the address struct 
    memset((char*) &server_addr, '\0', sizeof(server_addr));

    // Address is network capable 
    server_addr.sin_family = AF_INET;
    // Store port num 
    server_addr.sin_port = htons(port);
    // Accepts all connections
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Associate our socket w/ server socket address 
    if(bind(sockfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) {
        // if error then print and exit
        perror("ENC SERVER: Error with bind");
        exit(1);
    }

    // Open server socket to start listening for client connections (w/ backlog of 5)
    if(listen(sockfd, BACKLOG) == -1) {
        // if error then print and exit
        perror("ENC SERVER: Error with listen");
        exit(1);
    }

    // Create loop to accept connections 
    while (1) {
        // Record size of socket struct size 
        sin_size = sizeof(struct sockaddr_in);
        // Create new socket based on connection 
        new_fd = accept(sockfd, (struct sockaddr *)&client_addr, &sin_size);
        if(new_fd == -1) {
            // if error then print and skip connection
            perror("ENC SERVER: Error with accept");
            continue;
        }
        // If child process
        if(!fork()) {
            // close listening socket (child's server)
            close(sockfd);
            // use new socket file descriptor to communicate with client
            handle_client(new_fd);
            // close new socket 
            close(new_fd);
            // child process terminates
            exit(0);
        }
        close(new_fd);  // Parent closes connection to new socket
    }
    close(sockfd);      // Would close server listening socket
    return 0;
}

/* 
Function:   handle_client

Desc:   Takes socket file descriptor
and uses it to receive plain text
and key from client. It then 
encrypts this text by calling
the encrypt_char function for
each character in the text.

*/
void handle_client(int new_fd) {
    char buffer[BUF_SIZE];
    int numbytes;
    
    // Receive plaintext and key
    numbytes = recv(new_fd, buffer, BUF_SIZE-1, 0);
    if(numbytes == -1) {
        // If error, print and return 
        perror("ENC SERVER: Error with recv");
        return;
    }
    buffer[numbytes] = '\0';                        // Set last to null char

    // Use delimiter to get plaintext and key
    char *plaintext = strtok(buffer, "|");
    char *key = strtok(NULL, "|");

    // Encryption process
    int len = strlen(plaintext);                    // Get length of plaintext
    char ciphertext[len+1];                         // Create array for cipher

    // Go through every character and build up cipher text using encrypt_char
    for(int i = 0; i < len; i++) {             
        ciphertext[i] = encrypt_char(plaintext[i], key[i]);
    }
    ciphertext[len] = '\0';                         // Set last to null char

    // Send ciphertext back
    if(send(new_fd, ciphertext, len, 0) == -1) {
        perror("ENC SERVER: Error with send");
    }
}


/* 
Function:   encrypt_char

Desc:   Takes a plain text char and key char
and uses their integer values, 0-25 for A-Z
and 26 for a space ' ', to encrypt them.
It returns the encrypted text 

*/
char encrypt_char(char plain, char key) {
    // Use plaintext char to get corresponding int val in alphabet (0-25), or 26 for space 
    int plain_val = plain == ' ' ? 26 : plain - 'A';
    // Again but with the key char
    int key_val = key == ' ' ? 26 : key - 'A';
    // Add vals together and modulo of 27 is taken (keeps in range)
    int cipher_val = (plain_val + key_val) % MOD;
    // Return encrypted character based on val, again checking for space
    return cipher_val == 26 ? ' ' : cipher_val + 'A';
}
