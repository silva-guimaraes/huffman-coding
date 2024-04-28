
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

#define BUFFER_SIZE 2e9
// #define TABLE_SIZE 256+1
#define TABLE_SIZE 128

typedef struct node {
    char c;
    int count;
    struct node *left, *right;

} node;

void insert_sort(node* list[TABLE_SIZE]) {
    for (int i = 1; i < TABLE_SIZE; i++)
    {
        for (int j = i; j > 0; j--)
        {
            if (list[j]->count < list[j-1]->count) {
                node* temp = list[j];
                list[j] = list[j-1];
                list[j-1] = temp;
            }
            else break;
        }
    }
}

void _create_lookup_table(char* table[], node* tree, char* bit_string, long int* bits_amount) {

    if (tree->c > -1) {
        table[tree->c] = bit_string;
        *bits_amount += strlen(bit_string) * tree->count; 
    }
    else {
        char* left = strdup(bit_string);
        left = strcat(left, "0");

        char* right = strdup(bit_string);
        right = strcat(right, "1");

        free(bit_string);

        _create_lookup_table(table, tree->left, left, bits_amount);
        _create_lookup_table(table, tree->right, right, bits_amount);
    }

}

void create_lookup_table(char* table[], node* tree, long int* bits_amount) {

    char* bit_string_initial = malloc(sizeof(char));
    bit_string_initial[0] = '\0';
    _create_lookup_table(table, tree, bit_string_initial, bits_amount);
}


// debug
void _graph(node* root) {

    if (root->c == -1)
        printf("\"%p\" [label=\"%d\"];\n", root, root->count);
    else
        printf("\"%p\" [label=\"%c\"];\n", root, root->c);

    if (root->left) {
        printf("\"%p\" -> \"%p\";\n", root, root->left);
        _graph(root->left);
    }

    if (root->right) {
        printf("\"%p\" -> \"%p\";\n", root, root->right);
        _graph(root->right);
    }
}

// debug
void graph(node* root) {
    printf("digraph tree {\n");
    _graph(root);
    printf("}\n");
}

int main(int argc, char* argv[]) {

    const char* output_file_name = "output.huff";

    // if (argc == 1) {
    //     fprintf(stderr, "ERRO: Nome do arquivo de output é necessário como primeiro argumento.\n");
    //     exit(1);
    // }

    char* input = malloc(BUFFER_SIZE);
    unsigned long long size = 0;

    // le input do stdin
    int bytes_read = 0;

    while ((bytes_read = read(0, input+size, BUFFER_SIZE)) > 0) 
    {
        size += bytes_read;
        input = realloc(input, size + BUFFER_SIZE);
    }
    input = realloc(input, size);

    if (size >= 3 && memcmp(input, (char[]) {0xEF, 0xBB, 0xBF}, 3 ) == 0) {
        fprintf(stderr, "ERRO: Texto em formato UTF-8, não suportado no momento. Apenas aceito ASCII.\n");
        exit(1);
    }

    node* count[TABLE_SIZE] = { 0 };


    for (int i = 0; i < TABLE_SIZE; i++) {
        count[i] = malloc(sizeof(node));

        *count[i] = (node) { 
            .count = 0, 
            .c = i, 
            .left = NULL, 
            .right = NULL,
        };
    }

    for (int i = 0; i < size; i++)
        count[input[i]]->count++;

    // int foo = 0;
    // for (int i = 0; i < TABLE_SIZE; i++)
    //     foo += count[i]->count;

    insert_sort(count);


    int first = -1;
    while (count[++first]->count == 0);

    for (int i = first; i < TABLE_SIZE-1; i++) {

        node* left = count[i];

        node* right = count[i+1];

        node* n = malloc(sizeof(node));
        *n = (node) {
            .c = -1,
            .count = left->count + right->count,
            .left = left,
            .right = right,
        };

        count[i] = n;

        insert_sort(count);
    }

    node* root = count[TABLE_SIZE-1];

    char* lookup[TABLE_SIZE] = { 0 };
    long int bits_amount = 0;

    create_lookup_table(lookup, root, &bits_amount);

    char* compressed_bits = malloc(ceil((float) bits_amount / 8));

    long int current_byte = 0;
    int bits_counter = 0;

    for (int i = 0; i < size; i++) {

        char* bit_string = lookup[input[i]];
        size_t len = strlen(bit_string);

        for (int j = 0; j < len; j++) {
            char bit = bit_string[j] == '1' ? 1 : 0;

            compressed_bits[current_byte] <<= 1;
            compressed_bits[current_byte] |= bit;

            bits_counter++;

            if (bits_counter == 8) {
                current_byte++;
                bits_counter = 0;
            }
        }
    }

    FILE* output_file = fopen(output_file_name, "w");
    fwrite(compressed_bits, sizeof(char), current_byte, output_file);

    printf("%ld bytes salvos em %s\n", current_byte, output_file_name);


    return 0;
}


