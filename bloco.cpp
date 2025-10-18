#include "registro.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <cstring>

#define TAM_BLOCO 4096

struct bloco {
    registro regs[2];
    char espaco_livre[1078];

    int tam_bloco() { return TAM_BLOCO; }

    void criar_arquivo_blocos() {
        std::cout << "[INPUT] Insira o nome do arquivo de entrada (deixe ele no mesmo diretório [essa parte de estar no mesmo diretório é só pra testes inciais ta?]): ";
        std::string arq_origem;
        std::cin >> arq_origem;

        std::ifstream origem(arq_origem);
        if (!origem.is_open()) {
            std::cerr << "[ERROR] Não foi possível abrir o arquivo de origem: " 
                      << arq_origem << std::endl;
            return;
        }

        std::cout << "[INFO] Arquivo de origem aberto com sucesso: " << arq_origem << std::endl;
        
        std::string arq_destino = "dados.in";
        std::ofstream destino(arq_destino, std::ios::binary);
        if (!destino.is_open()) {
            std::cerr << "[ERROR] Não foi possível criar o arquivo de destino: " 
                      << arq_destino << std::endl;
            origem.close();
            return;
        }

        std::cout << "[INFO] Arquivo de destino criado com sucesso: " << arq_destino << std::endl;

        bloco b;
        std::memset(&b, 0, sizeof(bloco));  // inicializando o bloco
        int ind = 0;        // indice para saber em que posicao do bloco colocar o registro
        std::string linha;
        int linha_num = 0;
        
        while (std::getline(origem, linha)) {
            linha_num++;
            if (linha.empty()) {
                std::cout << "[WARN] Linha " << linha_num << " está vazia, pulando...\n";
                continue;
            }

            std::istringstream stream(linha);
            std::string campo;
            registro reg;
            reg.null_snippet = false;

            // id
            std::getline(stream, campo, '\t');
            std::cout << "[DEBUG] Linha " << linha_num << ", campo ID: " << campo << std::endl;
            if (campo.empty()) {
                std::cerr << "[ERROR] Linha " << linha_num << ": campo ID vazio, pulando linha\n";
                continue;
            }
            reg.id = std::stoul(campo);

            // titulo
            std::getline(stream, campo, '\t');
            std::cout << "[DEBUG] Linha " << linha_num << ", campo Titulo: " << campo << std::endl;
            std::strncpy(reg.titulo, campo.c_str(), 300);
            reg.titulo[300] = '\0';

            // ano
            std::getline(stream, campo, '\t');
            std::cout << "[DEBUG] Linha " << linha_num << ", campo Ano: " << campo << std::endl;
            if (campo.empty()) {
                std::cerr << "[WARN] Linha " << linha_num << ": campo Ano vazio, usando 0\n";
                reg.ano = 0;
            } else {
                reg.ano = static_cast<std::uint16_t>(std::stoi(campo));
            }

            // autores
            std::getline(stream, campo, '\t');
            std::cout << "[DEBUG] Linha " << linha_num << ", campo Autores: " << campo << std::endl;
            std::strncpy(reg.autores, campo.c_str(), 150);
            reg.autores[150] = '\0';

            // citacoes
            std::getline(stream, campo, '\t');
            std::cout << "[DEBUG] Linha " << linha_num << ", campo Citacoes: " << campo << std::endl;
            if (campo.empty()) {
                std::cerr << "[WARN] Linha " << linha_num << ": campo Citacoes vazio, usando 0\n";
                reg.citacoes = 0;
            } else {
                reg.citacoes = std::stoul(campo);
            }

            // data
            std::getline(stream, campo, '\t');
            std::cout << "[DEBUG] Linha " << linha_num << ", campo Data: " << campo << std::endl;
            std::strncpy(reg.data, campo.c_str(), 20);
            reg.data[19] = '\0';

            // snippet
            if (std::getline(stream, campo, '\t')) {
                if (campo.length() < 100) {
                    reg.null_snippet = true;
                    reg.snippet[0] = '\0';
                    std::cout << "[INFO] Linha " << linha_num << ": snippet muito curto, marcado como null\n";
                } else {
                    std::strncpy(reg.snippet, campo.c_str(), 1024);
                    reg.snippet[1024] = '\0';
                }
            } else {
                reg.null_snippet = true;
                reg.snippet[0] = '\0';
                std::cout << "[INFO] Linha " << linha_num << ": snippet ausente, marcado como null\n";
            }

            b.regs[ind] = reg;
            ind++;

            if (ind == 2) {
                std::memset(b.espaco_livre, 0, sizeof(b.espaco_livre));
                destino.write(reinterpret_cast<char*>(&b), sizeof(bloco));
                std::cout << "[INFO] Bloco escrito com 2 registros (linha " << linha_num << ")\n";
                ind = 0;
                std::memset(&b, 0, sizeof(bloco));
            }
        }

        // caso ainda tenha espaço de sobra no bloco mas não tenha arquivo
        if (ind > 0) {
            std::memset(b.espaco_livre, 0, sizeof(b.espaco_livre));
            if (ind == 1) b.regs[1] = registro(); // registro vazio na posição 1
            destino.write(reinterpret_cast<char*>(&b), sizeof(bloco));
        }

        // quando termina a leitura
        std::cout << "Arquivo 'dados.in' criado com sucesso!\n";

        // fechando os arquivos
        origem.close();
        destino.close();
    }
};

int main(){
    bloco b;
    b.criar_arquivo_blocos();
}
