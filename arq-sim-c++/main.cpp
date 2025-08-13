#include <iostream>
#include <string>
#include "SimuladorCPU.h"

int main(int argc, char *argv[]) {

    if (argc != 2) {
        std::cerr << "Erro: Uso incorreto." << std::endl;
        std::cerr << "Sintaxe: " << argv[0] << " <caminho_do_arquivo.bin>" << std::endl;
        return 1;
    }

    std::string arquivo_binario = argv[1];

    SimuladorCPU cpu;
    if (!cpu.carregar_programa(arquivo_binario)) {
        std::cerr << "Falha ao carregar o programa: " << arquivo_binario << std::endl;
        return 1;
    }

    std::cout << "--- Iniciando a simulacao do processador ---" << std::endl;
    cpu.executar();
    std::cout << "--- Simulacao finalizada ---" << std::endl;

    return 0;
}