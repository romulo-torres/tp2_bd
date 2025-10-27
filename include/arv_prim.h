#ifndef ARV_PRIM_H_
#define ARV_PRIM_H_
#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include "../include/registro.h" // Necessário para sizeof(registro) e definição
#include <cstdint> // Para std::uint32_t
#include <cstddef> // Para size_t, offsetof

// Definindo Offset como long
typedef long Offset;

// --- Constantes Globais ---
const unsigned TAM_BLOCO_ARV_PRIM = 4096; // Tamanho do bloco para nós da árvore

// Calcula MAX_CHAVES baseado em TAM_BLOCO_ARV_PRIM e nos structs abaixo
const size_t TAMANHO_PAR_FOLHA = sizeof(uint32_t) + sizeof(Offset);
const size_t METADADOS_FOLHA = sizeof(bool) + sizeof(uint32_t) + sizeof(Offset); // ehFolha + nChaves + prox
const int MAX_CHAVES_FOLHA = (TAMANHO_PAR_FOLHA > 0 && TAM_BLOCO_ARV_PRIM > METADADOS_FOLHA)
                             ? (TAM_BLOCO_ARV_PRIM - METADADOS_FOLHA) / TAMANHO_PAR_FOLHA
                             : 0;

const size_t TAMANHO_PAR_INTERNO = sizeof(uint32_t) + sizeof(Offset);
const size_t METADADOS_INTERNO = sizeof(bool) + sizeof(uint32_t) + sizeof(Offset); // ehFolha + nChaves + filhos[0]
const int MAX_CHAVES_INTERNO = (TAMANHO_PAR_INTERNO > 0 && TAM_BLOCO_ARV_PRIM > METADADOS_INTERNO)
                               ? (TAM_BLOCO_ARV_PRIM - METADADOS_INTERNO) / TAMANHO_PAR_INTERNO
                               : 0;

// --- Calcula o tamanho exato do espaço livre ---
const size_t ESPACO_USADO_FOLHA = METADADOS_FOLHA + (MAX_CHAVES_FOLHA * TAMANHO_PAR_FOLHA);
const size_t TAM_ESPACO_LIVRE_FOLHA = (TAM_BLOCO_ARV_PRIM >= ESPACO_USADO_FOLHA) ? (TAM_BLOCO_ARV_PRIM - ESPACO_USADO_FOLHA) : 0;

const size_t ESPACO_USADO_INTERNO = METADADOS_INTERNO + (MAX_CHAVES_INTERNO * TAMANHO_PAR_INTERNO); // filhos[0] já está nos metadados
const size_t TAM_ESPACO_LIVRE_INTERNO = (TAM_BLOCO_ARV_PRIM >= ESPACO_USADO_INTERNO) ? (TAM_BLOCO_ARV_PRIM - ESPACO_USADO_INTERNO) : 0;


// --- Variável Global (COM sufixo _prim) ---
extern Offset raizOffset_prim;
extern int totalBlocosCriados_prim;


// --- Estruturas (sem alteração nos nomes) ---
#pragma pack(push, 1) // Desativa o padding
struct NoFolha {
    bool ehFolha;
    uint32_t nChaves;
    uint32_t chaves[MAX_CHAVES_FOLHA];
    Offset offsetsRegistros[MAX_CHAVES_FOLHA];
    Offset prox;
    // CORREÇÃO: Usa o valor constante calculado
    char espaco_livre[TAM_ESPACO_LIVRE_FOLHA];

    NoFolha() { std::memset(this, 0, sizeof(NoFolha)); ehFolha = true; prox = -1;}
};
#pragma pack(pop)

#pragma pack(push, 1)
struct NoInterno {
    bool ehFolha;
    uint32_t nChaves;
    uint32_t chaves[MAX_CHAVES_INTERNO];
    Offset filhos[MAX_CHAVES_INTERNO + 1];
    // CORREÇÃO: Usa o valor constante calculado
    char espaco_livre[TAM_ESPACO_LIVRE_INTERNO];

    NoInterno() { std::memset(this, 0, sizeof(NoInterno)); ehFolha = false;}
};
#pragma pack(pop)

// Verificações em tempo de compilação
static_assert(sizeof(NoFolha) == TAM_BLOCO_ARV_PRIM, "Tamanho do struct NoFolha está incorreto!");
static_assert(sizeof(NoInterno) == TAM_BLOCO_ARV_PRIM, "Tamanho do struct NoInterno está incorreto!");
static_assert(MAX_CHAVES_FOLHA > 0, "MAX_CHAVES_FOLHA calculado como 0 ou negativo!");
static_assert(MAX_CHAVES_INTERNO > 0, "MAX_CHAVES_INTERNO calculado como 0 ou negativo!");


// --- Protótipos das Funções (COM sufixo _prim) ---
Offset escreverNoFolha_prim(std::fstream &arq, const NoFolha &no, Offset offset);
Offset lerNoFolha_prim(std::fstream &arq, Offset offset, NoFolha &no);
Offset escreverNoInterno_prim(std::fstream &arq, const NoInterno &no, Offset offset);
Offset lerNoInterno_prim(std::fstream &arq, Offset offset, NoInterno &no);
void atualizarProxFolha_prim(std::fstream &arq, Offset offsetFolhaAnterior, Offset offsetProx);
std::vector<Offset> construirFolhas_prim(std::fstream &arvore, std::ifstream &dados, Offset &offsetAtual);
std::vector<Offset> construirNivelInterno_prim(std::fstream &arvore, const std::vector<Offset> &nivelAnterior, Offset &offsetAtual);
Offset buscarNaArvoreBPlus_prim(std::fstream &arvore, std::uint32_t chaveBusca, Offset ofRaiz);
void lerEImprimirRegistro_prim(Offset offsetRegistro); // Recebe Offset
int cria_arvore_primaria(); // Função principal mantém o nome original para chamada externa

#endif // ARV_PRIM_H_

