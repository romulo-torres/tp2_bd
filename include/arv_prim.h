#ifndef ARV_PRIM_H_
#define ARV_PRIM_H_
#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include "../include/registro.h"

const int MAX_CHAVES_FOLHA = 340; // exemplo, ajustável conforme bloco de 4KB
const int MAX_CHAVES_INTERNO = 340; // número máximo de chaves
typedef long Offset; // Meu tipo pra retornar o endereço/onde ta meu registro no arquivo de dados
extern Offset raizOffset;

#pragma pack(push, 1) // desativa o padding

struct NoFolha {
    bool ehFolha = true;
    std::uint32_t nChaves = 0;
    unsigned chaves[MAX_CHAVES_FOLHA];
    Offset offsetsRegistros[MAX_CHAVES_FOLHA]; // ponteiros para dados
    Offset prox = -1; // próxima folha
};

#pragma pack(pop) // restaura o alinhamento padrão

#pragma pack(push, 1) // desativa o padding


struct NoInterno {
    bool ehFolha = false;
    std::uint32_t nChaves = 0;
    unsigned chaves[MAX_CHAVES_INTERNO];
    Offset filhos[MAX_CHAVES_INTERNO + 1];
};

#pragma pack(pop) // restaura o alinhamento padrão


Offset escreverNoFolha(std::fstream &arq, NoFolha &no, Offset offsetEscrita);

Offset escreverNoInterno(std::fstream &arq, NoInterno &no, Offset offsetEscrita);

void atualizarProxFolha(std::fstream &arq, Offset offsetFolhaAnterior, Offset offsetProx);

std::vector<Offset> construirFolhas(std::fstream &arvore, std::ifstream &dados, Offset &offsetAtual);

std::vector<Offset> construirNivelInterno(std::fstream &arvore, const std::vector<Offset> &nivelAnterior, Offset &offsetAtual);


Offset buscarNaArvoreBPlus(std::fstream &arvore, std::uint32_t chaveBusca, Offset ofRaiz);

void lerEImprimirRegistro(Offset offsetRegistro);

int cria_arvore_primaria();

#endif
