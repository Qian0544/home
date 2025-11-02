#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "FILE.h"
#include "compression.h"
#include "encryption.h"

/* Duplicate a string */
static char *xstrdup(const char *s) {
    size_t n;
    char *p;
    if (!s) s = "";
    n = strlen(s) + 1;
    p = (char*)malloc(n);
    if (p) { memcpy(p, s, n); }
    return p;
}

/* Get timestamp from user input */
char* getCurrentTimestamp() {
    char* timestamp = malloc(20);
    if (!timestamp) {
        return NULL;
    }
    
    char input[50];
    int year, month, day, hour, minute;
    
    printf("\nEnter date and time for this entry:\n");
    printf("Format: YYYY-MM-DD HH:MM (e.g., 2025-01-02 14:30)\n");
    printf("Or press Enter to use a simple timestamp: ");
    
    if (fgets(input, sizeof(input), stdin) == NULL) {
        free(timestamp);
        return NULL;
    }
    
    /* Empty input - use counter */
    if (input[0] == '\n') {
        static int counter = 0;
        counter++;
        sprintf(timestamp, "Unspecified-Time-%d", counter);
        return timestamp;
    }
    
    /* Try to parse user's date/time */
    if (sscanf(input, "%d-%d-%d %d:%d", &year, &month, &day, &hour, &minute) == 5) {
        /* Check if date/time is valid */
        if (year >= 1900 && year <= 2100 &&
            month >= 1 && month <= 12 &&
            day >= 1 && day <= 31 &&
            hour >= 0 && hour <= 23 &&
            minute >= 0 && minute <= 59) {
            
            sprintf(timestamp, "%04d-%02d-%02d %02d:%02d", year, month, day, hour, minute);
            return timestamp;
        }
    }
    
    /* Invalid format - ask if user wants to use counter instead */
    printf("Invalid format. Use current counter instead? (y/n): ");
    char choice;
    if (scanf("%c", &choice) == 1 && (choice == 'y' || choice == 'Y')) {
        while (getchar() != '\n');
        
        static int counter = 0;
        counter++;
        sprintf(timestamp, "Entry-%d", counter);
        return timestamp;
    }
    
    free(timestamp);
    return NULL;
}

/* Count words in a string */
static int countWrds(const char *s) {
    int count = 0, inword = 0;
    const unsigned char *p;

    if (!s) return 0;

    for (p = (const unsigned char*)s; *p; ++p) {
        if (*p > ' ') { 
            if (!inword) { ++count; inword = 1; }
        }
        else { inword = 0; }
    }
    return count;
}

/* Convert linked list to text string for saving */
static char* serializeEntries(const DiaryEntry* head, size_t* outSize) {
    const DiaryEntry* cur;
    size_t totalSize = 0;
    char* result;
    char* ptr;
    
    /* Calculate how much space we need */
    for (cur = head; cur; cur = cur->next) {
        totalSize += strlen("ENTRY_START\n");
        totalSize += strlen("DATE:") + strlen(cur->datetime) + 1;
        totalSize += strlen("CONTENT:") + strlen(cur->content) + 1;
        totalSize += strlen("WORDCOUNT:") + 20 + 1;
        totalSize += strlen("ENTRY_END\n");
    }
    
    result = malloc(totalSize + 1);
    if (!result) {
        *outSize = 0;
        return NULL;
    }
    
    /* Write all entries to string */
    ptr = result;
    for (cur = head; cur; cur = cur->next) {
        ptr += sprintf(ptr, "ENTRY_START\n");
        ptr += sprintf(ptr, "DATE:%s\n", cur->datetime);
        ptr += sprintf(ptr, "CONTENT:%s\n", cur->content);
        ptr += sprintf(ptr, "WORDCOUNT:%d\n", cur->wordCount);
        ptr += sprintf(ptr, "ENTRY_END\n");
    }
    
    *outSize = ptr - result;
    return result;
}

/* Convert text string back to linked list */
static DiaryEntry* deserializeEntries(const char* data, size_t dataSize) {
    DiaryEntry* head = NULL;
    const char* ptr = data;
    const char* end = data + dataSize;
    
    if (!data || dataSize == 0) {
        printf("ERROR: Invalid data for deserialization\n");
        return NULL;
    }
    
    if (strstr(data, "ENTRY_START") == NULL) {
        printf("ERROR: No valid entry markers found in data\n");
        return NULL;
    }
    
    /* Walk through the string and parse entries */
    while (ptr < end) {
        /* Skip whitespace */
        while (ptr < end && (*ptr == '\n' || *ptr == '\r' || *ptr == ' ')) {
            ptr++;
        }
        
        if (ptr >= end) break;
        
        /* Found start of an entry */
        if (strncmp(ptr, "ENTRY_START", 11) == 0) {
            ptr += 11;
            
            char datetime[256] = {0};
            char content[10000] = {0};
            int wordCount = 0;
            int foundDate = 0;
            int foundContent = 0;
            
            /* Read entry fields until ENTRY_END */
            while (ptr < end) {
                while (ptr < end && (*ptr == '\n' || *ptr == '\r')) ptr++;
                if (ptr >= end) break;
                
                /* Check what field this is */
                if (strncmp(ptr, "ENTRY_END", 9) == 0) {
                    ptr += 9;
                    break;
                }
                
                if (strncmp(ptr, "DATE:", 5) == 0) {
                    ptr += 5;
                    size_t i = 0;
                    while (ptr < end && *ptr != '\n' && *ptr != '\r' && i < sizeof(datetime) - 1) {
                        datetime[i++] = *ptr++;
                    }
                    datetime[i] = '\0';
                    foundDate = 1;
                }
                else if (strncmp(ptr, "CONTENT:", 8) == 0) {
                    ptr += 8;
                    size_t i = 0;
                    while (ptr < end && *ptr != '\n' && *ptr != '\r' && i < sizeof(content) - 1) {
                        content[i++] = *ptr++;
                    }
                    content[i] = '\0';
                    foundContent = 1;
                }
                else if (strncmp(ptr, "WORDCOUNT:", 10) == 0) {
                    ptr += 10;
                    wordCount = atoi(ptr);
                    while (ptr < end && *ptr != '\n' && *ptr != '\r') ptr++;
                }
                else {
                    /* Unknown field, skip it */
                    while (ptr < end && *ptr != '\n' && *ptr != '\r') ptr++;
                }
            }
            
            /* Create entry if we got both date and content */
            if (foundDate && foundContent) {
                DiaryEntry* entry = createEntry(datetime, content);
                if (entry) {
                    entry->wordCount = wordCount;
                    addEntry(&head, entry);
                }
            }
        }
        else {
            ptr++;
        }
    }
    
    return head;
}

/* Read entire file into memory */
char* readFile(const char* filename) {
    FILE *filep = fopen(filename, "rb");
    long size;
    char *buffer;
    size_t readn;
    
    if (!filep) { 
        perror("fopen");
        return NULL; 
    }

    if (fseek(filep, 0, SEEK_END) != 0) { 
        perror("fseek"); 
        fclose(filep); 
        return NULL; 
    }
    
    size = ftell(filep);
    if (size < 0) { 
        perror("ftell"); 
        fclose(filep); 
        return NULL; 
    }
    rewind(filep);

    buffer = (char*) malloc((size_t)size + 1);
    if (!buffer) { 
        perror("malloc"); 
        fclose(filep); 
        return NULL; 
    }

    readn = fread(buffer, 1, (size_t)size, filep);
    buffer[readn] = '\0';
    fclose(filep);
    return buffer;
}

/* Write data to file */
int writeFile(const char* filename, const char* data, size_t dataSize){
    FILE *filep;
    size_t written;

    filep = fopen(filename, "wb");
    if (!filep) { 
        perror("fopen for write"); 
        return 0; 
    }
    written = fwrite(data, 1, dataSize, filep);
    fclose(filep);
    return written == dataSize;
}

/* Check if file exists */
int fileExists(const char* filename) {
    FILE *filep = fopen(filename, "rb");
    if (!filep) { 
        perror("fileExists");
        return 0; 
    }
    fclose(filep);
    return 1;
}

/* Get file size in bytes */
long getFileSize(const char* filename) {
    FILE* filep = fopen(filename, "rb");
    long size;

    if (!filep) { return -1; }

    if (fseek(filep, 0, SEEK_END) != 0) { 
        fclose(filep); 
        return -1; 
    }
    size = ftell(filep);

    fclose(filep);
    return size;
}

/* Create a new diary entry */
DiaryEntry* createEntry(const char* datetime, const char* content) {
    DiaryEntry* entry = malloc(sizeof(DiaryEntry));
    if (!entry) return NULL;
    
    entry->datetime = xstrdup(datetime ? datetime : "");
    entry->content = xstrdup(content ? content : "");
    
    if (!entry->datetime || !entry->content) {
        free(entry->datetime);
        free(entry->content);
        free(entry);
        return NULL;
    }
    
    entry->wordCount = countWrds(entry->content);
    entry->next = NULL;
    
    return entry;
}

/* Add entry to start of list */
void addEntry(DiaryEntry** head, DiaryEntry* entry) {
    if (!entry) return;
    
    entry->next = *head;
    *head = entry;
}

/* Free all entries in list */
void freeAllEntries(DiaryEntry* head) {
    DiaryEntry* current = head;
    DiaryEntry* next;
    
    while (current) {
        next = current->next;
        free(current->datetime);
        free(current->content);
        free(current);
        current = next;
    }
}

/* Delete entry by datetime */
void delEntry(DiaryEntry** head, const char* datetime) {
    DiaryEntry *cur, *prev;

    if (!head || !*head || !datetime) { 
        return; 
    }

    cur = *head;
    prev = NULL;

    while (cur) {
        if (strcmp(cur->datetime, datetime) == 0) {
            if (prev) { 
                prev->next = cur->next; 
            } else { 
                *head = cur->next; 
            }
            
            free(cur->datetime);
            free(cur->content);
            free(cur);
            
            printf("✓ Deleted entry from %s\n", datetime);
            return;
        }
        prev = cur;
        cur = cur->next;
    }
    
    printf("✗ No entry found for %s\n", datetime);
}

/* Save all entries to encrypted file */
int saveAllEntries(const DiaryEntry* head, const char* filename, const char* key) {
    char* serialized;
    size_t serializedSize;
    char* compressed;
    size_t compressedSize;
    char* encrypted;
    size_t encryptedSize;
    int result;
    
    /* Convert to text */
    serialized = serializeEntries(head, &serializedSize);
    if (!serialized) {
        printf("Failed to serialize entries\n");
        return -1;
    }
    
    /* Compress */
    compressed = compress(serialized, &compressedSize);
    free(serialized);
    
    if (!compressed) {
        printf("Failed to compress data\n");
        return -1;
    }
    
    /* Encrypt */
    encryptedSize = compressedSize;
    encrypted = malloc(encryptedSize);
    if (!encrypted) {
        free(compressed);
        return -1;
    }
    memcpy(encrypted, compressed, encryptedSize);
    xorEncrypt(encrypted, encryptedSize, key);
    free(compressed);
    
    /* Write to file */
    result = writeFile(filename, encrypted, encryptedSize);
    free(encrypted);
    
    return result ? 0 : -1;
}

/* Load all entries from encrypted file */
DiaryEntry* loadAllEntries(const char* filename, const char* key) {
    long fileSize;
    FILE* file;
    char* encrypted;
    char* decrypted;
    char* decompressed;
    DiaryEntry* entries;
    
    /* Get file size */
    fileSize = getFileSize(filename);
    if (fileSize <= 0) {
        printf("Failed to get file size\n");
        return NULL;
    }
    
    printf("Loading diary file (%ld bytes)...\n", fileSize);
    
    /* Read file */
    file = fopen(filename, "rb");
    if (!file) {
        printf("Failed to open file\n");
        return NULL;
    }
    
    encrypted = malloc(fileSize);
    if (!encrypted) {
        fclose(file);
        return NULL;
    }
    
    size_t bytesRead = fread(encrypted, 1, fileSize, file);
    fclose(file);
    
    if (bytesRead != (size_t)fileSize) {
        printf("Failed to read complete file\n");
        free(encrypted);
        return NULL;
    }
    
    /* Check key */
    if (!key || strlen(key) < 4) {
        printf("ERROR: Invalid encryption key\n");
        free(encrypted);
        return NULL;
    }
    
    /* Decrypt */
    decrypted = malloc(fileSize);
    if (!decrypted) {
        printf("ERROR: Failed to allocate memory for decryption\n");
        free(encrypted);
        return NULL;
    }
    memcpy(decrypted, encrypted, fileSize);
    xorDecrypt(decrypted, fileSize, key);
    free(encrypted);
    
    /* Decompress */
    decompressed = decompress(decrypted, fileSize);
    free(decrypted);
    
    if (!decompressed) {
        printf("ERROR: Failed to decompress (wrong key?)\n");
        return NULL;
    }
    
    size_t decompressedSize = strlen(decompressed);
    
    if (decompressedSize == 0) {
        printf("WARNING: Empty data after decompression\n");
        free(decompressed);
        return NULL;
    }
    
    /* Parse entries */
    entries = deserializeEntries(decompressed, decompressedSize);
    free(decompressed);
    
    return entries;
}

/* Search for entries containing search term */
DiaryEntry* searchEntries(DiaryEntry* head, const char* searchTerm) {
    DiaryEntry* results = NULL;
    DiaryEntry* current = head;
    
    if (!head || !searchTerm || strlen(searchTerm) == 0) {
        return NULL;
    }
    
    printf("\nSearching for: '%s'\n", searchTerm);
    
    while (current) {
        int match = 0;
        
        /* Check date */
        if (strstr(current->datetime, searchTerm) != NULL) {
            match = 1;
        }
        
        /* Check content */
        if (strstr(current->content, searchTerm) != NULL) {
            match = 1;
        }
        
        /* Copy matching entry */
        if (match) {
            DiaryEntry* copy = createEntry(current->datetime, current->content);
            if (copy) {
                copy->wordCount = current->wordCount;
                addEntry(&results, copy);
            }
        }
        
        current = current->next;
    }
    
    return results;
}