#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

char *strsep(char **stringp, const char *delim) {
    char *rv = *stringp;
    if (rv) {
        *stringp += strcspn(*stringp, delim);
        if (**stringp)
            *(*stringp)++ = '\0';
        else
            *stringp = 0;
    }
    return rv;
}


#define TAMANHO 1000

typedef struct Lista {
    char *palavra;
    int ocorrencias;
    struct Lista *proximo;
} Lista;

typedef struct Arvore {
    char *palavra;
    int ocorrencias;
    struct Arvore *esquerda;
    struct Arvore *direita;
} Arvore;

void inserePalavraLista(Lista **lista, char *novaPalavra) {
    Lista *atual = *lista;
    while (atual != NULL) {
        if (strcmp(atual->palavra, novaPalavra) == 0) {
            atual->ocorrencias++;
            return;
        }
        atual = atual->proximo;
    }

    // Se a palavra não existir na lista, insere uma nova entrada
    Lista *novoLista = (Lista *)malloc(sizeof(Lista));
    novoLista->palavra = strdup(novaPalavra);
    novoLista->ocorrencias = 1;
    novoLista->proximo = *lista;
    *lista = novoLista;
}

void converteParaMinusculas(char *palavra) {
    while (*palavra) {
        *palavra = tolower((unsigned char)*palavra);
        palavra++;
    }
}

void removeCaracteres(char *palavra) {
    // Remove vírgulas, hifens e quebras de linha da palavra
    char *p = palavra;
    char *q = palavra;

    while (*q) {
        if (*q == ',' || *q == '-' || *q == '\n' || *q == '.' || *q == '/') {
            // Substitui o caractere indesejado por um espaço
            *p++ = ' ';
        } else {
            *p++ = *q;
        }
        q++;
    }

    *p = '\0';
}

void inserePalavraFormatadaLista(Lista **lista, char *novaPalavra) {
    converteParaMinusculas(novaPalavra);
    removeCaracteres(novaPalavra);

    char *palavra = strtok(novaPalavra, " ");

    while (palavra != NULL) {
        inserePalavraLista(lista, palavra);
        palavra = strtok(NULL, " ");
    }
}

int buscaPalavraLista(FILE *in, Lista *lista, char *palavra) {
    clock_t inicio = clock();

    int contador_linha = 0;
    int ocorre = 0;
    char *linha;
    char *linha_comparacao;

    linha = (char *)malloc((TAMANHO + 1) * sizeof(char));
    linha_comparacao = (char *)malloc((TAMANHO + 1) * sizeof(char));

    rewind(in);

    Lista *atual = lista;

    while (atual != NULL) {
        if (strcmp(atual->palavra, palavra) == 0) {
            ocorre = 1;
            printf("Existem %d ocorrências da palavra '%s' na(s) seguinte(s) linha(s):\n", atual->ocorrencias, palavra);

            while (fgets(linha, TAMANHO, in)) {
                if (strchr(linha, '\n')) *strchr(linha, '\n') = 0;

                linha_comparacao = strdup(linha);
                converteParaMinusculas(linha_comparacao);

                if (strstr(linha_comparacao, palavra)) {
                    printf("linha %05d: '%s'\n", contador_linha + 1, linha);
                }

                contador_linha++;
            }

            free(linha);
            break;
        }

        atual = atual->proximo;
    }

    if (ocorre == 0) {
        printf("Palavra '%s' nao encontrada\n", palavra);
    }

    clock_t fim = clock();
    double tempoTotal = ((double)(fim - inicio)) / CLOCKS_PER_SEC * 1000.0;
    printf("Tempo de busca: %.2f ms\n", tempoTotal);

    return 0;
}


Arvore* criarNo(char *novaPalavra) {
    Arvore *novoNo = (Arvore *)malloc(sizeof(Arvore));
    novoNo->palavra = strdup(novaPalavra);
    novoNo->ocorrencias = 1;
    novoNo->esquerda = NULL;
    novoNo->direita = NULL;
    return novoNo;
}

Arvore* inserirPalavraArvore(Arvore *raiz, char *novaPalavra) {
    if (raiz == NULL) {
        return criarNo(novaPalavra);
    }

    int comparacao = strcmp(novaPalavra, raiz->palavra);

    if (comparacao < 0) {
        raiz->esquerda = inserirPalavraArvore(raiz->esquerda, novaPalavra);
    } else if (comparacao > 0) {
        raiz->direita = inserirPalavraArvore(raiz->direita, novaPalavra);
    } else {
        // Palavra já existe na árvore, incrementa o contador de ocorrências
        raiz->ocorrencias++;
    }

    return raiz;
}

Arvore* inserePalavraFormatadaArvore(Arvore *raiz, char *novaPalavra) {
    converteParaMinusculas(novaPalavra);
    removeCaracteres(novaPalavra);

    char *palavra = strtok(novaPalavra, " ");

    while (palavra != NULL) {
        raiz = inserirPalavraArvore(raiz, palavra);
        palavra = strtok(NULL, " ");
    }

    return raiz;
}

void buscaPalavraArvoreRecursiva(Arvore *raiz, char *palavra, FILE *in) {
    if (raiz == NULL) {
        printf("Palavra '%s' nao encontrada\n", palavra);
        return;
    }

    int comparacao = strcmp(palavra, raiz->palavra);

    if (comparacao == 0 && strcmp(raiz->palavra, palavra) == 0) {
        printf("Existem %d ocorrências da palavra '%s' na(s) seguinte(s) linha(s):\n", raiz->ocorrencias, palavra);

        rewind(in);

        int contador_linha = 0;
        char *linha;
        char *linha_comparacao;

        linha = (char *)malloc((TAMANHO + 1) * sizeof(char));
        linha_comparacao = (char *)malloc((TAMANHO + 1) * sizeof(char));

        while (fgets(linha, TAMANHO, in)) {
            if (strchr(linha, '\n')) *strchr(linha, '\n') = 0;

            linha_comparacao = strdup(linha);
            converteParaMinusculas(linha_comparacao);

            if (strstr(linha_comparacao, palavra)) {
                printf("linha %05d: '%s'\n", contador_linha + 1, linha);
            }

            contador_linha++;
        }

        free(linha);
    } else if (comparacao < 0) {
        buscaPalavraArvoreRecursiva(raiz->esquerda, palavra, in);
    } else {
        buscaPalavraArvoreRecursiva(raiz->direita, palavra, in);
    }
}

void buscaPalavraArvore(FILE *in, Arvore *raiz, char *palavra) {
    clock_t inicio = clock();

    converteParaMinusculas(palavra);
    removeCaracteres(palavra);

    buscaPalavraArvoreRecursiva(raiz, palavra, in);

    clock_t fim = clock();
    double tempoTotal = ((double)(fim - inicio)) / CLOCKS_PER_SEC * 1000.0;
    printf("Tempo de busca: %.2f ms\n", tempoTotal);
}

int main(int argc, char ** argv){

	FILE * in;
	char * linha;
	char * copia_ponteiro_linha;
	char * copia_ponteiro_palavra;
	char * quebra_de_linha;
	char * palavra;
	int contador_linha;

    char * tipo_indice;
    char * nome_arquivo;

    char * palavra_a_ser_pesquisada;
    char * comando;

    Lista *lista_palavras = NULL;
    Arvore *arvore_palavras = NULL;

	if(argc == 3) {
        clock_t inicio = clock();
        tipo_indice = argv[2];
        nome_arquivo = argv[1];

		in = fopen(argv[1], "r");

		contador_linha = 0;
 		linha = (char *) malloc((TAMANHO + 1) * sizeof(char));

		while(in && fgets(linha, TAMANHO, in)){

			if( (quebra_de_linha = strrchr(linha, '\n')) ) *quebra_de_linha = 0;

			copia_ponteiro_linha = linha;

			while( (palavra = strsep(&copia_ponteiro_linha, " ")) ) {
                copia_ponteiro_palavra = palavra;

                if (strcmp(tipo_indice, "lista") == 0) {
                    inserePalavraFormatadaLista(&lista_palavras, copia_ponteiro_palavra);
			    }

                if (strcmp(tipo_indice, "arvore") == 0) {
                    arvore_palavras = inserePalavraFormatadaArvore(arvore_palavras, palavra);
			    }
            }

			contador_linha++;
		}

        clock_t fim = clock();
        double tempoTotal = ((double)(fim - inicio)) / CLOCKS_PER_SEC * 1000.0;

        printf("Tipo de indice: %s \n", tipo_indice);
        printf("Arquivo texto: %s \n", nome_arquivo);
        printf("Numero de linhas no arquivo: %d \n", contador_linha);
		printf("Tempo para carregar o arquivo e construir o indice: %.2f ms\n", tempoTotal);

        palavra_a_ser_pesquisada = (char *) malloc (sizeof(char));
        comando = (char *) malloc (sizeof(char));
        printf("> ");
        fgets(comando, 100, stdin);
        comando[strcspn(comando, "\n")] = '\0';



        while (strcmp(comando, "fim") != 0)
        {
            if (strncmp(comando, "busca", 5) == 0) {
                if (sscanf(comando, "busca %s", palavra_a_ser_pesquisada) == 1) {
                    converteParaMinusculas(palavra_a_ser_pesquisada);
                    if (strcmp(tipo_indice, "lista") == 0) {
                        buscaPalavraLista(in, lista_palavras, palavra_a_ser_pesquisada);
                    }

                    if (strcmp(tipo_indice, "arvore") == 0) {
                        buscaPalavraArvore(in, arvore_palavras, palavra_a_ser_pesquisada);
                    }
                }
            } else {
                printf("Opcao invalida!\n");
            }

            printf("> ");
            fgets(comando, 50, stdin);
            comando[strcspn(comando, "\n")] = '\0';
        }
	}

	return 1;
}
