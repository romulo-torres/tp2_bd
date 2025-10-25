#include "hashE.h"

// --- Funções de I/O de DISCO ---

Bucket HashE::ler_bucket_do_disco(long offset) {
    Bucket b;
    bucket_file.seekg(offset);
    bucket_file.read(reinterpret_cast<char*>(&b), sizeof(Bucket));
    if (bucket_file.fail()) {
        throw std::runtime_error("Erro ao ler bucket do disco.");
    }
    num_reads++; // contagem de I/O (leituras) para fins de análise
    return b;
}

void HashE::escrever_bucket_no_disco(long offset, const Bucket& bucket) {
    bucket_file.seekp(offset);
    bucket_file.write(reinterpret_cast<const char*>(&bucket), sizeof(Bucket));
    if (bucket_file.fail()) {
        throw std::runtime_error("Erro ao escrever bucket no disco.");
    }
    num_writes++; // contagem de I/O (escritas) para fins de análise
}

long HashE::anexar_bucket_no_disco(const Bucket& bucket) {
    long offset_para_escrever = proximo_offset_livre;
    
    bucket_file.seekp(offset_para_escrever);
    bucket_file.write(reinterpret_cast<const char*>(&bucket), sizeof(Bucket));
    if (bucket_file.fail()) {
        throw std::runtime_error("Erro ao anexar bucket no disco.");
    }
    
    proximo_offset_livre = bucket_file.tellp(); // atualiza posição de anexação (fim do arquivo)
    num_anexos++; // contagem de anexos/escritas
    return offset_para_escrever;
}

// --- Funções de Gerenciamento do CACHE ---

void HashE::cache_touch(long offset) {
    if (cache_lru_map.count(offset)) {
        cache_lru_list.erase(cache_lru_map[offset]);
    }
    cache_lru_list.push_front(offset);
    cache_lru_map[offset] = cache_lru_list.begin();
}

void HashE::cache_evict() {
    long lru_offset = cache_lru_list.back();
    
    if (cache_dirty_pages[lru_offset]) {
        escrever_bucket_no_disco(lru_offset, cache_pool[lru_offset]);
    }
    
    cache_pool.erase(lru_offset);
    cache_lru_map.erase(lru_offset);
    cache_dirty_pages.erase(lru_offset);
    cache_lru_list.pop_back();
}

void HashE::flush_cache() {
    for (auto const& [offset, is_dirty] : cache_dirty_pages) {
        if (is_dirty) {
            escrever_bucket_no_disco(offset, cache_pool[offset]);
        }
    }
    cache_dirty_pages.clear();
}

// --- Funções de I/O de CACHE (as novas) ---

Bucket HashE::ler_bucket(long offset) {
    if (cache_pool.count(offset)) { // Acerto no cache (cache hit)
        cache_touch(offset);
        return cache_pool[offset];
    }
    
    // Falha no cache (cache miss)
    if (cache_pool.size() >= cache_max_size) {
        cache_evict();
    }
    
    Bucket b = ler_bucket_do_disco(offset);
    
    cache_pool[offset] = b;
    cache_touch(offset); // Adiciona na lista LRU
    cache_dirty_pages[offset] = false; 
    
    return b;
}

void HashE::escrever_bucket(long offset, const Bucket& bucket) {
    cache_pool[offset] = bucket;
    cache_dirty_pages[offset] = true;
    cache_touch(offset); // Escrever também conta como "usar"
}

long HashE::anexar_bucket(const Bucket& bucket) {
    // Anexar sempre vai para o disco para obter um offset
    long offset = anexar_bucket_no_disco(bucket);
    
    // E já que acabamos de escrever, podemos colocar no cache
    if (cache_pool.size() >= cache_max_size) {
        cache_evict();
    }
    cache_pool[offset] = bucket;
    cache_touch(offset);
    cache_dirty_pages[offset] = false; // Acabou de ser escrito, não está "sujo"
    
    return offset;
}


// --- Lógica do Hashing (Adaptada para (Chave, Valor)) ---

int HashE::hash(int chave) {
    return std::hash<int>{}(chave);
}

int HashE::get_index(int chave, int prof) {
    int h = hash(chave);
    return h & ((1 << prof) - 1);
}

void HashE::duplicar_diretorio() {
    int tam_antigo = diretorio.size();
    prof_global++;
    diretorio.resize(1 << prof_global);
    for (int i = 0; i < tam_antigo; ++i) {
        diretorio[i + tam_antigo] = diretorio[i];
    }
}

void HashE::inserir_chave(int chave, long offset, bool rehashing) {
    int indice = get_index(chave, prof_global);
    long offset_antigo = diretorio[indice];
    Bucket bucket_antigo = ler_bucket(offset_antigo); // <<< USA O CACHE

    // Tenta inserir na cópia em RAM
    if (!bucket_antigo.ta_cheio()) {
        bucket_antigo.inserir(chave, offset);
        escrever_bucket(offset_antigo, bucket_antigo); // <<< USA O CACHE
        return; 
    }

    // --- Bucket cheio ---
    if (rehashing) { 
        std::cerr << "Aviso: Falha ao reinserir chave " << chave << std::endl;
        return;
    }

    // 1. Duplicar diretório se necessário
    if (bucket_antigo.prof_local == prof_global) {
        duplicar_diretorio();
    }

    // 2. Preparar buckets em RAM
    int ld_antigo = bucket_antigo.prof_local;
    int ld_novo = ld_antigo + 1;

    bucket_antigo.prof_local = ld_novo;
    Bucket bucket_novo(ld_novo);

    // Salva chaves E offsets antigos
    std::vector<int> chaves_temporarias = bucket_antigo.get_chaves();
    std::vector<long> offsets_temporarios = bucket_antigo.get_offsets();
    
    // Adiciona a nova chave/offset
    chaves_temporarias.push_back(chave);
    offsets_temporarios.push_back(offset);

    // Limpa o bucket antigo (em RAM)
    bucket_antigo.num_chaves = 0; 

    // 3. Anexa o novo bucket VAZIO ao disco para obter seu offset
    long offset_novo = anexar_bucket(bucket_novo); // <<< ESCREVE NO DISCO

    // 4. Re-apontar o diretório (em memória)
    int primeiro_indice_base = get_index(chaves_temporarias[0], ld_antigo);
    for (int i = 0; i < diretorio.size(); ++i) {
        if (diretorio[i] == offset_antigo) {
            if ((i >> ld_antigo) & 1) {
                diretorio[i] = offset_novo;
            }
        }
    }
    
    // 5. Redistribuir TODAS as chaves e offsets (em RAM)
    for (size_t i = 0; i < chaves_temporarias.size(); ++i) {
        int k = chaves_temporarias[i];
        long o = offsets_temporarios[i];
        
        int idx_k = get_index(k, prof_global);
        
        if (diretorio[idx_k] == offset_antigo) {
            bucket_antigo.inserir(k, o); // Insere no bucket_antigo (RAM)
        } else {
            bucket_novo.inserir(k, o); // Insere no bucket_novo (RAM)
        }
    }

    // 6. Escrever os dois buckets (agora preenchidos) de volta no disco
    escrever_bucket(offset_antigo, bucket_antigo); // <<< USA O CACHE
    escrever_bucket(offset_novo, bucket_novo);     // <<< USA O CACHE
}

// --- Métodos Públicos ---

HashE::HashE(int prof_inicial, const std::string& filename, size_t cache_size)
    : prof_global(prof_inicial), 
      bucket_filename(filename),
      proximo_offset_livre(0),
      num_reads(0),
      num_writes(0),
      num_anexos(0),
      cache_max_size(cache_size)
{
    if (cache_size < 2) {
        throw std::invalid_argument("Tamanho do cache deve ser pelo menos 2.");
    }

    // Abre o arquivo (trunc = apaga se existir)
    bucket_file.open(bucket_filename, std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);
    if (!bucket_file.is_open()) {
        throw std::runtime_error("Nao foi possivel abrir/criar o arquivo de buckets.");
    }

    int tam_inicial = 1 << prof_global;
    diretorio.resize(tam_inicial);
    
    // Cria o primeiro bucket no arquivo
    Bucket primeiro_bucket(prof_global);
    long primeiro_offset = anexar_bucket(primeiro_bucket); // Usa o anexar (com cache)

    for (int i = 0; i < tam_inicial; ++i) {
        diretorio[i] = primeiro_offset;
    }
}

HashE::~HashE() {
    flush_cache(); // Salva tudo
    if (bucket_file.is_open()) {
        bucket_file.close();
    }
}

void HashE::inserir(int chave, long offset) {
    // Antes de inserir, verifica se a chave já existe
    if (buscar(chave) != -1) {
         std::cerr << "Aviso: Chave " << chave << " ja existe. Insercao ignorada." << std::endl;
         return;
    }
    inserir_chave(chave, offset, false);
}

long HashE::buscar(int chave) {
    int indice = get_index(chave, prof_global);
    long offset = diretorio[indice];
    Bucket b = ler_bucket(offset); // <<< USA O CACHE
    
    // Otimização: Se o bucket não tem a chave e sua profundidade
    // local for menor que a global, não precisamos fazer mais nada.
    // (O bucket de destino real já foi verificado).
    
    return b.buscar(chave);
}

long HashE::get_total_blocos() const {
    // O número total de blocos é o tamanho do arquivo
    // dividido pelo tamanho do bloco (sizeof(Bucket))
    
    // 'proximo_offset_livre' é o tamanho total em bytes
    if (proximo_offset_livre == 0) return 0;
    
    return proximo_offset_livre / sizeof(Bucket);
}

void HashE::print() {
    flush_cache();
    std::cout << "== Estado do Hash Index ==" << std::endl;
    std::cout << "Profundidade Global: " << prof_global << std::endl;
    std::cout << "Total de Blocos no Arquivo: " << get_total_blocos() << std::endl;

    std::map<long, int> bucket_refs;
    for (long offset : diretorio) {
        bucket_refs[offset]++; 
    }
    
    std::cout << "Total de Buckets unicos: " << bucket_refs.size() << std::endl;
    
    for (auto const& [offset, ref_count] : bucket_refs) {
        std::cout << "\n  Bucket (Offset: " << offset << ") - " << ref_count << " ponteiros" << std::endl;
        Bucket b = ler_bucket_do_disco(offset); // Lê direto do disco para printar
        std::cout << "    Prof. Local: " << b.prof_local << std::endl;
        std::cout << "    Chaves (" << b.num_chaves << "/" << BUCKET_CAPACITY << "): ";
        for (size_t i = 0; i < b.num_chaves; ++i) {
            std::cout << b.chaves[i] << " (->" << b.offsets[i] << ") ";
            if (i > 5) { std::cout << "..."; break; } // Evita poluir
        }
        std::cout << std::endl;
    }
}

std::map<long, std::list<int>> HashE::snapshot_buckets() {
    std::map<long, std::list<int>> snapshot;
    std::map<long, bool> seen;

    for (long offset : diretorio) {
        if (seen[offset]) continue;
        // Lê diretamente do disco (não usa cache para não mudar contadores)
        Bucket b = ler_bucket_do_disco(offset);
        std::vector<int> chs = b.get_chaves();
        std::list<int> lst(chs.begin(), chs.end());
        snapshot[offset] = lst;
        seen[offset] = true;
    }

    return snapshot;
}