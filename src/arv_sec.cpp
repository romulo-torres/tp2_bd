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
// MUDANÇA: Renomeado para _sec para evitar conflito
int totalBlocosCriados_sec = 0; 

// MUDANÇA: Renomeado para _sec para evitar conflito
Offset raizOffset_sec = -1;

// MUDANÇA: Corrigido o typo (removido o _sec extra)
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
    // MUDANÇA: Usando NoFolha_sec
    arq.seekp(offsetFolhaAnterior + offsetof(NoFolha_sec, prox), std::ios::beg);
    arq.write(reinterpret_cast<char*>(&offsetProx), sizeof(Offset));
    arq.flush();
}

Offset escreverListasDeOffsets_sec(std::fstream &arvore, const std::vector<Offset> &offsets, Offset &offsetAtual) {
    
    BlocoLista bloco;
    std::memset(&bloco, 0, sizeof(BlocoLista));
    bloco.proxBlocoLista = -1;

    Offset primeiroBlocoOffset = offsetAtual; // Salva o offset do primeiro bloco
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
    char tituloTruncado[300]; 

    while (dados.read(reinterpret_cast<char*>(&reg), sizeof(registro))) {
        Offset offsetDoRegistro = dados.tellg();
        offsetDoRegistro -= sizeof(registro);

        strncpy(tituloTruncado, reg.titulo, 299);
        tituloTruncado[299] = '\0';

        mapaChaves[tituloTruncado].push_back(offsetDoRegistro);
    }

    std::vector<Offset> offsetsFolhas;
    NoFolha_sec folha; 
    Offset offsetFolhaAnterior = -1;
    
    std::memset(&folha, 0, sizeof(NoFolha_sec)); 
    folha.ehFolha = true; 
    folha.prox = -1;

    for (auto const& [titulo, listaDeOffsets] : mapaChaves) {
        
        Offset primeiroBlocoOffset = escreverListasDeOffsets_sec(arvore, listaDeOffsets, offsetAtual);

        strcpy(folha.chaves[folha.nChaves].titulo, titulo.c_str());
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
        
        // MUDANÇA: Renomeado
        char buffer_leitura[sizeof(NoInterno_sec)]; 
        arvore.seekg(filhoOffset, std::ios::beg);
        
        // MUDANÇA: Renomeado
        if (!arvore.read(buffer_leitura, sizeof(NoInterno_sec))) {
            std::cerr << "ERRO FATAL: Falha ao ler nó filho no offset " << filhoOffset << ". Ponteiro inválido no nivel anterior!\n";
            return {}; 
        }
        
        bool is_folha = *reinterpret_cast<bool*>(buffer_leitura);
        ChaveTitulo chavePromovida;
        
        if (is_folha) {
            // MUDANÇA: Renomeado
            strcpy(chavePromovida.titulo, reinterpret_cast<NoFolha_sec*>(buffer_leitura)->chaves[0].titulo);
        } else {
            // MUDANÇA: Renomeado
            strcpy(chavePromovida.titulo, reinterpret_cast<NoInterno_sec*>(buffer_leitura)->chaves[0].titulo);
        }
        
        std::uint32_t indiceChave = noInterno.nChaves; 
        
        strcpy(noInterno.chaves[indiceChave].titulo, chavePromovida.titulo); 
        
        noInterno.filhos[indiceChave + 1] = filhoOffset; 
        noInterno.nChaves++;

        // MUDANÇA: Renomeado
        if (noInterno.nChaves == MAX_CHAVES_INTERNO_SEC) { 
            
            Offset offsetEscrita = offsetAtual;
            escreverNoInterno_sec(arvore, noInterno, offsetEscrita);
            totalBlocosCriados_sec++; // MUDANÇA: Renomeado

            offsetsNovoNivel.push_back(offsetEscrita);

            offsetAtual += sizeof(NoInterno_sec); // MUDANÇA: Renomeado

            Offset ultimoFilho = noInterno.filhos[MAX_CHAVES_INTERNO_SEC]; // MUDANÇA: Renomeado
            std::memset(&noInterno, 0, sizeof(NoInterno_sec)); // MUDANÇA: Renomeado
            noInterno.ehFolha = false;
            noInterno.filhos[0] = ultimoFilho; 
            noInterno.nChaves = 0; 
        }
    }

    if (noInterno.nChaves > 0 || offsetsNovoNivel.empty()) { 
        Offset offsetEscrita = offsetAtual;
        escreverNoInterno_sec(arvore, noInterno, offsetEscrita);
        totalBlocosCriados_sec++; // MUDANÇA: Renomeado
        offsetsNovoNivel.push_back(offsetEscrita);
        offsetAtual += sizeof(NoInterno_sec); // MUDANÇA: Renomeado
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

    while (currentOffset != -1) {
        
        char bufferNo[sizeof(NoInterno_sec)]; // MUDANÇA: Renomeado
        arvore.seekg(currentOffset, std::ios::beg);
        
        if (!arvore.read(bufferNo, sizeof(NoInterno_sec))) { // MUDANÇA: Renomeado
            std::cerr << "Erro ao ler nó no offset " << currentOffset << ".\n";
            std::cout << "Total de blocos lidos: " << blocosLidos << ".\n"; 
            return {}; 
        }

        blocosLidos++; 

        bool ehFolha = *reinterpret_cast<bool*>(bufferNo);

        if (ehFolha) {
            NoFolha_sec* folha = reinterpret_cast<NoFolha_sec*>(bufferNo); // MUDANÇA: Renomeado
            
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

            if (indiceEncontrado != -1) {
                std::cout << "\nSUCESSO: Chave \"" << chaveBusca << "\" encontrada.\n";
                
                std::vector<Offset> resultados;
                
                Offset offsetLista = folha->offsetsRegistros[indiceEncontrado];
                BlocoLista blocoLista; 

                while (offsetLista != -1) {
                    arvore.seekg(offsetLista, std::ios::beg);
                    if (!arvore.read(reinterpret_cast<char*>(&blocoLista), sizeof(BlocoLista))) {
                         std::cerr << "Erro fatal: Bloco de lista corrompido no offset " << offsetLista << "\n";
                         break; 
                    }
                    blocosLidos++; 

                    for (std::uint32_t i = 0; i < blocoLista.nOffsets; ++i) {
                        resultados.push_back(blocoLista.offsetsRegistros[i]);
                    }
                    
                    offsetLista = blocoLista.proxBlocoLista; 
                }

                std::cout << "Total de " << resultados.size() << " registro(s) encontrado(s).\n";
                std::cout << "Quantidade de blocos lidos para encontrar: " << blocosLidos << "\n"; 
                return resultados; 
            }

            std::cout << "Chave não encontrada na folha.\n";
            std::cout << "Quantidade de blocos lidos para encontrar: " << blocosLidos << "\n"; 
            return {}; 
        } 
        else { 
            NoInterno_sec* no = reinterpret_cast<NoInterno_sec*>(bufferNo); // MUDANÇA: Renomeado
            std::uint32_t i = 0;
            while (i < no->nChaves && strcmp(chaveBusca, no->chaves[i].titulo) >= 0) {
                i++;
            }
            currentOffset = no->filhos[i];
            
            if (currentOffset <= 0) { 
                std::cerr << "No interno encontrou ponteiro filho inválido: " << no->filhos[i] << ".\n";
                std::cout << "Total de blocos lidos: " << blocosLidos << ".\n"; 
                return {}; 
            }
        }
    }
    std::cout << "Total de blocos lidos: " << blocosLidos << ".\n"; 
    return {}; 
}

void lerEImprimirRegistro_sec(const std::vector<Offset>& offsetsRegistros) { 
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

    if (offsetsRegistros.empty()) { 
        if (blocosarq > 0) {
            std::cout << "Total de blocos (da arvore): " << blocosarq << "\n";
        }
        std::cout << "[AVISO] Registro não encontrado.\n";
        return;
    }
    
    std::ifstream arq_dados("../bin/dados.bin", std::ios::binary);
    if (!arq_dados.is_open()) { 
         std::cerr << "Erro ao abrir ../bin/dados.bin\n";
         return;
    }
    
    int contador = 1;
    for (Offset offset : offsetsRegistros) {
        registro reg;
        arq_dados.seekg(offset, std::ios::beg);

        if (arq_dados.read(reinterpret_cast<char*>(&reg), sizeof(registro))) {
            if (contador == 1 && blocosarq > 0) { 
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
    
    arq_dados.close();
}

int cria_arvore_secundaria() { 
    std::ifstream dados("../bin/dados.bin", std::ios::binary);
    std::fstream arvore("../bin/arvore_sec.bin", std::ios::binary | std::ios::trunc | std::ios::in | std::ios::out);
    std::ofstream estatistica_arv_sec("../data/estatistica.txt"); 


    if (!dados.is_open() || !arvore.is_open()) {
        std::cerr << "Erro ao abrir arquivos.\n";
        return 1;
    }

    Offset placeholderRaiz = 0; 
    arvore.write(reinterpret_cast<char*>(&placeholderRaiz), sizeof(Offset));
    
    Offset offsetAtual = sizeof(Offset); 
    arvore.flush();

    std::vector<Offset> nivelAtual = construirFolhas_sec(arvore, dados, offsetAtual);
    
    while (nivelAtual.size() > 1) {
        nivelAtual = construirNivelInterno_sec(arvore, nivelAtual, offsetAtual);
    }

   
    raizOffset_sec = (nivelAtual.empty() ? -1 : nivelAtual[0]); 
    
    arvore.seekp(0, std::ios::beg);
 
    arvore.write(reinterpret_cast<char*>(&raizOffset_sec), sizeof(Offset));
    arvore.flush();

    
    std::cout << "Árvore Secundária construída. Offset da raiz eh: " << raizOffset_sec << "\n";

    if (!estatistica_arv_sec.is_open()) { 
        std::cerr << "Erro ao abrir estatistica.txt\n";
    } else {
 
        estatistica_arv_sec << "Total de blocos do arquivo de dados: " << totalBlocosCriados_sec << "\n";
        estatistica_arv_sec.close(); 
    }
    
    totalBlocosCriados_sec = 0;

    dados.close();
    arvore.close();

    return 0;
}

