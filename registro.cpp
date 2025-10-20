#include <iostream>
#include <cstdint>
#include <cstring>
#include "registro.h"



registro::registro()
    : id(0), ano(0), citacoes(0), null_snippet(false)
{
    titulo[0] = '\0';
    autores[0] = '\0';
    data[0] = '\0';
    snippet[0] = '\0';
}


// construtor completo
registro::registro(unsigned id_,
                   const char* titulo_,
                   std::uint16_t ano_,
                   const char* autores_,
                   unsigned citacoes_,
                   const char* data_,
                   bool null_snippet_,
                   const char* snippet_)
    : id(id_), ano(ano_), citacoes(citacoes_), null_snippet(null_snippet_)
{
    // título
    std::strncpy(titulo, titulo_, 300);
    titulo[300] = '\0';

    // autores
    std::strncpy(autores, autores_, 150);
    autores[150] = '\0';

    // data (fixa em 20 chars)
    std::strncpy(data, data_, 19);
    data[19] = '\0';

    // snippet
    if (!null_snippet_ && snippet_ != nullptr) {
        std::strncpy(snippet, snippet_, 1024);
        snippet[1024] = '\0';
    } else {
        snippet[0] = '\0';
    }
}

