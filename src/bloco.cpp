#include "registro.h"
#include "bloco.h"
#include "logger.h"
#include "hashE.h"
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
    LOG_INPUT("Insira o nome do arquivo de entrada (deixe ele no mesmo diretório [essa parte de estar no mesmo diretório é só pra testes inciais ta?]): ");
    std::string arq_origem;
    std::cin >> arq_origem;

    std::ifstream entrada(arq_origem);
    if(!entrada.is_open()){
        LOG_ERROR("Não foi possível abrir o arquivo de origem\n");
        return;
    }
    
    // o arquivo de dados ficara na /data na versão final
    std::string arq_destino = "dados.in";
    std::ofstream destino(arq_destino, std::ios::binary);
    if (!destino.is_open()) {
        LOG_ERROR(std::string("Não foi possível criar o arquivo de destino\n") + arq_destino);
        return;
    }

    LOG_INFO(std::string("Arquivo de destino criado com sucesso: ") + arq_destino);

    bloco b;
    unsigned ind = 0;        // indice para saber em que posicao do bloco colocar o registro
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
            LOG_WARNING(std::string("Linha ") + std::to_string(linha_num) + " incompleta com algum campo a menos, ignorada.\n");
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
        LOG_INFO(std::string("Bloco escrito incompleto com somente ") + std::to_string(ind) + " registros\n");
    }

    LOG_INFO("Arquivo 'dados.in' preenchido com sucesso!\n");
    destino.close();
}
    // escreve arquivo organizado por hash extensivo (usa HashE)
    void bloco::criar_arquivo_blocos_hash(size_t bucket_capacity) {
        LOG_INFO("Insira o nome do arquivo de entrada (CSV): ");
        std::string arq_origem;
        std::cin >> arq_origem;

        std::ifstream entrada(arq_origem);
        if(!entrada.is_open()){
            LOG_ERROR("Não foi possível abrir o arquivo de origem\n");
            return;
        }

        // arq destino
        std::string arq_destino = "dados_hash_ext.in";
        std::ofstream destino(arq_destino, std::ios::binary | std::ios::trunc);
        if (!destino.is_open()) {
            LOG_ERROR(std::string("Não foi possível criar o arquivo de destino\n") + arq_destino);
            return;
        }

    // prepara diretório para dados persistentes (usa DATA_DIR ou ./data)
    std::string data_dir = std::getenv("DATA_DIR") ? std::getenv("DATA_DIR") : std::string("./data");
    std::filesystem::create_directories(std::filesystem::path(data_dir) / "db");
    std::string bucket_file = (std::filesystem::path(data_dir) / "db" / "buckets.dat").string();

    // cria a tabela extensível em arquivo: profundidade inicial escolhida como 10 (~1024 entradas)
    // o terceiro parâmetro é o tamanho do cache de páginas do HashE
    HashE tabela(10, bucket_file, 1000);

        // map key -> registros (para armazenar dados completos)
        std::unordered_map<int, std::vector<registro>> key_to_regs;

        // função FNV-1a para títulos (determinística)
        auto fnv1a = [](const char* data)->int {
            const unsigned long long FNV_prime = 1099511628211ULL;
            unsigned long long hash = 1469598103934665603ULL;
            for (size_t i = 0; data[i] != '\0'; ++i) {
                hash ^= static_cast<unsigned char>(data[i]);
                hash *= FNV_prime;
            }
            return static_cast<int>(hash & 0x7fffffff);
        };

        std::string linha;
        int linha_num = 0;

        while (ler_linha(entrada, linha)) {
            linha_num++;
            registro reg;
            reg.null_snippet = false;

            std::vector<std::string> campos;
            separa_csv(linha, campos);

            if(campos.size() < 7){
                LOG_WARNING(std::string("Linha ") + std::to_string(linha_num) + " incompleta, ignorada.\n");
                continue;
            }

            for(std::string &c : campos) c = remover_aspas(c);

            reg.id = eh_numero(campos[0]) ? std::stoul(campos[0]) : 0;
            if (campos[1].length() > 300) reg.titulo[0] = '\0';
            else { std::strncpy(reg.titulo, campos[1].c_str(), sizeof(reg.titulo)-1); reg.titulo[300] = '\0'; }
            reg.ano = eh_numero(campos[2]) ? std::stoi(campos[2]) : 0;
            if (campos[3].length() > 150) reg.autores[0] = '\0';
            else { std::strncpy(reg.autores, campos[3].c_str(), sizeof(reg.autores)-1); reg.autores[150] = '\0'; }
            reg.citacoes = eh_numero(campos[4]) ? std::stoul(campos[4]) : 0;
            std::strncpy(reg.data, campos[5].c_str(), sizeof(reg.data)-1); reg.data[19] = '\0';
            if(campos[6].length() < 100 || campos[6].length() > 1024){ reg.null_snippet = true; reg.snippet[0]='\0'; }
            else{ std::strncpy(reg.snippet, campos[6].c_str(), sizeof(reg.snippet)-1); reg.snippet[1024] = '\0'; }

            int key = (reg.id != 0) ? static_cast<int>(reg.id) : fnv1a(reg.titulo);
            // inserimos com offset 0 pois nesta fase só precisamos mapear chaves -> buckets
            tabela.inserir(key, 0);
            key_to_regs[key].push_back(reg);
        }

    // agrupa por buckets usando snapshot (offset_bucket -> lista_de_chaves)
    auto snapshot = tabela.snapshot_buckets();

        // preparar blocos: pack registros por bucket em blocos (regs[2] por bloco)
        const int REGS_PER_BLOCO = 2; // conforme struct bloco
        std::vector<bloco> blocks;
        // metadados: bucket_id -> (start_index, num_blocks)
        std::map<int, std::pair<int,int>> metadata;

        // snapshot keys are keyed by bucket file offset (long). We'll map them to sequential
        // integer bucket IDs for metadata output.
        std::map<long,int> offset_to_bucketid;
        int next_bucket_id = 0;

        for (const auto &entry : snapshot) {
            long bucket_offset = entry.first;
            if (offset_to_bucketid.find(bucket_offset) == offset_to_bucketid.end()) {
                offset_to_bucketid[bucket_offset] = next_bucket_id++;
            }
            int bucket_id = offset_to_bucketid[bucket_offset];
            const std::list<int> &keys = entry.second;

            int start_index = blocks.size();
            int blocks_for_bucket = 0;

            // acumular todos os registros deste bucket (preservando a ordem das chaves)
            std::vector<registro> all_recs;
            for (int key : keys) {
                auto it = key_to_regs.find(key);
                if (it != key_to_regs.end()){
                    for (const registro &r : it->second) all_recs.push_back(r);
                }
            }

            // empacotar
            size_t pos = 0;
            while (pos < all_recs.size()){
                bloco b;
                // preencher registros usados e resetar os demais
                int cnt = 0;
                for (; cnt < REGS_PER_BLOCO && pos < all_recs.size(); ++cnt, ++pos){
                    b.regs[cnt] = all_recs[pos];
                }
                // zera registros restantes no bloco
                for (unsigned k = cnt; k < num_registros; ++k) b.regs[k] = registro();
                // espaço livre zero
                std::memset(b.espaco_livre, 0, tam_espaco_livre);
                blocks.push_back(b);
                blocks_for_bucket++;
            }

            // se bucket vazio, ainda criaremos 1 bloco vazio para representá-lo
        if (blocks_for_bucket == 0){
            bloco b;
            for (unsigned k = 0; k < num_registros; ++k) b.regs[k] = registro();
            std::memset(b.espaco_livre, 0, tam_espaco_livre);
            blocks.push_back(b);
            blocks_for_bucket = 1;
        }

            metadata[bucket_id] = {start_index, blocks_for_bucket};
        }

        // escreve todos os blocos no arquivo destino
        for (const bloco &b : blocks){
            destino.write(reinterpret_cast<const char*>(&b), sizeof(bloco));
        }
        destino.close();

        // escreve metadados em texto
        std::string meta_name = "dados_hash_ext.meta";
        std::ofstream meta(meta_name, std::ios::trunc);
        if (meta.is_open()){
            for (const auto &m : metadata){
                meta << m.first << " " << m.second.first << " " << m.second.second << "\n";
            }
            meta.close();
        }

        LOG_INFO(std::string("Arquivo 'dados_hash_ext.in' criado com ") + std::to_string(blocks.size()) + " blocos.\n");
        LOG_INFO(std::string("Metadados escritos em: ") + meta_name);
    }

// versão não-interativa que recebe o caminho do CSV
void bloco::criar_arquivo_blocos_hash_file(const std::string &arq_origem, size_t bucket_capacity){
    // Two-pass streaming implementation to avoid storing all records in memory.
    namespace fs = std::filesystem;

    std::ifstream entrada(arq_origem);
    if(!entrada.is_open()){
        LOG_ERROR(std::string("Não foi possível abrir o arquivo de origem: ") + arq_origem);
        return;
    }

    // destino final
    std::string arq_destino = "dados_hash_ext.in";
    std::ofstream destino(arq_destino, std::ios::binary | std::ios::trunc);
    if (!destino.is_open()) {
        LOG_ERROR(std::string("Não foi possível criar o arquivo de destino: ") + arq_destino);
        return;
    }

    // Diretório temporário para arquivos por bucket
    const std::string db_dir = "data/db";
    const std::string tmp_dir = db_dir + "/buckets_tmp";
    try { fs::create_directories(tmp_dir); } catch (...) { /* ignore */ }

    // --- PASSO 1: construir tabela de hash apenas com as chaves ---
    std::string data_dir = std::getenv("DATA_DIR") ? std::getenv("DATA_DIR") : std::string("./data");
    std::filesystem::create_directories(std::filesystem::path(data_dir) / "db");
    std::string bucket_file = (std::filesystem::path(data_dir) / "db" / "buckets.dat").string();

    // Use extensible hash with initial global depth = 10 (~1024 directory entries)
    HashE tabela(10, bucket_file, 1000);

    auto fnv1a = [](const char* data)->int {
        const unsigned long long FNV_prime = 1099511628211ULL;
        unsigned long long hash = 1469598103934665603ULL;
        for (size_t i = 0; data[i] != '\0'; ++i) {
            hash ^= static_cast<unsigned char>(data[i]);
            hash *= FNV_prime;
        }
        return static_cast<int>(hash & 0x7fffffff);
    };

    std::string linha;
    int linha_num = 0;
    // Rewind input stream: we are at begin so OK. Read all keys.
    while (ler_linha(entrada, linha)) {
        linha_num++;
        std::vector<std::string> campos;
        separa_csv(linha, campos);
        if (campos.size() < 7) continue;
        for (std::string &c : campos) c = remover_aspas(c);

        registro reg;
        reg.id = eh_numero(campos[0]) ? std::stoul(campos[0]) : 0;
        if (campos[1].length() > 300) reg.titulo[0] = '\0';
        else { std::strncpy(reg.titulo, campos[1].c_str(), sizeof(reg.titulo)-1); reg.titulo[300]='\0'; }

        int key = (reg.id != 0) ? static_cast<int>(reg.id) : fnv1a(reg.titulo);
    tabela.inserir(key, 0);
    }

    // snapshot: bucket_offset -> list of keys
    auto snapshot = tabela.snapshot_buckets();

    // map bucket file offset -> sequential bucket id (int) and key -> bucket id
    std::map<long,int> offset_to_bucketid;
    std::unordered_map<int,int> key_to_bucket;
    int next_bucket_id = 0;
    for (const auto &entry : snapshot){
        long bucket_offset = entry.first;
        if (offset_to_bucketid.find(bucket_offset) == offset_to_bucketid.end()) {
            offset_to_bucketid[bucket_offset] = next_bucket_id++;
        }
        int bucket_id = offset_to_bucketid[bucket_offset];
        for (int k : entry.second) key_to_bucket[k] = bucket_id;
    }

    // --- PASSO 2: stream de registros para arquivos temporários por bucket ---
    // Rewind file to beginning
    entrada.clear();
    entrada.seekg(0);
    linha_num = 0;

    while (ler_linha(entrada, linha)) {
        linha_num++;
        registro reg;
        reg.null_snippet = false;
        std::vector<std::string> campos;
        separa_csv(linha, campos);
        if (campos.size() < 7) continue;
        for (std::string &c : campos) c = remover_aspas(c);

        reg.id = eh_numero(campos[0]) ? std::stoul(campos[0]) : 0;
        if (campos[1].length() > 300) reg.titulo[0] = '\0';
        else { std::strncpy(reg.titulo, campos[1].c_str(), sizeof(reg.titulo)-1); reg.titulo[300]='\0'; }
        reg.ano = eh_numero(campos[2]) ? std::stoi(campos[2]) : 0;
        if (campos[3].length() > 150) reg.autores[0] = '\0';
        else { std::strncpy(reg.autores, campos[3].c_str(), sizeof(reg.autores)-1); reg.autores[150]='\0'; }
        reg.citacoes = eh_numero(campos[4]) ? std::stoul(campos[4]) : 0;
        std::strncpy(reg.data, campos[5].c_str(), sizeof(reg.data)-1); reg.data[19] = '\0';
        if (campos[6].length() < 100 || campos[6].length() > 1024) { reg.null_snippet = true; reg.snippet[0] = '\0'; }
        else { std::strncpy(reg.snippet, campos[6].c_str(), sizeof(reg.snippet)-1); reg.snippet[1024] = '\0'; }

        int key = (reg.id != 0) ? static_cast<int>(reg.id) : fnv1a(reg.titulo);
        auto it = key_to_bucket.find(key);
        int bucket_id = (it != key_to_bucket.end()) ? it->second : 0;

        std::string bucket_file = tmp_dir + "/bucket_" + std::to_string(bucket_id) + ".bin";
        std::ofstream bfile(bucket_file, std::ios::binary | std::ios::app);
        if (!bfile.is_open()) {
            LOG_WARNING(std::string("Não foi possível abrir arquivo do bucket: ") + bucket_file + " (linha " + std::to_string(linha_num) + ")\n");
            continue;
        }
        bfile.write(reinterpret_cast<const char*>(&reg), sizeof(registro));
        bfile.close();
    }

    entrada.close();

    // --- PASSO 3: ler arquivos por bucket e escrever blocos no destino ---
    const int REGS_PER_BLOCO = 2; // conforme struct bloco
    std::map<int, std::pair<int,int>> metadata; // bucket -> (start_block, num_blocks)
    int global_block_index = 0;

    for (const auto &entry : snapshot){
        long bucket_offset = entry.first;
        int bucket_id = offset_to_bucketid[bucket_offset];
        int start_index = global_block_index;
        int blocks_for_bucket = 0;

        std::string bucket_file = tmp_dir + "/bucket_" + std::to_string(bucket_id) + ".bin";
        if (!fs::exists(bucket_file)) {
            // bucket vazio: escrever um bloco vazio
            bloco b;
            for (unsigned k = 0; k < num_registros; ++k) b.regs[k] = registro();
            for (unsigned k = 0; k < tam_espaco_livre; ++k) b.espaco_livre[k] = 0;
            destino.write(reinterpret_cast<const char*>(&b), sizeof(bloco));
            blocks_for_bucket = 1;
            global_block_index += 1;
            metadata[bucket_id] = {start_index, blocks_for_bucket};
            continue;
        }

        std::ifstream bfile(bucket_file, std::ios::binary);
        if (!bfile.is_open()) {
            LOG_WARNING(std::string("Não foi possível abrir arquivo do bucket para leitura: ") + bucket_file + "\n");
            // escrever 1 bloco vazio para manter consistência
            bloco b;
            for (unsigned k = 0; k < num_registros; ++k) b.regs[k] = registro();
            for (unsigned k = 0; k < tam_espaco_livre; ++k) b.espaco_livre[k] = 0;
            destino.write(reinterpret_cast<const char*>(&b), sizeof(bloco));
            blocks_for_bucket = 1;
            global_block_index += 1;
            metadata[bucket_id] = {start_index, blocks_for_bucket};
            continue;
        }

        // ler registros do arquivo do bucket e empacotar em blocos
        while (true){
            bloco b;
            int cnt = 0;
            registro tmp;
            for (; cnt < (int)num_registros && bfile.read(reinterpret_cast<char*>(&tmp), sizeof(registro)); ++cnt){
                b.regs[cnt] = tmp;
            }
            // se leu algum registro, completar o bloco e escrever
            if (cnt > 0){
                for (unsigned k = cnt; k < num_registros; ++k) b.regs[k] = registro();
                for (unsigned k = 0; k < tam_espaco_livre; ++k) b.espaco_livre[k] = 0;
                destino.write(reinterpret_cast<const char*>(&b), sizeof(bloco));
                blocks_for_bucket++;
                global_block_index++;
            } else break;
        }

        bfile.close();
        // remover arquivo temporário
        try { fs::remove(bucket_file); } catch (...) {}

        if (blocks_for_bucket == 0){
            // garantir pelo menos 1 bloco por bucket
            bloco b;
            for (unsigned k = 0; k < num_registros; ++k) b.regs[k] = registro();
            for (unsigned k = 0; k < tam_espaco_livre; ++k) b.espaco_livre[k] = 0;
            destino.write(reinterpret_cast<const char*>(&b), sizeof(bloco));
            blocks_for_bucket = 1;
            global_block_index++;
        }

        metadata[bucket_id] = {start_index, blocks_for_bucket};
    }

    destino.close();

    // escreve metadados
    std::string meta_name = "dados_hash_ext.meta";
    std::ofstream meta(meta_name, std::ios::trunc);
    if (meta.is_open()){
        for (const auto &m : metadata){
            meta << m.first << " " << m.second.first << " " << m.second.second << "\n";
        }
        meta.close();
    }

    LOG_INFO(std::string("Arquivo 'dados_hash_ext.in' criado com ") + std::to_string(global_block_index) + " blocos.\n");
    LOG_INFO(std::string("Metadados escritos em: ") + meta_name);
}

/* se descomentar isso da pra testar */

// int main(){
//    bloco b;
//    b.criar_arquivo_blocos();
//
//    return 0;
//}
