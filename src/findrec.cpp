#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include "hashE.h"

// --- Funções para LER de data.bin ---
std::string read_string(std::fstream& file) {
    size_t len;
    file.read(reinterpret_cast<char*>(&len), sizeof(len));
    char* buffer = new char[len + 1];
    file.read(buffer, len);
    buffer[len] = '\0';
    std::string s(buffer);
    delete[] buffer;
    return s;
}
// ---

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Uso: ./bin/findrec <ID>" << std::endl;
        return 1;
    }
    int id_busca = std::stoi(argv[1]);
    std::string data_path = "/data/db/data.bin";
    std::string hash_path = "/data/db/hash_index.bin";

    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Abre o índice hash (com cache)
    HashE hash_index(10, hash_path, 5000); // Prof. inicial não importa se o arq existe*
                                         // (*Precisamos melhorar o construtor para LER
                                         // prof_global do arquivo em vez de começar do zero)
    
    // Reseta contadores
    hash_index.resetar_contadores_io();

    // 1. Busca no índice hash
    long record_offset = hash_index.buscar(id_busca);

    // Mede o tempo da busca
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    if (record_offset == -1) {
        std::cout << "Registro com ID " << id_busca << " nao encontrado." << std::endl;
    } else {
        std::cout << "Registro encontrado no offset " << record_offset << std::endl;
        
        // 2. Busca no arquivo de dados
        std::fstream data_file(data_path, std::ios::in | std::ios::binary);
        data_file.seekg(record_offset);

        // --- Lógica para ler o registro ---
        // int id; data_file.read(reinterpret_cast<char*>(&id), sizeof(id));
        // std::string titulo = read_string(data_file);
        // ... (lê todos os campos) ...
        // std::cout << "ID: " << id << ", Titulo: " << titulo << std::endl;
        // ---
        
        data_file.close();
    }
    
    std::cout << "--- Estatisticas (findrec) ---" << std::endl;
    std::cout << "Tempo de busca: " << duration.count() << " ms" << std::endl;
    std::cout << "Blocos lidos do indice hash: " << hash_index.get_num_reads() << std::endl;
    std::cout << "Total de blocos no indice hash: " << hash_index.get_total_blocos() << std::endl;
    
    return 0;
}