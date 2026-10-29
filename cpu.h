#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include <stddef.h>

#define CS_TAM 256
#define MEM_TAM 4096
#define NUM_REGS 16

typedef enum {
    REG_PC = 0,
    REG_AC,
    REG_SP,
    REG_IR,
    REG_TIR,
    REG_U,
    REG_UM,        
    REG_MENOS_UM,  
    REG_AMASK,     
    REG_SMASK,     
    REG_A,
    REG_B,
    REG_C,
    REG_D,
    REG_E,
    REG_F
} registrador_id;

enum { COND_NAO_DESVIA = 0, COND_SE_N = 1, COND_SE_Z = 2, COND_SEMPRE = 3 };
enum { ALU_SOMA = 0, ALU_AND = 1, ALU_A = 2, ALU_NOT_A = 3 };
enum { SH_NENHUM = 0, SH_ESQUERDA = 1, SH_DIREITA = 2 };

typedef struct {
    uint32_t raw;        /* palavra original, para exibição          */
    uint8_t  amux;       /* 0 = latch A, 1 = MBR                     */
    uint8_t  cond;       /* COND_*                                   */
    uint8_t  ula;        /* ALU_*                                    */
    uint8_t  desl;       /* SH_*                                     */
    uint8_t  load_mbr;   /* 1 = MBR recebe a saída do deslocador     */
    uint8_t  load_mar;   /* 1 = MAR recebe o barramento B            */
    uint8_t  rd;         /* 1 = lê memória em MAR para MBR           */
    uint8_t  wr;         /* 1 = escreve MBR na memória em MAR        */
    uint8_t  enc;        /* 1 = escreve a saída em Bar C             */
    uint8_t  bar_c;      /* 4 bits                                   */
    uint8_t  bar_b;      /* 4 bits                                   */
    uint8_t  bar_a;      /* 4 bits                                   */
    uint8_t  addr;       /* próximo endereço (ADDR)                  */
} microinstrucao;

typedef struct {
    uint16_t reg[NUM_REGS];          /* PC, AC, SP, IR*/
    uint8_t  mpc;                    /* endereço da microinstrução atual        */
    microinstrucao mir;              /* microinstrução atual                    */
    uint16_t mar;                    /* 16 bits, só os 12 de baixo são usados   */
    uint16_t mbr;                    /* 16 bits                                 */
    int      n;                      /* flag: resultado da ALU negativo         */
    int      z;                      /* flag: resultado da ALU zero             */
    int      halted;                 /* 1 quando o programa terminou            */
    unsigned long ciclos;            /* microinstruções executadas              */
    uint8_t  endereco_fim;           /* endereço de microinstrução de término   */
    size_t   cs_tamanho;             /* quantas palavras da control store estão carregadas */
    uint32_t cs[CS_TAM];             /* control store                           */
    uint16_t mem[MEM_TAM];           /* memória principal                       */
} cpu_estado;

/* Funções públicas                                                    */

 
/* Zera o estado e define as constantes dos barramentos. */
void cpu_init(cpu_estado *cpu);
 
/* Decodifica uma palavra de 32 bits em microinstrucao. */
microinstrucao cpu_decodificar(uint32_t palavra);
 
/* Carrega n palavras de microinstrução a partir do endereço 0.
   Retorna 0 em sucesso, -1 se n > CS_TAM. */
int cpu_carregar_cs(cpu_estado *cpu, const uint32_t *palavras, size_t n);
 
/* Copia n palavras de 16 bits para a memória principal a partir de endereco.
   Retorna 0 em sucesso, -1 se estourar a memória. */
int cpu_carregar_memoria(cpu_estado *cpu, uint16_t endereco,
                         const uint16_t *dados, size_t n);
 
/* Executa uma microinstrução. Retorna 0 em sucesso, -1 se a control store
   não tiver a microinstrução do MPC atual. */
int cpu_step(cpu_estado *cpu);
 
/* Executa até halted ou até max_passos.
   Retorna 0 se terminou, 1 se atingiu max_passos, -1 em erro. */
int cpu_run(cpu_estado *cpu, unsigned long max_passos);


#endif CPU_H