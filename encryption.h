#ifndef ENCRYPTION_H
#define ENCRYPTION_H

#include <stdio.h>
#include <stdlib.h>

void xorEncrypt(char* data, size_t dataSize, const char* key);
void xorDecrypt(char* data, size_t dataSize, const char* key);
char* generateKey(size_t keyLength);
int validateKey(const char* key);

#endif
