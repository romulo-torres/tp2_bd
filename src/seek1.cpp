#include <iostream>

int main(int argc, char** argv){
    if(argc < 2){
        std::cerr << "Usage: seek1 <id>\n";
        return 1;
    }
    std::cout << "seek1: functionality not implemented yet. Received id='" << argv[1] << "'\n";
    return 0;
}
