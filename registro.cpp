#include "registro.h"
#include <cstring>

registro::registro() {
    id = 0;
    std::memset(titulo, 0, sizeof(titulo));
    ano = 0;
    std::memset(autores, 0, sizeof(autores));
    citacoes = 0;
    std::memset(data, 0, sizeof(data));
    null_snippet = false;
    std::memset(snippet, 0, sizeof(snippet));
}

registro::registro(unsigned id_, const char* titulo_, std::uint16_t ano_,
                   const char* autores_, unsigned citacoes_, const char* data_,
                   bool null_snippet_, const char* snippet_) {
    id = id_;
    ano = ano_;
    citacoes = citacoes_;
    null_snippet = null_snippet_;

    std::memset(titulo, 0, sizeof(titulo));
    std::memset(autores, 0, sizeof(autores));
    std::memset(data, 0, sizeof(data));
    std::memset(snippet, 0, sizeof(snippet));

    if (titulo_) std::strncpy(titulo, titulo_, 300);
    if (autores_) std::strncpy(autores, autores_, 150);
    if (data_) std::strncpy(data, data_, 20);

    if (!null_snippet && snippet_) {
        std::size_t len = std::strlen(snippet_);
        if (len >= 100 && len < 1024) {
            std::strncpy(snippet, snippet_, 1024);
        } else {
            null_snippet = true;
        }
    }
}
