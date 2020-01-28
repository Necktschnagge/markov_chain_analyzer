#pragma once

#include <iostream>
#include <chrono>
#include <string>

inline std::string interprete_bool_n(const bool& success) { return success ? "okay\n" : "failed\n"; };

template< class _Doc>
class surr_log {
	const _Doc* doc;

	inline void print_entry_message() const { std::cout << *doc << "...\n"; }
	inline void print_exit_message() const { std::cout << doc << "  DONE!\n"; }
public:
	inline surr_log(const _Doc& doc) : doc(&doc) { print_entry_message(); }
	surr_log(const surr_log&) = delete;
	void operator=(const surr_log&) = delete;
	inline void exit() { print_exit_message(); doc = nullptr; }
	inline ~surr_log() { if (doc) print_exit_message(); }
};

template<class _Doc>
surr_log<_Doc> make_surr_log(const _Doc& doc) { return surr_log<_Doc>(doc); }


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