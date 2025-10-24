#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <algorithm>
#include <iomanip> // Para formatar a saída do tempo

// Função auxiliar para contar a ocorrência de uma substring (ex: '";"') em uma string,
// simulando a funcionalidade de contagem de substrings do Python.
int count_substring(const std::string& str, const std::string& sub) {
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
void replace_all(std::string& str, const std::string& from, const std::string& to) {
    if(from.empty())
        return;
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Continua a busca após a string substituída
    }
}


int main() {
    // Definindo os nomes dos arquivos
    const std::string input_filename = "artigo.csv";
    const std::string output_filename = "artigo_novo.csv";

    // 1. Abertura dos arquivos
    std::ifstream arquivo(input_filename);
    std::ofstream arquivo2(output_filename);

    if (!arquivo.is_open()) {
        std::cerr << "Erro: Não foi possível abrir o arquivo de entrada (" << input_filename << "). "
                  << "Certifique-se de que ele exista." << std::endl;
        return 1;
    }
    if (!arquivo2.is_open()) {
        std::cerr << "Erro: Não foi possível criar/abrir o arquivo de saída (" << output_filename << ")." << std::endl;
        return 1;
    }

    // 2. Início do cronômetro
    auto inicio = std::chrono::high_resolution_clock::now();
    
    int quant_linhas = 0;
    std::string linha;

    // O C++ `getline` lê a linha, removendo o '\n' final.
    // O loop continua enquanto houver linhas para ler.
    while (std::getline(arquivo, linha)) {
        
        // Contamos a linha lida
        quant_linhas++;
        
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
                if (std::getline(arquivo, prox_linha)) {
                    
                    // Concatena a linha atual (que não tem \n por causa do getline) 
                    // com o substituto '\ n' e a próxima linha
                    linha += " \\n" + prox_linha;
                    
                    // Contamos a linha seguinte que foi lida e concatenada
                    quant_linhas++; 
                }
                // Se não conseguir ler a prox_linha (fim do arquivo), a linha é escrita como está.
            
            } else { 
                // Se tiver 6 ou mais ';', mas menos de 6 '";"', é porque tem elementos nulos (ex: ;;;;) ou erro de aspas.
                // O Python substitui ';;' por ';NULL;'. 
                replace_all(linha, ";;", ";NULL;");

                // Note: Esta lógica de substituição de nulos é executada apenas se 
                // a primeira verificação (quant_separacoes < 6) for verdadeira.
            }
        }

        // Escreve a linha processada no arquivo de saída.
        // Adicionamos o '\n' manualmente, pois o `getline` o removeu.
        arquivo2 << linha << "\n";
    }

    // 3. Fim do cronômetro
    auto fim = std::chrono::high_resolution_clock::now();
    
    // Calcula a duração
    std::chrono::duration<double> duracao_segundos = fim - inicio;

    // 4. Fechamento dos arquivos
    arquivo.close();
    arquivo2.close();

    // 5. Impressão dos resultados
    std::cout << "Duração: " << std::fixed << std::setprecision(2) 
              << duracao_segundos.count() << " segundos" << std::endl;
    std::cout << "Numero de linhas totais: " << quant_linhas << std::endl;

    return 0;
}
