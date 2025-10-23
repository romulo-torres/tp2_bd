    #ifndef BLOCO_H
    #define BLOCO_H

    #include "registro.h"
    #include <fstream>
    #include <iostream>
    #include <sstream>
    #include <cstring>
    #include <vector>
    #include <unistd.h>

    extern unsigned tam_bloco;
    extern unsigned tam_espaco_livre;
    extern unsigned num_registros;

    struct bloco {
        registro* regs;
        char* espaco_livre;

        bloco(){
            if(tam_bloco == 0){
                tam_bloco = getpagesize();
                num_registros = tam_bloco / TAM_REGISTRO;
                tam_espaco_livre = tam_bloco - num_registros * TAM_REGISTRO;
            }

            regs = new registro[num_registros];
            espaco_livre = new char[tam_espaco_livre];
            std::memset(espaco_livre, 0, tam_espaco_livre);
        }

        ~bloco(){
            delete[] regs;
            delete[] espaco_livre;
        }

        // métodos 
        bool eh_numero(const std::string& s);

        bool ler_linha(std::ifstream &entrada, std::string &linha);

        void separa_csv(const std::string &linha, std::vector<std::string> &campos);

        void criar_arquivo_blocos();
        // cria arquivo usando hash extensivo (bucket_capacity = capacidade do bucket)
        void criar_arquivo_blocos_hash(size_t bucket_capacity = 2);
    };


    #endif
