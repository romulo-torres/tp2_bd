// Implementação do Logger (métodos e membros estáticos)
#include "../include/logger.h"

#include <iostream>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <memory>

// Definições dos membros estáticos
std::mutex Logger::logMutex;
std::chrono::steady_clock::time_point Logger::startTime = std::chrono::steady_clock::now();
LogLevel Logger::minLevel = LogLevel::INFO;
std::unique_ptr<std::ofstream> Logger::outFile = nullptr;

// Converte nível para string
std::string Logger::levelToString(LogLevel level) {
	switch (level) {
		case LogLevel::INFO:    return "[INFO]   ";
		case LogLevel::WARNING: return "[WARNING]";
		case LogLevel::ERROR:   return "[ERROR]  ";
		case LogLevel::INPUT:   return "[INPUT]  ";
		default:                return "[UNKNOWN]";
	}
}

void Logger::setLogFile(const std::string& path) {
	std::lock_guard<std::mutex> lock(logMutex);
	outFile.reset(new std::ofstream(path, std::ios::app));
}

void Logger::setMinLevel(LogLevel level) {
	std::lock_guard<std::mutex> lock(logMutex);
	minLevel = level;
}

void Logger::log(LogLevel level, const std::string& message) {
	if (level < minLevel) return;

	auto now = std::chrono::system_clock::now();
	auto in_time_t = std::chrono::system_clock::to_time_t(now);

	auto now_steady = std::chrono::steady_clock::now();
	std::chrono::duration<double> elapsed = now_steady - startTime;

	std::stringstream ss;
	ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S");
	std::string timestamp = ss.str();

	std::string levelStr = levelToString(level);

	std::ostringstream elapsed_ss;
	elapsed_ss << std::fixed << std::setprecision(3) << elapsed.count();

	std::lock_guard<std::mutex> lock(logMutex);

	std::ostream& output = (level == LogLevel::INFO) ? std::cout : std::cerr;
	output << "[" << timestamp << "] [" << elapsed_ss.str() << "s] " << levelStr << ": " << message << std::endl;

	if (outFile && outFile->is_open()) {
		(*outFile) << "[" << timestamp << "] [" << elapsed_ss.str() << "s] " << levelStr << ": " << message << std::endl;
		outFile->flush();
	}
}
