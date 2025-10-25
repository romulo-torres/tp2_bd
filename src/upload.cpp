#include <iostream>
#include <fstream>
#include <string>
#include <chrono> // Para medir o tempo
#include "hashE.h"
#include "bloco.h"
#include "logger.h"
// #include "bplustree.h" // Você também precisará disso

// --- Funções para escrever em data.bin ---
// Você precisa criar estas funções para lidar com
// a escrita de registros de tamanho variável.
// Ex: escrever "hello"
// 5 (int) | 'h''e''l''l''o'
void write_string(std::fstream& file, const std::string& s) {
    size_t len = s.length();
    file.write(reinterpret_cast<const char*>(&len), sizeof(len));
    file.write(s.c_str(), len);
}
// ... e funções para write_int, etc.
// ---

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Uso: ./bin/upload <caminho_csv>" << std::endl;
        return 1;
    }
    std::string csv_path = argv[1];
    std::string data_path = "/data/db/data.bin";
    std::string hash_path = "/data/db/hash_index.bin";

    LOG_INFO(std::string("Iniciando carga de dados de ") + csv_path);

    // Chama o empacotador de blocos (versão não interativa)
    bloco b;
    // capacidade do bucket: escolha pequena por padrão (2) — pode ajustar conforme necessidade
    size_t bucket_capacity = 2;
    b.criar_arquivo_blocos_hash_file(csv_path, bucket_capacity);

    LOG_INFO("Processo de empacotamento finalizado.");

    return 0;
}