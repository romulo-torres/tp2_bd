#pragma once // Evita que o arquivo seja incluído múltiplas vezes

#include <iostream> // Para std::cout e std::cerr
#include <string>   // Para std::string
#include <mutex>    // Para std::mutex (segurança em threads)
#include <chrono>   // Para std::chrono (pegar a hora atual)
#include <iomanip>  // Para std::put_time (formatar a hora)
#include <sstream>  // Para std::stringstream (construir a string de hora)

// Define os níveis de log que queremos
enum class LogLevel {
    INFO,
    WARNING,
    ERROR,
    INPUT
};

class Logger {
public:
    // A função principal de log
    // Usamos 'static' para não precisar criar um objeto Logger para usá-lo
    static void log(LogLevel level, const std::string& message) {
        // Pega a hora atual
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);

    // Tempo de execução desde o início do programa (steady clock)
    auto now_steady = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed = now_steady - startTime;

        // Formata a hora para [YYYY-MM-DD HH:MM:SS]
        std::stringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S");
        std::string timestamp = ss.str();

        // Pega o prefixo do nível (ex: "[INFO]")
        std::string levelStr = levelToString(level);

        // Usa um 'mutex' para bloquear a saída
        // Isso garante que se duas threads tentarem logar ao mesmo tempo,
        // uma espera a outra terminar, impedindo que as mensagens se misturem.
        std::lock_guard<std::mutex> lock(logMutex);

        // Escolhe para onde enviar o log:
        // std::cerr é a saída de erro (geralmente vermelha no terminal)
        // std::cout é a saída padrão
        std::ostream& output = (level == LogLevel::INFO) ? std::cout : std::cerr;

        // Imprime a mensagem final formatada (inclui tempo de execução em segundos)
        std::ostringstream elapsed_ss;
        elapsed_ss << std::fixed << std::setprecision(3) << elapsed.count();
        output << "[" << timestamp << "] [" << elapsed_ss.str() << "s] " << levelStr << ": " << message << std::endl;
    }

private:
    // Mutex estático para ser compartilhado por todas as chamadas
    static std::mutex logMutex;

    // tempo de início do programa (usado para medir tempo de execução)
    inline static std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();

    // Função auxiliar para converter o enum em string
    static std::string levelToString(LogLevel level) {
        switch (level) {
            case LogLevel::INFO:    return "[INFO]   ";
            case LogLevel::WARNING: return "[WARNING]";
            case LogLevel::ERROR:   return "[ERROR]  ";
            case LogLevel::INPUT:   return "[INPUT]  ";
            default:                return "[UNKNOWN]";
        }
    }
};

// A definição (instanciação) do mutex estático fica em src/logger.cpp

// --- Macros Auxiliares (Opcional, mas muito útil) ---
// Isso facilita a chamada da função de log no seu código
#define LOG_INFO(message)    Logger::log(LogLevel::INFO, message)
#define LOG_WARNING(message) Logger::log(LogLevel::WARNING, message)
#define LOG_ERROR(message)   Logger::log(LogLevel::ERROR, message)
#define LOG_INPUT(message)   Logger::log(LogLevel::INPUT, message)