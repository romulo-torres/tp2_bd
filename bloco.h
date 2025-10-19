#ifndef BLOCO_H
#define BLOCO_H

#include "registro.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <cstring>
#include <vector>

#define TAM_BLOCO 4096

struct bloco {
    registro regs[2];
    char espaco_livre[1078];

    // métodos 
    int tam_bloco();

    bool eh_numero(const std::string& s);

    bool ler_linha(std::ifstream &entrada, std::string &linha);

    void separa_csv(const std::string &linha, std::vector<std::string> &campos);

    void criar_arquivo_blocos();

    int tamanho_registro();
};


#endif
