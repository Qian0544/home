/*
 *Mahmudunnabi Siam
 * ID : 25924388
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "compression.h"

/******  *Function  :  initialise_Frequency
 *Description : initialises all huffman frequency table by setting all of them to 256
 *Parameters : code_list - pointer to a struct code_list_t whose freq array
 *Effects : For all i in [0, 255], sets code_list->freq[i] = 0.
 *Returned : int - 0 on success; -1 if code_list is NULL.
 */

int initialise_Frequency(struct frequency_table *ft){
    unsigned int i;

    if (ft == NULL) {
        return -1;
    }

    for (i = 0U; i < NUM_SYMBOLS; i++) {
        ft->freq[i] = 0ULL;
    }

    return 0;
}


/******  *Function  : int find_min_weight(struct huffman_node **pool, unsigned int limit)
 *Description : find the minimum weight of node in the pool as frequency as primary criterion and level as tie-breaker
 *Parameters : pool - array of pointer to huffman_node
 *              limit - number of valid node
 *Effects :
 *Returned : minimum weight
 */

int find_min_weight(struct huffman_node **pool, unsigned int limit)
{
    unsigned int i;
    int min_weight;
    count_Table min_freq;
    int min_level;

    min_weight = -1;
    min_freq = 0ULL;
    min_level = 0;

    for (i = 0U; i < limit; i++) {
        if (pool[i] != NULL && pool[i]->ignore == 0) {
            min_weight = (int)i;
            min_freq = pool[i]->freq;
            min_level = pool[i]->level;
            break;
        }
    }

    if (min_weight < 0) {
        return -1;
    }


    for (i = (unsigned int)(min_weight + 1); i < limit; i++) {
        if (pool[i] == NULL || pool[i]->ignore != 0) {
            continue;
        }
        if (pool[i]->freq < min_freq ||
            (pool[i]->freq == min_freq && pool[i]->level < min_level)) {
            min_weight = (int)i;
            min_freq = pool[i]->freq;
            min_level = pool[i]->level;
            }
    }

    return min_weight;
}

/******  *Function  : struct huffman_node * merge_tree (struct huffman_node *left, struct huffman_node *right)
 *Description : merges two composite node into a parent node
 *Parameters : left = left child node
 *              right - right child node
 *Effects : allocates memory for parent and update parent and children pointer
 *Returned : pointer to a parent node or NULL
 */

struct huffman_node * merge_tree (struct huffman_node *left, struct huffman_node *right) {
    int max_level;
    struct huffman_node *parent;

    if (left == NULL || right == NULL) {
        return  NULL;
    }
    parent = (struct huffman_node *)malloc(sizeof(struct huffman_node));
    if (parent == NULL) {
        return NULL;
    }

    parent -> value = COMPOSITE_NODE;  /*integrates other nodes by linking their ports*/
    parent -> freq = left -> freq + right -> freq;  /* sum weights */

    if (left -> level > right -> level) {
        max_level = left -> level;
    } else {
        max_level = right->level;
    }
    parent -> level = max_level + 1;

    parent -> ignore = 0;
    parent -> left = left;
    parent -> right = right;
    parent -> parent = NULL;

    left-> parent = parent;
    right-> parent = parent;
    left -> ignore = 1;
    right -> ignore = 1;

    return parent;
}


/******  *Function  : static struct huffman_node*create_leaf_node(int symbol, count_Table freq)
 *Description :
 *Parameters :
 *Effects :
 *Returned :
 */


static struct huffman_node*create_leaf_node(int symbol, count_Table freq) {
    struct huffman_node *node;
    node = (struct huffman_node *)malloc(sizeof(struct huffman_node));
    if (node == NULL) {
        return NULL;
    }
    node->value = symbol;
    node->freq = freq;
    node->level = 0;
    node->ignore = 0;
    node->left = NULL;
    node->right = NULL;
    node->parent = NULL;

    return node;
}


/******  *Function  : int build_Tree(const struct frequency_table *ft, struct huffman_node **out_root)
 *Description :
 *Parameters :
 *Effects :
 *Returned :
 */


int build_Tree(const struct frequency_table *ft, struct huffman_node **out_root) {  /* out root is the pointer inside the pointer*/
    struct huffman_node *root;

    if (ft == NULL || out_root == NULL) {
        return -1;
    }
    *out_root = NULL;

    root = build_tree_from_frequency(ft);
    if (root == NULL) {
        return -1;
    }
    *out_root = root;
    return 0;
}

/******  *Function  : count_Freq
 *Description : count all the char one by one and tallies them to ft->freq[]
 *Parameters :
 *Effects : this will count the freq so it can be used to weight to create the greedy algo.
 *Returned : 1 on success and 0 on failure
 */

int count_Freq(FILE*input,struct frequency_table*ft) {

    int character; /* count character to give them weight*/

    if (input == NULL || ft == NULL) {
        return -1;
    }

    while ((character = fgetc(input)) != EOF) {
        ft->freq[(unsigned char)character]++;
    }

    return 0;
}


/******  *Function  : build_tree_using_greedy
 *Description : Builds Huffman tree using greedy algorithm
 *Parameters : ft - frequency table with symbol counts
 *Effects :  allocates memory for tree nodes
 *Returned : root of huffman tree
 */

struct huffman_node *build_tree_from_frequency(const struct frequency_table *ft) {
    struct huffman_node * pool[NUM_SYMBOLS *2];
    unsigned int pool_size = 0;
    unsigned int i;
    struct huffman_node *merged;
    int min1, min2;
    int create_Tree = 1;  // 1 = true, 0 = false

    

    if (ft == NULL) {
        return NULL;
    }

    for (i = 0U; i < NUM_SYMBOLS; i++) {
        if (ft->freq[i] > 0UL) {
            pool[pool_size] = create_leaf_node((int)i, ft->freq[i]);
            if (pool[pool_size] == NULL) {
                while (pool_size > 0) {
                    free(pool[--pool_size]);
                }
                return NULL;
            }
            pool_size++;
        }
    }

    if (pool_size == 0) {
        return NULL;
    }
    if (pool_size == 1) {
        return pool[0];
    }

    while (create_Tree) {
        min1 = find_min_weight(pool, pool_size);

        if (min1 == -1) {
            break;
        }
        pool[min1]-> ignore = 2; 
        min2 = find_min_weight(pool, pool_size);
        pool[min1]-> ignore = 0;

        if (min2 == -1) {
            return pool[min1];
        }

        merged = merge_tree(pool[min1], pool[min2]);
        if (merged == NULL) {
            return NULL;
        }

        pool[pool_size] = merged;
        pool_size++;
    }

    for (i = 0U; i < pool_size; i++) {
        if (pool[i] != NULL && pool[i]->ignore == 0) {
            return pool[i];
        }
    }
    return NULL;
}

/******  *Function  : int initialise_code_table(struct code_table *ct)
 *Description : initialises all code table by setting all of them to 0
 *Parameters : ct - pointer to code_table
 *Effects :  set all code length to 0
 *Returned : 0 on success and -1 on fail
 */


int initialise_code_table(struct code_table *ct) {
    unsigned int i;

    if (ct == NULL) {
        return -1;
    }

    for (i = 0U; i < NUM_SYMBOLS; i++) {
        ct->length[i] = 0UL;
        ct->code[i][0] = '\0';
    }

    return 0;
}


/******  *Function  : static void determine_path(struct huffman_node *node,struct code_table *ct,char *current_code,unsigned int depth)
 *Description :
 *Parameters :
 *Effects :
 *Returned :
 */


static void determine_path(struct huffman_node *node,struct code_table *ct,char *current_code,unsigned int depth) {

    if (node->left == NULL && node->right == NULL) {
        unsigned int i;
        int symbol ;
        symbol = node->value;


        for (i = 0U; i < depth; i++) {
            ct->code[symbol][i] = current_code[i];
        }
        ct->code[symbol][depth] = '\0';
        ct->length[symbol] = depth;

        return;
    }


    if (node->left != NULL) {
        current_code[depth] = '0';
        determine_path(node->left, ct, current_code, depth + 1);
    }


    if (node->right != NULL) {
        current_code[depth] = '1';
        determine_path(node->right, ct, current_code, depth + 1);
    }
}


/******  *Function  : static void determine_path(struct huffman_node *node,struct code_table *ct,char *current_code,unsigned int depth)
 *Description :
 *Parameters :
 *Effects :
 *Returned :
 */



int generate_encoding(struct huffman_node *node, struct code_table*ct) {

    char current_code[NUM_SYMBOLS];

    if (node == NULL || ct == NULL) {
        return -1;
    }

    if (initialise_code_table(ct) != 0) {
        return -1;
    }


    if (node -> left == NULL && node -> right == NULL) {
        int sysm;
        sysm = node -> value;
        if (sysm >= 0 && sysm < (int)NUM_SYMBOLS) {
            ct->code[(unsigned)sysm][0] = '0';
            ct->code[(unsigned)sysm][1] = '\0';
            ct->length[(unsigned)sysm] = 1U;
        }
        return 0;
    }
    determine_path((struct huffman_node *)node, ct, current_code, 0U);
    return 0;
}


/******  *Function  : void free_huffman_tree(struct huffman_node *root)
 *Description :
 *Parameters :
 *Effects :
 *Returned :
 */



void free_huffman_tree(struct huffman_node *node){

    if (!node) {
        return;
    }

    free_huffman_tree(node->left);
    free_huffman_tree(node->right);
    free(node);
}


/******  *Function  : int encode_file(FILE*input, FILE*output, struct code_table *ct)
 *Description :
 *Parameters :
 *Effects :
 *Returned :
 */



int encode_file(FILE*input, FILE*output, struct code_table *ct){
    int character;
    unsigned int i;
    unsigned char symbol;

    if (input == NULL || output == NULL || ct == NULL){
        return -1;
    }

    rewind(input);

    while ((character = fgetc(input)) != EOF) {
        symbol = (unsigned char)character;

        for (i = 0; i <ct->length[symbol]; i++) {
            fputc(ct->code[symbol][i], output);
        }
    }

    return 0;
}


/******  *Function  : int encode_file(FILE*input, FILE*output, struct code_table *ct)
 *Description :
 *Parameters :
 *Effects :
 *Returned :
 */



int compress_file(const char *input_file, const char *output_file) {
    FILE *input, *output;
    struct frequency_table ft;
    struct code_table ct;
    struct huffman_node *node;

    input = fopen(input_file, "rb");
    if (input == NULL) {
        return -1;
    }

    initialise_Frequency(&ft);
    if (count_Freq(input, &ft) != 0) {
        fclose(input);
        return -1;
    }

    if (build_Tree(&ft, &node) != 0) {
        fclose(input);
        return -1;
    }

    if (generate_encoding(node, &ct) != 0) {
        free_huffman_tree(node);
        fclose(input);
        return -1;
    }

    output = fopen(output_file, "wb");
    if (output == NULL) {
        free_huffman_tree(node);
        fclose(input);
        return -1;
    }

    encode_file(input, output, &ct);
    free_huffman_tree(node);
    fclose(input);
    fclose(output);
    
    return 0;
}



/******  *Function  : int encode_file(FILE*input, FILE*output, struct code_table *ct)
 *Description :
 *Parameters :
 *Effects :
 *Returned :
 */



int save_Tree(const char *filename, struct code_table *ct){
    FILE *file;
    unsigned int i;
    unsigned int count = 0;

    if (filename == NULL || ct == NULL){
        return -1;
    }

    file = fopen(filename, "w");
    if (file == NULL){
        return -1;
    }

    fprintf(file, "HUFFMAN_TREE\n");

    for (i=0; i<NUM_SYMBOLS; i++){
        if(ct->length[i] > 0UL){
            count++;
        }
    }

    fprintf(file, "%u\n", count);

    for (i = 0U; i<NUM_SYMBOLS; i++){
        if (ct->length[i] > 0UL){
            if (i == '\n'){
                fprintf(file, "\\n %s %u\n", ct->code[i], ct->length[i]);
            }
            else if (i == ' '){
                fprintf(file, "\\s %s %u\n", ct->code[i], ct->length[i]);
            }
            else if (i == '\\'){
                fprintf(file, "\\\\ %s %u\n", ct->code[i], ct->length[i]);
            }
            else if ( i>= 32 && i <=126){
                fprintf(file, "%c %s %u\n", (char)i,ct->code[i], ct->length[i]);
            }
            else{
                fprintf(file, "\\x%02X %s %u\n", i, ct->code[i], ct->length[i]);
            }
        }
    }

    fclose(file);


    return 0;
}


/******  *Function  : int encode_file(FILE*input, FILE*output, struct code_table *ct)
 *Description :
 *Parameters :
 *Effects :
 *Returned :
 */


 int load_Tree(const char*filename, struct code_table *ct){

    FILE *file;
    unsigned int i, count;
    char code[NUM_SYMBOLS];
    unsigned long length;
    unsigned char symbol;
    char header[50];
    char symbol_str[10];

    if (filename == NULL || ct == NULL){
        return -1;
    }

    file = fopen(filename, "r");

    if (file == NULL){
        return -1;
    }

    if (fgets(header, sizeof(header), file) == NULL){
        fclose(file);
        return -1;
    }

    if (strncmp(header, "HUFFMAN_TREE", 18) != 0){
        fclose(file);
        return -1;
    }

    if (initialise_code_table(ct) != 0){
        fclose(file);
        return -1;
    }

    if (fscanf(file, "%u\n", &count) != 1){
        fclose(file);
        return -1;
    }

    for (i = 0U; i<count ; i++){
        if (fscanf(file, "%s %s %lu\n", symbol_str, code, &length) != 3) {
            fclose(file);
            return -1;
        }
        

        if (strcmp(symbol_str, "\\n") == 0) 
        {
            symbol = '\n';
        } 
        else if (strcmp(symbol_str, "\\s") == 0) 
        {
            symbol = ' ';
        } 
        else if (strcmp(symbol_str, "\\\\") == 0) 
        {
            symbol = '\\';
        } 
        else if (symbol_str[0] == '\\' && symbol_str[1] == 'x') 
        {
            sscanf(symbol_str + 2, "%hhx", &symbol);
        } 
        else 
        {
            symbol = (unsigned char)symbol_str[0];
        }
        

        strcpy(ct->code[symbol], code);
        ct->length[symbol] = length;
    }


    fclose(file);


    return 0;
 }

 // Helper: Serialize code table to a string
static char* serialize_code_table(struct code_table *ct, size_t *output_size) {
    char *buffer = NULL;
    size_t buffer_capacity = 4096;  // Start with 4KB
    size_t buffer_used = 0;
    unsigned int count = 0;
    
    if (!ct || !output_size) {
        return NULL;
    }
    
    buffer = malloc(buffer_capacity);
    if (!buffer) {
        return NULL;
    }
    
    // Count symbols with codes
    for (unsigned int i = 0; i < NUM_SYMBOLS; i++) {
        if (ct->length[i] > 0) {
            count++;
        }
    }
    
    // Write header
    buffer_used += sprintf(buffer + buffer_used, "HUFFMAN_TABLE\n%u\n", count);
    
    // Write each symbol's code
    for (unsigned int i = 0; i < NUM_SYMBOLS; i++) {
        if (ct->length[i] > 0) {
            // Check if we need more space
            if (buffer_used + 300 > buffer_capacity) {
                buffer_capacity *= 2;
                char *new_buffer = realloc(buffer, buffer_capacity);
                if (!new_buffer) {
                    free(buffer);
                    return NULL;
                }
                buffer = new_buffer;
            }
            
            // Format: symbol code length
            // Handle special characters
            if (i == '\n') {
                buffer_used += sprintf(buffer + buffer_used, "\\n %s %u\n", 
                                      ct->code[i], ct->length[i]);
            } else if (i == ' ') {
                buffer_used += sprintf(buffer + buffer_used, "\\s %s %u\n", 
                                      ct->code[i], ct->length[i]);
            } else if (i == '\\') {
                buffer_used += sprintf(buffer + buffer_used, "\\\\ %s %u\n", 
                                      ct->code[i], ct->length[i]);
            } else if (i >= 32 && i <= 126) {
                buffer_used += sprintf(buffer + buffer_used, "%c %s %u\n", 
                                      (char)i, ct->code[i], ct->length[i]);
            } else {
                buffer_used += sprintf(buffer + buffer_used, "\\x%02X %s %u\n", 
                                      i, ct->code[i], ct->length[i]);
            }
        }
    }
    
    *output_size = buffer_used;
    return buffer;
}

 // New function - works with memory buffers
char* compress(const char* input, size_t* outputSize) {
    struct frequency_table ft;
    struct code_table ct;
    struct huffman_node *node;
    char *compressed_bits = NULL;
    char *tree_data = NULL;
    char *final_output = NULL;
    size_t bits_size = 0;
    size_t tree_size = 0;
    size_t offset = 0;
    
    if (!input || !outputSize) {
        return NULL;
    }
    
    size_t input_len = strlen(input);
    if (input_len == 0) {
        *outputSize = 0;
        return NULL;
    }
    
    // Step 1: Build frequency table
    initialise_Frequency(&ft);
    for (size_t i = 0; i < input_len; i++) {
        ft.freq[(unsigned char)input[i]]++;
    }
    
    // Step 2: Build Huffman tree
    if (build_Tree(&ft, &node) != 0) {
        return NULL;
    }
    
    // Step 3: Generate encoding table
    if (generate_encoding(node, &ct) != 0) {
        free_huffman_tree(node);
        return NULL;
    }
    
    // Step 4: Serialize code table to string
    tree_data = serialize_code_table(&ct, &tree_size);
    if (!tree_data) {
        free_huffman_tree(node);
        return NULL;
    }
    
    // Step 5: Encode input to binary string
    size_t estimated_size = 0;
    for (size_t i = 0; i < input_len; i++) {
        unsigned char symbol = (unsigned char)input[i];
        estimated_size += ct.length[symbol];
    }
    
    compressed_bits = malloc(estimated_size + 1);
    if (!compressed_bits) {
        free(tree_data);
        free_huffman_tree(node);
        return NULL;
    }
    
    bits_size = 0;
    for (size_t i = 0; i < input_len; i++) {
        unsigned char symbol = (unsigned char)input[i];
        for (unsigned int j = 0; j < ct.length[symbol]; j++) {
            compressed_bits[bits_size++] = ct.code[symbol][j];
        }
    }
    compressed_bits[bits_size] = '\0';
    
    // Step 6: Combine tree + compressed data
    // Format: [tree_size][tree_data][bits_size][compressed_bits]
    size_t total_size = sizeof(size_t) + tree_size + sizeof(size_t) + bits_size;
    final_output = malloc(total_size);
    if (!final_output) {
        free(tree_data);
        free(compressed_bits);
        free_huffman_tree(node);
        return NULL;
    }
    
    // Write tree size
    memcpy(final_output + offset, &tree_size, sizeof(size_t));
    offset += sizeof(size_t);
    
    // Write tree data
    memcpy(final_output + offset, tree_data, tree_size);
    offset += tree_size;
    
    // Write bits size
    memcpy(final_output + offset, &bits_size, sizeof(size_t));
    offset += sizeof(size_t);
    
    // Write compressed bits
    memcpy(final_output + offset, compressed_bits, bits_size);
    
    // Cleanup
    free(tree_data);
    free(compressed_bits);
    free_huffman_tree(node);
    
    *outputSize = total_size;
    return final_output;
}

// Helper: Deserialize code table from string
static int deserialize_code_table(const char* data, size_t data_size, struct code_table *ct) {
    const char *ptr = data;
    const char *end = data + data_size;
    unsigned int count = 0;
    unsigned int i;
    
    if (!data || !ct) {
        return -1;
    }
    
    // Initialize code table
    if (initialise_code_table(ct) != 0) {
        return -1;
    }
    
    // Read header "HUFFMAN_TABLE\n"
    if (strncmp(ptr, "HUFFMAN_TABLE\n", 14) != 0) {
        return -1;
    }
    ptr += 14;
    
    // Read count
    if (sscanf(ptr, "%u\n", &count) != 1) {
        return -1;
    }
    
    // Skip to next line
    while (ptr < end && *ptr != '\n') ptr++;
    if (ptr < end) ptr++;  // Skip newline
    
    // Read each symbol entry
    for (i = 0; i < count; i++) {
        char symbol_str[10];
        char code[MAX_CODE_LEN];
        unsigned int length;
        unsigned char symbol;
        
        // Read one line: symbol code length
        int items = sscanf(ptr, "%s %s %u\n", symbol_str, code, &length);
        if (items != 3) {
            return -1;
        }
        
        // Parse symbol
        if (strcmp(symbol_str, "\\n") == 0) {
            symbol = '\n';
        } else if (strcmp(symbol_str, "\\s") == 0) {
            symbol = ' ';
        } else if (strcmp(symbol_str, "\\\\") == 0) {
            symbol = '\\';
        } else if (symbol_str[0] == '\\' && symbol_str[1] == 'x') {
            sscanf(symbol_str + 2, "%hhx", &symbol);
        } else {
            symbol = (unsigned char)symbol_str[0];
        }
        
        // Store in code table
        strcpy(ct->code[symbol], code);
        ct->length[symbol] = length;
        
        // Move to next line
        while (ptr < end && *ptr != '\n') ptr++;
        if (ptr < end) ptr++;  // Skip newline
    }
    
    return 0;
}

char* decompress(const char* compressed, size_t compressedSize) {
    struct code_table ct;
    char *output_buffer = NULL;
    size_t output_capacity = 1024;
    size_t output_size = 0;
    size_t tree_size = 0;
    size_t bits_size = 0;
    const char *tree_data = NULL;
    const char *compressed_bits = NULL;
    size_t offset = 0;
    
    if (!compressed || compressedSize == 0) {
        return NULL;
    }
    
    // Step 1: Extract tree size
    if (compressedSize < sizeof(size_t)) {
        return NULL;
    }
    memcpy(&tree_size, compressed + offset, sizeof(size_t));
    offset += sizeof(size_t);
    
    // Step 2: Extract tree data
    if (offset + tree_size > compressedSize) {
        return NULL;
    }
    tree_data = compressed + offset;
    offset += tree_size;
    
    // Step 3: Deserialize code table
    if (deserialize_code_table(tree_data, tree_size, &ct) != 0) {
        return NULL;
    }
    
    // Step 4: Extract bits size
    if (offset + sizeof(size_t) > compressedSize) {
        return NULL;
    }
    memcpy(&bits_size, compressed + offset, sizeof(size_t));
    offset += sizeof(size_t);
    
    // Step 5: Extract compressed bits
    if (offset + bits_size > compressedSize) {
        return NULL;
    }
    compressed_bits = compressed + offset;
    
    // Step 6: Decode bits using code table
    output_buffer = malloc(output_capacity);
    if (!output_buffer) {
        return NULL;
    }
    
    char current_code[MAX_CODE_LEN];
    unsigned int code_pos = 0;
    
    for (size_t i = 0; i < bits_size; i++) {
        current_code[code_pos++] = compressed_bits[i];
        current_code[code_pos] = '\0';
        
        // Check if current_code matches any symbol's code
        for (unsigned int symbol = 0; symbol < NUM_SYMBOLS; symbol++) {
            if (ct.length[symbol] > 0 && strcmp(current_code, ct.code[symbol]) == 0) {
                // Found matching code!
                
                // Expand buffer if needed
                if (output_size + 1 >= output_capacity) {
                    output_capacity *= 2;
                    char *new_buffer = realloc(output_buffer, output_capacity);
                    if (!new_buffer) {
                        free(output_buffer);
                        return NULL;
                    }
                    output_buffer = new_buffer;
                }
                
                output_buffer[output_size++] = (char)symbol;
                code_pos = 0;  // Reset for next code
                break;
            }
        }
    }
    
    output_buffer[output_size] = '\0';
    return output_buffer;
}
