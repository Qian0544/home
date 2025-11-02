/*
- Jackson Littrup
ID: 26130916
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "UI.h"

#define MAX_ENTRY_SIZE 1024
#define INPUT_BUFFER_SIZE 32

/* Global state */
static DiaryEntry* diary_head = NULL;
static char current_filename[256] = "diary.enc";
static char encryption_key[256] = "";

/* Display main menu */
void displaymenue(void){
    printf("\n========================================\n");
    printf("       SECURE DIARY SYSTEM\n");
    printf("========================================\n");
    printf("1. Write new diary\n");
    printf("2. View all diaries\n");
    printf("3. Search diary\n");         
    printf("4. Delete a diary\n");
    printf("5. Exit\n");
    printf("========================================\n");
}

/* Get user's menu choice */
int getUserChoice(void){
    char buffer[INPUT_BUFFER_SIZE];
    int choice;
    int scanResult;

    printf("Your selection: ");
    fflush(stdout);

    if (fgets(buffer, sizeof(buffer), stdin) == NULL){
        fprintf(stderr, "Input error\n");
        return -1;
    }

    scanResult = sscanf(buffer, "%d", &choice);
    if (scanResult != 1){
        return -1;
    }
    if (choice < 1 || choice > 5){  
        return -1;
    }
    return choice;
}

/* Create new diary entry */
int diaryCreateEntry(DiaryEntry** head){
    char content[MAX_ENTRY_SIZE];
    char line[256];
    char* timestamp;
    DiaryEntry* entry;
    
    content[0] = '\0';
    
    printf("Create New Diary Entry\n");
    printf("Type your entry below. Exit with 'end' on a new line\n");
    
    /* Read lines until 'end' */
    while (1) {
        if (fgets(line, sizeof(line), stdin) == NULL) {
            break;
        }
        
        if (strcmp(line, "end\n") == 0 || strcmp(line, "end\r\n") == 0) {
            break;
        }
        
        if (strlen(content) + strlen(line) >= MAX_ENTRY_SIZE - 1) {
            printf("Entry size limit reached.\n");
            break;
        }
        
        strcat(content, line);
    }
    
    if (strlen(content) == 0) {
        printf("No entry captured.\n");
        return 0;
    }
    
    /* Get timestamp from user */
    timestamp = getCurrentTimestamp();
    if (!timestamp) {
        printf("Failed to generate timestamp.\n");
        return 0;
    }
    
    /* Create and add entry */
    entry = createEntry(timestamp, content);
    free(timestamp);
    
    if (!entry) {
        printf("Failed to create entry.\n");
        return 0;
    }
    
    addEntry(head, entry);
    
    printf("\n✓ Entry created successfully at %s\n", entry->datetime);
    printf("  Word count: %d\n", entry->wordCount);
    
    return 1;
}

/* Display all diary entries */
void diaryDisplayAllEntries(DiaryEntry* head){
    DiaryEntry* current = head;
    int count = 0;
    
    if (!head) {
        printf("No diary entries found.\n");
        return;
    }
    
    printf("\n========================================\n");
    printf("         YOUR DIARY ENTRIES\n");
    printf("========================================\n\n");
    
    while (current) {
        count++;
        printf("--- Entry #%d ---\n", count);
        printf("Date/Time: %s\n", current->datetime);
        printf("Words: %d\n", current->wordCount);
        printf("Content:\n%s", current->content);
        
        size_t len = strlen(current->content);
        if (len > 0 && current->content[len - 1] != '\n') {
            printf("\n");
        }
        
        printf("\n");
        current = current->next;
    }
    
    printf("========================================\n");
    printf("Total entries: %d\n", count);
    printf("========================================\n");
}

/* Save diary to encrypted file */
int diarySaveEncrypted(DiaryEntry* head, const char* filename, const char* key){
    if (!head) {
        printf("No entries to save. Create an entry first.\n");
        return 0;
    }
    
    printf("\nSaving encrypted diary to '%s'\n", filename);
    
    int result = saveAllEntries(head, filename, key);
    return (result == 0) ? 1 : 0;
}

/* Load diary from encrypted file */
int diaryLoadEncrypted(DiaryEntry** head, const char* filename, const char* key){
    if (!fileExists(filename)) {
        printf("File '%s' does not exist.\n", filename);
        return 0;
    }
    
    /* Free existing entries */
    if (*head) {
        freeAllEntries(*head);
        *head = NULL;
    }
    
    *head = loadAllEntries(filename, key);
    
    if (*head) {
        int count = 0;
        DiaryEntry* current = *head;
        while (current) {
            count++;
            current = current->next;
        }
        printf("Diary loaded successfully (%d entries)\n", count);
        return 1;
    } else {
        printf("Failed to load diary. Wrong key or corrupted file\n");
        return 0;
    }
}

/* Set encryption password */
static int setEncryptionKey(char* key_buffer, size_t buffer_size){
    char input[256];
    printf("Enter password (min 4 characters): ");
    fflush(stdout);
    
    if (fgets(input, sizeof(input), stdin) == NULL) {
        printf("Input error.\n");
        return 0;
    }
    
    input[strcspn(input, "\r\n")] = '\0';
    
    if (!validateKey(input)) {
        printf("Invalid password. Must be at least 4 characters.\n");
        return 0;
    }
    
    strncpy(key_buffer, input, buffer_size - 1);
    key_buffer[buffer_size - 1] = '\0';
    
    printf("Password set successfully.\n");
    return 1;
}

/* Search diary entries by keyword or date */
int diarySearchEntries(DiaryEntry* head) {
    char searchTerm[256];
    
    if (!head) {
        printf("\nNo diaries to search.\n");
        return 0;
    }
    
    printf("\n=== Search Diary Entries ===\n");
    printf("Enter search term (date or keyword): ");
    fflush(stdout);
    
    if (fgets(searchTerm, sizeof(searchTerm), stdin) == NULL) {
        printf("Input error.\n");
        return 0;
    }
    
    searchTerm[strcspn(searchTerm, "\r\n")] = '\0';
    
    if (strlen(searchTerm) == 0) {
        printf("Search cancelled.\n");
        return 0;
    }
    
    /* Search and get results */
    DiaryEntry* results = searchEntries(head, searchTerm);
    
    if (!results) {
        printf("\nNo diaries found matching '%s'.\n", searchTerm);
        return 0;
    }
    
    /* Display results */
    DiaryEntry* current = results;
    int count = 0;
    
    printf("\n========================================\n");
    printf("         SEARCH RESULTS\n");
    printf("========================================\n\n");
    
    while (current) {
        count++;
        printf("--- Result #%d ---\n", count);
        printf("Date/Time: %s\n", current->datetime);
        printf("Words: %d\n", current->wordCount);
        printf("\n%s", current->content);
        
        size_t len = strlen(current->content);
        if (len > 0 && current->content[len - 1] != '\n') {
            printf("\n");
        }
        
        printf("\n");
        current = current->next;
    }
    
    printf("========================================\n");
    printf("Found %d matching diary (diaries)\n", count);
    printf("========================================\n");
    
    freeAllEntries(results);
    
    return 1;
}

/* Delete a diary entry by number */
int diaryDeleteEntry(DiaryEntry** head) {
    DiaryEntry* current = *head;
    int count = 0, choice;
    char* datetimes[100];
    
    if (!current) {
        printf("No entries to delete.\n");
        return 0;
    }
    
    printf("\n========================================\n");
    printf("         DELETE DIARY ENTRY\n");
    printf("========================================\n\n");
    
    /* Display all entries */
    while (current && count < 100) {
        datetimes[count] = current->datetime;
        count++;
        printf("%d. Entry from %s\n", count, current->datetime);
        current = current->next;
    }
    
    printf("\nEnter entry number to delete (1-%d) or 0 to cancel: ", count);
    char buffer[256];
    
    if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
        printf("Input error.\n");
        return 0;
    }
    
    if (sscanf(buffer, "%d", &choice) != 1) {
        printf("Invalid input.\n");
        return 0;
    }
    
    if (choice == 0) {
        printf("Deletion cancelled.\n");
        return 0;
    }
    
    if (choice < 1 || choice > count) {
        printf("Invalid entry number.\n");
        return 0;
    }
    
    /* Delete selected entry */
    delEntry(head, datetimes[choice-1]);
    printf("Entry #%d deleted successfully.\n", choice);
    return 1;
}

/* Main diary menu loop */
int diaryMenuLoop(void){
    int running = 1;
    
    printf("\n");
    printf("╔════════════════════════════════════════╗\n");
    printf("║     WELCOME TO SECURE DIARY SYSTEM     ║\n");
    printf("╚════════════════════════════════════════╝\n");
    printf("\n");
    
    /* Step 1: Set password */
    printf("To access your diary, you must set an encryption key(password).\n");

    int diaryExists = fileExists(current_filename);
    if (diaryExists) {
        printf("\n⚠️  IMPORTANT: An encrypted diary already exists.\n");
        printf("    You must enter the CORRECT password to access your previous diaries.\n");
        printf("    Using a different password will start a NEW diary (old entries will be lost).\n\n");
    }

    int keySet = 0;
    int maxAttempts = 3;
    int attempts = 0;
    
    while (!keySet && attempts < maxAttempts) {
        keySet = setEncryptionKey(encryption_key, sizeof(encryption_key));
        if (!keySet) {
            printf("\nPlease try again. Key is required to continue.\n");
            attempts++;
        }
    }
    
    if (!keySet) {
        printf("\nToo many failed attempts. Exiting.\n");
        return -1;
    }

    /* Step 2: Auto-load if diary exists */
    if (diaryExists) {
        printf("\nExisting diary found. Loading...\n");
        int loadSuccess = diaryLoadEncrypted(&diary_head, current_filename, encryption_key);
        
        if (!loadSuccess) {
            printf("\n╔════════════════════════════════════════╗\n");
            printf("║  ⚠️  FAILED TO LOAD DIARY!            ║\n");
            printf("╚════════════════════════════════════════╝\n");
            printf("\nThis usually means you entered the WRONG key.\n");
            printf("Your old entries are still safe in '%s',\n", current_filename);
            printf("but you cannot access them with this key.\n\n");
            
            printf("What would you like to do?\n");
            printf("1. Exit and try again with correct key\n");
            printf("2. Continue with NEW empty diary (will OVERWRITE old diary!)\n");
            printf("\nYour choice (1 or 2): ");
            
            char choice[10];
            if (fgets(choice, sizeof(choice), stdin) == NULL || choice[0] != '2') {
                printf("\nExiting. Please restart and enter the correct key.\n");
                printf("Your old diary is safe at: %s\n\n", current_filename);
                return 0;
            }
            
            printf("\n⚠️  WARNING: You are starting a NEW diary.\n");
            printf("    When you save, it will OVERWRITE your old diary!\n");
            printf("    Are you ABSOLUTELY SURE? (type 'YES' to confirm): ");
            
            char confirm[10];
            if (fgets(confirm, sizeof(confirm), stdin) == NULL || 
                strncmp(confirm, "YES", 3) != 0) {
                printf("\nCancelled. Exiting for safety.\n");
                return 0;
            }
            
            printf("\n⚠️  Proceeding with NEW empty diary...\n\n");
            diary_head = NULL;
        }
    } else {
        printf("\nNo existing diary found. Starting fresh!\n");
    }

    /* Step 3: Main menu */
    while (running) {
        displaymenue();
        int choice = getUserChoice();
        
        if (choice == -1) {
            printf("Invalid input. Please enter a number 1-5.\n");
            continue;
        }
        
        switch (choice) {
            case 1:
                if (diaryCreateEntry(&diary_head)) {
                    diarySaveEncrypted(diary_head, current_filename, encryption_key);
                }
                break;
                
            case 2:
                diaryDisplayAllEntries(diary_head);
                break;

            case 3:  
                diarySearchEntries(diary_head);
                break;

            case 4:
                if (diaryDeleteEntry(&diary_head)) {
                   diarySaveEncrypted(diary_head, current_filename, encryption_key);
                }
                break;
                
            case 5:
                printf("\n========================================\n");
                printf("  Exiting Secure Diary System\n");
                printf("========================================\n");
                
                if (diary_head && strlen(encryption_key) > 0) {
                    printf("Auto-saving diary entries before exit...\n");
                    diarySaveEncrypted(diary_head, current_filename, encryption_key);
                }
                
                if (diary_head) {
                    freeAllEntries(diary_head);
                    diary_head = NULL;
                }
                
                printf("Goodbye!\n");
                running = 0;
                break;
                
            default:
                printf("Invalid choice. Please try again.\n");
                break;
        }
        
        if (running && choice != -1) {
            printf("Press Enter to continue");
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
        }
    }
    
    return 0;
}