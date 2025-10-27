#include "../include/bloco.h"
#include "../include/arv_prim.h" // Funções cria_arvore_primaria, etc. (nomes originais)
#include "../include/arv_sec.h"  // Funções cria_arvore_secundaria_sec, etc. (nomes com _sec)
#include "../include/hashE.h"
#include "../include/logger.h"
#include "../include/limpa_csv.h" // Função ajeita_csv()
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>

int main(){
    std::string csv_caminho_origem = "../data/artigos.csv";
    std::string csv_caminho_limpo = "../data/artigo_novo.csv";

    LOG_INFO(std::string("Iniciando carga de dados a partir de ") + csv_caminho_origem + " (limpo em " + csv_caminho_limpo + ")");

    size_t bucket_capacity = 2; // Capacidade para o hash extensível

    // PASSO 1: Limpa o CSV
    LOG_INFO("Executando ajeita_csv()...");
    ajeita_csv(); // Assume que lê csv_caminho_origem e cria csv_caminho_limpo

    // PASSO 2: Cria o arquivo de blocos binário (dados.in)
    // As funções dentro de bloco.cpp agora usam as constantes do bloco.h
    LOG_INFO("Executando bloco::criar_arquivo_blocos()...");
    bloco::criar_arquivo_blocos(); // Lê csv_caminho_limpo (hardcoded) e cria ../bin/dados.in

    // PASSO 3: Cria a Árvore Primária
    LOG_INFO("Executando cria_arvore_primaria()...");
    // Assume que arv_prim.cpp usa os nomes de função originais
    if (cria_arvore_primaria() != 0) {
        LOG_ERROR("Falha ao criar a árvore primária.");
        return 1; // Aborta se a criação da árvore falhar
    }

    // PASSO 4: Cria a Árvore Secundária
    LOG_INFO("Executando cria_arvore_secundaria()...");
    // Assume que arv_sec.cpp usa os nomes com sufixo _sec
    if (cria_arvore_secundaria() != 0) {
        LOG_ERROR("Falha ao criar a árvore secundária.");
        return 1; // Aborta se a criação da árvore falhar
    }

    // PASSO 5: Cria o arquivo de dados organizado por Hash Extensível
    LOG_INFO("Executando bloco::criar_arquivo_blocos_hash_file()...");
    bloco::criar_arquivo_blocos_hash_file(csv_caminho_limpo, bucket_capacity); // Usa o CSV limpo

    LOG_INFO("Processo de empacotamento finalizado.");
    return 0;
}

