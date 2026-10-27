#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// instrução poderá ser lida de um arquivo.
typedef struct conjuntoInstrucoes{
    char mnemonico[5];
    int opcode;
}instrucao;

// guarda o nome da label e o endereço de memória onde ela ta armazenada.
typedef struct Simbolo{
    char label[20];
    int endereco_memoria;
}simbolo;


int main(){
    
    instrucao listaInstrucao[] = 

    {
        {"LODD", 0}, {"STOD", 1,}, {"ADDD", 2},
        {"SUBD", 3}, {"JPOS", 4}, {"JZER", 5}, {"JUMP", 6},
        {"LOCO", 7}, {"LODL", 8}, {"STOL", 9}, {"ADDL", 10},
        {"SUBL", 11}, {"JNEG", 12}, {"JNZE", 13}, {"CALL", 14},
        {"PSHI", 120}, {"POPI", 121}, {"PUSH", 122}, {"POP", 123},
        {"RETN", 124}, {"SWAP", 125},{"INSP", 126}, {"DESP", 127}
    };

    //variavel pc (program couter): contará as linhas de códigos válidas
    int pc = 0;
    int totalSimbolo = 0;
    simbolo vetor[50];
    
    FILE *arquivo = fopen("arquivo-de-instrucoes.txt", "r");
    if(arquivo == NULL){
        printf("Erro ao abrir o arquivo-de-instrucoes");
        return 1;
    }

    char linhaInstrucao[50];

    // laço para leitura do arquivo de instruções. Esse primeiro laço servirá para lermos as instruções e verificarmos suas labels. 
    while(fgets(linhaInstrucao, sizeof(linhaInstrucao), arquivo) != NULL){
        char *token = strtok(linhaInstrucao, " \t\n");
        
        if(token != NULL){
            int tamPalavra = strlen(token);

            //verifica se possui uma label na instrução. se tiver, guarda ela no vetor para ser acessada futuramente.
            if(token[tamPalavra-1] == ':'){

                // substitui o : por \0
                token[tamPalavra-1] = '\0';

                //salva a label e o endereço de memória 
                strcpy(vetor[totalSimbolo].label, token); 
                vetor[totalSimbolo].endereco_memoria = pc;
                totalSimbolo++;

                // verifica se ainda tem instruções na >MESMA< linha
                token = strtok(NULL, " \t\n");

                // se encontrou, incrementa o pc
                if (token != NULL) {
                pc++;
                }

            }else{
                pc++;
            }
        }
    }
    rewind(arquivo);

    fclose(arquivo);
    return 0;
}