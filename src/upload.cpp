#include "bloco.h"
#include <iostream>

int main(int argc, char** argv){
    if(argc < 2){
        std::cerr << "Usage: upload <csv-path>\n";
        return 1;
    }
    std::string path = argv[1];
    bloco b;
    // chama a versão que recebe path
    b.criar_arquivo_blocos_hash_file(path, 2);
    return 0;
}
