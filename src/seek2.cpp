#include <iostream>
#include <chrono>
#include <iomanip>
#include <algorithm>
#include <vector>  
#include <fstream> 

#include "../include/arv_sec.h"

int main(int argc, char** argv){
    auto inicio = std::chrono::high_resolution_clock::now(); 
    const char* chaveBusca; 

    std::fstream arq_arvore("../bin/arvore_sec.in", std::ios::binary | std::ios::in); 

    if (!arq_arvore.is_open()) {
            std::cerr << "Erro: Não foi possível abrir ../bin/arvore_sec.in.\n";
            return -1;
    }

    // Ler offset da raiz
    Offset offsetRaiz;
    arq_arvore.read(reinterpret_cast<char*>(&offsetRaiz), sizeof(Offset));

    // // Descer até a primeira folha
    // Offset current = offsetRaiz;
    // while (true) {
    //     arq_arvore.seekg(current, std::ios::beg);
    //     bool ehFolha;
    //     arq_arvore.read(reinterpret_cast<char*>(&ehFolha), sizeof(bool));
    //     if (ehFolha) break;

    //     NoInterno_sec no;
    //     arq_arvore.read(reinterpret_cast<char*>(&no), sizeof(NoInterno_sec));
    //     current = no.filhos[0];
    // }

    // Offset offsetFolha = current;

    // std::ifstream dados("../bin/dados.in", std::ios::binary);
    // if (!dados.is_open()) {
    //     std::cerr << "Erro ao abrir dados.in\n";
    //     return -1;
    // }

    // while (offsetFolha != -1) {
    //     arq_arvore.seekg(offsetFolha, std::ios::beg);
    //     NoFolha_sec folha;
    //     arq_arvore.read(reinterpret_cast<char*>(&folha), sizeof(NoFolha_sec));

    //     std::cout << "\n--- Folha no offset " << offsetFolha << " ---\n";

    //     for (int i = 0; i < folha.nChaves; ++i) {
    //         std::cout << "Chave " << i << ": " << folha.chaves[i].titulo << "\n";

    //         Offset offsetLista = folha.offsetsRegistros[i];
    //         BlocoLista bloco;

    //         while (offsetLista != -1) {
    //             arq_arvore.seekg(offsetLista, std::ios::beg);
    //             arq_arvore.read(reinterpret_cast<char*>(&bloco), sizeof(BlocoLista));

    //             for (uint32_t j = 0; j < bloco.nOffsets; ++j) {
    //                 registro reg;
    //                 dados.seekg(bloco.offsetsRegistros[j], std::ios::beg);
    //                 dados.read(reinterpret_cast<char*>(&reg), sizeof(registro));

    //                 std::cout << "  Registro ID: " << reg.id
    //                           << ", Titulo: " << reg.titulo
    //                           << ", Ano: " << reg.ano << "\n";
    //             }

    //             offsetLista = bloco.proxBlocoLista;
    //         }
    //     }

    //     offsetFolha = folha.prox; // próxima folha
    //     std::cout << "  -> Próxima folha: " << folha.prox << std::endl;
    //     if (folha.prox == offsetFolha) {
    //         std::cerr << "[ERRO] Loop detectado: folha aponta para si mesma!" << std::endl;
    //         break;
    //     }

    // }

    if(argc != 2){ 
        std::cout << "Uso: " << argv[0] << " \"Titulo a ser buscado\"\n";
        std::cout << "Nota: Use aspas se o título tiver espaços.\n";
        arq_arvore.close();
        return -1;
    }

    chaveBusca = argv[1]; 
    std::cout << "Buscando pelo título: \"" << chaveBusca << "\"...\n";

    std::vector<Offset> offsetsChave = buscarNaArvoreBPlus_sec(arq_arvore, chaveBusca, offsetRaiz); 
    lerEImprimirRegistro_sec(offsetsChave);

    verificar_titulo_em_dados("DIY World Builder: An immersive level-editing system.");

    arq_arvore.close(); 



    auto fim = std::chrono::high_resolution_clock::now(); 
    std::chrono::duration<double, std::milli> duracao_ms = fim - inicio;

    std::cout << std::fixed << std::setprecision(3) << "\nDuração total: " << duracao_ms.count() << " ms" << std::endl;

    return 0;
}