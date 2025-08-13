#ifndef SIMULADOR_CPU_H
#define SIMULADOR_CPU_H

#include <cstdint>
#include <string>
#include <vector>

const unsigned int TAMANHO_MEMORIA = 32768;
const unsigned int NUM_REGISTRADORES = 8;
const unsigned int LIMITE_CICLOS = 3000;

class SimuladorCPU {
public:
    SimuladorCPU();
    bool carregar_programa(const std::string& caminho_arquivo);
    void executar();

private:
    uint16_t pc;
    uint16_t registradores[NUM_REGISTRADORES];

    uint16_t memoria[TAMANHO_MEMORIA];

    bool em_execucao;

    void ciclo_de_instrucao();
    void tratar_syscall();
    void imprimir_estado_debug();

    static inline uint16_t extrair_bits(uint16_t valor, uint8_t inicio, uint8_t tamanho) {
        if (tamanho == 0) return 0;
        const uint8_t w = (tamanho >= 16) ? 16 : tamanho;
        const uint16_t mascara = (w == 16)
            ? static_cast<uint16_t>(0xFFFFu)
            : static_cast<uint16_t>((1u << w) - 1u);
        return static_cast<uint16_t>((valor >> inicio) & mascara);
    }
};

#endif