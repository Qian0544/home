#ifndef FILE_H
#define FILE_H

#include <stddef.h>   /* for size_t */

typedef struct DiaryEntry {
    char *datetime,              /* e.g., "YYYY-MM-DD" */
         *content;           /* entry text - one line 4 simple format */
    int wordCount;           /* cached word count */
    struct DiaryEntry *next; /* singly-linked list */
} DiaryEntry;

/* ---------- File utilities ---------- */
char *readFile(const char *filename);

int writeFile(const char *filename, const char *data, size_t dataSize);

int fileExists(const char *filename);       /* Returns 1 if file exists, 0 if not (no stderr printing here) */

long getFileSize(const char *filename);


/* ---------- Diary list/entry operations ---------- */
DiaryEntry *createEntry(const char *datetime, const char *content);

void addEntry(DiaryEntry **head, DiaryEntry *newEntry);     

void delEntry(DiaryEntry **head, const char *datetime);

// UPDATED: Now includes key parameter
int saveAllEntries(const DiaryEntry* head, const char* filename, const char* key);

DiaryEntry* loadAllEntries(const char* filename, const char* key);

void freeAllEntries(DiaryEntry *head);

char* getCurrentTimestamp(void);

#endif /* FILE_H */
