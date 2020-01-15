#pragma once

#include <iostream>
#include <chrono>

inline static const auto interprete_bool_n = [](const bool& success) { return success ? "okay\n" : "failed\n"; };

inline static const auto surround_log = [](const auto& doc, const auto& task) { std::cout << doc << "...\n"; task(); std::cout << doc << "  DONE!\n"; };

template<class _Duration>
inline void printDuration(const _Duration& duration) {
	using namespace std::chrono;
	std::cout << "TIME:\n"
		<< "nanoseconds:   " << duration_cast<nanoseconds>(duration).count()
		<< "\nmicroseconds:   " << duration_cast<microseconds>(duration).count()
		<< "\nmilliseconds:   " << duration_cast<milliseconds>(duration).count()
		<< "\nseconds:   " << duration_cast<seconds>(duration).count()
		<< "\n\n";
}