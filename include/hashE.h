#ifndef HASHE_H_
#define HASHE_H_

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <map>
#include <list>
#include <stdexcept>
#include <functional> // Para std::hash
#include <cmath>

// Capacidade do bucket (número de pares chave/valor)
// É importante que o sizeof(Bucket) seja um bom tamanho de bloco
// (ex: próximo de 4096 bytes). 
// Com 256, (256 * 4) + (256 * 8) + ... ~ 3KB. É um bom começo.
const size_t BUCKET_CAPACITY = 256; 

/**
 * @brief Estrutura do Bucket (página de disco).
 * Armazena pares (chave, valor) onde:
 * - chave = ID do Artigo (int)
 * - valor = offset do registro no arquivo de dados (long)
 */
struct Bucket {
    int prof_local;
    size_t num_chaves; // num_chaves == num_offsets
    int chaves[BUCKET_CAPACITY];
    long offsets[BUCKET_CAPACITY];

    Bucket(int prof = 0) 
        : prof_local(prof), num_chaves(0) {}

    bool ta_cheio() const { return num_chaves >= BUCKET_CAPACITY; }
    
    // Insere o par (chave, valor)
    bool inserir(int chave, long offset) {
        if (ta_cheio()) return false;
        // Não checa duplicata, assume que 'inserir_chave' já checou
        chaves[num_chaves] = chave;
        offsets[num_chaves] = offset;
        num_chaves++;
        return true;
    }

    // Busca pela chave e retorna o offset (valor)
    // Retorna -1 se não encontrar
    long buscar(int chave) {
        for (size_t i = 0; i < num_chaves; ++i) {
            if (chaves[i] == chave) {
                return offsets[i];
            }
        }
        return -1; // Não encontrado
    }

    // Usado pelo 'split' para pegar todas as chaves
    std::vector<int> get_chaves() const {
        return std::vector<int>(chaves, chaves + num_chaves);
    }

    // Usado pelo 'split' para pegar todos os offsets
    std::vector<long> get_offsets() const {
        return std::vector<long>(offsets, offsets + num_chaves);
    }
};


// --- A classe HashE, com Buffer Pool (Cache) ---

class HashE {
private:
    int prof_global;
    std::vector<long> diretorio;
    
    std::string bucket_filename;
    std::fstream bucket_file;
    long proximo_offset_livre; // Onde o próximo bucket será anexado

    // --- Contadores de I/O (para o TP) ---
    long num_reads;
    long num_writes;
    long num_anexos; // (Contado como 'write')

    // --- Membros do Buffer Pool (Cache) ---
    size_t cache_max_size;
    std::map<long, Bucket> cache_pool;
    std::list<long> cache_lru_list;
    std::map<long, std::list<long>::iterator> cache_lru_map;
    std::map<long, bool> cache_dirty_pages;
    // --- Fim: Membros do Buffer Pool ---

    // --- Funções de I/O de DISCO ---
    Bucket ler_bucket_do_disco(long offset);
    void escrever_bucket_no_disco(long offset, const Bucket& bucket);
    long anexar_bucket_no_disco(const Bucket& bucket);

    // --- Funções de I/O de CACHE (as novas) ---
    Bucket ler_bucket(long offset);
    void escrever_bucket(long offset, const Bucket& bucket);
    long anexar_bucket(const Bucket& bucket); 

    // --- Funções de gerenciamento do CACHE ---
    void cache_touch(long offset); 
    void cache_evict();          
    void flush_cache();          

    // --- Lógica do Hashing ---
    int hash(int chave);
    int get_index(int chave, int prof);
    void duplicar_diretorio();
    void inserir_chave(int chave, long offset, bool rehashing);

public:
    // Construtor
    HashE(int prof_inicial, const std::string& filename, size_t cache_size = 1000);

    // Destrutor
    ~HashE();

    // Funções públicas adaptadas para o TP
    void inserir(int chave, long offset);
    long buscar(int chave); // Retorna offset ou -1

    // Funções de relatório para o TP
    void resetar_contadores_io() { num_reads = 0; num_writes = 0; }
    long get_num_reads() const { return num_reads; }
    long get_num_writes() const { return num_writes + num_anexos; } // Total de escritas
    long get_total_blocos() const; // Calcula total de blocos no arquivo

    // Funções de debug (como 'print')
    void print();
    // Retorna mapa de <offset_bucket, lista_de_chaves> lendo os buckets atuais
    std::map<long, std::list<int>> snapshot_buckets();
};

#endif // HASHE_H_