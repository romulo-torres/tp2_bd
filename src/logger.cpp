// Implementação mínima para Logger declarado em include/logger.h
#include "../include/logger.h"

// Definição do mutex estático usado pelo Logger
std::mutex Logger::logMutex;

// Nota: o startTime é uma variável inline definida no header (C++17),
// portanto não precisamos defini-la aqui.
