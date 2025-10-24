#include <iostream>

int main(int argc, char** argv){
    if(argc < 2){
        std::cerr << "Usage: findrec <id>\n";
        return 1;
    }
    std::cout << "findrec: functionality not implemented yet. Received id='" << argv[1] << "'\n";
    return 0;
}
