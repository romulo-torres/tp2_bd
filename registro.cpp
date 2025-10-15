#include <iostream>
#include <cstdint>
#include <cstring>

struct registro {
    /* quero verificar algumas coisas ainda como qual podem obter o valor NULL
     isso pode ser tratado de colocando um bool com o nome do tipo
     bora deixar um padrao tipo null_[nome do campo] */

    /* maior que o uint16_t e nao tem uint24_t entao vai ser um uint32_t que
     e um unsigned */
    unsigned id;

    /* tamanho do titulo, so vai ate 300 */
    std::uint16_t tam_titulo; 
    char* titulo;
    
    /* usando uint16_t em alguns desses pois nao sao negativos e nao vao ate muito */
    std::uint16_t ano;      

    /* so vai ate 150 */
    std::uint8_t tam_autores;
    char* autores;

    /* citacoes ele nao limita na descricao mas acho que nao deve ser tao grande
     por precaucao coloquei como unsigned so */
    unsigned citacoes;  

    /* data sempre é um char de 20 caracteres */
    char data[20];

    /* esse eu tenho certeza que vai ter null, vi no arquivo */
    bool null_snippet;
    std::uint16_t tam_snippet;
    char* snippet;

    /* função para criar um objeto de um registro */
        registro(unsigned id_,
             const char* titulo_,
             std::uint16_t ano_,
             const char* autores_,
             unsigned citacoes_,
             const char* data_,
             bool null_snippet_,
             const char* snippet_)
        : id(id_),
          ano(ano_),
          citacoes(citacoes_),
          null_snippet(null_snippet_) 
    {
        // título
        tam_titulo = std::strlen(titulo_);
        titulo = new char[tam_titulo + 1];
        std::strcpy(titulo, titulo_);

        // autores
        tam_autores = std::strlen(autores_);
        autores = new char[tam_autores + 1];
        std::strcpy(autores, autores_);

        // data (fixa em 20 chars)
        std::strncpy(data, data_, 20);
        data[19] = '\0'; // garantir fim da string

        // snippet
        if (!null_snippet && snippet_ != nullptr) {
            tam_snippet = std::strlen(snippet_);
            snippet = new char[tam_snippet + 1];
            std::strcpy(snippet, snippet_);
        } else {
            tam_snippet = 0;
            snippet = nullptr;
        }
    }

    /* função para retornar o tamanho real */
    int tamanho_registro(){
        return sizeof(id) + sizeof(tam_titulo) + sizeof(char)*tam_titulo + sizeof(std::uint16_t) + sizeof(tam_autores)
         + sizeof(char)*tam_autores +  sizeof(citacoes) + sizeof(char)*20 + sizeof(null_snippet) + sizeof(tam_snippet) + sizeof(char)*tam_snippet;
    }

};