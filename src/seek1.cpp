#include <iostream>
#include <chrono>
#include <iomanip> // Para formatar a saída do tempo
#include <algorithm>

#include "../include/arv_prim.h"


int main(int argc, char** argv){
    auto inicio = std::chrono::high_resolution_clock::now(); // Pra cronometrar o tempo

    unsigned chaveBusca; // Aqui é pra guardar a chave que vai ler do terminal
    std::fstream arq_arvore("../data/arvore.bin", std::ios::binary | std::ios::in); // Abro a árvore como binario
    arq_arvore.seekg(0, std::ios::beg); // Coloco o cursor no inicio do arquivo
    arq_arvore.read(reinterpret_cast<char*>(&raizOffset), sizeof(Offset)); // Leio o primeiro

    if(argc != 2){ // Mensagem generica pra caso tenha erro
        std::cout << "Tem que ser: arg[0]: Executavel, argv[1]: Chave a ser buscada" << "\n";
        return -1;
    }

    chaveBusca = atoi(argv[1]); // Leio do terminal
    Offset offsetChave = buscarNaArvoreBPlus(arq_arvore, chaveBusca, raizOffset); // Aplico a função de busca (ta implementada no .h da árvore primaria)

    lerEImprimirRegistro(offsetChave); // Printo as informações
    arq_arvore.close(); // Fecho o arquivo

    auto fim = std::chrono::high_resolution_clock::now(); // Pra cronometrar o tempo
    std::chrono::duration<double, std::milli> duracao_ms = fim - inicio;

    // Exibe com maior precisão (6 casas decimais, por exemplo)
    std::cout << std::fixed << std::setprecision(3) << "Duração: " << duracao_ms.count() << " ms" << std::endl;

    return 0;
}
