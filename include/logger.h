#pragma once // Evita que o arquivo seja incluído múltiplas vezes

#include <string>
#include <mutex>
#include <chrono>
#include <memory>
#include <fstream>

// Define os níveis de log que queremos
enum class LogLevel {
    INFO,
    WARNING,
    ERROR,
    INPUT
};

class Logger {
public:
    // Registro de log principal (implementação em src/logger.cpp)
    static void log(LogLevel level, const std::string& message);

    // Configurações adicionais
    static void setLogFile(const std::string& path); // escreve logs também em arquivo
    static void setMinLevel(LogLevel level);         // nível mínimo para logar

private:
    // membros estáticos (definidos em src/logger.cpp)
    static std::mutex logMutex;
    static std::chrono::steady_clock::time_point startTime;
    static LogLevel minLevel;
    static std::unique_ptr<std::ofstream> outFile;

    static std::string levelToString(LogLevel level);
};

// Macros auxiliares para conveniência
#define LOG_INFO(message)    Logger::log(LogLevel::INFO, message)
#define LOG_WARNING(message) Logger::log(LogLevel::WARNING, message)
#define LOG_ERROR(message)   Logger::log(LogLevel::ERROR, message)
#define LOG_INPUT(message)   Logger::log(LogLevel::INPUT, message)