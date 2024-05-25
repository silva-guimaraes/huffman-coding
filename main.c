
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

#define BUFFER_SIZE 2e9
#define TABLE_SIZE 256

// A PESSOA QUE ESCREVEU ESSE CÓDIGO TEM A PESSIMA MANIA DE NOMEAR TUDO EM INGLÊS.
//
//
//
//
// ESTEJA AVISADO.


// 258370


// salva quantas vezes um caractere apareceu no documento e ao mesmo tempo serve
// de nó para montar a árvore binaria
typedef struct node {
    // caractere
    unsigned char c;
    // frequência absoluta do caractere. nós intermediarios terão a soma da frequência de todos os nós filhos
    int count;
    // se nó é um nó intermediario.
    bool subtree;
    // para os nós intermediarios apenas: 
    struct node *left, *right;

} node;

void insert_sort(node*[], int);
void create_lookup_table(char*[], node*, long int*);
void graph(node*);
void encode();
void decode(char*);
node* build_tree(node*[], int);
char* append_bit(char* bit_string, char bit);

int main(int argc, char* argv[]) {

    if (argc > 1)
        decode(argv[1]);
    else
        encode();

}

void encode() {
    const char* output_file_name = "output.huff";

    // arquivo pra onde salvar toda a compressão
    FILE* output_file = fopen(output_file_name, "wb");
    if (!output_file) {
        fprintf(stderr, "ERRO: Erro ao abrir arquivo\n");
        exit(1);
    }

    // documento sera carregado todo em memória para esse input.
    unsigned char* input = malloc(BUFFER_SIZE);
    // tamanho em bytes do documento.
    unsigned long size = 0;

    // lê input do stdin
    // ./main < texto.txt
    int bytes_read = 0;

    while ((bytes_read = read(0, input+size, BUFFER_SIZE)) > 0) 
    {
        size += bytes_read;
        input = realloc(input, size + BUFFER_SIZE);
    }
    input = realloc(input, size);

    // o programa só funciona com ascii no momento 
    // ignorar arquivos UTF-8
    if (size >= 3 && memcmp(input, (char[]) {0xEF, 0xBB, 0xBF}, 3 ) == 0) {
        fprintf(stderr, "ERRO: Texto em formato UTF-8, não suportado no momento. Apenas aceito ASCII.\n");
        exit(1);
    }

    // uma look up table. isso facilita fazer a contagem.
    node* count[TABLE_SIZE] = { NULL };
    // TABLE_SIZE == total de caracteres na codificação ascii.

    // inicializar cada struct node da contagem.
    for (int i = 0; i < TABLE_SIZE; i++) {

        count[i] = malloc(sizeof(node)); // não esquecer de fazer o free()!!!!

        *count[i] = (node) { 
            .count = 0, // todos esses contadores tem contagem 0 por enquanto.
            .c = i, // indice da posição do elemento == caractere em ascii que ele conta.
                    // exemplo:
                    // 'a' == 97
                    // count['a'] == count[97]
            .subtree = false,
            .left = NULL, 
            .right = NULL,
        };
    }

    // isso faz toda a contagem do arquivo.
    for (int i = 0; i < size; i++)
        count[input[i]]->count++;

    // tendo toda a contagem feita, vamos salvar essa tabela no arquivo de output.
    // é importante que o arquivo tenha a tabela de contagens para que decodificação
    // seja possível.
    
    int header_size = 0; // tamanho em espaço que o header toma conta no arquivo comprimido.

    int amount = 0; // numero de itens da tabela
    for (int i = 0; i < TABLE_SIZE; i++)
        if (count[i]->count > 0) // não incluir caracteres que não aparecem no texto
            amount++;

    fwrite("huff", sizeof(char), 4, output_file); // magic number. isso identifica o arquivo
    fwrite(&amount, sizeof(int), 1, output_file);
    fwrite(&size, sizeof(unsigned long), 1, output_file); // tamanho em bytes do arquivo descomprimido

    header_size += sizeof(char) * 4 + sizeof(int) + sizeof(unsigned long);

    for (int i = 0; i < TABLE_SIZE; i++) {
        if (count[i]->count == 0)
            continue;
        fwrite(&count[i]->c, sizeof(char), 1, output_file);
        fwrite(&count[i]->count, sizeof(int), 1, output_file);
        header_size += sizeof(char) + sizeof(int);
    }


    // monta a nossa arvore.
    node* root = build_tree(count, TABLE_SIZE);


    #ifdef GRAPH
    graph(root);
    return;
    #endif /* ifdef GRAPH */

    // tabela pra traduzir caracteres para a codificação que criamos com a árvore.
    char* lookup[TABLE_SIZE] = { NULL };
    long int bits_amount = 0; // quantos bits o documento comprimido tem no total.
    create_lookup_table(lookup, root, &bits_amount);

    // array de bytes para o output final comprimido. cada byte dessa array será preenchido com os bits.
    // o grande problema é que os bits da nossa codificação não todos se encaixam exatamente em um byte.
    // por causa disso iremos preencher cada byte até o seu maximo e passar para o proximo byte caso os bits
    // transbordem.
    unsigned char* compressed_bits = calloc(ceil((float) bits_amount / 8), sizeof(char));
    // byte atual onde bits estarão sendo inseridos.
    unsigned long int current_byte = 0;
    // quantos bits do byte atual foram inseridos. 8 == hora de passar para o proximo byte
    int bits_counter = 0;

    // itera pelo documento inteiro caractere por caractere e traduz esse caractere para o bit string equivalente,
    // que logo em seguida é adicionado aos bits comprimidos.
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
    compressed_bits[current_byte] <<= 8 - bits_counter;

    // salvar bits comprimidos
    fwrite(compressed_bits, sizeof(char), current_byte+1, output_file);
    free(compressed_bits);
    fclose(output_file);

    unsigned long total_size = header_size + (current_byte+1) * sizeof(char);

    #ifndef GRAPH
    printf("%ld -> %ld bytes salvos em \"%s\". %.2f%% de taxa, %.2f%% salvos\n",
           size, total_size, output_file_name,
           (float)total_size / size * 100,
           (1 - (float) total_size / size) * 100);
    #endif /* ifndef GRAPH */
}

/*
 * assim fica a formatação do arquivo depois de salvo:
 * 4 bytes para o 'magic number', um indentificador para dizer se o arquivo foi comprimido com essa codificaçá̃o.
 * 4 bytes (int) para o tamanho da tabela de frequencia.
 * 8 bytes (unsigned long) tamanho em bytes do arquivo descomprimido.
 * (1 byte (char) + 4 bytes (int)) * N, onde char é o caractere, int a frequencia, e N o numero de caracteres.
 * todo o resto do arquivo é dedicado aos bits comprimidos.
 */
// não sabia um jeito melhor de salvar como arquivo peço desculpas pela complexidade.

void decode(char* input_filepath) {
    FILE * input_file = fopen (input_filepath, "rb");
    if (!input_file) {
        fprintf(stderr, "ERRO: falha ao abrir arquivo \"%s\"!\n", input_filepath);
        exit(1);
    }

    char* magic = malloc(sizeof(char) * 5);
    magic[4] = '\0';

    fread(magic, sizeof(char), 4, input_file);

    if (strcmp(magic, "huff") != 0) {
        fprintf(stderr, "ERRO: este arquivo não foi codificado com a codificação de huffman!\n\n");
        fprintf(stderr, "\tPara codificar um arquivo: ./main < arquivo_a_ser_codificado.txt\n");
        fprintf(stderr, "\tPara decodificar um arquivo: ./main arquivo_a_ser_decodificado.huff\n");
        exit(1);
    }

    int table_size;
    fread(&table_size, sizeof(int), 1, input_file);

    unsigned long decoded_size;
    fread(&decoded_size, sizeof(unsigned long), 1, input_file);

    node** table = malloc(sizeof(node*) * table_size);

    for (int i = 0; i < table_size; i++) {
        table[i] = malloc(sizeof(node));

        char c;
        fread(&c, sizeof(char), 1, input_file);
        int count;
        fread(&count, sizeof(int), 1, input_file);

        *table[i] = (node) {
            .c = c,
            .count = count,
            .left = NULL,
            .right = NULL,
        };

    }

    long start = ftell(input_file);
    fseek(input_file, 0, SEEK_END);

    long end = ftell(input_file);
    fseek(input_file, start, SEEK_SET);

    long size = end - start;

    char* input = malloc(size);
    char* output = malloc(decoded_size);
    fread(input, 1, size, input_file);

    node* root = build_tree(table, table_size);


    int current_bit = 0;
    int current_output = 0;
    node* current_node = root;

    for (int i = 0; i < size;) {
        
        if ((input[i] & 0x80) > 0)
            current_node = current_node->right;
        else 
            current_node = current_node->left;

        if (current_node->c > 0) {
            output[current_output] = current_node->c;
            current_output++;
            current_node = root;

            if (current_output == decoded_size) 
                break;
        }

        input[i] <<= 1;
        current_bit++;

        if (current_bit == 8) {
            current_bit = 0;
            i++;
        }
    }

    fwrite(output, sizeof(char), decoded_size, stdout);

    // graph(root);

    fclose(input_file);

}

node* build_tree(node* count[], int size) {
    // ordem crescente pela frenquência absoluta.
    insert_sort(count, size);

    // sempre sobram alguns caracteres que não estavam no documento (contagem 0) e o sort fez com que eles 
    // ficassem no inicio da lista ordena.
    // isso encontra o primeiro elemento com contagem > 0
    int first = -1;
    while (count[++first]->count == 0);


    // itera a lista de contagem ordenada, começando do primeiro elemento com contagem > 0
    // e executa o grosso do algoritmo (não vou explicar isso aqui pesquise no google)
    for (int i = first; i < size-1 /* parar antes do ultimo elemento */; i++) {

        // primeiro elemento, vai pra esquerda
        node* left = count[i];

        // segundo elemento, vai pra direita
        node* right = count[i+1];

        // criar novo nó intermediario
        node* n = malloc(sizeof(node));
        *n = (node) {
            // .c = 0, // caractere 0 sinaliza ele ser um nó intermediario
            .subtree = true,
            .count = left->count + right->count,
            .left = left,
            .right = right,
        };

        // o algoritmo pedia pra inserir o nó de volta na lista, mas eu achei mais facil por esse nó
        // intermediario no lugar do primeiro elemento (que agora é filho desse nó intermediario,
        // então não estamos perdendo ele) e deixar que o sort põe esse nó intermediario no lugar correto.

        count[i] = n;
        insert_sort(count, size);

        // no meio disso tudo, nós intermediario herdam outros nós intermediarios e isso monta toda
        // a árvore.
    }

    // tendo feito isso, o ultimo elemento é o root da nossa árvore
    node* root = count[size-1];

    return root;
}


// autoexplicativo
void insert_sort(node* list[], int size) {

    for (int i = 1; i < size; i++)
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

// tendo montado toda a arvore, precisamos associar cada caractere com a sua codificação correspondente.
// essa função percorre toda a arvore motando um bit string que se equivale a codificaçãodo caractere,
// e salva isso em uma array onde o valor ascii de um caractere é usado como chave para retornar a bit string.
// a bit string serve apenas como um representação intermediaria, iremos traduzir isso para bits
// de verdade logo depois.

// (não sabia um jeito melhor de fazer isso)
void _create_lookup_table(char* table[], node* tree, char* bit_string, long int* bits_amount) {

    if (!tree->subtree) {
        table[tree->c] = bit_string;
        *bits_amount += strlen(bit_string) * tree->count; 
    }
    else {
        char* left = append_bit(bit_string, '0');
        char* right = append_bit(bit_string, '1');

        _create_lookup_table(table, tree->left, left, bits_amount);
        _create_lookup_table(table, tree->right, right, bits_amount);

        // estamos em um nó intermediario e não precisamos dessa bit string que aponca pra esse caminho.
        free(bit_string);

    }

}
void create_lookup_table(char* table[], node* tree, long int* bits_amount) {

    char* bit_string_initial = malloc(sizeof(char));
    bit_string_initial[0] = '\0';
    _create_lookup_table(table, tree, bit_string_initial, bits_amount);
}

// perdi um dia inteiro nisso daqui porque C é uma linguagem burra.
// strcat NÃO realloca espaço na string pra fazer a concatenação
//
// char* left = strdup(bit_string);
// left = strcat(left, "0");
// ^ isso é errado e essa lingua não te impede de fazer isso!!!!!!!
//
// C foi uma pessima escolha. sinceramente acho que eu botei mais esforço nesse projeto do que eu deveria.
char* append_bit(char* bit_string, char bit) {

    int len = strlen(bit_string);
    char* dup = strdup(bit_string);

    dup = realloc(dup, len+2 * sizeof(char));

    dup[len] = bit;
    dup[len+1] = '\0';

    return dup;
}


// graphviz é um programa que desenha grafos. ele funciona a partir de uma linguagem especifica.
// essa função transforma toda a árvore nessa linguagem especifica. use com o script ./graph.sh
// isso é para debug
#ifdef GRAPH
void _graph(node* root) {

    if (root->subtree)
        printf("\"%p\" [label=\"%d\"];\n", root, root->count);
    else {
        char c = root->c;

        switch (c) {
            case '\n':
                printf("\"%p\" [label=\"\\\\n\"];\n", root);
                break;
            case '\t':
                printf("\"%p\" [label=\"\\\\t\"];\n", root);
                break;
            case '\\':
                printf("\"%p\" [label=\"\\\\\"];\n", root);
                break;
            case '"':
                printf("\"%p\" [label=\"\\\"\"];\n", root);
                break;
            default:
                printf("\"%p\" [label=\"%c\"];\n", root, c);
                break;
        }

    }

    if (root->left) {
        printf("\"%p\" -> \"%p\";\n", root, root->left);
        _graph(root->left);
    }

    if (root->right) {
        printf("\"%p\" -> \"%p\";\n", root, root->right);
        _graph(root->right);
    }
}

void graph(node* root) {
    printf("digraph tree {\n");
    _graph(root);
    printf("}\n");
}

#endif /* ifdef GRAPH */


