/*Name: Qian Zhao
 *ID: 25777830
 * XOR Encryption Functions
 * 
 * IMPORTANT: Call srand(time(NULL)) once in main() before using generateKey()
 * 
 * Usage example:
 *   char data[] = "secret diary";
 *   xorEncrypt(data, strlen(data), "mypassword");
 *   // data is now encrypted
 *   xorDecrypt(data, strlen(data), "mypassword");
 *   // data is back to original
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>  
#include <time.h>   
#include "encryption.h"


void xorEncrypt(char* data, size_t dataSize, const char* key) {
    size_t keyLen = strlen(key);
    size_t i;
    for (i = 0; i < dataSize; i++) {
        data[i] ^= key[i % keyLen]; 
        /*xorDecrypt ^=, data[i] = data[i] ^ key[] */
        /* Cycle through key*/
    }
}

void xorDecrypt(char* data, size_t dataSize, const char* key) {
    /* Same as encrypt - XOR is symmetric*/
    xorEncrypt(data, dataSize, key);
}

char* generateKey(size_t keyLength) {
    if (keyLength == 0) return NULL; 
    char* key = malloc(keyLength + 1);
    if (key == NULL) return NULL;
    /* + 1 for the null terminator \0 */
    
    size_t i;
    for (i = 0; i < keyLength; i++) {
        key[i] = 33 + (rand() % 94);  
        /* Printable ASCII characters (33-126)*/
    }
    key[keyLength] = '\0';
    return key; 
    /*return the address*/
}

int validateKey(const char* key) {
    if (!key || strlen(key) == 0) return 0;  /* Invalid*/
    if (strlen(key) < 4) return 0;           /* Too short*/
    return 1;  // Valid
}
