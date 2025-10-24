#include <iostream>
#include <vector>
#include <memory>
#include <map>
#include <set>
#include <cmath>
#include <list>
#include "hashE.h"

// construtor
static int next_bucket_id = 0;

HashE::HashE(int prof_inicial, size_t cap_bucket) : prof_global(prof_inicial), capacidade_bucket(cap_bucket) {
    size_t tam_inicial = size_t(1) << prof_global; // 2^prof_inicial
    diretorio.resize(tam_inicial);
    for(size_t i = 0; i < tam_inicial; ++i){
        diretorio[i] = std::make_shared<Bucket>(prof_global, capacidade_bucket);
        diretorio[i]->id = next_bucket_id++;
    }
}

// -- métodos privados --

int HashE::hash(int chave){
    return chave; // hash simples (identidade)
}

int HashE::get_index(int chave, int prof){
    int h = hash(chave);
    return h & ((1 << prof) - 1); // máscara para obter os bits relevantes
}

void HashE::duplicar_diretorio(){
    int tam_antigo = diretorio.size();
    prof_global++;
    diretorio.resize(1 << prof_global);

    // copiando os ponteiros antigos para as novas posições
    for (int i = 0; i < tam_antigo; ++i){
        diretorio[i + tam_antigo] = diretorio[i];
    }
}

void HashE::dividir_bucket(int indice_bucket){
    BucketPtr bucket_antigo = diretorio[indice_bucket];
    int ld = bucket_antigo->prof_local;

    bucket_antigo->prof_local++;
    BucketPtr bucket_novo = std::make_shared<Bucket>(bucket_antigo->prof_local, capacidade_bucket);
    bucket_novo->id = next_bucket_id++;

    std::list<int> chaves_temporarias = bucket_antigo->chaves;
    bucket_antigo->chaves.clear();

    int primeiro_indice = get_index(chaves_temporarias.front(), ld);
    int passo = 1 << ld;

    for (size_t i = primeiro_indice; i < diretorio.size(); i += passo){
        if (((i >> ld) & 1u)){
            diretorio[i] = bucket_novo;
        }else{
            diretorio[i] = bucket_antigo;
        }
    }
    
    // re-inserindo as chaves
    for (int chave : chaves_temporarias){
        inserir_chave(chave, true); // re-inserindo as chaves
    }
}

void HashE::inserir_chave(int chave, bool rehashing){
    int indice = get_index(chave, prof_global);
    BucketPtr bucket = diretorio[indice];

    if (!bucket->inserir(chave)){
        // se o bucket estiver cheio
        if (bucket->prof_local == prof_global){
            duplicar_diretorio();
            indice = get_index(chave, prof_global); // recalcula o índice
            bucket = diretorio[indice];
        }
        dividir_bucket(indice); // divide o bucket cheio

        if(!rehashing){
            inserir_chave(chave, false); // tenta inserir novamente
        }
        
    }
}

// -- metodos públicos --


// metodo para inserir uma chave
void HashE::inserir(int chave){
    inserir_chave(chave, false);
}

// metodo para buscar uma chave
bool HashE::buscar(int chave){
    int indice = get_index(chave, prof_global);
    return diretorio[indice]->buscar(chave);
}

// metodo para imprimir o estado
void HashE::print(){
    std::cout << "== Estado da Tabela ==" << std::endl;
    std::cout << "Profundidade Global: " << prof_global << std::endl;
    // agrupa por ponteiro
    std::map<Bucket*, int> bucket_ids;
    for (size_t i = 0; i < diretorio.size(); i++){
        bucket_ids[diretorio[i].get()]++; // conta quantas entradas do diretório apontam para o bucket
    }

    for (auto const& [bucket_ptr, ids_count] : bucket_ids){
        std::cout << "  Bucket id=" << bucket_ptr->id << " (ptr: " << bucket_ptr << ")" << std::endl;
        std::cout << "    Profundidade Local: " << bucket_ptr->prof_local << std::endl;
        std::cout << "    Entradas do diretorio: " << ids_count << std::endl;
        std::cout << "    Chaves: ";

        for (int chave : bucket_ptr->chaves){
            std::cout << chave << " ";
        }
        std::cout << std::endl;
    }

    std::cout << "== Diretório ==" << std::endl;
    for (size_t i = 0; i < diretorio.size(); i++){
        std::cout << "  Índice " << i << ": Bucket id " << diretorio[i]->id << " ptr " << diretorio[i].get() << std::endl;
    }
}

std::map<int, std::list<int>> HashE::snapshot_buckets() const {
    std::map<int, std::list<int>> out;
    std::set<Bucket*> seen;
    for (auto &bp : diretorio) {
        Bucket* p = bp.get();
        if (seen.insert(p).second) {
            out[p->id] = p->chaves;
        }
    }
    return out;
}
