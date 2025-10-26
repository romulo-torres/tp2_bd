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
extern Offset raizOffset;

#pragma pack(push, 1) // pra ter exatamente 300 bytes de chave
struct ChaveTitulo {
    char titulo[300];
};
#pragma pack(pop)

const int MAX_CHAVES_FOLHA = (TAM_BLOCO - sizeof(bool) - sizeof(std::uint32_t) - sizeof(Offset)) 
                             / (sizeof(ChaveTitulo) + sizeof(Offset)); // Resultado: 13

const int MAX_CHAVES_INTERNO = (TAM_BLOCO - sizeof(bool) - sizeof(std::uint32_t) - sizeof(Offset)) 
                               / (sizeof(ChaveTitulo) + sizeof(Offset)); // Resultado: 13

#pragma pack(push, 1) // desativa o padding

struct NoFolha {
    bool ehFolha = true;
    std::uint32_t nChaves = 0;
    ChaveTitulo chaves[MAX_CHAVES_FOLHA];
    Offset offsetsRegistros[MAX_CHAVES_FOLHA]; // ponteiros para dados
    Offset prox = -1; // próxima folha

    static const size_t tamanho_usado = offsetof(NoFolha,prox) + sizeof(Offset);
    char espaco_livre[TAM_BLOCO - tamanho_usado];
};

#pragma pack(pop) // restaura o alinhamento padrão

#pragma pack(push, 1) // desativa o padding


struct NoInterno {
    bool ehFolha = false;
    std::uint32_t nChaves = 0;
    ChaveTitulo chaves[MAX_CHAVES_INTERNO];
    Offset filhos[MAX_CHAVES_INTERNO + 1];

    static const size_t tamanho_usado = 
        offsetof(NoInterno, filhos) + (sizeof(Offset) * (MAX_CHAVES_INTERNO + 1));
    char espaco_livre[TAM_BLOCO - tamanho_usado];
};

#pragma pack(pop) // restaura o alinhamento padrão


Offset escreverNoFolha(std::fstream &arq, NoFolha &no, Offset offsetEscrita);

Offset escreverNoInterno(std::fstream &arq, NoInterno &no, Offset offsetEscrita);

void atualizarProxFolha(std::fstream &arq, Offset offsetFolhaAnterior, Offset offsetProx);

std::vector<Offset> construirFolhas(std::fstream &arvore, std::ifstream &dados, Offset &offsetAtual);

std::vector<Offset> construirNivelInterno(std::fstream &arvore, const std::vector<Offset> &nivelAnterior, Offset &offsetAtual);

Offset buscarNaArvoreBPlus(std::fstream &arvore, const char* chaveBusca, Offset ofRaiz);

void lerEImprimirRegistro(Offset offsetRegistro);

int cria_arvore_primaria();

#endif
