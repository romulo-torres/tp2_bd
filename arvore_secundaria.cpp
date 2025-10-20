#include "registro.h"
#include "bloco.h"
#include <iostream>
#include <cstring>

#define TAM_INDICE 4096;

struct no_intermediario {
    char chaves[12][300];
    no_intermediario* descendentes[13];
};

struct no_folha {
    char chaves[12][300];
    bloco* blocos[12];
    no_folha* prox;
};

/* o tamanho do nó será */

struct arvore_secundaria{



};