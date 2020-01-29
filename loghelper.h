/**
 * @file loghelper.h
 *
 * Utilities to enhance logging output.
 *
 */
#pragma once

#include <iostream>
#include <chrono>
#include <string>

/// @brief Turns a bool into an "okay" or "failed" followed by line break "\n"
inline std::string interprete_bool_n(const bool& success) { return success ? "okay\n" : "failed\n"; };

/**
	@brief For creating objects that print the same string on construction and destruction to log begin and end of some processing.
	@details For details see code.
*/
template< class _Doc>
class surround_logger {
	const _Doc* doc;

	inline void print_entry_message() const { std::cout << *doc << "...\n"; }
	inline void print_exit_message() const { std::cout << doc << "  DONE!\n"; }
public:
	inline surround_logger(const _Doc& doc) : doc(&doc) { print_entry_message(); }
	surround_logger(const surround_logger&) = delete;
	void operator=(const surround_logger&) = delete;
	inline void exit() { print_exit_message(); doc = nullptr; }
	inline ~surround_logger() { if (doc) print_exit_message(); }
};

/// @brief Creates a \a surround_logger from ostream - printable \a doc
template<class _Doc>
surround_logger<_Doc> make_surround_log(const _Doc& doc) { return surround_logger<_Doc>(doc); }


/// @brief prints a duration (see \a std::chrono::duration)
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