#include <iostream>
#include <cstdint>

struct registro {
    // quero verificar algumas coisas ainda como qual podem obter o valor NULL
    // isso pode ser tratado de colocando um bool com o nome do tipo
    // bora deixar um padrao tipo null_[nome do campo]

    // maior que o uint16_t e nao tem uint24_t entao vai ser um uint32_t que
    // e um unsigned
    unsigned id;

    // tamanho do titulo, so vai ate 300
    std::uint16_t tam_titulo; 
    char* titulo;
    
    // usando uint16_t em alguns desses pois nao sao negativos e nao vao ate muito
    std::uint16_t ano;      

    // so vai ate 150
    std::uint8_t tam_autores;
    char* autores;

    // citacoes ele nao limita na descricao mas acho que nao deve ser tao grande
    // por precaucao coloquei como unsigned so
    unsigned citacoes;  

    char data[20];


    // esse eu tenho certeza que vai ter null, vi no arquivo
    bool null_snippet;
    std::uint16_t tam_snippet;
    char* snieppet;
};