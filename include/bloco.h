#ifndef BLOCO_H
#define BLOCO_H

#include "../include/registro.h" // Para sizeof(registro)
#include <unistd.h>           // Para sysconf, _SC_PAGESIZE
#include <cstring>            // Para std::memset
#include <stddef.h>           // Para offsetof (alternativa)
#include <vector>             // Para std::vector (apenas para declarações de função)
#include <string>             // Para std::string (apenas para declarações de função)
#include <fstream>            // Para std::ifstream, std::ofstream (apenas para declarações de função)
#include <iostream>           // Para std::cerr em get_page_size_for_bloco

// --- Cálculo dos tamanhos baseado na página ---
// Função auxiliar para obter o tamanho da página de forma segura
inline unsigned get_page_size_for_bloco() {
    long page_size = sysconf(_SC_PAGESIZE);
    if (page_size == -1) {
        perror("sysconf em bloco.h");
        std::cerr << "AVISO: Falha ao obter tamanho da página, usando fallback 4096." << std::endl;
        return 4096; // Fallback
    }
    return static_cast<unsigned>(page_size);
}

// CORREÇÃO: Usar um valor fixo para garantir constante em tempo de compilação
const unsigned TAM_BLOCO_CALC = 4096; // Tamanho total do bloco (página) - Usando valor fixo
// const unsigned TAM_BLOCO_CALC = get_page_size_for_bloco(); // Removido getpagesize daqui

// Calcula quantos registros cabem no bloco (Agora é uma expressão constante)
const unsigned NUM_REGISTROS_CALC = (sizeof(registro) > 0 && TAM_BLOCO_CALC >= sizeof(registro))
                                   ? (TAM_BLOCO_CALC / sizeof(registro))
                                   : 0;

// Calcula o espaço que sobra após alocar os registros (Agora é uma expressão constante)
const unsigned TAM_ESPACO_LIVRE_CALC = (NUM_REGISTROS_CALC > 0)
                                      ? (TAM_BLOCO_CALC - (NUM_REGISTROS_CALC * sizeof(registro)))
                                      : TAM_BLOCO_CALC; // Se nenhum registro cabe, tudo é espaço livre (caso extremo)

// Função para obter o tamanho real da página em runtime (pode ser usada em .cpp se necessário)
inline unsigned get_runtime_page_size() { return get_page_size_for_bloco(); }
// ------------------------------------------


// --- Definição da Estrutura Bloco (Tamanho Fixo) ---
// Garante que o compilador não adicione preenchimento extra (padding)
#pragma pack(push, 1)
struct bloco {
    // Array de tamanho fixo para os registros
    registro regs[NUM_REGISTROS_CALC];

    // Array de char para preencher o restante do bloco até TAM_BLOCO_CALC bytes
    // Garante que o bloco tenha o tamanho exato da página (4096 neste caso)
    char espaco_livre[TAM_ESPACO_LIVRE_CALC];

    // Construtor padrão (opcional, mas bom para inicializar)
    bloco() {
        // Zera toda a estrutura ao criar um bloco
        std::memset(this, 0, sizeof(bloco));
    }

    // --- Métodos (Declarados como static pois não dependem de 'this') ---
    // Você pode remover o 'static' e a implementação em bloco.cpp se preferir
    // que sejam funções globais normais. Mantive como static por enquanto.
    static bool eh_numero(const std::string& s);
    static bool ler_linha(std::ifstream &entrada, std::string &linha);
    static void separa_csv(const std::string &linha, std::vector<std::string> &campos);
    static void criar_arquivo_blocos();
    static void criar_arquivo_blocos_hash(size_t bucket_capacity = 2);
    static void criar_arquivo_blocos_hash_file(const std::string &arq_origem, size_t bucket_capacity = 2);
    static void processar_titulo_consistente(char* dest, const std::string& titulo_origem);
};
#pragma pack(pop) // Restaura o alinhamento padrão
std::string remover_aspas(std::string campo);


// Verificação em tempo de compilação (requer C++11 ou superior)
static_assert(sizeof(bloco) == TAM_BLOCO_CALC, "Tamanho do struct bloco está incorreto (não é 4096)!");
static_assert(NUM_REGISTROS_CALC > 0, "Cálculo de NUM_REGISTROS_CALC resultou em 0!");


// --- Declarações Externas Removidas ---
// As variáveis globais serão definidas e calculadas em bloco.cpp
// extern unsigned tam_bloco;
// extern unsigned num_registros;
// extern unsigned tam_espaco_livre;
// extern bool bloco_initialized;
// void initialize_bloco_calculations();
// unsigned get_tam_bloco();
// unsigned get_num_registros();
// unsigned get_tam_espaco_livre();

#endif // BLOCO_H

