//João Pedro Peres Bertoncelo ra112650
//João Vitor Pereira de Bortioli ra125910
#include<stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

#define ORDEM 5

// Estrutura para um nó da árvore B
typedef struct BTreeNode {
    int int_keys[ORDEM - 1];
    char string_keys[ORDEM - 1][255];
    struct BTreeNode* children[ORDEM];
    int num_keys;
    int leaf; // 1 se for folha, 0 se não for
} BTreeNode;

// Função para criar um novo nó da árvore B
BTreeNode* createNode() {
    BTreeNode* newNode = (BTreeNode*)malloc(sizeof(BTreeNode));
    if (newNode != NULL) {
        int i;
        for (i = 0; i < ORDEM; i++) {
            newNode->children[i] = NULL;
        }
        newNode->num_keys = 0;
        newNode->leaf = 1; // Inicialmente, o novo nó é uma folha
    }
    return newNode;
}

// Função para liberar a memória da árvore B recursivamente, começando pelo nó raiz
void freeBTree(BTreeNode *node) {
    if (node != NULL) {
        for (int i = 0; i < ORDEM; i++) {
            freeBTree(node->children[i]);
        }
        free(node);
    }
}

// Função para encerrar a árvore B, liberando toda a memória
void end(BTreeNode *root) {
    freeBTree(root); // Libera toda a árvore começando pelo nó raiz
}

// Função para dividir um nó pai quando um filho está cheio
void splitChild(BTreeNode *parent, int index) {
    BTreeNode *child = parent->children[index];
    BTreeNode *newChild = createNode();

    // Mova metade das chaves e filhos para o novo filho
    newChild->num_keys = ORDEM / 2 - 1;
    for (int i = 0; i < ORDEM / 2 - 1; i++) {
        newChild->int_keys[i] = child->int_keys[i + ORDEM / 2];
        strcpy(newChild->string_keys[i], child->string_keys[i + ORDEM / 2]);
    }

    if (!child->leaf) {
        for (int i = 0; i < ORDEM / 2; i++) {
            newChild->children[i] = child->children[i + ORDEM / 2];
        }
    }

    child->num_keys = ORDEM / 2 - 1;

    // Abre espaço para o novo filho no nó pai
    for (int i = parent->num_keys; i > index; i--) {
        parent->children[i + 1] = parent->children[i];
    }

    parent->children[index + 1] = newChild;
    parent->num_keys++;

    // Move a chave do meio do filho para o pai
    for (int i = parent->num_keys - 1; i > index; i--) {
        parent->int_keys[i] = parent->int_keys[i - 1];
        strcpy(parent->string_keys[i], parent->string_keys[i - 1]);
    }

    parent->int_keys[index] = child->int_keys[ORDEM / 2 - 1];
    strcpy(parent->string_keys[index], child->string_keys[ORDEM / 2 - 1]);
}

// Função para inserir uma chave em um nó não cheio
void insertNonFull(BTreeNode *node, int int_key, const char *string_key) {
    int i = node->num_keys - 1;

    if (node->leaf) {
        // Nó é uma folha, insira a chave no lugar correto para manter a ordenação
        while (i >= 0 && int_key < node->int_keys[i]) {
            node->int_keys[i + 1] = node->int_keys[i];
            strcpy(node->string_keys[i + 1], node->string_keys[i]);
            i--;
        }
        node->int_keys[i + 1] = int_key;
        strcpy(node->string_keys[i + 1], string_key);
        node->num_keys++;
    } else {
        // Encontre o filho apropriado para continuar a inserção
        while (i >= 0 && int_key < node->int_keys[i]) {
            i--;
        }

        i++;
        if (node->children[i]->num_keys == ORDEM - 1) {
            splitChild(node, i);
            if (int_key > node->int_keys[i]) {
                i++;
            }
        }

        insertNonFull(node->children[i], int_key, string_key);
    }
}

// Função para inserir valores na árvore B
void op_add(BTreeNode **node, int int_key, const char *string_key) {
    if ((*node)->num_keys == ORDEM - 1) {
        // A raiz está cheia, crie um novo nó raiz
        BTreeNode *newRoot = createNode();
        newRoot->leaf = 0;
        newRoot->children[0] = *node;
        *node = newRoot;
        splitChild(*node, 0);
    }
    insertNonFull(*node, int_key, string_key);
}

// Função para escrever os dados da árvore B em um arquivo
void writeBTreeToFile(FILE *file, BTreeNode *node) {
    if (node != NULL) {
        // Escrever os dados do nó no arquivo
        fwrite(node->int_keys, sizeof(int), node->num_keys, file);
        for (int i = 0; i < node->num_keys; i++) {
            fwrite(node->string_keys[i], sizeof(char), 255, file);
        }

        if (!node->leaf) {
            // Recursivamente escrever os filhos no arquivo
            for (int i = 0; i <= node->num_keys; i++) {
                writeBTreeToFile(file, node->children[i]);
            }
        }
    }
}


void insertValuesFromFile(BTreeNode **root, const char *filename) {
    FILE *file = fopen(filename, "r");

    if (file == NULL) {
        printf("Erro ao abrir o arquivo %s.\n", filename);
        return;
    }
    printf("Sucesso ao abrir o arquivo %s.\n", filename);

    int int_key;
    char string_key[255];
    // Ler todas as chaves do arquivo existente
    while (fread(&int_key, sizeof(int_key), 1, file) == 1) {
        fread(string_key, sizeof(string_key[0]), sizeof(string_key), file);
        printf("Importar: %d %s\n", int_key, string_key);
        op_add(root, int_key, string_key);
    }
    // Abrir o arquivo "BTree.dat" para escrever (ou criar um novo arquivo)
    FILE *btree_file = fopen("BTree.dat", "wb");
    if (btree_file == NULL) {
        printf("Erro ao abrir o arquivo BTree.dat.\n");
        return;
    }

    // Escrever os dados da árvore B no arquivo
    writeBTreeToFile(btree_file, *root);

    // Fechar o arquivo
    fclose(btree_file);

    printf("Arquivo BTree.dat atualizado com sucesso.\n");
    fclose(file);
}


// Função para criar uma árvore B a partir de um arquivo de entrada
void createBTree(const char *arquivo, BTreeNode **root) {
    FILE *dados;
    dados = fopen(arquivo, "rb");

    if (dados == NULL) {
        printf("Erro ao abrir o arquivo de entrada.\n");
        return;
    }

    int int_key;
    char string_key[255];

    // Ler todas as chaves do arquivo existente
    while (fread(&int_key, sizeof(int), 1, dados) == 1) {
        fread(string_key, sizeof(char), 255, dados);
        op_add(root, int_key, string_key);
    }

    printf("Arquivo de entrada importado com sucesso.\n");
    fclose(dados);
}


// Função para realizar uma busca na árvore B
void searchBTree(BTreeNode *node, int int_key, const char *string_key) {
    int i = 0;
    while (i < node->num_keys && (int_key > node->int_keys[i] || (int_key == node->int_keys[i] && strcmp(string_key, node->string_keys[i]) > 0))) {
        i++;
    }

    if (i < node->num_keys && int_key == node->int_keys[i] && strcmp(string_key, node->string_keys[i]) == 0) {
        printf("Chave %d - %s encontrada na árvore B.\n", int_key, string_key);
        return;
    } else if (node->leaf) {
        printf("Chave %d - %s nao encontrada na árvore B.\n", int_key, string_key);
        return;
    } else {
        // Recursivamente continue a busca nos filhos
        searchBTree(node->children[i], int_key, string_key);
    }
}

// Função para realizar as operações de um arquivo de entrada
void execBTree(BTreeNode *root, const char *arquivo) {
    FILE *operacoes;
    operacoes = fopen(arquivo, "r+");
    printf("Executando operacoes do arquivo %s\n", arquivo);
    if (operacoes == NULL) {
        printf("Erro ao abrir o arquivo de operacoes.\n");
        return;
    }
    int int_key;
    char string_key[255];

    char line[256];  // Suponha que as linhas têm até 255 caracteres (ajuste conforme necessário)

    while (fgets(line, sizeof(line), operacoes) != NULL) {
        char operacao = line[0];  // Obtém o primeiro caractere da linha

        if (operacao == 'b' || operacao == 'B') {
            if (sscanf(line + 1, " %d", &int_key) == 1) {
                // Busca
                searchBTree(root, int_key, string_key);
            } else {
                printf("Formato inválido para busca na linha: %s", line);
            }
        } else if (operacao == 'i' || operacao == 'I') {
            if (sscanf(line + 1, " %d %s", &int_key, string_key) == 2) {
                // Inserção
                op_add(&root, int_key, string_key);
            } else {
                printf("Formato inválido para inserção na linha: %s", line);
            }
        } else {
            printf("Operacao desconhecida: %c\n", operacao);
        }
    }

    fclose(operacoes);
}

// Função para imprimir a árvore B
void printBTree(BTreeNode *node) {
    if (node != NULL) {
        int i;
        for (i = 0; i < node->num_keys; i++) {
            // Imprima a chave
            printf("Nível %d: %d - %s\n", i, node->int_keys[i], node->string_keys[i]);
        }

        if (!node->leaf) {
            // Recursivamente imprima os filhos
            for (i = 0; i <= node->num_keys; i++) {
                printBTree(node->children[i]);
            }
        }
    }
}


int main(int argc, char *argv[]){
    BTreeNode *root = createNode();
    root->leaf = 1;
    root->num_keys = 0;
    if (argc < 2) {
        printf("Argumentos incorretos!\n");
        exit(1);
    }
    if(strcmp(argv[1],"-c")==0){
        createBTree(argv[2], &root);
        insertValuesFromFile(&root, "dados.dat");
    }else if(strcmp(argv[1],"-e")==0){
        execBTree(root, argv[2]); // Chama execBTree com o nó raiz e o arquivo de operações
    }else if(strcmp(argv[1],"-p")==0){
        printBTree(root);
    }else{
        printf("Argumentos incorretos!");
        exit(1);
    }
    end(root); // Libera a memória da árvore antes de encerrar o programa
    return 0;
}