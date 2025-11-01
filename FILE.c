#include <stdio.h>    /* FILE, printf, etc. */
#include <stdlib.h>   /* malloc, free, etc. */
#include <string.h>   /* strlen, strcpy, etc. */
#include <time.h>
#include "FILE.h"
#include "compression.h"
#include "encryption.h"

/* ---------- Local helpers ---------- */
static char *xstrdup(const char *s) {
    size_t n;
    char *p;
    if (!s) s = "";
    n = strlen(s) + 1;
    p = (char*)malloc(n);
    if (p) { memcpy(p, s, n); }
    return p;
}

char* getCurrentTimestamp() {
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    
    char* timestamp = malloc(20);  
    if (!timestamp) {
        return NULL;
    }
    
    sprintf(timestamp, "%04d-%02d-%02d %02d:%02d",
            t->tm_year + 1900,
            t->tm_mon + 1,
            t->tm_mday,
            t->tm_hour,
            t->tm_min);
    
    return timestamp;
}

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

/* ---------- NEW: Serialization functions ---------- */
// Convert linked list to single text string
static char* serializeEntries(const DiaryEntry* head, size_t* outSize) {
    const DiaryEntry* cur;
    size_t totalSize = 0;
    char* result;
    char* ptr;
    
    // Calculate total size needed
    for (cur = head; cur; cur = cur->next) {
        totalSize += strlen("ENTRY_START\n");
        totalSize += strlen("DATE:") + strlen(cur->datetime) + 1;
        totalSize += strlen("CONTENT:") + strlen(cur->content) + 1;
        totalSize += strlen("WORDCOUNT:") + 20 + 1;  // 20 digits for number, safe buffer
        totalSize += strlen("ENTRY_END\n");
    }
    
    result = malloc(totalSize + 1);
    if (!result) {
        *outSize = 0;
        return NULL;
    }
    
    ptr = result;
    for (cur = head; cur; cur = cur->next) {
        ptr += sprintf(ptr, "ENTRY_START\n");
        ptr += sprintf(ptr, "DATE:%s\n", cur->datetime);
        ptr += sprintf(ptr, "CONTENT:%s\n", cur->content);
        ptr += sprintf(ptr, "WORDCOUNT:%d\n", cur->wordCount);
        ptr += sprintf(ptr, "ENTRY_END\n");
    }
    
    *outSize = ptr - result;//the exact length
    return result;
}

// Parse text string back to linked list
static DiaryEntry* deserializeEntries(const char* data, size_t dataSize) {
    DiaryEntry* head = NULL;
    char* saveptr = NULL;
    
    // Validate input data
    if (!data || dataSize == 0) {
        printf("ERROR: Invalid data for deserialization\n");
        return NULL;
    }
    
    // Check if data contains valid entry markers
    if (strstr(data, "ENTRY_START") == NULL) {
        printf("ERROR: No valid entry markers found in data\n");
        return NULL;
    }
    
    // Make a copy of the data so we can use strtok_r safely
    char* dataCopy = malloc(dataSize + 1);
    if (!dataCopy) {
        printf("ERROR: Failed to allocate memory for data copy\n");
        return NULL;
    }
    memcpy(dataCopy, data, dataSize);
    dataCopy[dataSize] = '\0';
    
    // First tokenize by ENTRY_START/ENTRY_END
    char* token = strtok_r(dataCopy, "\n", &saveptr);
    
    while (token != NULL) {
        if (strcmp(token, "ENTRY_START") == 0) {
            // Start of a new entry
            char datetime[256] = {0};
            char content[10000] = {0};
            int wordCount = 0;
            int foundDate = 0;
            int foundContent = 0;
            
            // Process lines until ENTRY_END
            while ((token = strtok_r(NULL, "\n", &saveptr)) != NULL) {
                if (strcmp(token, "ENTRY_END") == 0) {
                    break;
                }
                
                // Check for DATE
                if (strncmp(token, "DATE:", 5) == 0) {
                    strncpy(datetime, token + 5, sizeof(datetime) - 1);
                    foundDate = 1;
                }
                // Check for CONTENT
                else if (strncmp(token, "CONTENT:", 8) == 0) {
                    strncpy(content, token + 8, sizeof(content) - 1);
                    foundContent = 1;
                }
                // Check for WORDCOUNT
                else if (strncmp(token, "WORDCOUNT:", 10) == 0) {
                    wordCount = atoi(token + 10);
                }
            }
            
            // Create entry if we found both date and content
            if (foundDate && foundContent) {
                DiaryEntry* entry = createEntry(datetime, content);
                if (entry) {
                    entry->wordCount = wordCount;
                    addEntry(&head, entry);
                }
            }
        }
        
        // Get next token
        token = strtok_r(NULL, "\n", &saveptr);
    }
    
    free(dataCopy);
    return head;
}

/* ---------- File utilities ---------- */
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

int writeFile(const char* filename, const char* data, size_t dataSize){
    FILE *filep;
    size_t written;

    filep = fopen(filename, "wb");
    if (!filep) { perror("fopen for write"); return 0; }
    written = fwrite(data, 1, dataSize, filep);
    fclose(filep);
    return written == dataSize;
}

int fileExists(const char* filename) {
    FILE *filep = fopen(filename, "rb");
    if (!filep) { 
        perror("fileExists");  // Shows actual error
        return 0; 
    }
    fclose(filep);
    return 1;
}

long getFileSize(const char* filename) {
    FILE* filep = fopen(filename, "rb");
    long size;

    if (!filep) { return -1; }

    if (fseek(filep, 0, SEEK_END) != 0)
     { fclose(filep); return -1; }
    size = ftell(filep);

    fclose(filep);
    return size;
}


/* ---------- Diary list/entry operations ---------- */
DiaryEntry* createEntry(const char* datetime, const char* content) {
    DiaryEntry* entry = malloc(sizeof(DiaryEntry));
    if (!entry) return NULL;
    
    entry->datetime = xstrdup(datetime ? datetime : "");  // Changed from date
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

void addEntry(DiaryEntry** head, DiaryEntry* entry) {
    if (!entry) return;
    
    // Add to beginning of list
    entry->next = *head;
    *head = entry;
}

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
            
            // Free memory
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

/* ---------- Diary file operations ---------- */
int saveAllEntries(const DiaryEntry* head, const char* filename, const char* key) {
    char* serialized;
    size_t serializedSize;
    char* compressed;
    size_t compressedSize;
    char* encrypted;
    size_t encryptedSize;
    int result;
    
    // Serialize entries to text
    serialized = serializeEntries(head, &serializedSize);
    if (!serialized) {
        printf("Failed to serialize entries\n");
        return -1;
    }
    
    // Compress serialized data
    compressed = compress(serialized, &compressedSize);
    free(serialized);
    
    if (!compressed) {
        printf("Failed to compress data\n");
        return -1;
    }
    
    // Encrypt compressed data (in-place)
    encryptedSize = compressedSize;
    encrypted = malloc(encryptedSize);
    if (!encrypted) {
        free(compressed);
        return -1;
    }
    memcpy(encrypted, compressed, encryptedSize);
    xorEncrypt(encrypted, encryptedSize, key);
    free(compressed);
    
    // Write encrypted data to file
    result = writeFile(filename, encrypted, encryptedSize);
    free(encrypted);
    
    return result ? 0 : -1;
}

DiaryEntry* loadAllEntries(const char* filename, const char* key) {
    long fileSize;
    FILE* file;
    char* encrypted;
    char* decrypted;
    char* decompressed;
    DiaryEntry* entries;
    
    // Get actual file size first
    fileSize = getFileSize(filename);
    if (fileSize <= 0) {
        printf("Failed to get file size\n");
        return NULL;
    }
    
    printf("Loading diary file (%ld bytes)...\n", fileSize);
    
    // Open and read file
    file = fopen(filename, "rb");
    if (!file) {
        printf("Failed to open file\n");
        return NULL;
    }
    
    // Allocate buffer for exact file size
    encrypted = malloc(fileSize);
    if (!encrypted) {
        fclose(file);
        return NULL;
    }
    
    // Read exact number of bytes
    size_t bytesRead = fread(encrypted, 1, fileSize, file);
    fclose(file);
    
    if (bytesRead != (size_t)fileSize) {
        printf("Failed to read complete file\n");
        free(encrypted);
        return NULL;
    }
    
    
    // Validate key before decryption
    if (!key || strlen(key) < 4) {
        printf("ERROR: Invalid encryption key (must be at least 4 characters)\n");
        free(encrypted);
        return NULL;
    }
    
    // Decrypt data (in-place)
    decrypted = malloc(fileSize);
    if (!decrypted) {
        printf("ERROR: Failed to allocate memory for decryption\n");
        free(encrypted);
        return NULL;
    }
    memcpy(decrypted, encrypted, fileSize);
    xorDecrypt(decrypted, fileSize, key);  // Use fileSize, not strlen!
    free(encrypted);
    
    // Decompress data
    decompressed = decompress(decrypted, fileSize);  // Use fileSize, not strlen!
    free(decrypted);
    
    // Check if decompression was successful
    if (!decompressed) {
        printf("ERROR: Failed to decompress data (possibly wrong encryption key)\n");
        return NULL;
    }
    
    size_t decompressedSize = strlen(decompressed);
    
    // Check if decompressed data is valid
    if (decompressedSize == 0) {
        printf("WARNING: Empty data after decompression\n");
        free(decompressed);
        return NULL;
    }
    
    // Deserialize entries
    entries = deserializeEntries(decompressed, decompressedSize);
    free(decompressed);
    
    return entries;
}

// validateKey is already defined in encryption.h
