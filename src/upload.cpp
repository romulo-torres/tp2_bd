#include "../include/bloco.h"
#include "../include/arv_prim.h"
#include "../include/arv_sec.h"
#include "../include/hashE.h"
#include "../include/logger.h"
#include "../include/limpa_csv.h"
#include <iostream>
#include <fstream>
#include <string>
#include <chrono> 
#include <iostream>

// faz esse sem receber o caminho, só fixar e deixar o csv no /data

void write_string(std::fstream& file, const std::string& s) {
    size_t len = s.length();
    file.write(reinterpret_cast<const char*>(&len), sizeof(len));
    file.write(s.c_str(), len);
}

int main(){

    std::string csv_caminho = "../data/ ";
    std::string data_caminho = "../bin/data.bin";
    std::string hash_caminho = "../bin/hash_index.bin";
    bloco b;
        
    LOG_INFO(std::string("Iniciando carga de dados de ") + csv_caminho);
    
    size_t bucket_capacity = 2;
    ajeita_csv(); // Função pra ajeitar o csv e gerar o artigo_novo.csv
    b.criar_arquivo_blocos(); // Crio o arquivo de blocos pra árvore primaria
    cria_arvore_primaria(); // Crio a árvore primaria em mémoria secundaria
    cria_arvore_secundaria();

    b.criar_arquivo_blocos_hash_file(csv_caminho, bucket_capacity);

    LOG_INFO("Processo de empacotamento finalizado.");
    return 0;
}
