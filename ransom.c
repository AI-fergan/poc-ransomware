#include <stdio.h>
#include <stdlib.h>
#include <time.h>


#define RANSOM_VERSION 1
#define MAGIC_LEN 1000
#define MAGIC_KEY 2

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
    
    printf("key: %lu\n", generate_key());
    printf("key: %lu\n", generate_key());
    printf("key: %lu\n", generate_key());

    return 0;
}