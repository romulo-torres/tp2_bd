#include "registro.h"
#include "bloco.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <cstring>
#include <vector>
#include <algorithm>

unsigned tam_bloco = 0;
unsigned tam_espaco_livre = 0;
unsigned num_registros = 0;


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

static int count_substring(const std::string& str, const std::string& sub) {
    if (sub.length() == 0) {
        return 0;
    }
    int count = 0;
    // Encontra a substring, começando do último ponto encontrado + o tamanho da substring
    for (size_t offset = str.find(sub); offset != std::string::npos;
         offset = str.find(sub, offset + sub.length())) {
        ++count;
    }
    return count;
}

// Função auxiliar para substituir todas as ocorrências de uma substring (ex: ';;')
static void replace_all(std::string& str, const std::string& from, const std::string& to) {
    if(from.empty())
        return;
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Continua a busca após a string substituída
    }
}

/* função auxiliar para ler uma linha */
bool bloco::ler_linha(std::ifstream &entrada, std::string &linha){

    //linha.clear(); // esvaziando a string para evitar erros por leituras passadas

    //linha += "\n"; // mantém o '\n' dentro de um campo entre aspas
    // 1. Contar '";"' (separador completo)
    // O ideal é ter 6 separadores para 7 campos.
    int quant_separacoes = count_substring(linha, "\";\"");

    if (quant_separacoes < 6) { // Se tem menos do que 6, é porque pode haver elementos vazios ou quebra de linha
        
        // 2. Contar ";" (separador simples, usado para verificar quebra de linha)
        int quant_separacoes2 = std::count(linha.begin(), linha.end(), ';');
        
        if (quant_separacoes2 < 6) { // Se tiver menos que 6, que é o normal, então houve quebra de linha
            
            // O código Python lê a próxima linha e concatena, substituindo o \n
            // por ' \ n' (espaço, barra, n) no processo de união.
            std::string prox_linha;
            
            // Tentamos ler a próxima linha física
            if (std::getline(entrada, prox_linha)) {
                
                // Concatena a linha atual (que não tem \n por causa do getline) 
                // com o substituto '\ n' e a próxima linha
                linha += " \\n" + prox_linha;
            }
            // Se não conseguir ler a prox_linha (fim do arquivo), a linha é escrita como está.
            else{
                return false;
            }
        } 
        else { 
            // Se tiver 6 ou mais ';', mas menos de 6 '";"', é porque tem elementos nulos (ex: ;;;;) ou erro de aspas.
            // O Python substitui ';;' por ';NULL;'. 
            replace_all(linha, ";;", ";NULL;");

            // Note: Esta lógica de substituição de nulos é executada apenas se 
            // a primeira verificação (quant_separacoes < 6) for verdadeira.
        }
    }
    linha += '\n';
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
    }\
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
    int ind = 0;        // indice para saber em que posicao do bloco colocar o registro
    int linha_num = 0;
    std::string linha;

    
    while(std::getline(entrada, linha)) {
        ler_linha(entrada, linha); // Lendo a linha e tratando os erro

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

        if(ind == num_registros){
            std::memset(b.espaco_livre, 0, tam_espaco_livre);
            destino.write(reinterpret_cast<char*>(b.regs), num_registros * sizeof(registro));
            destino.write(reinterpret_cast<char*>(b.espaco_livre), tam_espaco_livre);
            //std::cout << "[INFO] Bloco escrito com " << num_registros << " registros (linha " << linha_num << ")\n";
            ind = 0;
            std::memset(b.espaco_livre, 0, tam_espaco_livre);
        }
    }

    if (ind > 0) {
        std::memset(b.espaco_livre, 0, tam_espaco_livre);
        destino.write(reinterpret_cast<char*>(b.regs), ind * sizeof(registro));
        destino.write(reinterpret_cast<char*>(b.espaco_livre), tam_espaco_livre);
        std::cout << "[INFO] Bloco escrito incompleto com somente " << ind << " registros\n";
    }

    std::cout << "[INFO] Arquivo 'dados.in' preenchido com sucesso!\n";
    destino.close();
}

/* se descomentar isso da pra testar */

// int main(){
//    bloco b;
//    b.criar_arquivo_blocos();
//
//    return 0;
//}
