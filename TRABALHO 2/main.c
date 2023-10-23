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
    int keys[5 - 1];
    struct BTreeNode* children[5];
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
        newChild->keys[i] = child->keys[i + ORDEM / 2];
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
        parent->keys[i] = parent->keys[i - 1];
    }

    parent->keys[index] = child->keys[ORDEM / 2 - 1];
}

// Função para inserir uma chave em um nó não cheio
void insertNonFull(BTreeNode *node, int key) {
    int i = node->num_keys - 1;

    if (node->leaf) {
        // Nó é uma folha, insira a chave no lugar correto para manter a ordenação
        while (i >= 0 && key < node->keys[i]) {
            node->keys[i + 1] = node->keys[i];
            i--;
        }
        node->keys[i + 1] = key;
        node->num_keys++;
    } else {
        // Encontre o filho apropriado para continuar a inserção
        while (i >= 0 && key < node->keys[i]) {
            i--;
        }

        i++;
        if (node->children[i]->num_keys == ORDEM - 1) {
            // Nó filho está cheio, faça a divisão
            splitChild(node, i);

            // Determine qual dos dois filhos recém-criados deve conter a nova chave
            if (key > node->keys[i]) {
                i++;
            }
        }

        insertNonFull(node->children[i], key);
    }
}

// Função para adicionar um valor em um nó da árvore B
void op_add(BTreeNode **node, int key) {
    if ((*node)->num_keys == ORDEM - 1) {
        // A raiz está cheia, crie um novo nó raiz
        BTreeNode *newRoot = createNode();
        newRoot->leaf = 0;
        newRoot->children[0] = *node;
        *node = newRoot;
        splitChild(*node, 0);
    }

    insertNonFull(*node, key);
}

// Função para criar uma árvore B a partir de um arquivo de entrada
void createBTree(char *arquivo, BTreeNode **root) {
    FILE *chaves;
    chaves = fopen(arquivo, "r");

    if (chaves == NULL) {
        // O arquivo não existe, então crie um novo arquivo e adicione valores
        chaves = fopen(arquivo, "w");
        if (chaves == NULL) {
            printf("Erro ao criar o arquivo de entrada.\n");
            return;
        }
        printf("Arquivo de entrada criado com sucesso.\n");
        fclose(chaves);
    } else {
        char buffer_chave[255];
        int valChave;
        // Ler todas as chaves do arquivo existente
        while (fscanf(chaves, " %s", buffer_chave) == 1) {
            valChave = atoi(buffer_chave);
            printf("Importar: %d\n", valChave);
            op_add(root, valChave);
        }
        printf("Arquivo de entrada importado com sucesso.\n");
        fclose(chaves);
    }
}


// Função para realizar uma busca na árvore B
void searchBTree(BTreeNode *node, int key) {
    int i = 0;
    while (i < node->num_keys && key > node->keys[i]) {
        i++;
    }

    if (i < node->num_keys && key == node->keys[i]) {
        printf("Chave %d encontrada na árvore B.\n", key);
        return;
    } else if (node->leaf) {
        printf("Chave %d não encontrada na árvore B.\n", key);
        return;
    } else {
        // Recursivamente continue a busca nos filhos
        searchBTree(node->children[i], key);
    }
}

// Função para executar as operações de um arquivo de entrada
void execBTree(BTreeNode *root, char *arquivo) {
    FILE *operacoes;
    operacoes = fopen(arquivo, "r");
    printf("Executando operacoes do arquivo %s\n", arquivo);
    if (operacoes == NULL) {
        printf("Erro ao abrir o arquivo de operacoes.\n");
        return;
    }

    char operacao;
    int chave;

    while (fscanf(operacoes, " %c %d", &operacao, &chave) == 2) {
        if (operacao == 'b' || operacao == 'B') {
            // Busca
            searchBTree(root, chave);
        } else if (operacao == 'i' || operacao == 'I') {
            // Inserção
            op_add(&root, chave);
        } else {
            printf("Operacao desconhecida: %c\n", operacao);
        }
    }

    fclose(operacoes);
}

void printBTree(BTreeNode *node) {
    if (node != NULL) {
        int i;
        for (i = 0; i < node->num_keys; i++) {
            // Imprima a chave
            printf("%d ", node->keys[i]);
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
    if (argc < 3) {
        printf("Argumentos incorretos!\n");
        exit(1);
    }
    if(strcmp(argv[1],"-c")==0){
        createBTree(argv[2], &root);
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