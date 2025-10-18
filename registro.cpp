#include <iostream>
#include <cstdint>
#include <cstring>


#define tam_registro 1509


#pragma pack(push, 1)

struct registro {
    /* quero verificar algumas coisas ainda como qual podem obter o valor NULL
     isso pode ser tratado de colocando um bool com o nome do tipo
     bora deixar um padrao tipo null_[nome do campo] */

    /* maior que o uint16_t e nao tem uint24_t entao vai ser um uint32_t que
     e um unsigned */
    unsigned id;

    /* tamanho do titulo, so vai ate 300 */
    char titulo[301];
    
    /* usando uint16_t em alguns desses pois nao sao negativos e nao vao ate muito */
    std::uint16_t ano;      

    /* so vai ate 150 */
    char autores[151];

    /* citacoes ele nao limita na descricao mas acho que nao deve ser tao grande
     por precaucao coloquei como unsigned so */
    unsigned citacoes;  

    /* data sempre é um char de 20 caracteres */
    char data[20];

    /* esse eu tenho certeza que vai ter null, vi no arquivo */
    bool null_snippet;
    char snippet[1025];

    /* função para criar um objeto de um registro */
    registro(unsigned id_, const char* titulo_, std::uint16_t ano_, const char* autores_, 
             unsigned citacoes_, const char* data_, bool null_snippet_, const char* snippet_) {
        std::memset(this, 0, sizeof(registro));
        
        id = id_;
        ano = ano_;
        citacoes = citacoes_;
        null_snippet = null_snippet_;
        
        // Título
        if (titulo_ != nullptr) {
            std::strncpy(titulo, titulo_, 300);
            titulo[300] = '\0';
        }
        
        // Autores
        if (autores_ != nullptr) {
            std::strncpy(autores, autores_, 150);
            autores[150] = '\0';
        }
        
        // Data
        if (data_ != nullptr) {
            std::strncpy(data, data_, 20);
            data[20] = '\0';
        }
        
        // Snippet
        if (!null_snippet && snippet_ != nullptr) {
            std::size_t len = std::strlen(snippet_);
            if (len > 100 && len < 1024) {
                std::strncpy(snippet, snippet_, 1024);
                snippet[1024] = '\0';
            } else {
                null_snippet = true;
            }
        }
    }


    /* função para retornar o tamanho real */
    int tamanho_registro(){
        return tam_registro;
    }

};