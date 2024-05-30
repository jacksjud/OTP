#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>

#define CHAR_SET "ABCDEFGHIJKLMNOPQRSTUVWXYZ "

void generate_key(int length);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s keylength\n", argv[0]);
        exit(1);
    }

    int keylength = atoi(argv[1]);
    if (keylength <= 0) {
        fprintf(stderr, "Error: keylength must be a positive integer.\n");
        exit(1);
    }

    srand(time(NULL) ^ (getpid()<<16)); // Add more randomness by incorporating the process ID
    generate_key(keylength);

    return 0;
}

void generate_key(int length) {
    for (int i = 0; i < length; i++) {
        printf("%c", CHAR_SET[rand() % 27]);
    }
    printf("\n");
}
