#include <string.h>
#include "cpu.h"


static int eh_constante(unsigned idx)
{
    return idx >= REG_ZERO && idx <= REG_SMASK;
}

void cpu_init(cpu_estado *cpu)
{
    memset(cpu, 0, sizeof *cpu);

    cpu->reg[REG_ZERO]       = 0x0000;
    cpu->reg[REG_UM]         = 0x0001;
    cpu->reg[REG_MENOS_UM]   = 0xFFFF;
    cpu->reg[REG_AMASK]      = 0x0FFF;
    cpu->reg[REG_SMASK]      = 0x00FF;

    cpu->endereco_fim = 0xFF;   
}

/* Posições contadas do bit mais significativo (bit 31 = índice 0). */
microinstrucao cpu_decodificar(uint32_t p)
{
    microinstrucao m;

    m.raw      = p;
    m.amux     = (p >> 31) & 0x1;
    m.cond     = (p >> 29) & 0x3;
    m.ula      = (p >> 27) & 0x3;
    m.desl     = (p >> 25) & 0x3;
    m.load_mbr = (p >> 24) & 0x1;
    m.load_mar = (p >> 23) & 0x1;
    m.rd       = (p >> 22) & 0x1;
    m.wr       = (p >> 21) & 0x1;
    m.enc      = (p >> 20) & 0x1;
    m.bar_c    = (p >> 16) & 0xF;
    m.bar_b    = (p >> 12) & 0xF;
    m.bar_a    = (p >>  8) & 0xF;
    m.addr     = (p >>  0) & 0xFF;

    return m;
}

int cpu_carregar_cs(cpu_estado *cpu, const uint32_t *palavras, size_t n)
{
    if (n > CS_TAM)
        return -1;

    memcpy(cpu->cs, palavras, n * sizeof cpu->cs[0]);
    cpu->cs_tamanho = n;
    return 0;
}

int cpu_carregar_memoria(cpu_estado *cpu, uint16_t endereco,
                         const uint16_t *dados, size_t n)
{
    if (endereco >= MEM_TAM || (size_t)endereco + n > MEM_TAM)
        return -1;

    memcpy(&cpu->mem[endereco], dados, n * sizeof cpu->mem[0]);
    return 0;
}

int cpu_step(cpu_estado *cpu)
{
    if (cpu->halted)
        return 0;

    if ((size_t)cpu->mpc >= cpu->cs_tamanho)
        return -1;

    cpu->mir = cpu_decodificar(cpu->cs[cpu->mpc]);
    const microinstrucao *mi = &cpu->mir;

    /* 1. Latches A e B recebem os registradores escolhidos por Bar A e Bar B. */
    uint16_t latch_a = cpu->reg[mi->bar_a];
    uint16_t latch_b = cpu->reg[mi->bar_b];

    /* 2. AMUX escolhe a entrada A da ULA: latch A ou MBR. */
    uint16_t entrada_a = mi->amux ? cpu->mbr : latch_a;

    /* 3. ULA. N e Z são guardados para o cálculo do próximo endereço. */
    uint16_t r;
    switch (mi->ula) {
    case ALU_SOMA:  r = (uint16_t)(entrada_a + latch_b); break;
    case ALU_AND:   r = entrada_a & latch_b;             break;
    case ALU_A:     r = entrada_a;                       break;
    default:        r = (uint16_t)~entrada_a;            break;
    }
    cpu->n = (r >> 15) & 1;
    cpu->z = (r == 0);

    /* 4. Deslocador. Valor 3 não é usado: trata como sem deslocamento. */
    uint16_t saida = r;
    if (mi->desl == SH_ESQUERDA)
        saida = (uint16_t)(r << 1);
    else if (mi->desl == SH_DIREITA)
        saida = (uint16_t)(r >> 1);

    /* 5. Escritas. Todas usam os valores lidos no início do passo. */
    if (mi->load_mar)
        cpu->mar = latch_b;
    if (mi->load_mbr)
        cpu->mbr = saida;
    if (mi->enc && !eh_constante(mi->bar_c))
        cpu->reg[mi->bar_c] = saida;

    /* 6. Memória. Simplificação: a latência real é de 6 subciclos (slides).
          RD tem prioridade sobre load_mbr, se ambos estiverem ligados. */
    uint16_t endereco = cpu->mar & 0x0FFF;
    if (mi->rd)
        cpu->mbr = cpu->mem[endereco];
    if (mi->wr)
        cpu->mem[endereco] = cpu->mbr;

    /* 7. Próximo endereço. */
    int desvia = (mi->cond == COND_SEMPRE) ||
                 (mi->cond == COND_SE_N && cpu->n) ||
                 (mi->cond == COND_SE_Z && cpu->z);

    cpu->mpc = desvia ? mi->addr : (uint8_t)((cpu->mpc + 1) & 0xFF);
    cpu->ciclos++;

    if (cpu->mpc == cpu->endereco_fim)
        cpu->halted = 1;

    return 0;
}

int cpu_run(cpu_estado *cpu, unsigned long max_passos)
{
    for (unsigned long i = 0; i < max_passos && !cpu->halted; i++) {
        if (cpu_step(cpu) != 0)
            return -1;
    }
    return cpu->halted ? 0 : 1;
}