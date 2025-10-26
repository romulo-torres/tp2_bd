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
int totalBlocosCriados = 0; 

// Variável global para a raiz (usada apenas para diagnóstico agora)
Offset raizOffset = -1;

Offset escreverNoFolha(std::fstream &arq, NoFolha &no, Offset offsetEscrita) {
    arq.seekp(offsetEscrita, std::ios::beg);
    arq.write(reinterpret_cast<char*>(&no), sizeof(NoFolha));
    arq.flush();
    return offsetEscrita;
}

Offset escreverNoInterno(std::fstream &arq, NoInterno &no, Offset offsetEscrita) {
    arq.seekp(offsetEscrita, std::ios::beg);
    arq.write(reinterpret_cast<char*>(&no), sizeof(NoInterno));
    arq.flush();
    return offsetEscrita;
}

Offset escreverBlocoLista(std::fstream &arq, BlocoLista &bloco, Offset offsetEscrita) {
    arq.seekp(offsetEscrita, std::ios::beg);
    arq.write(reinterpret_cast<char*>(&bloco), sizeof(BlocoLista));
    arq.flush();
    return offsetEscrita;
}

void atualizarProxFolha(std::fstream &arq, Offset offsetFolhaAnterior, Offset offsetProx) {
    arq.seekp(offsetFolhaAnterior + offsetof(NoFolha, prox), std::ios::beg);
    arq.write(reinterpret_cast<char*>(&offsetProx), sizeof(Offset));
    arq.flush();
}

Offset escreverListasDeOffsets(std::fstream &arvore, const std::vector<Offset> &offsets, Offset &offsetAtual) {
    
    BlocoLista bloco;
    std::memset(&bloco, 0, sizeof(BlocoLista));
    bloco.proxBlocoLista = -1;

    Offset primeiroBlocoOffset = offsetAtual; // Salva o offset do primeiro bloco
    Offset offsetBlocoAnterior = -1;

    for (size_t i = 0; i < offsets.size(); ++i) {
        // Adiciona o offset do registro ao bloco de lista
        bloco.offsetsRegistros[bloco.nOffsets] = offsets[i];
        bloco.nOffsets++;

        // Verifica se o bloco de lista está cheio
        if (bloco.nOffsets == MAX_OFFSETS_POR_BLOCO) {
            Offset offsetEscritaAtual = offsetAtual;
            offsetAtual += sizeof(BlocoLista); // Prepara o offset para o *próximo* bloco

            // Se não for o primeiro bloco, precisa atualizar o 'prox' do bloco anterior
            if (offsetBlocoAnterior != -1) {
                // Aponta o bloco anterior para este novo bloco
                arvore.seekp(offsetBlocoAnterior + offsetof(BlocoLista, proxBlocoLista), std::ios::beg);
                arvore.write(reinterpret_cast<char*>(&offsetEscritaAtual), sizeof(Offset));
            }

            bloco.proxBlocoLista = -1; // O 'prox' deste bloco ainda é -1
            escreverBlocoLista(arvore, bloco, offsetEscritaAtual);
            totalBlocosCriados++;

            // Prepara o próximo bloco
            offsetBlocoAnterior = offsetEscritaAtual;
            std::memset(&bloco, 0, sizeof(BlocoLista));
            bloco.proxBlocoLista = -1;
        }
    }

    // Escreve o último bloco (parcialmente preenchido)
    if (bloco.nOffsets > 0) {
        Offset offsetEscritaAtual = offsetAtual;
        offsetAtual += sizeof(BlocoLista);

        if (offsetBlocoAnterior != -1) {
            arvore.seekp(offsetBlocoAnterior + offsetof(BlocoLista, proxBlocoLista), std::ios::beg);
            arvore.write(reinterpret_cast<char*>(&offsetEscritaAtual), sizeof(Offset));
        }

        escreverBlocoLista(arvore, bloco, offsetEscritaAtual);
        totalBlocosCriados++;
    }

    return primeiroBlocoOffset;
}

std::vector<Offset> construirFolhas(std::fstream &arvore, std::ifstream &dados, Offset &offsetAtual) {
    
    // --- PASSO 1: Ler todos os dados e agrupar por chave ---
    // (Chave (título truncado), Lista de Offsets (para dados.bin))
    std::map<std::string, std::vector<Offset>> mapaChaves;
    
    dados.clear();
    dados.seekg(0, std::ios::beg);
    
    registro reg;
    char tituloTruncado[300]; // Buffer para truncamento seguro

    while (dados.read(reinterpret_cast<char*>(&reg), sizeof(registro))) {
        Offset offsetDoRegistro = dados.tellg();
        offsetDoRegistro -= sizeof(registro);

        // Truncamento seguro (301 -> 300)
        strncpy(tituloTruncado, reg.titulo, 299);
        tituloTruncado[299] = '\0';

        // Adiciona o offset à lista daquele título
        mapaChaves[tituloTruncado].push_back(offsetDoRegistro);
    }

    // --- PASSO 2: Escrever os Blocos de Lista e os Nós Folha ---

    std::vector<Offset> offsetsFolhas;
    NoFolha folha;
    Offset offsetFolhaAnterior = -1;
    
    std::memset(&folha, 0, sizeof(NoFolha));
    folha.ehFolha = true; 
    folha.prox = -1;

    // O std::map já armazena as chaves em ordem alfabética
    for (auto const& [titulo, listaDeOffsets] : mapaChaves) {
        
        // 1. Para cada título, escreve sua lista de offsets (pode ser 1 ou N blocos)
        //    'offsetAtual' é passado por referência e incrementado dentro desta função.
        Offset primeiroBlocoOffset = escreverListasDeOffsets(arvore, listaDeOffsets, offsetAtual);

        // 2. Adiciona a chave (título) e o ponteiro para o PRIMEIRO BlocoLista na folha
        strcpy(folha.chaves[folha.nChaves].titulo, titulo.c_str());
        folha.offsetsRegistros[folha.nChaves] = primeiroBlocoOffset;
        folha.nChaves++;

        // 3. Se a folha estiver cheia, escreve no disco
        if (folha.nChaves == MAX_CHAVES_FOLHA) {
            Offset offsetEscrita = offsetAtual;
            offsetAtual += sizeof(NoFolha); // Reserva espaço para a folha

            if (offsetFolhaAnterior != -1) {
                atualizarProxFolha(arvore, offsetFolhaAnterior, offsetEscrita);
            }
            
            escreverNoFolha(arvore, folha, offsetEscrita);
            totalBlocosCriados++;
            offsetsFolhas.push_back(offsetEscrita);
            
            offsetFolhaAnterior = offsetEscrita;
            
            std::memset(&folha, 0, sizeof(NoFolha));
            folha.ehFolha = true;
            folha.prox = -1;
        }
    }
    
    // Escreve a última folha (parcialmente preenchida)
    if (folha.nChaves > 0) {
        Offset offsetEscrita = offsetAtual;
        offsetAtual += sizeof(NoFolha); // Reserva espaço
        
        if (offsetFolhaAnterior != -1) {
            atualizarProxFolha(arvore, offsetFolhaAnterior, offsetEscrita);
        }
        
        escreverNoFolha(arvore, folha, offsetEscrita);
        totalBlocosCriados++;
        offsetsFolhas.push_back(offsetEscrita);
    }
    
    return offsetsFolhas;
}


// Manter a função construirNivelInterno para contexto.
std::vector<Offset> construirNivelInterno(std::fstream &arvore, const std::vector<Offset> &nivelAnterior, Offset &offsetAtual) {
    std::vector<Offset> offsetsNovoNivel;
    NoInterno noInterno;
    
    std::memset(&noInterno, 0, sizeof(NoInterno));
    noInterno.ehFolha = false;

    if (nivelAnterior.empty()) {
        return {}; 
    }
    
    // O primeiro filho do novo nó interno é sempre o primeiro nó do nível anterior.
    noInterno.filhos[0] = nivelAnterior[0];

    // Percorre o resto dos nós do nível anterior (i=1 a N)
    for (size_t i = 1; i < nivelAnterior.size(); ++i) {
        Offset filhoOffset = nivelAnterior[i];
        
        // 1. Ler o nó filho para obter a chave a ser promovida
        // Lendo apenas o cabeçalho para obter a primeira chave
        char buffer_leitura[sizeof(NoInterno)]; 
        arvore.seekg(filhoOffset, std::ios::beg);
        
        if (!arvore.read(buffer_leitura, sizeof(NoInterno))) {
            std::cerr << "ERRO FATAL: Falha ao ler nó filho no offset " << filhoOffset << ". Ponteiro inválido no nivel anterior!\n";
            return {}; 
        }
        
        bool is_folha = *reinterpret_cast<bool*>(buffer_leitura);
        ChaveTitulo chavePromovida;
        
        // A chave promovida é a primeira chave do nó filho (regra do B+ Tree Bulk Load)
        if (is_folha) {
            // Acessa a primeira chave do NoFolha
            strcpy(chavePromovida.titulo, reinterpret_cast<NoFolha*>(buffer_leitura)->chaves[0].titulo);
        } else {
            // Acessa a primeira chave do NoInterno
            strcpy(chavePromovida.titulo, reinterpret_cast<NoInterno*>(buffer_leitura)->chaves[0].titulo);
        }
        
        // 2. Inserir a chave e o ponteiro para o novo filho (índice i)
        std::uint32_t indiceChave = noInterno.nChaves; 
        
        // A chave é a separadora
        strcpy(noInterno.chaves[indiceChave].titulo, chavePromovida.titulo); 
        
        noInterno.filhos[indiceChave + 1] = filhoOffset; 
        noInterno.nChaves++;


        // 3. Overflow: Inicia um novo nó interno, movendo o último ponteiro para P0
        if (noInterno.nChaves == MAX_CHAVES_INTERNO) { 
            
            Offset offsetEscrita = offsetAtual;
            escreverNoInterno(arvore, noInterno, offsetEscrita);
            totalBlocosCriados++; // Necessário que 'totalBlocosCriados' esteja definida.
            offsetsNovoNivel.push_back(offsetEscrita);

            offsetAtual += sizeof(NoInterno);

            // Transfere o último filho para ser o primeiro filho do novo nó
            Offset ultimoFilho = noInterno.filhos[MAX_CHAVES_INTERNO];
            std::memset(&noInterno, 0, sizeof(NoInterno));
            noInterno.ehFolha = false;
            noInterno.filhos[0] = ultimoFilho; 
            noInterno.nChaves = 0; 
        }
    }

    // Escreve o nó interno que sobrou (ou o único nó criado)
    if (noInterno.nChaves > 0 || offsetsNovoNivel.empty()) { 
        Offset offsetEscrita = offsetAtual;
        escreverNoInterno(arvore, noInterno, offsetEscrita);
        totalBlocosCriados++; // Necessário que 'totalBlocosCriados' esteja definida.
        offsetsNovoNivel.push_back(offsetEscrita);
        offsetAtual += sizeof(NoInterno);
    }
    
    // printf("Total de blocos do arquivo de arvore: %d\n", totalBlocosCriados); // Removido para evitar erro de variável global não definida.
    return offsetsNovoNivel;
}


std::vector<Offset> buscarNaArvoreBPlus(std::fstream &arvore, const char* chaveBusca, Offset ofRaiz) { 
    arvore.clear();
    Offset currentOffset = ofRaiz;
    std::uint32_t blocosLidos = 0; 

    if (currentOffset <= 0) {
         std::cerr << "[Error] Offset da raiz é inválido: " << currentOffset << "\n";
         return {}; // Retorna vetor vazio
    }

    while (currentOffset != -1) {
        
        char bufferNo[sizeof(NoInterno)]; // 4KB
        arvore.seekg(currentOffset, std::ios::beg);
        
        if (!arvore.read(bufferNo, sizeof(NoInterno))) {
            std::cerr << "Erro ao ler nó no offset " << currentOffset << ".\n";
            std::cout << "Total de blocos lidos: " << blocosLidos << ".\n"; 
            return {}; // Retorna vetor vazio
        }

        blocosLidos++; 

        bool ehFolha = *reinterpret_cast<bool*>(bufferNo);

        if (ehFolha) {
            NoFolha* folha = reinterpret_cast<NoFolha*>(bufferNo);
            
            // Busca binária na folha (igual a antes)
            int inicio = 0;
            int fim = folha->nChaves - 1;
            int indiceEncontrado = -1;

            while (inicio <= fim) {
                int meio = inicio + (fim - inicio) / 2;
                int cmp = strcmp(chaveBusca, folha->chaves[meio].titulo);
                if (cmp == 0) {
                    indiceEncontrado = meio;
                    break;
                } else if (cmp < 0) { fim = meio - 1; }
                  else { inicio = meio + 1; }
            }

            // lista de blocos para achar todos os registros iguais
            if (indiceEncontrado != -1) {
                // Chave encontrada! Agora, percorre a lista de BlocoLista.
                std::cout << "\nSUCESSO: Chave \"" << chaveBusca << "\" encontrada.\n";
                
                std::vector<Offset> resultados;
                
                // Pega o ponteiro para o *início* da lista de duplicados.
                Offset offsetLista = folha->offsetsRegistros[indiceEncontrado];
                BlocoLista blocoLista; // Buffer para ler os blocos de lista

                // Loop para percorrer a lista encadeada de BlocoLista
                while (offsetLista != -1) {
                    arvore.seekg(offsetLista, std::ios::beg);
                    if (!arvore.read(reinterpret_cast<char*>(&blocoLista), sizeof(BlocoLista))) {
                         std::cerr << "Erro fatal: Bloco de lista corrompido no offset " << offsetLista << "\n";
                         break; // Retorna o que encontrou até agora
                    }
                    blocosLidos++; // Conta o BlocoLista como um bloco lido

                    // Adiciona todos os offsets deste bloco ao vetor de resultados
                    for (std::uint32_t i = 0; i < blocoLista.nOffsets; ++i) {
                        resultados.push_back(blocoLista.offsetsRegistros[i]);
                    }
                    
                    offsetLista = blocoLista.proxBlocoLista; // Vai para o próximo bloco
                }

                std::cout << "Total de " << resultados.size() << " registro(s) encontrado(s).\n";
                std::cout << "Quantidade de blocos lidos para encontrar: " << blocosLidos << "\n"; 
                return resultados; 
            }
            // --- Fim da mudança ---

            std::cout << "Chave não encontrada na folha.\n";
            std::cout << "Quantidade de blocos lidos para encontrar: " << blocosLidos << "\n"; 
            return {}; // Retorna vetor vazio
        } 
        else { // Nó Interno (lógica idêntica a antes)
            NoInterno* no = reinterpret_cast<NoInterno*>(bufferNo);
            std::uint32_t i = 0;
            while (i < no->nChaves && strcmp(chaveBusca, no->chaves[i].titulo) >= 0) {
                i++;
            }
            currentOffset = no->filhos[i];
            
            if (currentOffset <= 0) { 
                std::cerr << "No interno encontrou ponteiro filho inválido: " << no->filhos[i] << ".\n";
                std::cout << "Total de blocos lidos: " << blocosLidos << ".\n"; 
                return {}; // Retorna vetor vazio
            }
        }
    }
    std::cout << "Total de blocos lidos: " << blocosLidos << ".\n"; 
    return {}; // Retorna vetor vazio
}

void lerEImprimirRegistro(const std::vector<Offset>& offsetsRegistros) { 
    std::ifstream estatistica_arv_sec("../data/estatistica.txt"); 
    int blocosarq = 0;
    std::string linha;

    if (estatistica_arv_sec.is_open()) { 
        if(std::getline(estatistica_arv_sec, linha)) {
            std::sscanf(linha.c_str(), "Total de blocos do arquivo de dados: %d\n", &blocosarq);
        }
        estatistica_arv_sec.close(); 
    } else {
         std::cerr << "Aviso: Nao foi possivel abrir estatistica.txt\n";
    }  

    // Se o vetor de offsets estiver vazio, o registro não foi encontrado
    if (offsetsRegistros.empty()) { 
        if (blocosarq > 0) {
            std::cout << "Total de blocos (da arvore): " << blocosarq << "\n";
        }
        std::cout << "[AVISO] Registro não encontrado.\n";
        return;
    }
    
    // CORREÇÃO: Caminho para dados.bin
    std::ifstream arq_dados("../bin/dados.bin", std::ios::binary);
    if (!arq_dados.is_open()) { 
         std::cerr << "Erro ao abrir ../bin/dados.bin\n";
         return;
    }
    
    // --- MUDANÇA: Loop para imprimir todos os registros ---
    int contador = 1;
    for (Offset offset : offsetsRegistros) {
        registro reg;
        arq_dados.seekg(offset, std::ios::beg);

        if (arq_dados.read(reinterpret_cast<char*>(&reg), sizeof(registro))) {
            if (contador == 1 && blocosarq > 0) { // Imprime o total de blocos só uma vez
                std::cout << "Total de blocos (da arvore): " << blocosarq << "\n";
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
    // --- Fim da mudança ---
    
    arq_dados.close();
}

int cria_arvore_secundaria() { // Função pra criar e povoar a árvore em memoria secundaria
    std::ifstream dados("../bin/dados.bin", std::ios::binary);
    std::fstream arvore("../bin/arvore_sec.bin", std::ios::binary | std::ios::trunc | std::ios::in | std::ios::out);
    std::ofstream estatistica_arv_sec("../data/estatistica.txt"); // Para escrever a quantidade de blocos e não precisar ficar recalculando


    if (!dados.is_open() || !arvore.is_open()) {
        std::cerr << "Erro ao abrir arquivos.\n";
        return 1;
    }

    Offset placeholderRaiz = 0; 
    arvore.write(reinterpret_cast<char*>(&placeholderRaiz), sizeof(Offset));
    
    // Offset inicial para os nós da árvore (APÓS o ponteiro da raiz)
    Offset offsetAtual = sizeof(Offset); 
    arvore.flush();

    std::vector<Offset> nivelAtual = construirFolhas(arvore, dados, offsetAtual);
    
    while (nivelAtual.size() > 1) {
        nivelAtual = construirNivelInterno(arvore, nivelAtual, offsetAtual);
    }

    // Pega a raiz (ou -1 se o vetor estiver vazio)
    raizOffset = (nivelAtual.empty() ? -1 : nivelAtual[0]); 
    
    // Agora escreve o offset da raiz real no início do arquivo
    arvore.seekp(0, std::ios::beg);
    arvore.write(reinterpret_cast<char*>(&raizOffset), sizeof(Offset));
    arvore.flush();

    std::cout << "Árvore construída. Offset da raiz eh: " << raizOffset << "\n";

    if (!estatistica_arv_sec.is_open()) { 
        std::cerr << "Erro ao abrir estatistica.txt\n";
        // Não retorna 1, a árvore foi criada
    } else {
        estatistica_arv_sec << "Total de blocos do arquivo de dados: " << totalBlocosCriados << "\n";
        estatistica_arv_sec.close(); // CORREÇÃO: Fechar arquivo
    }
    
    totalBlocosCriados = 0; 

    dados.close();
    arvore.close();

    return 0;
}
