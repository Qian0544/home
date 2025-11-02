
#ifndef UI_H
#define UI_H

#define MAX_ENTRY_SIZE 1024
#define MAX_FILENAME_SIZE 256
#define INPUT_BUFFER_SIZE 32
#define LINE_BUFFER_SIZE 256

#include "FILE.h"
#include "compression.h"
#include "encryption.h"

/* External global variables */
extern char Entry[MAX_ENTRY_SIZE];
extern char Filename[MAX_FILENAME_SIZE];
extern int EntryExists;
// UI functions
void displaymenue(void);
int getUserChoice(void);
int diaryMenuLoop(void);
// Entry management functions
int diaryCreateEntry(DiaryEntry** head);
void diaryDisplayAllEntries(DiaryEntry* head);
int diarySaveEncrypted(DiaryEntry* head, const char* filename, const char* key);
int diaryLoadEncrypted(DiaryEntry** head, const char* filename, const char* key);
int diaryDeleteEntry(DiaryEntry** head);
int diarySearchEntries(DiaryEntry* head);
#endif

