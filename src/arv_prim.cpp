#include "../include/arv_prim.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include <cstddef>


Offset raizOffset_prim = -1; 
int totalBlocosCriados_prim = 0;
unsigned primeiraChaveLida_prim = 0;


#define TAM_BLOCO 4096

// Variável global para a raiz (usada apenas para diagnóstico agora)
Offset raizOffset = -1;

Offset escreverNoFolha_prim(std::fstream &arq, NoFolha &no, Offset offsetEscrita) {
    arq.seekp(offsetEscrita, std::ios::beg);
    arq.write(reinterpret_cast<char*>(&no), sizeof(NoFolha));
    arq.flush();
    return offsetEscrita;
}

Offset escreverNoInterno_prim(std::fstream &arq, NoInterno &no, Offset offsetEscrita) {
    arq.seekp(offsetEscrita, std::ios::beg);
    arq.write(reinterpret_cast<char*>(&no), sizeof(NoInterno));
    arq.flush();
    return offsetEscrita;
}

void atualizarProxFolha_prim(std::fstream &arq, Offset offsetFolhaAnterior, Offset offsetProx) {
    arq.seekp(offsetFolhaAnterior + offsetof(NoFolha, prox), std::ios::beg);
    arq.write(reinterpret_cast<char*>(&offsetProx), sizeof(Offset));
    arq.flush();
}


std::vector<Offset> construirFolhas_prim(std::fstream &arvore, std::ifstream &dados, Offset &offsetAtual) {
    std::vector<Offset> offsetsFolhas;
    NoFolha folha;
    // Buffer para ler 2 registros (TAM_BLOCO)
    char buffer[TAM_BLOCO]; 
    Offset offsetFolhaAnterior = -1; 
    
    std::memset(&folha, 0, sizeof(NoFolha));
    folha.ehFolha = true; 
 
    dados.clear();
    dados.seekg(0, std::ios::beg);
    
    bool primeiraLeitura = true; 
    
    // CORREÇÃO: Calcular offset manualmente para evitar problemas de conversão
    Offset offsetRegistroAtual = 0;
    
    // controlamos a saída com gcount() para garantir o processamento do bloco parcial no EOF.
    while (true) {
        
        // 1. Captura o offset inicial ANTES de tentar a leitura.
        Offset offsetInicioBloco = offsetRegistroAtual;
        
        // Tenta ler TAM_BLOCO bytes. Se houver menos (EOF), a leitura falha, mas os bytes lidos são armazenados.
        dados.read(buffer, TAM_BLOCO);
        
        // 2. Verifica quantos bytes foram REALMENTE lidos.
        std::streamsize bytes_lidos = dados.gcount();
        
        // Se 0 bytes foram lidos, chegamos ao EOF e não há mais nada a processar.
        if (bytes_lidos == 0) {
            break;
        }

        // 3. Determina o número de registros COMPLETOS lidos.
        int num_registros_no_bloco = bytes_lidos / TAM_REGISTRO;

        // Processa os registros lidos (será 1 ou 2, dependendo do tamanho do bloco)
        for(int i = 0; i < num_registros_no_bloco; i ++){
            // Calcula o endereço do registro dentro do buffer
            registro* reg = reinterpret_cast<registro*>(buffer + (i * TAM_REGISTRO));
            
            if (primeiraLeitura) {
                primeiraLeitura = false;
            }

            folha.chaves[folha.nChaves] = reg->id;
            
            // CORREÇÃO: Usa offset calculado manualmente
            folha.offsetsRegistros[folha.nChaves] = offsetInicioBloco + (i * TAM_REGISTRO); 
            
            folha.nChaves++;

            if (folha.nChaves == MAX_CHAVES_FOLHA) {
                
                Offset offsetEscrita = offsetAtual;
                
                if (offsetFolhaAnterior != -1) {
                    atualizarProxFolha_prim(arvore, offsetFolhaAnterior, offsetEscrita);
                }
                
                escreverNoFolha_prim(arvore, folha, offsetEscrita);
                totalBlocosCriados_prim++; // Necessário que 'totalBlocosCriados' esteja definida.
                
                offsetsFolhas.push_back(offsetEscrita);
                
                offsetFolhaAnterior = offsetEscrita;
                offsetAtual += sizeof(NoFolha); 
                
                std::memset(&folha, 0, sizeof(NoFolha));
                folha.ehFolha = true;
            }
        }
        
        // Atualiza o offset para o próximo bloco
        offsetRegistroAtual += bytes_lidos;
        
        // 4. Se a leitura falhou (possivelmente por EOF após leitura parcial), 
        // limpamos o estado e o loop deve encerrar na próxima iteração (gcount() == 0).
        if (dados.fail()) {
            dados.clear();
        }
        
        // Se o número de bytes lidos for menor que o TAM_BLOCO, processamos o bloco parcial e encerramos.
        if (bytes_lidos < TAM_BLOCO) {
            break;
        }
    }
    
    // Última folha incompleta (se houver chaves restantes no buffer 'folha')
    if (folha.nChaves > 0) {
        Offset offsetEscrita = offsetAtual;
        
        if (offsetFolhaAnterior != -1) {
            atualizarProxFolha_prim(arvore, offsetFolhaAnterior, offsetEscrita);
        }
        
        escreverNoFolha_prim(arvore, folha, offsetEscrita);
        totalBlocosCriados_prim++; // Necessário que 'totalBlocosCriados' esteja definida.
        offsetsFolhas.push_back(offsetEscrita);
        
        offsetAtual += sizeof(NoFolha);
    }
    
    return offsetsFolhas;
}

// Manter a função construirNivelInterno para contexto.
std::vector<Offset> construirNivelInterno_prim(std::fstream &arvore, const std::vector<Offset> &nivelAnterior, Offset &offsetAtual) {
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
        std::uint32_t chavePromovida;
        
        // A chave promovida é a primeira chave do nó filho (regra do B+ Tree Bulk Load)
        if (is_folha) {
            // Acessa a primeira chave do NoFolha
            chavePromovida = reinterpret_cast<NoFolha*>(buffer_leitura)->chaves[0];
        } else {
            // Acessa a primeira chave do NoInterno
            chavePromovida = reinterpret_cast<NoInterno*>(buffer_leitura)->chaves[0];
        }
        
        // 2. Inserir a chave e o ponteiro para o novo filho (índice i)
        std::uint32_t indiceChave = noInterno.nChaves; 
        
        // A chave é a separadora
        noInterno.chaves[indiceChave] = chavePromovida; 
        
        // O ponteiro para o filho (filhoOffset) vai para a direita da chave
        noInterno.filhos[indiceChave + 1] = filhoOffset; 
        
        noInterno.nChaves++;

        // 3. Overflow: Inicia um novo nó interno, movendo o último ponteiro para P0
        if (noInterno.nChaves == MAX_CHAVES_INTERNO) { 
            
            Offset offsetEscrita = offsetAtual;
            escreverNoInterno_prim(arvore, noInterno, offsetEscrita);
            totalBlocosCriados_prim++; // Necessário que 'totalBlocosCriados' esteja definida.
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
        escreverNoInterno_prim(arvore, noInterno, offsetEscrita);
        totalBlocosCriados_prim++; // Necessário que 'totalBlocosCriados' esteja definida.
        offsetsNovoNivel.push_back(offsetEscrita);
        offsetAtual += sizeof(NoInterno);
    }
    
    // printf("Total de blocos do arquivo de arvore: %d\n", totalBlocosCriados); // Removido para evitar erro de variável global não definida.
    return offsetsNovoNivel;
}


Offset buscarNaArvoreBPlus_prim(std::fstream &arvore, std::uint32_t chaveBusca, Offset ofRaiz) { // Implementamos com busca binaria nas folhas pra melhorar o desempenho
    arvore.clear();
    Offset currentOffset = ofRaiz;
    std::uint32_t blocosLidos = 0; // Inicializa o contador de blocos lidos

    while (currentOffset != -1) {
        
        // Usa o tamanho do NoInterno como buffer de leitura por ser geralmente o maior.
        char bufferNo[sizeof(NoInterno)];
        
        arvore.seekg(currentOffset, std::ios::beg);
        
        if (!arvore.read(bufferNo, sizeof(NoInterno))) {
            std::cerr << "Erro ao ler nó no offset " << currentOffset << ". Offset inválido ou I/O falhou.\n";
            std::cout << "Total de blocos lidos: " << blocosLidos << ".\n"; // Imprime antes de sair por erro
            return -1;
        }

        blocosLidos++; // Incrementa o contador após a leitura bem-sucedida de um bloco

        bool ehFolha = *reinterpret_cast<bool*>(bufferNo);

        if (ehFolha) {
            // O cast é seguro porque o bufferNo contém o conteúdo do NoFolha
            NoFolha* folha = reinterpret_cast<NoFolha*>(bufferNo);
                        
            
            // 1. Usa lower_bound para encontrar a posição onde a chaveBusca deveria estar
            auto it = std::lower_bound(
                folha->chaves, 
                folha->chaves + folha->nChaves, 
                chaveBusca
            );
            
            // 2. Calcula o índice (distância)
            std::uint32_t indice = std::distance(folha->chaves, it);

            // 3. Verifica se o índice está dentro dos limites do array E se a chave no índice corresponde à chave de busca
            if (indice < folha->nChaves && folha->chaves[indice] == chaveBusca) {
                // Chave encontrada!
                std::cout << "\nSUCESSO: Chave " << chaveBusca << " encontrada via busca binária no índice " << indice << " (Offset Reg: " << folha->offsetsRegistros[indice] << ")\n";
                std::cout << "\nQuantidade de blocos lidos para encontrar: " << blocosLidos << "\n"; // Imprime no sucesso
                return folha->offsetsRegistros[indice]; 
            }
            
            std::cout << "Quantidade de blocos lidos para encontrar: " << blocosLidos << "\n"; // Imprime se a chave não for encontrada na folha
            
            return -1; // Chave não encontrada na folha
        } else {
            NoInterno* no = reinterpret_cast<NoInterno*>(bufferNo);

            std::uint32_t i = 0;
            // Busca o primeiro índice onde a chave de busca é menor que a chave do nó
            while (i < no->nChaves && chaveBusca >= no->chaves[i])
                i++;

            currentOffset = no->filhos[i];
            
            if (currentOffset <= 0) { 
                std::cerr << "No interno no offset " << currentOffset << " tem ponteiro filho inválido: " << no->filhos[i] << ".\n";
                std::cout << "Total de blocos lidos: " << blocosLidos << ".\n"; // Imprime antes de sair
                return -1;
            }
        }
    }

    std::cout << "Total de blocos lidos: " << blocosLidos << ".\n"; // Imprime se sair do loop (raiz -1)
    return -1;
}


void lerEImprimirRegistro_prim(Offset offsetRegistro) { // Função que recebe o Offset do registro e imprime os campos deles
    std::ifstream estatistica_arv_prim("../data/estatistica.txt"); // Para escrever a quantidade de blocos e não precisar ficar recalculando
    int blocosarq;
    std::string linha;

    if (offsetRegistro < 0) { // Se for menor q zero
        std::getline(estatistica_arv_prim, linha);
        std::sscanf(linha.c_str(), "Total de blocos do arquivo de dados: %d\n", &blocosarq);
        std::cout << "Total de blocos: " << blocosarq << "\n";
        std::cout << "Registro não encontrado (offset inválido)\n";
        return;
    }
    
    if (!estatistica_arv_prim.is_open()) { // Se dê pra abrir o arquivo de estatatisticas
        std::cerr << "Erro ao abrir estatistica.txt\n";
        return;
    }
    
    std::getline(estatistica_arv_prim, linha);
    std::sscanf(linha.c_str(), "Total de blocos do arquivo de dados: %d\n", &blocosarq);

    std::ifstream arq_dados("../bin/dados.in", std::ios::binary);
    registro reg;
    arq_dados.seekg(offsetRegistro, std::ios::beg);

    if (arq_dados.read(reinterpret_cast<char*>(&reg), sizeof(registro))) {
        std::cout << "Total de blocos: " << blocosarq << "\n";
        std::cout << "Registro encontrado!\n";
        std::cout << "\nCampos:\n\n";
        std::cout << "ID: " << reg.id << "\n";
        std::cout << "Titulo: " << reg.titulo << "\n";
        std::cout << "Ano: " << reg.ano << "\n";
        std::cout << "Autores: " << reg.autores << "\n";
        std::cout << "Citacoes: " << reg.citacoes << "\n";
        std::cout << "Atualizacao: " << reg.data << "\n";
        std::cout << "Null Snippet: " << reg.null_snippet << "\n";
        std::cout << "Snippet: " << reg.snippet << "\n";
        
    } else {
        std::cout << "Erro ao ler registro no offset " << offsetRegistro << "\n";
    }
    //Fechamdo os arquivos
    arq_dados.close();
    estatistica_arv_prim.close();
}

int cria_arvore_primaria() { // Função pra criar e povoar a árvore em memoria secundaria
    std::ifstream dados("../bin/dados.in", std::ios::binary);
    std::fstream arvore("../bin/arvore_prim.in", std::ios::binary | std::ios::trunc | std::ios::in | std::ios::out);
    std::ofstream estatistica_arv_prim("../data/estatistica.txt"); // Para escrever a quantidade de blocos e não precisar ficar recalculando


    if (!dados.is_open() || !arvore.is_open()) {
        std::cerr << "Erro ao abrir arquivos.\n";
        return 1;
    }

    Offset offsetAtual = sizeof(Offset);
    arvore.write(reinterpret_cast<char*>(&offsetAtual), sizeof(Offset));
    arvore.flush();

    std::vector<Offset> nivelAtual = construirFolhas_prim(arvore, dados, offsetAtual);
    while (nivelAtual.size() > 1)
        nivelAtual = construirNivelInterno_prim(arvore, nivelAtual, offsetAtual);

    raizOffset = nivelAtual[0];
    arvore.seekp(0, std::ios::beg);
    arvore.write(reinterpret_cast<char*>(&raizOffset), sizeof(Offset));
    arvore.flush();

    std::cout << "Árvore construída. Offset da raiz eh: " << raizOffset << "\n";

    if (!estatistica_arv_prim.is_open()) { // Vejo se ta ok pra continuar
        std::cerr << "Erro ao abrir estatistica.txt\n";
        return 1;
    }
    estatistica_arv_prim << "Total de blocos do arquivo de dados: " << totalBlocosCriados_prim << "\n";
    totalBlocosCriados_prim = 0; // Reseto a variavel pra não acumular infinitamente

    //Fecho os arquivo
    dados.close();
    arvore.close();
    estatistica_arv_prim.close();

    return 0;
}