
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

#define BUFFER_SIZE 2e9
#define TABLE_SIZE 256
// #define TABLE_SIZE 128

// salva quantas vezes um caractere apareceu no documento e ao mesmo tempo serve de nó para montar a árvore binaria
typedef struct node {
    // caractere
    unsigned char c;
    // frequência absoluta do caractere. nós intermediarios terão a soma da frequência de todos os nós filhos
    int count;
    // para os nós intermediarios apenas: 
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

// tendo montado toda a arvore, precisamos associar cada caractere com a sua codificação correspondente.
// essa função percorre toda a arvore motando um bit string que se equivale a codificaçãodo caractere, e salva isso em
// uma array onde o valor ascii de um caractere é usado como chave para retornar a bit string.
// a bit string serve apenas como um representação intermediaria, iremos traduzir isso para bits de verdade logo depois.
// (não sabia um jeito melhor de fazer isso)
void _create_lookup_table(char* table[], node* tree, char* bit_string, long int* bits_amount) {

    if (tree->c > 0) {
        table[tree->c] = bit_string;
        *bits_amount += strlen(bit_string) * tree->count; 
    }
    else {
        char* left = strdup(bit_string);
        left = strcat(left, "0");

        char* right = strdup(bit_string);
        right = strcat(right, "1");

        _create_lookup_table(table, tree->left, left, bits_amount);
        _create_lookup_table(table, tree->right, right, bits_amount);

        free(bit_string);

    }

}
void create_lookup_table(char* table[], node* tree, long int* bits_amount) {

    char* bit_string_initial = malloc(sizeof(char));
    bit_string_initial[0] = '\0';
    _create_lookup_table(table, tree, bit_string_initial, bits_amount);
}


// graphviz é um programa que desenha grafos. ele funciona a partir de uma linguagem especifica.
// essa função transforma toda a árvore nessa linguagem especifica. use com o script ./script.sh
// isso é para debug
void _graph(node* root) {

    if (root->c == 0)
        printf("\"%p\" [label=\"%d\"];\n", root, root->count);
    else {
        char c = root->c;

        switch (c) {
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

// debug
void graph(node* root) {
    printf("digraph tree {\n");
    _graph(root);
    printf("}\n");
}

int main(int argc, char* argv[]) {

    const char* output_file_name = "output.huff";

    // arquivo pra onde salvar toda a compressão
    FILE* output_file = fopen(output_file_name, "wb");
    if (!output_file) {
        fprintf(stderr, "ERRO: Erro ao abrir arquivo\n");
        return 1;
    }

    // documento sera carregado todo em memória para esse input.
    unsigned char* input = malloc(BUFFER_SIZE);
    // tamanho em bytes do documento.
    unsigned long int size = 0;

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
            .left = NULL, 
            .right = NULL,
        };
    }

    // isso faz toda a contagem.
    for (int i = 0; i < size; i++)
        count[input[i]]->count++;


    // ordem crescente pela frenquência absoluta.
    insert_sort(count);


    // sempre sobram alguns caracteres que não estavam no documento (contagem 0) e o sort fez com que eles 
    // ficassem no inicio da lista ordena.
    // isso encontra o primeiro elemento com contagem > 0
    int first = -1;
    while (count[++first]->count == 0);

    // itera a lista de contagem ordenada, começando do primeiro elemento com contagem > 0
    // e executa o grosso do algoritmo (não vou explicar isso aqui pesquise no google)
    for (int i = first; i < TABLE_SIZE-1 /* parar antes do ultimo elemento */; i++) {

        // primeiro elemento, vai pra esquerda
        node* left = count[i];

        // segundo elemento, vai pra direita
        node* right = count[i+1];

        // criar novo nó intermediario
        node* n = malloc(sizeof(node));
        *n = (node) {
            .c = 0, // caractere 0 sinaliza ele ser um nó intermediario
            .count = left->count + right->count,
            .left = left,
            .right = right,
        };

        // o algoritmo pedia pra inserir o nó de volta na lista, mas eu achei mais facil por esse nó
        // intermediario no lugar do primeiro elemento (que agora é filho desse nó intermediario,
        // então não estamos perdendo ele) e deixar que o sort põe esse nó intermediario no lugar dele.

        count[i] = n; // nó intermediario no lugar do primeiro elemento
        insert_sort(count); // poe o nó intermediario no lugar dele.

        // no meio disso tudo, acontece que nós intermediario herdam outros nós intermediarios e isso monta toda
        // a árvore.
    }

    // tendo feito isso, o ultimo elemento é o root da nossa árvore
    node* root = count[TABLE_SIZE-1];

    // debug. passe o primeiro argumeto como "graph" para imprimir o grafo pra stdout.
    if (argc > 1 && strcmp(argv[1], "graph") == 0)
        graph(root);

    // tabela pra traduzir caracteres para a codificação que criamos com a árvore.
    char* lookup[TABLE_SIZE] = { 0 };
    // quantos bits o documento comprimido tem no total.
    long int bits_amount = 0;
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

    // eu não sabia um jeito melhor de fazer isso e nenhuma outra fonte na internet foi util. não sei pesquisar no google?

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


    // salva o arquivo final
    fwrite(compressed_bits, sizeof(char), current_byte, output_file);
    // TODO!!!!:
    // salvar tabela de contagens!!!
    // não da pra fazer a decodificação sem isso!!!

    free(compressed_bits);
    fclose(output_file);

    // printf("%ld -> %ld bytes salvos em \"%s\". %.2f%% de diferença.\n", size, current_byte, output_file_name, (float) current_byte / size * 100);
    // printf("%ld bytes salvos em %s\n", current_byte, output_file_name);


    return 0;
}

