#include "SimuladorCPU.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstdio>

SimuladorCPU::SimuladorCPU() {
    pc = 1;
    em_execucao = false;

    for (unsigned int i = 0; i < NUM_REGISTRADORES; ++i) {
        registradores[i] = 0;
    }
    for (unsigned int i = 0; i < TAMANHO_MEMORIA; ++i) {
        memoria[i] = 0;
    }
}

bool SimuladorCPU::carregar_programa(const std::string& caminho_arquivo) {
    FILE* arquivo = fopen(caminho_arquivo.c_str(), "rb");
    if (!arquivo) {
        std::cerr << "Erro: Nao foi possivel abrir o arquivo binario '" << caminho_arquivo << "'. ";
        perror(nullptr);
        return false;
    }

    const size_t lidos = fread(memoria, sizeof(uint16_t), TAMANHO_MEMORIA, arquivo);
    fclose(arquivo);

    em_execucao = (lidos > 0);
    if (!em_execucao) {
        std::cerr << "AVISO: 0 palavras lidas. Nada a executar.\n";
    } else {
        std::cerr << "[Loader] " << lidos << " palavras (16b) carregadas.\n";
    }
    return em_execucao;
}

void SimuladorCPU::executar() {
    uint64_t ciclos = 0;
    while (em_execucao) {
        imprimir_estado_debug();

        if (pc >= TAMANHO_MEMORIA) {
            std::cerr << "FALHA: PC (indice " << std::dec << pc
                      << ") aponta para fora dos limites da memoria.\n";
            em_execucao = false;
            continue;
        }

        ciclo_de_instrucao();

        ciclos++;
        if (ciclos > LIMITE_CICLOS) {
            std::cerr << "FALHA: Limite de " << std::dec << LIMITE_CICLOS
                      << " ciclos atingido. Possivel loop infinito.\n";
            em_execucao = false;
        }
    }
}

void SimuladorCPU::ciclo_de_instrucao() {
    const uint16_t instrucao_atual = memoria[pc];
    pc++;

    // Decode
    const uint16_t formato = extrair_bits(instrucao_atual, 15, 1);

    if (formato == 0) { // Formato R
        const uint16_t opcode = extrair_bits(instrucao_atual, 9, 6);
        const uint16_t rd     = extrair_bits(instrucao_atual, 6, 3);
        const uint16_t rs1    = extrair_bits(instrucao_atual, 3, 3);
        const uint16_t rs2    = extrair_bits(instrucao_atual, 0, 3);

        const uint16_t val1 = registradores[rs1];
        const uint16_t val2 = registradores[rs2];

        switch (opcode) {
            case 0:  registradores[rd] = static_cast<uint16_t>(val1 + val2); break; // add
            case 1:  registradores[rd] = static_cast<uint16_t>(val1 - val2); break; // sub
            case 2:  registradores[rd] = static_cast<uint16_t>(val1 * val2); break; // mul
            case 3:  registradores[rd] = (val2 == 0) ? 0 : static_cast<uint16_t>(val1 / val2); break; // div
            case 4:  registradores[rd] = static_cast<uint16_t>(val1 == val2); break; // cmp_equal
            case 5:  registradores[rd] = static_cast<uint16_t>(val1 != val2); break; // cmp_neq

            case 15: { // load
                const uint16_t endereco_palavra = registradores[rs1];
                if (endereco_palavra >= TAMANHO_MEMORIA) {
                    std::cerr << "FALHA: LOAD fora da memoria (addr=" << std::dec
                              << endereco_palavra << ").\n";
                    em_execucao = false; break;
                }
                registradores[rd] = memoria[endereco_palavra];
                break;
            }

            case 16: { // store
                const uint16_t endereco_palavra = registradores[rs1];
                if (endereco_palavra >= TAMANHO_MEMORIA) {
                    std::cerr << "FALHA: STORE fora da memoria (addr=" << std::dec
                              << endereco_palavra << ").\n";
                    em_execucao = false; break;
                }
                const uint16_t dado = registradores[rs2];
                memoria[endereco_palavra] = dado;
                break;
            }

            case 63: // syscall
                tratar_syscall();
                break;

            default:
                std::cerr << "AVISO: Opcode R nao implementado (" << std::dec << opcode << ").\n";
                break;
        }

    } else { // Formato I
        const uint16_t opcode          = extrair_bits(instrucao_atual, 13, 2);
        const uint16_t rd_ou_regcond   = extrair_bits(instrucao_atual, 10, 3);
        const uint16_t imediato        = extrair_bits(instrucao_atual, 0, 10); // absoluto em PALAVRAS

        switch (opcode) {
            case 0: { // jump absoluto
                if (imediato >= TAMANHO_MEMORIA) {
                    std::cerr << "FALHA: jump para alvo invalido (" << std::dec << imediato << ").\n";
                    em_execucao = false;
                } else {
                    pc = imediato;
                }
                break;
            }
            case 1: { // jump_cond
                if (registradores[rd_ou_regcond] != 0) {
                    if (imediato >= TAMANHO_MEMORIA) {
                        std::cerr << "FALHA: jump_cond para alvo invalido (" << std::dec << imediato << ").\n";
                        em_execucao = false;
                    } else {
                        pc = imediato;
                    }
                }
                break;
            }
            case 3: { // mov imediato -> registrador
                registradores[rd_ou_regcond] = imediato;
                break;
            }
            default:
                std::cerr << "AVISO: Opcode I nao implementado (" << std::dec << opcode << ").\n";
                break;
        }
    }
}

void SimuladorCPU::tratar_syscall() {
    const uint16_t servico = registradores[0];
    switch (servico) {
        case 0: // Halt
            std::cerr << "[Syscall] Finalizando a execucao (servico 0).\n";
            em_execucao = false;
            break;
        default:
            std::cerr << "AVISO: Syscall com servico nao implementado ("
                      << std::dec << servico << ").\n";
            break;
    }
}

void SimuladorCPU::imprimir_estado_debug() {
    std::stringstream ss;

    const uint16_t pc_word = pc;
    const uint16_t pc_byte = static_cast<uint16_t>(pc * 2);

    ss << "PCw=" << std::dec << pc_word
       << " PCb=0x" << std::setw(4) << std::setfill('0') << std::hex << pc_byte
       << " | ";

    for (unsigned int i = 0; i < NUM_REGISTRADORES; ++i) {
        ss << "R" << i << "=0x"
           << std::setw(4) << std::setfill('0') << std::hex << registradores[i] << " ";
    }
    ss << "\n";

    std::cerr << ss.str();
}