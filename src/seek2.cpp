#include <iostream>

int main(int argc, char** argv){
    if(argc < 2){
        std::cerr << "Usage: seek2 <exact-title>\n";
        return 1;
    }
    std::cout << "seek2: functionality not implemented yet. Received title='" << argv[1] << "'\n";
    return 0;
}
