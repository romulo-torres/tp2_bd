#include "../include/arv_sec.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include <cstddef>
#include <map>
#include <algorithm>

// Definindo Offset como long
typedef long Offset; 
int totalBlocosCriados_sec = 0; 
Offset raizOffset_sec = -1;

Offset escreverNoFolha_sec(std::fstream &arq, NoFolha_sec &no, Offset offsetEscrita) {
    arq.seekp(offsetEscrita, std::ios::beg);
    arq.write(reinterpret_cast<char*>(&no), sizeof(NoFolha_sec));
    arq.flush();
    return offsetEscrita;
}

Offset escreverNoInterno_sec(std::fstream &arq, NoInterno_sec &no, Offset offsetEscrita) {
    arq.seekp(offsetEscrita, std::ios::beg);
    arq.write(reinterpret_cast<char*>(&no), sizeof(NoInterno_sec));
    arq.flush();
    return offsetEscrita;
}

Offset escreverBlocoLista_sec(std::fstream &arq, BlocoLista &bloco, Offset offsetEscrita) {
    arq.seekp(offsetEscrita, std::ios::beg);
    arq.write(reinterpret_cast<char*>(&bloco), sizeof(BlocoLista));
    arq.flush();
    return offsetEscrita;
}

void atualizarProxFolha_sec(std::fstream &arq, Offset offsetFolhaAnterior, Offset offsetProx) {
    arq.seekp(offsetFolhaAnterior + offsetof(NoFolha_sec, prox), std::ios::beg);
    arq.write(reinterpret_cast<char*>(&offsetProx), sizeof(Offset));
    arq.flush();
}

void verificar_titulo_em_dados(const std::string& titulo_procurado) {
    std::ifstream dados("../bin/dados.in", std::ios::binary);
    if (!dados.is_open()) {
        std::cerr << "Erro ao abrir dados.in" << std::endl;
        return;
    }

    registro reg;
    char titulo_truncado[301];
    strncpy(titulo_truncado, titulo_procurado.c_str(), 300);
    titulo_truncado[300] = '\0';

    std::cout << "VERIFICAÇÃO: Procurando título: \"" << titulo_procurado << "\"" << std::endl;
    std::cout << "VERIFICAÇÃO: Truncado para: \"" << titulo_truncado << "\"" << std::endl;

    int encontrados = 0;
    while (dados.read(reinterpret_cast<char*>(&reg), sizeof(registro))) {
        if (strcmp(reg.titulo, titulo_truncado) == 0) {
            std::cout << "ENCONTRADO no offset: " 
                      << (static_cast<long>(dados.tellg()) - static_cast<long>(sizeof(registro))) 
                      << std::endl;
            std::cout << "ID: " << reg.id << std::endl;
            encontrados++;
        }
    }

    std::cout << "VERIFICAÇÃO: Total encontrado em dados.in: " << encontrados << std::endl;
    dados.close();
}

// Função para normalizar string (remove espaços extras, converte para minúsculas)
void normalizar_string(char* dest, const char* src, size_t max_len) {
    std::string temp(src);
    
    // Remove espaços no início/fim
    size_t start = temp.find_first_not_of(" \t\n\r");
    size_t end = temp.find_last_not_of(" \t\n\r");
    
    if (start != std::string::npos && end != std::string::npos) {
        temp = temp.substr(start, end - start + 1);
    }
    
    
    // Trunca se necessário
    if (temp.length() > max_len - 1) {
        temp = temp.substr(0, max_len - 1);
    }
    
    strcpy(dest, temp.c_str());
}



Offset escreverListasDeOffsets_sec(std::fstream &arvore, const std::vector<Offset> &offsets, Offset &offsetAtual) {
    
    BlocoLista bloco;
    std::memset(&bloco, 0, sizeof(BlocoLista));
    bloco.proxBlocoLista = -1;

    Offset primeiroBlocoOffset = offsetAtual;
    Offset offsetBlocoAnterior = -1;

    for (size_t i = 0; i < offsets.size(); ++i) {
        bloco.offsetsRegistros[bloco.nOffsets] = offsets[i];
        bloco.nOffsets++;

        if (bloco.nOffsets == MAX_OFFSETS_POR_BLOCO) {
            Offset offsetEscritaAtual = offsetAtual;
            offsetAtual += sizeof(BlocoLista); 

            if (offsetBlocoAnterior != -1) {
                arvore.seekp(offsetBlocoAnterior + offsetof(BlocoLista, proxBlocoLista), std::ios::beg);
                arvore.write(reinterpret_cast<char*>(&offsetEscritaAtual), sizeof(Offset));
            }

            bloco.proxBlocoLista = -1; 
            escreverBlocoLista_sec(arvore, bloco, offsetEscritaAtual);
            totalBlocosCriados_sec++; 

            offsetBlocoAnterior = offsetEscritaAtual;
            std::memset(&bloco, 0, sizeof(BlocoLista));
            bloco.proxBlocoLista = -1;
        }
    }

    if (bloco.nOffsets > 0) {
        Offset offsetEscritaAtual = offsetAtual;
        offsetAtual += sizeof(BlocoLista);

        if (offsetBlocoAnterior != -1) {
            arvore.seekp(offsetBlocoAnterior + offsetof(BlocoLista, proxBlocoLista), std::ios::beg);
            arvore.write(reinterpret_cast<char*>(&offsetEscritaAtual), sizeof(Offset));
        }

        escreverBlocoLista_sec(arvore, bloco, offsetEscritaAtual);
        totalBlocosCriados_sec++; 
    }

    return primeiroBlocoOffset;
}

std::vector<Offset> construirFolhas_sec(std::fstream &arvore, std::ifstream &dados, Offset &offsetAtual) {
    std::map<std::string, std::vector<Offset>> mapaChaves;
    
    dados.clear();
    dados.seekg(0, std::ios::beg);
    
    registro reg;
    char tituloTruncado[301];  
    
    // DEBUG
    std::cout << "DEBUG: Tamanho do campo titulo no struct: " << sizeof(reg.titulo) << std::endl;
    std::cout << "DEBUG: Primeiros 5 títulos processados:" << std::endl;
    int debug_count = 0;

    while (dados.read(reinterpret_cast<char*>(&reg), sizeof(registro))) {
        Offset offsetDoRegistro = static_cast<Offset>(dados.tellg()) - static_cast<Offset>(sizeof(registro));

        char tituloProcessado[301];
        normalizar_string(tituloProcessado, reg.titulo, 300);
        
        mapaChaves[tituloProcessado].push_back(offsetDoRegistro);
    }

    std::cout << "DEBUG: Total de títulos únicos indexados: " << mapaChaves.size() << std::endl;

    std::vector<Offset> offsetsFolhas;
    NoFolha_sec folha; 
    Offset offsetFolhaAnterior = -1;
    
    std::memset(&folha, 0, sizeof(NoFolha_sec)); 
    folha.ehFolha = true; 
    folha.prox = -1;

    for (auto const& [titulo, listaDeOffsets] : mapaChaves) {
        
        Offset primeiroBlocoOffset = escreverListasDeOffsets_sec(arvore, listaDeOffsets, offsetAtual);

        char tituloNormalizado[301];
        normalizar_string(tituloNormalizado, titulo.c_str(), 300);
        strcpy(folha.chaves[folha.nChaves].titulo, tituloNormalizado);
        folha.offsetsRegistros[folha.nChaves] = primeiroBlocoOffset;
        folha.nChaves++;

        if (folha.nChaves == MAX_CHAVES_FOLHA_SEC) {
            Offset offsetEscrita = offsetAtual;
            offsetAtual += sizeof(NoFolha_sec); 

            if (offsetFolhaAnterior != -1) {
                atualizarProxFolha_sec(arvore, offsetFolhaAnterior, offsetEscrita);
            }
            
            escreverNoFolha_sec(arvore, folha, offsetEscrita); 
            totalBlocosCriados_sec++; 
            offsetsFolhas.push_back(offsetEscrita);
            
            offsetFolhaAnterior = offsetEscrita;
            
            std::memset(&folha, 0, sizeof(NoFolha_sec)); 
            folha.ehFolha = true;
            folha.prox = -1;
        }
    }
    
    if (folha.nChaves > 0) {
        Offset offsetEscrita = offsetAtual;
        offsetAtual += sizeof(NoFolha_sec); 
        
        if (offsetFolhaAnterior != -1) {
            atualizarProxFolha_sec(arvore, offsetFolhaAnterior, offsetEscrita);
        }
        
        escreverNoFolha_sec(arvore, folha, offsetEscrita); 
        totalBlocosCriados_sec++; 
        offsetsFolhas.push_back(offsetEscrita);
    }
    
    return offsetsFolhas;
}

std::vector<Offset> construirNivelInterno_sec(std::fstream &arvore, const std::vector<Offset> &nivelAnterior, Offset &offsetAtual) {
    std::vector<Offset> offsetsNovoNivel;
    NoInterno_sec noInterno; 
    
    std::memset(&noInterno, 0, sizeof(NoInterno_sec)); 
    noInterno.ehFolha = false;

    if (nivelAnterior.empty()) {
        return {}; 
    }
    
    noInterno.filhos[0] = nivelAnterior[0];

    for (size_t i = 1; i < nivelAnterior.size(); ++i) {
        Offset filhoOffset = nivelAnterior[i];
        
        char buffer_leitura[sizeof(NoInterno_sec)]; 
        arvore.seekg(filhoOffset, std::ios::beg);
        
        if (!arvore.read(buffer_leitura, sizeof(NoInterno_sec))) {
            std::cerr << "ERRO FATAL: Falha ao ler nó filho no offset " << filhoOffset << ". Ponteiro inválido no nivel anterior!\n";
            return {}; 
        }
        
        bool is_folha = *reinterpret_cast<bool*>(buffer_leitura);
        ChaveTitulo chavePromovida;
        
        if (is_folha) {
            strcpy(chavePromovida.titulo, reinterpret_cast<NoFolha_sec*>(buffer_leitura)->chaves[0].titulo);
        } else {
            strcpy(chavePromovida.titulo, reinterpret_cast<NoInterno_sec*>(buffer_leitura)->chaves[0].titulo);
        }
        
        std::uint32_t indiceChave = noInterno.nChaves; 
        
        strcpy(noInterno.chaves[indiceChave].titulo, chavePromovida.titulo); 
        
        noInterno.filhos[indiceChave + 1] = filhoOffset; 
        noInterno.nChaves++;

        if (noInterno.nChaves == MAX_CHAVES_INTERNO_SEC) { 
            
            Offset offsetEscrita = offsetAtual;
            escreverNoInterno_sec(arvore, noInterno, offsetEscrita);
            totalBlocosCriados_sec++;

            offsetsNovoNivel.push_back(offsetEscrita);

            offsetAtual += sizeof(NoInterno_sec);

            Offset ultimoFilho = noInterno.filhos[MAX_CHAVES_INTERNO_SEC];
            std::memset(&noInterno, 0, sizeof(NoInterno_sec));
            noInterno.ehFolha = false;
            noInterno.filhos[0] = ultimoFilho; 
            noInterno.nChaves = 0; 
        }
    }

    if (noInterno.nChaves > 0 || offsetsNovoNivel.empty()) { 
        Offset offsetEscrita = offsetAtual;
        escreverNoInterno_sec(arvore, noInterno, offsetEscrita);
        totalBlocosCriados_sec++;
        offsetsNovoNivel.push_back(offsetEscrita);
        offsetAtual += sizeof(NoInterno_sec);
    }
    
    return offsetsNovoNivel;
}

std::vector<Offset> buscarNaArvoreBPlus_sec(std::fstream &arvore, const char* chaveBusca, Offset ofRaiz) {
    arvore.clear();
    Offset currentOffset = ofRaiz;
    std::uint32_t blocosLidos = 0;

    if (currentOffset <= 0) {
        std::cerr << "[Error] Offset da raiz é inválido: " << currentOffset << "\n";
        return {};
    }

    char chaveBuscaProcessada[301];
    normalizar_string(chaveBuscaProcessada, chaveBusca, 300);

    std::cout << "DEBUG BUSCA: Título original: \"" << chaveBusca << "\"" << std::endl;
    std::cout << "DEBUG BUSCA: Título processado: \"" << chaveBuscaProcessada << "\"" << std::endl;
    std::cout << "DEBUG BUSCA: Comprimento processado: " << strlen(chaveBuscaProcessada) << " caracteres" << std::endl;

    while (currentOffset != -1) {
        // 1) Ler apenas o cabeçalho (bool) para saber o tipo do nó
        bool ehFolha = false;
        arvore.seekg(currentOffset, std::ios::beg);
        arvore.read(reinterpret_cast<char*>(&ehFolha), sizeof(bool));

        if (!arvore) {
            std::cerr << "Erro ao ler cabeçalho do nó no offset " << currentOffset << ".\n";
            std::cout << "Total de blocos lidos: " << blocosLidos << ".\n";
            return {};
        }

        if (ehFolha) {
            // 2) Ler a folha completa
            NoFolha_sec folha;
            arvore.seekg(currentOffset, std::ios::beg);
            if (!arvore.read(reinterpret_cast<char*>(&folha), sizeof(NoFolha_sec))) {
                std::cerr << "Erro ao ler NoFolha_sec no offset " << currentOffset << ".\n";
                return {};
            }
            blocosLidos++;

            // DEBUG: imprimir primeiras chaves
            std::cout << "DEBUG: Folha tem " << folha.nChaves << " chaves" << std::endl;
            for (int k = 0; k < std::min<uint32_t>(folha.nChaves, 3); ++k)
                std::cout << "DEBUG: Chave " << k << ": \"" << folha.chaves[k].titulo << "\"" << std::endl;

            // 3) Busca binária na folha
            int inicio = 0;
            int fim = static_cast<int>(folha.nChaves) - 1;
            int idx = -1;
            while (inicio <= fim) {
                int meio = inicio + (fim - inicio) / 2;
                int cmp = strcmp(chaveBuscaProcessada, folha.chaves[meio].titulo);
                if (cmp == 0) { idx = meio; break; }
                if (cmp < 0) fim = meio - 1; else inicio = meio + 1;
            }

            if (idx == -1) {
                std::cout << "Chave não encontrada na folha.\n";
                std::cout << "Quantidade de blocos lidos para encontrar: " << blocosLidos << "\n";
                return {};
            }

            // 4) Ler a lista encadeada de BlocoLista e coletar offsets de registros
            std::vector<Offset> resultados;
            Offset offsetLista = folha.offsetsRegistros[idx];
            BlocoLista blocoLista;
            while (offsetLista != -1) {
                arvore.seekg(offsetLista, std::ios::beg);
                if (!arvore.read(reinterpret_cast<char*>(&blocoLista), sizeof(BlocoLista))) {
                    std::cerr << "Erro fatal: Bloco de lista corrompido no offset " << offsetLista << "\n";
                    break;
                }
                blocosLidos++;
                for (std::uint32_t j = 0; j < blocoLista.nOffsets; ++j)
                    resultados.push_back(blocoLista.offsetsRegistros[j]);
                offsetLista = blocoLista.proxBlocoLista;
            }

            std::cout << "Total de " << resultados.size() << " registro(s) encontrado(s).\n";
            std::cout << "Quantidade de blocos lidos para encontrar: " << blocosLidos << "\n";
            return resultados;
        } else {
            // 5) Ler nó interno e descer para o filho adequado
            NoInterno_sec no;
            arvore.seekg(currentOffset, std::ios::beg);
            if (!arvore.read(reinterpret_cast<char*>(&no), sizeof(NoInterno_sec))) {
                std::cerr << "Erro ao ler NoInterno_sec no offset " << currentOffset << ".\n";
                return {};
            }
            blocosLidos++;

            std::uint32_t i = 0;
            while (i < no.nChaves && strcmp(chaveBuscaProcessada, no.chaves[i].titulo) >= 0) i++;
            Offset prox = no.filhos[i];
            if (prox <= 0) {
                std::cerr << "No interno encontrou ponteiro filho inválido: " << prox << ".\n";
                std::cout << "Total de blocos lidos: " << blocosLidos << ".\n";
                return {};
            }
            currentOffset = prox;
            // loop continua
        }
    }

    std::cout << "Total de blocos lidos: " << blocosLidos << ".\n";
    return {};
}

void lerEImprimirRegistro_sec(const std::vector<Offset>& offsetsRegistros) { 

    std::ifstream estatistica_arv_sec("../data/estatistica_sec.txt"); 
    int blocosarq = 0;
    std::string linha;

    if (estatistica_arv_sec.is_open()) { 
        if(std::getline(estatistica_arv_sec, linha)) {
            std::sscanf(linha.c_str(), "Total de blocos do arquivo de arvore: %d", &blocosarq);
        }
        estatistica_arv_sec.close(); 
    } else {
         std::cerr << "Aviso: Nao foi possivel abrir ../data/estatistica_sec.txt\n";
    }  

    if (offsetsRegistros.empty()) { 
        if (blocosarq > 0) {
            std::cout << "Total de blocos da árvore: " << blocosarq << "\n";
        }
        std::cout << "[AVISO] Registro não encontrado.\n";
        return;
    }
    
    std::ifstream arq_dados("../bin/dados.in", std::ios::binary);
    if (!arq_dados.is_open()) { 
         std::cerr << "Erro ao abrir ../bin/dados.in\n";
         return;
    }
    
    int contador = 1;
    for (Offset offset : offsetsRegistros) {
        registro reg;
        arq_dados.seekg(offset, std::ios::beg);

        if (arq_dados.read(reinterpret_cast<char*>(&reg), sizeof(registro))) {
            if (contador == 1 && blocosarq > 0) { 
                std::cout << "Total de blocos da árvore: " << blocosarq << "\n";
            }
            std::cout << "\n--- Registro " << contador++ << " (Offset: " << offset << ") ---\n";
            std::cout << "ID: " << reg.id << "\n";
            std::cout << "Titulo: " << reg.titulo << "\n"; 
            std::cout << "Ano: " << reg.ano << "\n";
            std::cout << "Autores: " << reg.autores << "\n";
            std::cout << "Citacoes: " << reg.citacoes << "\n";
            std::cout << "Atualizacao: " << reg.data << "\n";
            std::cout << "Null Snippet: " << (reg.null_snippet ? "true" : "false") << "\n"; 
            std::cout << "Snippet: " << reg.snippet << "\n";
        } else {
            std::cout << "\n--- Erro ao ler Registro (Offset: " << offset << ") ---\n";
        }
    }
    
    arq_dados.close();
}

int cria_arvore_secundaria() { 

    std::ifstream dados("../bin/dados.in", std::ios::binary);
    if (!dados.is_open()) {
        std::cerr << "Erro: Não foi possível abrir ../bin/dados.in" << std::endl;
        return 1;
    }


    dados.seekg(0, std::ios::end);
    long file_size = dados.tellg();
    dados.seekg(0, std::ios::beg);
    
    if (file_size == 0) {
        std::cerr << "Erro: Arquivo ../bin/dados.in está vazio!" << std::endl;
        dados.close();
        return 1;
    }

    std::fstream arvore("../bin/arvore_sec.in", std::ios::binary | std::ios::trunc | std::ios::in | std::ios::out);
    if (!arvore.is_open()) {
        std::cerr << "Erro: Não foi possível criar/abrir ../bin/arvore_sec.in" << std::endl;
        dados.close();
        return 1;
    }

    std::ofstream estatistica_arv_sec("../data/estatistica_sec.txt");

    // Resetar contador
    totalBlocosCriados_sec = 0;

    Offset placeholderRaiz = -1; 
    arvore.write(reinterpret_cast<char*>(&placeholderRaiz), sizeof(Offset));
    
    Offset offsetAtual = sizeof(Offset); 
    arvore.flush();

    std::cout << "Construindo folhas da árvore secundária..." << std::endl;
    std::vector<Offset> nivelAtual = construirFolhas_sec(arvore, dados, offsetAtual);
    
    if (nivelAtual.empty()) {
        std::cerr << "Erro: Nenhuma folha foi construída!" << std::endl;
        dados.close();
        arvore.close();
        return 1;
    }

    std::cout << "Construindo níveis internos..." << std::endl;
    while (nivelAtual.size() > 1) {
        nivelAtual = construirNivelInterno_sec(arvore, nivelAtual, offsetAtual);
        if (nivelAtual.empty()) {
            std::cerr << "Erro na construção dos níveis internos!" << std::endl;
            dados.close();
            arvore.close();
            return 1;
        }
    }

    raizOffset_sec = nivelAtual.empty() ? -1 : nivelAtual[0]; 
    
    arvore.seekp(0, std::ios::beg);
    arvore.write(reinterpret_cast<char*>(&raizOffset_sec), sizeof(Offset));
    arvore.flush();

    std::cout << "Árvore Secundária construída. Offset da raiz eh: " << raizOffset_sec << std::endl;
    std::cout << "Total de blocos na Árvore Secundária: " << totalBlocosCriados_sec << std::endl;

    if (estatistica_arv_sec.is_open()) { 
        estatistica_arv_sec << "Total de blocos do arquivo de arvore: " << totalBlocosCriados_sec << std::endl;
        estatistica_arv_sec.close(); 
        std::cout << "Estatísticas salvas em ../data/estatistica_sec.txt" << std::endl;
    }
    
    totalBlocosCriados_sec = 0;

    dados.close();
    arvore.close();

    return 0;
}