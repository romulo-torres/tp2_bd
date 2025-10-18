#ifndef REGISTRO_H
#define REGISTRO_H

#include <cstdint>
#include <cstring>

#define TAM_REGISTRO 1509

#pragma pack(push, 1)
struct registro {
    unsigned id;
    char titulo[301];
    std::uint16_t ano;
    char autores[151];
    unsigned citacoes;
    char data[20];
    bool null_snippet;
    char snippet[1025];

    // Construtores e métodos (apenas assinaturas)
    registro();
    registro(unsigned id_, const char* titulo_, std::uint16_t ano_,
             const char* autores_, unsigned citacoes_, const char* data_,
             bool null_snippet_, const char* snippet_);

    int tamanho_registro();
};
#pragma pack(pop)

#endif
