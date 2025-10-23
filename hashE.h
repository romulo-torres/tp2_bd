#ifndef HASHE_H_
#define HASHE_H_

#include <iostream>
#include <vector>
#include <memory> 
#include <map>
#include <list>
#include <cmath>

class Bucket;
using BucketPtr = std::shared_ptr<Bucket>;

class Bucket {
public:
    int prof_local; // profundidade local
    size_t capacidade; // capacidade máxima do bucket
    int id; // id único do bucket
    std::list<int> chaves; // chaves armazenadas no bucket

    Bucket(int prof, size_t cap) : prof_local(prof), capacidade(cap) {}

    bool ta_cheio() const {
        return chaves.size() >= capacidade;
    } 

    bool inserir(int c){ 
        if (ta_cheio()) return false;
        chaves.push_back(c);
        return true;
    }

    bool buscar(int c){
        for (int i : chaves){
            if (i == c) return true;
        }
        return false;
    }
};


class HashE {
private:
    int prof_global;
    size_t capacidade_bucket;
    std::vector<BucketPtr> diretorio;
    int hash(int chave);
    int get_index(int chave, int prof);
    void duplicar_diretorio();
    void dividir_bucket(int indice_bucket);
    void inserir_chave(int chave, bool rehashing);

public:
    // Construtor
    HashE(int prof_inicial, size_t cap_bucket);

    // Metodo para inserir uma chave
    void inserir(int chave);

    // Metodo para buscar uma chave
    bool buscar(int chave);

    // Metodo para imprimir o estado
    void print();

    // retorna um snapshot dos buckets: map<bucket_id, lista_de_chaves>
    std::map<int, std::list<int>> snapshot_buckets() const;
};

#endif