#include "registro.cpp"

// tamanho fixo de bloco
#define TAM_BLOCO 8192

/* esse bloco terá o tamanho fixo de 8096 Bytes, terá 5 registros por bloco
para cada registro havera um parametro que será o offset para saber em que lugar no bloco */

struct bloco {
    registro reg1;
    registro reg2;
};