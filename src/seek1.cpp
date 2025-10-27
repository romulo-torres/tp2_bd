#include <iostream>
#include <chrono>
#include <iomanip> // Para formatar a saída do tempo
#include <algorithm>
#include <fstream> // Necessário para std::fstream
#include <cstdlib> // Necessário para atoi

#include "../include/arv_prim.h" 

// Declaração externa para usar a variável global com sufixo
extern Offset raizOffset_prim;

int main(int argc, char** argv){
    auto inicio = std::chrono::high_resolution_clock::now(); // Para cronometrar o tempo

    unsigned chaveBusca; // Aqui é pra guardar a chave que vai ler do terminal

    // Abre o arquivo correto da árvore primária
    std::fstream arq_arvore("../bin/arvore_prim.in", std::ios::binary | std::ios::in);
    if (!arq_arvore.is_open()) {
        std::cerr << "Erro ao abrir o arquivo da árvore ../bin/arvore_prim.bin" << std::endl;
        return -1;
    }

    // Lê o offset da raiz (variável com sufixo) do arquivo

    Offset offsetRaizLido = -1;
    arq_arvore.seekg(0, std::ios::beg);
    if (!arq_arvore.read(reinterpret_cast<char*>(&offsetRaizLido), sizeof(Offset))) {
         std::cerr << "Erro ao ler o offset da raiz do arquivo arvore_prim.in" << std::endl;
         arq_arvore.close();
         return -1;
    }
    // Usa o offset lido do arquivo para a busca, não a variável global
    // arq_arvore.read(reinterpret_cast<char*>(&raizOffset_prim), sizeof(Offset)); // Linha original comentada


    if(argc != 2){ // Mensagem generica pra caso tenha erro
        std::cout << "Uso: " << argv[0] << " <Chave a ser buscada (ID numérico)>" << "\n";
        arq_arvore.close(); // Fecha o arquivo antes de sair
        return -1;
    }

    // Validação básica se o argumento é um número
    try {
        chaveBusca = std::stoul(argv[1]); // Usa stoul para melhor tratamento de erros
    } catch (const std::exception& e) {
        std::cerr << "Erro: A chave fornecida '" << argv[1] << "' não é um número válido." << std::endl;
        arq_arvore.close();
        return -1;
    }

    // Aplica a função de busca (COM sufixo) usando o offset lido do arquivo
    Offset offsetChave = buscarNaArvoreBPlus_prim(arq_arvore, chaveBusca, offsetRaizLido);

    // Chama a função de impressão (COM sufixo)
    lerEImprimirRegistro_prim(offsetChave);
    arq_arvore.close(); // Fecho o arquivo

    auto fim = std::chrono::high_resolution_clock::now(); // Pra cronometrar o tempo
    std::chrono::duration<double, std::milli> duracao_ms = fim - inicio;

    // Exibe com maior precisão
    std::cout << std::fixed << std::setprecision(3) << "\nDuração total da busca: " << duracao_ms.count() << " ms" << std::endl;

    return 0;
}
