#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
    long offset;
    short int size;
} LEDNode;

int input(char str[], int size) {
    int i = 0;
    char c = getchar();
    while (c != '\n') {
        if (i < size-1) {
        str[i] = c;
        i++;
        }
        c = getchar();
    }
    str[i] = '\0';
    return i;
}

void imprimirModoUso(char *nomePrograma) {
    fprintf(stderr, "Argumentos incorretos!\n");
    fprintf(stderr, "Modo de uso:\n");
    fprintf(stderr, "$ %s (-i|-e) nome_arquivo\n", nomePrograma);
    fprintf(stderr, "$ %s -p\n", nomePrograma);
    exit(EXIT_FAILURE);
}

void remocao(char *chave, char *nomeDoArquivo) {
    FILE *arquivo;

    arquivo = fopen(nomeDoArquivo, "rb+");
    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo\n");
        exit(1);
    }
    fseek(arquivo, 0, SEEK_SET);
    char buffer[256];
    char linha[256];
    char *token;

    printf("Remocao do registro de chave %s\n", chave);

    while (fgets(buffer, sizeof(buffer), arquivo) != NULL) {
        //printf("buffer atual: %s\n", buffer); // Saída de depuração
        strcpy(linha, buffer);

        token = strtok(buffer, "|");
        //printf("Token atual: %s\n", token); // Saída de depuração
        if (strcmp(token, chave) == 0) {
            printf("Registro removido! (%ld bytes)\n", strlen(linha)-1);
            printf("Local: offset = %ld bytes\n", ftell(arquivo)-strlen(linha)-2);

            fseek(arquivo, -strlen(linha)-2, SEEK_CUR);

            buffer[0] = '*';
            buffer[1] = '|';
            fwrite(buffer, sizeof(char), strlen(linha) + 1, arquivo);

            fclose(arquivo);
            return;
        }
        fseek(arquivo, 2, SEEK_CUR); // Pular para o próximo registro
    }

    printf("Erro: registro nao encontrado!\n");

    fclose(arquivo);
}


void insercao(char *conteudo, char *nomeDoArquivo) {
    FILE *arquivo;
    char chave[256];
    char *token;
    short int bytesLidos = strlen(conteudo);

    arquivo = fopen(nomeDoArquivo, "rb+");
    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo\n");
        exit(1);
    }

    strcpy(chave, conteudo);
    token = strtok(chave, "|");

    printf("Insercao do registro %s\n", token);

    fseek(arquivo, 0, SEEK_END);
    fwrite("\r\n", 1, 2, arquivo); // Escreve \r\n
    fwrite(&bytesLidos, sizeof(bytesLidos), 1, arquivo); // Escreve o tamanho do registro
    fwrite(conteudo, sizeof(char), strlen(conteudo), arquivo); // Escreve o conteúdo do registro

    printf("Local: offset = final do arquivo\n");

    fclose(arquivo);
}


void buscar(char *chave, char *nomeDoArquivo) {
    FILE *arquivo;

    if ((arquivo = fopen(nomeDoArquivo, "rb")) == NULL) {
        perror("Erro ao abrir o arquivo");
        exit(EXIT_FAILURE);
    }

    char linha[256];
    char *token;

    printf("Realizando busca pelo registro de chave %s\n", chave);

    while (fgets(linha, sizeof(linha), arquivo) != NULL) {
        char temp[256];
        strcpy(temp, linha);

        fseek(arquivo, sizeof(short int), SEEK_CUR); // Pular o campo de tamanho

        token = strtok(temp, "|");

        if (strcmp(token, chave) == 0) {
            printf("Registro encontrado:\n%s", linha);
            fclose(arquivo);
            return;
        }
    }

    printf("Erro: chave nao encontrada\n");

    fclose(arquivo);
}

void importar(char *nomeDoArquivo) {
    FILE *arquivo, *dadosEmMemoria;
    char nomeArquivoDados[] = "dados.dat";
    char linha[256];
    short int bytesLidos;

    arquivo = fopen(nomeDoArquivo, "r");
    dadosEmMemoria = fopen(nomeArquivoDados, "wb");

    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo\n");
        exit(1);
    }

    while (fgets(linha, sizeof(linha), arquivo) != NULL) {
        linha[strcspn(linha, "\r\n")] = '\0'; // Remover caracteres de nova linha
        bytesLidos = strlen(linha) + sizeof(short int);

        fwrite(&bytesLidos, sizeof(short int), 1, dadosEmMemoria);
        fwrite(linha, sizeof(char), strlen(linha), dadosEmMemoria);
    }

    fclose(arquivo);
    fclose(dadosEmMemoria);

    printf("Arquivo importado com sucesso\n");
}


void executar_operacoes(char *nomeDoArquivo) {
    FILE *arquivoDeOperacoes;
    char nomeArquivoOperacoes[256];
    char linha[256];
    char *operacao;
    char *chave;

    printf("Por favor, insira o nome do arquivo de operacoes: ");
    input(nomeArquivoOperacoes, sizeof(nomeArquivoOperacoes));
    nomeArquivoOperacoes[strcspn(nomeArquivoOperacoes, "\r\n")] = '\0';
    
    arquivoDeOperacoes = fopen(nomeArquivoOperacoes, "rb");

    if (arquivoDeOperacoes == NULL) {
        printf("Erro ao abrir o arquivo de operacoes\n");
        exit(1);
    }

    while (fgets(linha, sizeof(linha), arquivoDeOperacoes)) {
        linha[strcspn(linha, "\r\n")] = '\0'; // Remover caracteres de nova linha

        operacao = strtok(linha, " ");
        chave = strtok(NULL, "\r");

        if (strcmp(operacao, "i") == 0) {
            insercao(chave, nomeDoArquivo);
        } else if (strcmp(operacao, "b") == 0) {
            buscar(chave, nomeDoArquivo);
        } else if (strcmp(operacao, "r") == 0) {
            remocao(chave, nomeDoArquivo);
        } else {
            printf("Operacao nao reconhecida\n");
        }
    }

    fclose(arquivoDeOperacoes);
}

//imprime os offsets excluídos
void imprimir_led() {
    FILE *arquivo;
    char buffer[256];
    long int byteOffset = 0;

    arquivo = fopen("dados.dat", "rb");

    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo\n");
        exit(1);
    }

    while (fread(buffer, sizeof(buffer), 1, arquivo) == 1) {
        if (buffer[2] == '*') {
            printf("Offset em bytes: %ld\n", byteOffset);
        }
        byteOffset += sizeof(buffer);
    }

    fseek(arquivo, byteOffset, SEEK_SET);
    fclose(arquivo);
}

int main(int argc, char *argv[]) {
    
    LEDNode headLED;  // Cabeça da LED
    int numLEDNodes;   // Número de nós na LED

    if (argc < 2) {
        imprimirModoUso(argv[0]);
    }

    if (strcmp(argv[1], "-p") == 0) {
        printf("Modo de impressao da LED ativado ...\n");
        imprimir_led();
    } else if (argc == 3) {
        if (strcmp(argv[1], "-i") == 0) {
            printf("Modo de importacao ativado ... nome do arquivo = %s\n", argv[2]);
            importar(argv[2]);
        } else if (strcmp(argv[1], "-e") == 0) {
            printf("Modo de execucao de operacoes ativado ... nome do arquivo = %s\n", argv[2]);
            executar_operacoes(argv[2]);
        } else {
            imprimirModoUso(argv[0]);
        }
    } else {
        imprimirModoUso(argv[0]);
    }

    return 0;
}
