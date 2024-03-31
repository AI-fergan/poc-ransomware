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
    unsigned long key;
    printf("key: ");
    scanf("%lu", &key);
    printf("key: %d\n", validation_key(key));
    printf("key: %d\n", validation_key(generate_key()));
    printf("key: %d\n", validation_key(generate_key()));
    printf("key: %d\n", validation_key(generate_key()));


    return 0;
}