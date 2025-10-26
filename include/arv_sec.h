#ifndef ARV_SEC_H_
#define ARV_SEC_H_
#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <cstddef>   
#include <cstdint>   
#include "../include/registro.h"

const size_t TAM_BLOCO = 4096;
typedef long Offset; // Meu tipo pra retornar o endereço/onde ta meu registro no arquivo de dados
extern Offset raizOffset_sec;


#pragma pack(push, 1) // pra ter exatamente 300 bytes de chave
struct ChaveTitulo {
    char titulo[300];
};
#pragma pack(pop)

const int MAX_CHAVES_FOLHA_SEC = (TAM_BLOCO - sizeof(bool) - sizeof(std::uint32_t) - sizeof(Offset)) 
                             / (sizeof(ChaveTitulo) + sizeof(Offset)); // Resultado: 13

const int MAX_CHAVES_INTERNO_SEC = (TAM_BLOCO - sizeof(bool) - sizeof(std::uint32_t) - sizeof(Offset)) 
                               / (sizeof(ChaveTitulo) + sizeof(Offset)); // Resultado: 13

#pragma pack(push, 1) 
struct NoFolha_sec {
    bool ehFolha = true;
    std::uint32_t nChaves = 0;
    ChaveTitulo chaves[MAX_CHAVES_FOLHA_SEC];
    Offset offsetsRegistros[MAX_CHAVES_FOLHA_SEC]; 
    Offset prox = -1; 

    
    char espaco_livre[79];
};
#pragma pack(pop) 

#pragma pack(push, 1) 
struct NoInterno_sec {
    bool ehFolha = false;
    std::uint32_t nChaves = 0;
    ChaveTitulo chaves[MAX_CHAVES_INTERNO_SEC]; 
    Offset filhos[MAX_CHAVES_INTERNO_SEC + 1];

    char espaco_livre[79];
};
#pragma pack(pop) 

const int MAX_OFFSETS_POR_BLOCO = (TAM_BLOCO - sizeof(std::uint32_t) - sizeof(Offset)) / sizeof(Offset); // 510

#pragma pack(push, 1)
struct BlocoLista {
    std::uint32_t nOffsets = 0; 
    Offset proxBlocoLista = -1; 
    Offset offsetsRegistros[MAX_OFFSETS_POR_BLOCO]; 

    char espaco_livre[4];
};
#pragma pack(pop)

Offset escreverNoFolha_sec(std::fstream &arq, NoFolha_sec &no, Offset offsetEscrita);

Offset escreverNoInterno_sec(std::fstream &arq, NoInterno_sec &no, Offset offsetEscrita);

Offset escreverBlocoLista_sec(std::fstream &arq, BlocoLista &bloco, Offset offsetEscrita);

void atualizarProxFolha_sec(std::fstream &arq, Offset offsetFolhaAnterior, Offset offsetProx);

std::vector<Offset> construirFolhas_sec(std::fstream &arvore, std::ifstream &dados, Offset &offsetAtual);

std::vector<Offset> construirNivelInterno_sec(std::fstream &arvore, const std::vector<Offset> &nivelAnterior, Offset &offsetAtual);

std::vector<Offset> buscarNaArvoreBPlus_sec(std::fstream &arvore, const char* chaveBusca, Offset ofRaiz);

void lerEImprimirRegistro_sec(const std::vector<Offset>& offsetsRegistros);

int cria_arvore_secundaria();

#endif
