#ifndef COMPRESSION_H
#define COMPRESSION_H

#include <stdio.h>

#define NUM_SYMBOLS     256u
#define COMPOSITE_NODE  (-1)

typedef unsigned long long count_Table;

struct frequency_table { count_Table freq[NUM_SYMBOLS]; };

struct huffman_node {
    int value;
    count_Table freq;
    int level;
    int ignore;
    struct huffman_node *left, *right, *parent;
};

#define MAX_CODE_LEN 256
struct code_table {
    char code[NUM_SYMBOLS][MAX_CODE_LEN];
    unsigned int length[NUM_SYMBOLS];
};

int initialise_Frequency(struct frequency_table *ft);
int testMessage(void);
struct huffman_node * merge_tree (struct huffman_node *left, struct huffman_node *right);
int build_Tree(const struct frequency_table *ft, struct huffman_node **out_root);
int count_Freq(FILE*input,struct frequency_table*ft);
struct huffman_node *build_tree_from_frequency(const struct frequency_table *ft);
int initialise_code_table(struct code_table *ct);

int generate_encoding(struct huffman_node *node, struct code_table*ct);
void free_huffman_tree(struct huffman_node *node);


/*----------File-------------*/
int encode_file(FILE*input, FILE*output, struct code_table *ct);
int compress_file(const char *input_file, const char *output_file);


/*---------tree structure save----------*/
 int load_Tree(const char*filename, struct code_table *ct);
 int save_Tree(const char *filename, struct code_table *ct);

/* Memory-based compression/decompression for integration */
char* compress(const char* input, size_t* outputSize);
char* decompress(const char* compressed, size_t compressedSize);

#endif

