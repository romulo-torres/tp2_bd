#include "registro.h"
#include "bloco.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <cstring>
#include <vector>

#define TAM_BLOCO 4096

int bloco::tam_bloco() { return TAM_BLOCO; }

/* função auxiliar para tirar as aspas para processamento durante a leitura do arquivo */
std::string remover_aspas(std::string campo){
    if(campo.size() >= 2 && campo.front() == '"' && campo.back() == '"'){
        return campo.substr(1,campo.size() - 2);
    }
    return campo;
}

/* função auxiliar para saber se é número */
bool bloco::eh_numero(const std::string& s) {
    if (s.empty()) return false;
    for (char c : s) if (!isdigit(c)) return false;
    return true;
}

/* função auxiliar para ler uma linha */
bool bloco::ler_linha(std::ifstream &entrada, std::string &linha){
    linha.clear(); // esvaziando a string para evitar erros por leituras passadas
    std::string temp;
    bool dentro_aspas = false;

    while (std::getline(entrada, temp)) {
    linha += temp;  // adiciona a linha lida à variável acumuladora

    // contando aspas pra saber se está entre um campo entre aspas
    int aspas = 0;
    for (char c : temp){ 
        if (c == '"'){ 
            aspas++;
        }
    }
    if (aspas % 2 != 0){
        dentro_aspas = !dentro_aspas;
    }
    if (!dentro_aspas) break;  // linha completa lida, sai do loop

    linha += '\n'; // mantém o '\n' dentro de um campo entre aspas
    }
    return !linha.empty();
}

/* divive a linha nos campos, também considerando as aspas */
void bloco::separa_csv(const std::string &linha, std::vector<std::string> &campos) {
    campos.clear();
    std::string campo;
    bool dentro_aspas = false;

    for(size_t i = 0; i < linha.size(); ++i){
        char c = linha[i];
        if(c == '"') dentro_aspas = !dentro_aspas;
        else if(c == ';' && !dentro_aspas){
            campos.push_back(campo);
            campo.clear();
        }
        else campo += c;
    }
    campos.push_back(campo);
}


/* função para ler o arquivo csv e então criar o arquivo de blocos com nome dados.in
bom falar que não é o arquvo de dados que pede para hash, não organizei de maneira nenhuma por hash, só é um arquivo com blocos de registro*/
void bloco::criar_arquivo_blocos() {
    // versão final tem que ler o arquivo la na pasta /data quando o repositório tiver o formato certo da especificação
    std::cout << "[INPUT] Insira o nome do arquivo de entrada (deixe ele no mesmo diretório [essa parte de estar no mesmo diretório é só pra testes inciais ta?]): ";
    std::string arq_origem;
    std::cin >> arq_origem;

    std::ifstream entrada(arq_origem);
    if(!entrada.is_open()){
        std::cerr << "[ERROR] Não foi possível abrir o arquivo de origem\n";
        return;
    }
    
    // o arquivo de dados ficara na /data na versão final
    std::string arq_destino = "dados.in";
    std::ofstream destino(arq_destino, std::ios::binary);
    if (!destino.is_open()) {
        std::cerr << "[ERROR] Não foi possível criar o arquivo de destino\n" << arq_destino << std::endl;
        return;
    }

    std::cout << "[INFO] Arquivo de destino criado com sucesso: " << arq_destino << std::endl;

    bloco b;
    std::memset(&b, 0, sizeof(bloco));  // inicializando o bloco
    int ind = 0;        // indice para saber em que posicao do bloco colocar o registro
    int linha_num = 0;


    std::string linha;
    
    while(ler_linha(entrada,linha)) {
        linha_num++;
        registro reg;
        reg.null_snippet = false;

        std::vector<std::string> campos;
        separa_csv(linha, campos);

        if(campos.size() < 7){
            std::cerr << "[AVISO] Linha " << linha_num << " incompleta com algum campo a menos, ignorada.\n";
            continue;
        }

        for(std::string &c : campos){
            c = remover_aspas(c);
        }


        reg.id = eh_numero(campos[0]) ? std::stoul(campos[0]) : 0;
        if (campos[1].length() > 300) {
            reg.titulo[0] = '\0';  // se for mt grande só é inserido um '\0'
        } else {
            // caso esteja dentro do estipulado sera inserido normalmente
            std::strncpy(reg.titulo, campos[1].c_str(), sizeof(reg.titulo) - 1);
            reg.titulo[sizeof(reg.titulo) - 1] = '\0';
        }
        reg.ano = eh_numero(campos[2]) ? std::stoi(campos[2]) : 0;
        if (campos[3].length() > 150) {
            reg.autores[0] = '\0';  // se for mt grande só é inserido um '\0'
        } else {
            // caso esteja dentro do estipulado sera inserido normalmente
            std::strncpy(reg.autores, campos[3].c_str(), sizeof(reg.autores) - 1);
            reg.autores[sizeof(reg.autores) - 1] = '\0';
        }
                reg.citacoes = eh_numero(campos[4]) ? std::stoul(campos[4]) : 0;
        std::strncpy(reg.data, campos[5].c_str(), sizeof(reg.data)-1); reg.data[sizeof(reg.data)-1]='\0';

        if(campos[6].length() < 100 || campos[6].length() > 1024){ reg.null_snippet = true; reg.snippet[0]='\0'; }
        else{ std::strncpy(reg.snippet, campos[6].c_str(), sizeof(reg.snippet)-1); reg.snippet[sizeof(reg.snippet)-1]='\0'; }

        b.regs[ind++] = reg;

        if(ind == 2){
            std::memset(b.espaco_livre, 0, sizeof(b.espaco_livre));
            destino.write(reinterpret_cast<char*>(&b), sizeof(bloco));
            std::cout << "[INFO] Bloco escrito com 2 registros (linha " << linha_num << ")\n";
            ind = 0;
            std::memset(&b, 0, sizeof(bloco));
        }
    }

    if (ind > 0) {
        std::memset(b.espaco_livre, 0, sizeof(b.espaco_livre));
        if (ind == 1){
            std::memset(&b.regs[1], 0, regs[0].tamanho_registro());
        };
        destino.write(reinterpret_cast<char*>(&b), sizeof(bloco));
    }

    std::cout << "[INFO] Arquivo 'dados.in' preenchido com sucesso!\n";
    destino.close();
}

// int main(){
//     bloco b;
//     b.criar_arquivo_blocos();

//     return 0;
// }
