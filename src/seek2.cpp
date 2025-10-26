#include <iostream>
#include <chrono>
#include <iomanip> // Para formatar a saída do tempo
#include <algorithm>
#include <vector>  
#include <fstream> 

#include "../include/arv_sec.h" // Inclui seu header

// Use the functions provided for the secondary index (suffixed with _sec)
// and the corresponding root offset variable.
// Não precisamos das declarações ad-hoc abaixo porque estão no header.
extern Offset raizOffset_sec; // Usa a variável global da árvore secundária

int main(int argc, char** argv){
    auto inicio = std::chrono::high_resolution_clock::now(); 

    const char* chaveBusca; 

 
    std::fstream arq_arvore("../bin/arvore_sec.bin", std::ios::binary | std::ios::in); 
    
    if (!arq_arvore.is_open()) {
         std::cerr << "Erro: Não foi possível abrir o índice ../bin/arvore_sec.bin.\n";
         return -1;
    }

    arq_arvore.seekg(0, std::ios::beg); 
    arq_arvore.read(reinterpret_cast<char*>(&raizOffset_sec), sizeof(Offset)); 

    if(argc != 2){ 
        std::cout << "Uso: " << argv[0] << " \"Titulo a ser buscado\"\n";
        std::cout << "Nota: Use aspas se o título tiver espaços.\n";
        return -1;
    }

    chaveBusca = argv[1]; 
    std::cout << "Buscando pelo título: \"" << chaveBusca << "\"...\n";

    std::vector<Offset> offsetsChave = buscarNaArvoreBPlus_sec(arq_arvore, chaveBusca, raizOffset_sec); 

    lerEImprimirRegistro_sec(offsetsChave); 
    arq_arvore.close(); 

    auto fim = std::chrono::high_resolution_clock::now(); 
    std::chrono::duration<double, std::milli> duracao_ms = fim - inicio;

    std::cout << std::fixed << std::setprecision(3) << "\nDuração total: " << duracao_ms.count() << " ms" << std::endl;

    return 0;
}

