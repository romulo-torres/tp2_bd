#include "../include/bloco.h"
#include "../include/arv_prim.h"
#include "../include/hashE.h"
#include "../include/logger.h"
#include "../include/limpa_csv.h"
#include <iostream>
#include <fstream>
#include <string>
#include <chrono> 
#include <iostream>

void write_string(std::fstream& file, const std::string& s) {
    size_t len = s.length();
    file.write(reinterpret_cast<const char*>(&len), sizeof(len));
    file.write(s.c_str(), len);
}

int main(int argc, char** argv){
    if(argc < 2){
        std::cerr << "Formato certo: camiho_do_csv.csv\n";
        return 1;
    }
    std::string csv_caminho = argv[1];
    std::string data_caminho = "../data/db/data.bin";
    std::string hash_caminho = "../data/db/hash_index.bin";
    bloco b;
        
    LOG_INFO(std::string("Iniciando carga de dados de ") + csv_caminho);
    
    size_t bucket_capacity = 2;
    ajeita_csv(); // Função pra ajeitar o csv e gerar o artigo_novo.csv
    b.criar_arquivo_blocos(); // Crio o arquivo de blocos pra árvore primaria
    cria_arvore_primaria(); // Crio a árvore primaria em mémoria secundaria

    b.criar_arquivo_blocos_hash_file(csv_caminho, bucket_capacity);

    LOG_INFO("Processo de empacotamento finalizado.");
    return 0;
}
