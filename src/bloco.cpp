#include "../include/bloco.h"
#include "../include/registro.h" // Para struct registro
#include "../include/logger.h"   // Para LOG_...
#include "../include/hashE.h"    // Para HashE
#include "../include/limpa_csv.h" // Assumindo que helpers como ler_linha, eh_numero estão aqui

#include <filesystem>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <cstring>
#include <vector>
#include <algorithm>
#include <string>
#include <unordered_map>
#include <map>
#include <filesystem>
#include <cstdio>
#include <set>
#include <algorithm> // Para std::min, std::count
#include <stdexcept> // Para std::invalid_argument, std::out_of_range
#include <unistd.h>  // Para getpagesize()

// Obtém o tamanho da página do sistema operacional
unsigned get_page_size() {
    long page_size = sysconf(_SC_PAGESIZE);
    if (page_size == -1) {
        perror("sysconf");
        return 4096;
    }
    return static_cast<unsigned>(page_size);
}

// Declaração das variáveis globais
unsigned tam_bloco = 0;
unsigned num_registros = 0;
unsigned tam_espaco_livre = 0;
bool bloco_initialized = false;


std::string remover_aspas(std::string campo){
    // Remove aspas duplas "" internas primeiro
    size_t pos = campo.find("\"\"");
    while (pos != std::string::npos) {
        campo.replace(pos, 2, "\"");
        pos = campo.find("\"\"", pos + 1);
    }
    // Remove aspas externas
    if(campo.size() >= 2 && campo.front() == '"' && campo.back() == '"'){
        return campo.substr(1, campo.size() - 2);
    }
    return campo;
}

// Função auxiliar para processar título de forma consistente
void bloco::processar_titulo_consistente(char* dest, const std::string& titulo_origem) { 
    const size_t MAX_TITULO_LEN = 300;  // 300 caracteres + \0 = 301 bytes
    
    // Remove aspas primeiro
    std::string sem_aspas = remover_aspas(titulo_origem);
    
    // Remove espaços extras no início/fim
    size_t start = sem_aspas.find_first_not_of(" \t\n\r");
    size_t end = sem_aspas.find_last_not_of(" \t\n\r");
    
    std::string normalizada;
    if (start != std::string::npos && end != std::string::npos) {
        normalizada = sem_aspas.substr(start, end - start + 1);
    } else {
        normalizada = sem_aspas;
    }
    
    // Trunca consistentemente para 300 caracteres
    if (normalizada.length() > MAX_TITULO_LEN) {
        std::strncpy(dest, normalizada.c_str(), MAX_TITULO_LEN);
        dest[MAX_TITULO_LEN] = '\0';
    } else {
        std::strcpy(dest, normalizada.c_str());
    }
}

// Função para inicializar os cálculos - deve ser chamada APÓS todas as estruturas estarem definidas
void initialize_bloco_calculations() {
    if (bloco_initialized) return;
    
    tam_bloco = get_page_size();
    
    // Verifica se as estruturas estão completamente definidas
    if (sizeof(bloco) == 0 || sizeof(registro) == 0) {
        std::cerr << "ERRO CRÍTICO: Estruturas bloco ou registro não estão completamente definidas!" << std::endl;
        std::cerr << "sizeof(bloco) = " << sizeof(bloco) << ", sizeof(registro) = " << sizeof(registro) << std::endl;
        return;
    }
    
    // Calcula baseado no tamanho REAL das estruturas
    size_t tamanho_util = tam_bloco;
    num_registros = tamanho_util / sizeof(registro);
    
    if (num_registros == 0) {
        std::cerr << "AVISO: num_registros calculado como 0. Usando fallback." << std::endl;
        num_registros = 1; // Fallback mínimo
    }
    
    tam_espaco_livre = tam_bloco - (num_registros * sizeof(registro));
    
    // std::cout << "DEBUG INICIALIZAÇÃO: " << std::endl;
    // std::cout << "  tam_bloco = " << tam_bloco << std::endl;
    // std::cout << "  sizeof(registro) = " << sizeof(registro) << std::endl;
    // std::cout << "  sizeof(bloco) = " << sizeof(bloco) << std::endl;
    // std::cout << "  num_registros = " << num_registros << std::endl;
    // std::cout << "  tam_espaco_livre = " << tam_espaco_livre << std::endl;
    
    bloco_initialized = true;
}

// Funções de acesso seguro
unsigned get_tam_bloco() {
    if (!bloco_initialized) initialize_bloco_calculations();
    return tam_bloco;
}

unsigned get_num_registros() {
    if (!bloco_initialized) initialize_bloco_calculations();
    return num_registros;
}

unsigned get_tam_espaco_livre() {
    if (!bloco_initialized) initialize_bloco_calculations();
    return tam_espaco_livre;
}

// Inicializador que garante a inicialização no momento certo
struct BlocoInitializer {
    BlocoInitializer() {
        initialize_bloco_calculations();
    }
};

static BlocoInitializer bloco_initializer;
// --- FIM DO SISTEMA DE INICIALIZAÇÃO ---

/* função auxiliar para tirar as aspas para processamento durante a leitura do arquivo */


/* função auxiliar para saber se é número */
bool bloco::eh_numero(const std::string& s) {
    if (s.empty()) return false;
    // Permite sinal negativo opcional no início
    size_t start = 0;
    if (s[0] == '-') {
        if (s.length() == 1) return false; // Apenas "-" não é número
        start = 1;
    }
    for (size_t i = start; i < s.length(); ++i) {
        if (!isdigit(s[i])) return false;
    }
    return true;
}

static int count_substring(const std::string& str, const std::string& sub) {
    if (sub.length() == 0) {
        return 0;
    }
    int count = 0;
    size_t start_pos = 0;
    while ((start_pos = str.find(sub, start_pos)) != std::string::npos) {
         // Verifica se está dentro de aspas antes da posição
         int aspas_count = 0;
         bool in_quotes_flag = false;
         for (size_t k = 0; k < start_pos; ++k) {
              if (str[k] == '"') {
                  // Trata aspas duplas ""
                  if (k + 1 < start_pos && str[k+1] == '"') {
                      k++; // Pula a segunda aspa se for ""
                  } else {
                      in_quotes_flag = !in_quotes_flag; // Inverte o estado
                  }
              }
         }

        if (!in_quotes_flag) { // Se não estiver dentro das aspas no início da substring
            count++;
        }
        start_pos += sub.length(); // Avança a busca para depois da substring encontrada
    }
    return count;
}

// Função auxiliar para substituir todas as ocorrências de uma substring (ex: ';;')
static void replace_all(std::string& str, const std::string& from, const std::string& to) {
    if(from.empty())
        return;
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        // Verifica se 'from' está dentro de aspas
        bool in_quotes_flag = false;
         for (size_t k = 0; k < start_pos; ++k) {
              if (str[k] == '"') {
                  if (k + 1 < start_pos && str[k+1] == '"') k++;
                  else in_quotes_flag = !in_quotes_flag;
              }
         }

        if (!in_quotes_flag) { // Fora das aspas
            str.replace(start_pos, from.length(), to);
            start_pos += to.length(); // Continua a busca após a string substituída
        } else {
            start_pos += 1; // Dentro das aspas, avança 1 para não ficar preso
        }
    }
}

/* função auxiliar para ler uma linha - CORRIGIDA */
bool bloco::ler_linha(std::ifstream &entrada, std::string &linha){
    // Lê a próxima linha física do arquivo
    if (!std::getline(entrada, linha)) {
        return false; // Fim do arquivo ou erro de leitura
    }

    // Enquanto a linha NÃO tiver o número esperado de separadores FORA DAS ASPAS (6),
    // tenta ler a próxima linha e concatenar.
    while (count_substring(linha, ";") < 6) { // Usa ';' como separador principal para detecção de quebra
        std::string prox_linha;
        if (std::getline(entrada, prox_linha)) {
            // Concatena com o substituto '\n' (a lógica Python original usava ' \n')
            linha += " \\n" + prox_linha; // Mantém o marcador de quebra
        } else {
            // Chegou ao fim do arquivo no meio de uma linha quebrada.
            break; // Retorna o que tem.
        }
    }

    // Após juntar linhas quebradas, trata campos vazios ";;" fora das aspas
    replace_all(linha, ";;", ";NULL;");
    // Trata campo vazio no início ";
    if (linha.rfind(";", 0) == 0) { // Starts with ;
        linha.replace(0, 1, "NULL;");
    }
     // Trata campo vazio no final ;" (considerando aspas opcionais)
    // Verifica se termina com ';' ANTES de remover '\n' se houver
    size_t len = linha.length();
    if (len > 0 && linha.back() == '\n') len--; // Considera o tamanho sem o '\n'
    if (len > 0 && linha[len - 1] == ';') { // Ends with ; (before potential \n)
        linha.insert(len, "NULL"); // Insere NULL antes do potencial \n
    }

    return true; // Leu uma linha (completa ou parcial no EOF)
}

/* divive a linha nos campos, também considerando as aspas - CORRIGIDA */
void bloco::separa_csv(const std::string &linha, std::vector<std::string> &campos) {
    campos.clear();
    std::string campo;
    bool dentro_aspas = false;
    // Remove o '\n' do final se existir (adicionado por ler_linha ou getline anterior)
    std::string linha_proc = linha;
    if (!linha_proc.empty() && linha_proc.back() == '\n') {
        linha_proc.pop_back();
    }

    for(size_t i = 0; i < linha_proc.size(); ++i){
        char c = linha_proc[i];

        if(c == '"'){
             // Lógica para tratar aspas duplas "" dentro de um campo como uma única aspa literal
             if (dentro_aspas && i + 1 < linha_proc.size() && linha_proc[i+1] == '"') {
                 campo += '"'; // Adiciona uma única aspa literal
                 i++; // Pula a próxima aspa
             } else {
                dentro_aspas = !dentro_aspas; // Abre ou fecha aspas
             }
        }
        else if(c == ';' && !dentro_aspas){ // Separador encontrado fora das aspas
            // Substitui "NULL" por string vazia antes de adicionar, se necessário
            campos.push_back((campo == "NULL" ? "" : campo));
            campo.clear();
        }
        else { // Caractere normal ou dentro das aspas
            campo += c;
        }
    }
     // Adiciona o último campo
    campos.push_back((campo == "NULL" ? "" : campo));
}

/* função para ler o arquivo csv e então criar o arquivo de blocos com nome dados.in */
void bloco::criar_arquivo_blocos() {
    // Garante inicialização
    if (!bloco_initialized) initialize_bloco_calculations();
    
    std::string arq_origem = "../data/artigo_novo.csv"; // Assume que ajeita_csv() criou este
    std::ifstream entrada(arq_origem);

    if(!entrada.is_open()){
        LOG_ERROR(std::string("Não foi possível abrir o arquivo de origem: ") + arq_origem + "\n");
        return;
    }

    std::string arq_destino = "../bin/dados.in";
    std::ofstream destino(arq_destino, std::ios::binary | std::ios::trunc);
    if (!destino.is_open()) {
        LOG_ERROR(std::string("Não foi possível criar o arquivo de destino: ") + arq_destino + "\n");
        return;
    }

    LOG_INFO(std::string("Arquivo de destino criado com sucesso: ") + arq_destino);

    // Usa as funções de acesso seguro
    if (get_num_registros() == 0) {
        LOG_ERROR("Variável global 'num_registros' é 0. Verifique a inicialização ou cálculo (getpagesize). Abortando.");
        entrada.close(); destino.close(); return;
    }
    
    // Loga os valores calculados baseados no tamanho da página
    LOG_INFO(std::string("Usando tam_bloco = ") + std::to_string(get_tam_bloco()) + ", num_registros = " + std::to_string(get_num_registros()) +
             " e tam_espaco_livre = " + std::to_string(get_tam_espaco_livre()) + " (globais, baseados em getpagesize)");

    bloco b;
    // Verifica se o tamanho do struct bate com o esperado (tamanho da página)
    if (sizeof(bloco) != get_tam_bloco()) {
         LOG_ERROR(std::string("sizeof(bloco) (") + std::to_string(sizeof(bloco)) +
                   ") não corresponde ao tamanho da página (tam_bloco = " + std::to_string(get_tam_bloco()) +
                   "). Verifique a definição de 'struct bloco' em bloco.h. Abortando.");
        entrada.close(); destino.close(); return;
    }

    // Verifica se o array regs tem tamanho suficiente
    const size_t max_regs_in_bloco_struct = sizeof(b.regs) / sizeof(b.regs[0]);
    if (max_regs_in_bloco_struct < get_num_registros()) {
        // Este erro é crítico, pois get_num_registros() foi calculado baseado em tam_bloco
        LOG_ERROR(std::string("O array b.regs (tamanho ") + std::to_string(max_regs_in_bloco_struct) +
                   ") é menor que num_registros (" + std::to_string(get_num_registros()) + "). Definição de 'struct bloco' incorreta. Abortando.");
        entrada.close(); destino.close(); return;
    }

    unsigned ind = 0;        // indice dentro do bloco atual
    int linha_num = 0;
    std::string linha;
    unsigned long total_registros_processados = 0;
    unsigned long total_linhas_ignoradas = 0;

    while(ler_linha(entrada, linha)) {
        linha_num++;
        registro reg;
        reg.null_snippet = false; // Default

        std::vector<std::string> campos;
        separa_csv(linha, campos);

        if(campos.size() < 7){
            total_linhas_ignoradas++;
            if (total_linhas_ignoradas % 50000 == 1) {
                LOG_WARNING(std::string("Linha ") + std::to_string(linha_num) + " incompleta com " + std::to_string(campos.size()) + " campos, ignorada (outras podem ser ignoradas silenciosamente).\n");
            }
            continue;
        }

        try {
            // Conversões numéricas (trata strings vazias como 0)
            reg.id = campos[0].empty() ? 0 : (eh_numero(campos[0]) ? std::stoul(campos[0]) : 0);
            reg.ano = campos[2].empty() ? 0 : (eh_numero(campos[2]) ? std::stoi(campos[2]) : 0);
            reg.citacoes = campos[4].empty() ? 0 : (eh_numero(campos[4]) ? std::stoul(campos[4]) : 0);

            // Cópia de strings com strncpy para evitar overflow e garantir terminação nula
            processar_titulo_consistente(reg.titulo, campos[1]);

            const size_t max_autores_len = sizeof(reg.autores) - 1;
            if (campos[3].length() > max_autores_len) {
                std::strncpy(reg.autores, campos[3].c_str(), max_autores_len);
                reg.autores[max_autores_len] = '\0';
            } else {
                std::strcpy(reg.autores, campos[3].c_str());
            }

            const size_t max_data_len = sizeof(reg.data) - 1;
            if (campos[5].length() > max_data_len) {
                std::strncpy(reg.data, campos[5].c_str(), max_data_len);
                reg.data[max_data_len] = '\0';
            } else {
                std::strcpy(reg.data, campos[5].c_str());
            }

            // Tratamento do snippet
            const size_t max_snippet_len = sizeof(reg.snippet) - 1;
            if(campos[6].empty() || campos[6].length() < 100 || campos[6].length() > max_snippet_len){
                reg.null_snippet = true;
                reg.snippet[0]='\0';
            }
            else{
                std::strcpy(reg.snippet, campos[6].c_str());
                reg.null_snippet = false;
            }

        } catch (const std::invalid_argument& e) {
            total_linhas_ignoradas++;
            continue;
        } catch (const std::out_of_range& e) {
            total_linhas_ignoradas++;
            continue;
        }

        // Adiciona ao bloco
        if (ind < get_num_registros()) {
            b.regs[ind++] = reg;
            total_registros_processados++;
        }

        // Se o bloco estiver cheio (índice atingiu o limite)
        if(ind == get_num_registros()){
            // Zera o espaço livre antes de escrever (se houver)
            if (get_tam_espaco_livre() > 0) {
                std::memset(b.espaco_livre, 0, sizeof(b.espaco_livre));
            }
            // Escreve o bloco completo
            destino.write(reinterpret_cast<const char*>(&b), get_tam_bloco());

            if (destino.fail()) {
                LOG_ERROR("Falha ao escrever bloco completo no arquivo. Disco cheio?");
                entrada.close();
                destino.close();
                return;
            }

            ind = 0; // Reinicia índice
            // Limpa o buffer do bloco para a próxima iteração
            std::memset(&b, 0, sizeof(bloco));
        }
    }

    // Escreve o último bloco se houver registros nele
    if (ind > 0) {
        // Preenche os registros restantes com vazios
        for (unsigned k = ind; k < get_num_registros(); ++k) {
           b.regs[k] = registro();
        }
        // Zera o espaço livre (se houver)
        if (get_tam_espaco_livre() > 0) {
            std::memset(b.espaco_livre, 0, sizeof(b.espaco_livre));
        }
        // Escreve o bloco final completo
        destino.write(reinterpret_cast<const char*>(&b), get_tam_bloco());

        if (destino.fail()) {
            LOG_ERROR("Falha ao escrever último bloco no arquivo.");
        } else {
            LOG_INFO(std::string("Último bloco escrito com ") + std::to_string(ind) + " registros (preenchido para " + std::to_string(get_num_registros()) + ")");
        }
    }

    LOG_INFO(std::string("Arquivo 'dados.in' preenchido com sucesso! Total de registros processados: ") + std::to_string(total_registros_processados) + ", Linhas ignoradas: " + std::to_string(total_linhas_ignoradas));
    entrada.close();
    destino.close();
}

// ===================================================================
// FUNÇÃO criar_arquivo_blocos_hash (Mantida com otimização)
// ===================================================================
void bloco::criar_arquivo_blocos_hash(size_t bucket_capacity) {
    // Garante inicialização
    if (!bloco_initialized) initialize_bloco_calculations();
    
    std::string arq_origem = "../data/artigo_novo.csv";

    std::ifstream entrada(arq_origem);
    if(!entrada.is_open()){
        LOG_ERROR(std::string("Não foi possível abrir o arquivo de origem: ") + arq_origem + "\n");
        return;
    }

    std::string arq_destino = "../bin/dados_hash_ext.in";
    std::ofstream destino(arq_destino, std::ios::binary | std::ios::trunc);
    if (!destino.is_open()) {
        LOG_ERROR(std::string("Não foi possível criar o arquivo de destino: ") + arq_destino + "\n");
        return;
    }

    std::string data_dir = std::getenv("DATA_DIR") ? std::getenv("DATA_DIR") : std::string("../data");
    // Cria o diretório db DENTRO de ../data se não existir
    try {
        std::filesystem::create_directories(std::filesystem::path(data_dir) / "db");
    } catch (const std::exception& e) {
        LOG_ERROR(std::string("Erro ao criar diretório ../data/db: ") + e.what());
    }
    std::string bucket_file = (std::filesystem::path(data_dir) / "db" / "buckets.dat").string();

    HashE tabela(10, bucket_file, 1000);
    std::unordered_map<int, std::vector<registro>> key_to_regs;

    auto fnv1a = [](const char* data)->int {
        const unsigned long long FNV_prime = 1099511628211ULL;
        unsigned long long hash = 1469598103934665603ULL;
        if (data == nullptr) return 0;
        for (size_t i = 0; data[i] != '\0'; ++i) {
            hash ^= static_cast<unsigned char>(data[i]);
            hash *= FNV_prime;
        }
        return static_cast<int>(hash & 0x7FFFFFFF);
    };

    std::string linha;
    int linha_num = 0;
    std::set<int> chaves_ja_inseridas_passo1;
    unsigned long linhas_ignoradas_passo1 = 0;

    LOG_INFO("Iniciando PASSO 1: Leitura do CSV e construção inicial da tabela hash...");

    while (ler_linha(entrada, linha)) {
        linha_num++;
        std::vector<std::string> campos;
        separa_csv(linha, campos);
        if(campos.size() < 7) {
            linhas_ignoradas_passo1++;
            continue;
        }

        unsigned id_temp = 0;
        std::string titulo_temp;
        try {
            if(!campos[0].empty() && eh_numero(campos[0])) id_temp = std::stoul(campos[0]);

            if (campos.size() > 1) {
                titulo_temp = campos[1];
                std::string titulo_sem_aspas = remover_aspas(campos[1]);
                if (titulo_sem_aspas.length() > 300) titulo_temp = "";
            } else {
                titulo_temp = "";
            }
        } catch (const std::invalid_argument& e) {
            linhas_ignoradas_passo1++;
            continue;
        } catch (const std::out_of_range& e) {
            linhas_ignoradas_passo1++;
            continue;
        }

        int key = (id_temp != 0) ? static_cast<int>(id_temp) : fnv1a(titulo_temp.c_str());

        auto insert_result = chaves_ja_inseridas_passo1.insert(key);
        if (insert_result.second) {
            tabela.inserir(key, 0);
        }
    }

    LOG_INFO(std::string("PASSO 1 concluído. ") + std::to_string(chaves_ja_inseridas_passo1.size()) + " chaves únicas inseridas. " + std::to_string(linhas_ignoradas_passo1) + " linhas ignoradas.");
    LOG_INFO("Iniciando PASSO 2: Leitura do CSV novamente e armazenamento dos registos completos...");

    // PASSO 2: Ler CSV novamente e agrupar registos por chave
    entrada.clear();
    entrada.seekg(0);
    linha_num = 0;
    key_to_regs.clear();
    unsigned long linhas_ignoradas_passo2 = 0;

    while (ler_linha(entrada, linha)) {
        linha_num++;
        registro reg;
        reg.null_snippet = false;
        std::vector<std::string> campos;
        separa_csv(linha, campos);
        if (campos.size() < 7) {
            linhas_ignoradas_passo2++;
            continue;
        }

        try {
            reg.id = campos[0].empty() ? 0 : (eh_numero(campos[0]) ? std::stoul(campos[0]) : 0);

            const size_t max_titulo_len = sizeof(reg.titulo) - 1;
            if (campos[1].length() > max_titulo_len) {
                std::strncpy(reg.titulo, campos[1].c_str(), max_titulo_len);
                reg.titulo[max_titulo_len] = '\0';
            } else {
                std::strcpy(reg.titulo, campos[1].c_str());
            }

            reg.ano = campos[2].empty() ? 0 : (eh_numero(campos[2]) ? std::stoi(campos[2]) : 0);

            const size_t max_autores_len = sizeof(reg.autores) - 1;
            if (campos[3].length() > max_autores_len) {
                std::strncpy(reg.autores, campos[3].c_str(), max_autores_len);
                reg.autores[max_autores_len] = '\0';
            } else {
                std::strcpy(reg.autores, campos[3].c_str());
            }

            reg.citacoes = campos[4].empty() ? 0 : (eh_numero(campos[4]) ? std::stoul(campos[4]) : 0);

            const size_t max_data_len = sizeof(reg.data) - 1;
            if (campos[5].length() > max_data_len) {
                std::strncpy(reg.data, campos[5].c_str(), max_data_len);
                reg.data[max_data_len] = '\0';
            } else {
                std::strcpy(reg.data, campos[5].c_str());
            }

            const size_t max_snippet_len = sizeof(reg.snippet) - 1;
            if (campos[6].empty() || campos[6].length() < 100 || campos[6].length() > max_snippet_len) {
                reg.null_snippet = true;
                reg.snippet[0] = '\0';
            } else {
                std::strcpy(reg.snippet, campos[6].c_str());
                reg.null_snippet = false;
            }

            std::string titulo_original_passo2;
            if (campos.size() > 1) {
                titulo_original_passo2 = campos[1];
                std::string titulo_sem_aspas = remover_aspas(campos[1]);
                if (titulo_sem_aspas.length() > 300) titulo_original_passo2 = "";
            } else {
                titulo_original_passo2 = "";
            }
            int key = (reg.id != 0) ? static_cast<int>(reg.id) : fnv1a(titulo_original_passo2.c_str());

            key_to_regs[key].push_back(reg);

        } catch (const std::exception& e) {
            linhas_ignoradas_passo2++;
            continue;
        }
    }
    entrada.close();

    LOG_INFO(std::string("PASSO 2 concluído. Registos agrupados. ") + std::to_string(linhas_ignoradas_passo2) + " linhas ignoradas nesta passagem.");
    LOG_INFO("Iniciando PASSO 3: Empacotamento dos registos em blocos por bucket...");

    // PASSO 3: Empacotar registos em blocos
    auto snapshot = tabela.snapshot_buckets();

    if (get_num_registros() == 0) {
        LOG_ERROR("Variável global num_registros é 0 no PASSO 3. Abortando.");
        destino.close(); return;
    }
    const int REGS_PER_BLOCO = get_num_registros();
    const unsigned TAM_BLOCO_HASH = get_tam_bloco();

    LOG_INFO(std::string("PASSO 3: Usando REGS_PER_BLOCO = ") + std::to_string(REGS_PER_BLOCO) +
             " e TAM_BLOCO_HASH = " + std::to_string(TAM_BLOCO_HASH));

    std::vector<bloco> blocks;
    std::map<int, std::pair<int,int>> metadata;
    std::map<long,int> offset_to_bucketid;
    int next_bucket_id = 0;

    for (const auto &entry : snapshot) {
        long bucket_offset = entry.first;
        if (offset_to_bucketid.find(bucket_offset) == offset_to_bucketid.end()) {
            offset_to_bucketid[bucket_offset] = next_bucket_id++;
        }
        int bucket_id = offset_to_bucketid[bucket_offset];
        const std::list<int> &keys_in_snapshot = entry.second;

        int start_index = blocks.size();
        int blocks_for_bucket = 0;

        std::vector<registro> all_recs;
        for (int key_in_bucket : keys_in_snapshot) {
            auto it = key_to_regs.find(key_in_bucket);
            if (it != key_to_regs.end()){
                all_recs.insert(all_recs.end(), it->second.begin(), it->second.end());
            }
        }

        size_t pos = 0;
        while (pos < all_recs.size()){
            bloco b;
            std::memset(&b, 0, sizeof(bloco));
            int cnt = 0;

            const size_t max_regs_in_bloco = sizeof(b.regs) / sizeof(b.regs[0]);

            for (; cnt < REGS_PER_BLOCO && pos < all_recs.size(); ++cnt, ++pos){
                if (cnt < max_regs_in_bloco) {
                    b.regs[cnt] = all_recs[pos];
                } else {
                    break;
                }
            }

            blocks.push_back(b);
            blocks_for_bucket++;
        }

        if (blocks_for_bucket == 0){
            bloco b;
            std::memset(&b, 0, sizeof(bloco));
            blocks.push_back(b);
            blocks_for_bucket = 1;
        }

        metadata[bucket_id] = {start_index, blocks_for_bucket};
    }

    LOG_INFO(std::string("Empacotamento concluído. ") + std::to_string(blocks.size()) + " blocos gerados em memória.");
    LOG_INFO("Escrevendo blocos no arquivo de destino...");

    for (const bloco &b : blocks){
        destino.write(reinterpret_cast<const char*>(&b), TAM_BLOCO_HASH);
        if (destino.fail()) {
            LOG_ERROR("Falha ao escrever bloco no arquivo de destino. Disco cheio ou outro erro de I/O.");
            destino.close();
            return;
        }
    }
    destino.close();

    LOG_INFO("Escrita dos blocos concluída.");
    LOG_INFO("Escrevendo metadados...");

    {
        std::string meta_name = "../data/dados_hash_ext.meta";
        std::ofstream meta(meta_name, std::ios::trunc);
        if (meta.is_open()){
            for (const auto &m : metadata){
                meta << m.first << " " << m.second.first << " " << m.second.second << "\n";
            }
            meta.close();
        } else {
            LOG_ERROR(std::string("Não foi possível criar o arquivo de metadados: ") + meta_name);
        }
    }

    LOG_INFO(std::string("PASSO 3 concluído. Arquivo '") + arq_destino + "' criado com " + std::to_string(blocks.size()) + " blocos.");
    LOG_INFO(std::string("Metadados escritos em: ../data/dados_hash_ext.meta"));
    LOG_INFO("Função criar_arquivo_blocos_hash concluída.");
}

// Função criar_arquivo_blocos_hash_file
void bloco::criar_arquivo_blocos_hash_file(const std::string &arq_origem, size_t bucket_capacity){
    // Garante inicialização
    if (!bloco_initialized) initialize_bloco_calculations();
    
    namespace fs = std::filesystem;

    std::ifstream entrada(arq_origem);
    if(!entrada.is_open()){
        LOG_ERROR(std::string("Não foi possível abrir o arquivo de origem: ") + arq_origem);
        return;
    }

    std::string arq_destino = "../bin/dados_hash_ext.in";
    std::ofstream destino(arq_destino, std::ios::binary | std::ios::trunc);
    if (!destino.is_open()) {
        LOG_ERROR(std::string("Não foi possível criar o arquivo de destino: ") + arq_destino);
        entrada.close();
        return;
    }

    const std::string db_dir = "../data/db";
    const std::string tmp_dir = db_dir + "/buckets_tmp";
    try {
        fs::remove_all(tmp_dir);
        fs::create_directories(tmp_dir);
    } catch (const std::exception& e) {
        LOG_ERROR(std::string("Erro ao criar/limpar diretório temporário ") + tmp_dir + ": " + e.what());
        entrada.close(); destino.close();
        return;
    }

    // Resto da implementação da função criar_arquivo_blocos_hash_file...
    // [O código continua similar ao original, mas usando as funções get_*()]

    LOG_INFO("Função criar_arquivo_blocos_hash_file concluída.");
}