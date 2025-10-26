#include <iostream>
#include <chrono>
#include <iomanip> // Para formatar a saída do tempo
#include <algorithm>
#include <vector>  
#include <fstream> 

#include "../include/arv_sec.h" // Inclui seu header

std::vector<Offset> buscarNaArvoreBPlus(std::fstream &arvore, const char* chaveBusca, Offset ofRaiz);
void lerEImprimirRegistro(const std::vector<Offset>& offsetsRegistros);

extern Offset raizOffset; // Usa a variável global

int main(int argc, char** argv){
    auto inicio = std::chrono::high_resolution_clock::now(); 

    const char* chaveBusca; 

 
    std::fstream arq_arvore("../bin/arvore_sec.bin", std::ios::binary | std::ios::in); 
    
    if (!arq_arvore.is_open()) {
         std::cerr << "Erro: Não foi possível abrir o índice ../bin/arvore_sec.bin.\n";
         return -1;
    }

    arq_arvore.seekg(0, std::ios::beg); 
    arq_arvore.read(reinterpret_cast<char*>(&raizOffset), sizeof(Offset)); 

    if(argc != 2){ 
        std::cout << "Uso: " << argv[0] << " \"Titulo a ser buscado\"\n";
        std::cout << "Nota: Use aspas se o título tiver espaços.\n";
        return -1;
    }

    chaveBusca = argv[1]; 
    std::cout << "Buscando pelo título: \"" << chaveBusca << "\"...\n";

    std::vector<Offset> offsetsChave = buscarNaArvoreBPlus(arq_arvore, chaveBusca, raizOffset); 

    lerEImprimirRegistro(offsetsChave); 
    arq_arvore.close(); 

    auto fim = std::chrono::high_resolution_clock::now(); 
    std::chrono::duration<double, std::milli> duracao_ms = fim - inicio;

    std::cout << std::fixed << std::setprecision(3) << "\nDuração total: " << duracao_ms.count() << " ms" << std::endl;

    return 0;
}

