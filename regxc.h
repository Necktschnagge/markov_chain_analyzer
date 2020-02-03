/**
 * @file regxc.h
 *
 * Constants for regexes.
 *
 */
#pragma once


#include <string>
#include <boost/regex.hpp>

/**
	Contains strings to be compiled to regexes
*/
class regex_strings {
public:
	inline static const auto nonnegative_integer{ std::string(R"(([1-9][0-9]*|0))") };
	inline static const auto nonnegative_float{ std::string(R"(([0-9]*\.)?[0-9]+)") };

	inline static const auto spaces0{ std::string(R"(\s*)") };
	inline static const auto spaces1{ std::string(R"(\s+)") };
	inline static const auto new_line{ std::string(R"((\r\n|\r|\n))") };

	inline static const auto prism_file_header{ spaces0 + nonnegative_integer + spaces1 + nonnegative_integer + spaces0 + new_line };
	inline static const auto prism_file_value_line{ spaces0 + nonnegative_integer + spaces1 + nonnegative_integer + spaces1 + nonnegative_float + spaces0 };
	inline static const auto prism_file_format{ prism_file_header + "((\\s*|" + prism_file_value_line + ")(" + new_line + "|$))*" };

	inline static const auto gmc_column_name{ std::string(R"(\$\w+)") };
	inline static const auto native_ignored{ std::string(R"(\#.*)") };
	inline static const auto gmc_semantics_definition{ std::string(R"(\s*\$\w+\s*(,\s*\$\w+\s*)*)") }; // use gmc column name here!

	inline static const auto gmc_general{ std::string("^(") + native_ignored + new_line + ")*" +
		gmc_semantics_definition + "(" + new_line + ".*)*$" };
};

/**
	Contains const boost::regex objects
*/
class regxc {
public:
	inline static const auto nonnegative_integer{ boost::regex(regex_strings::nonnegative_integer) };
	inline static const auto prism_file_format{ boost::regex(regex_strings::prism_file_format) };
	inline static const auto prism_header{ boost::regex(regex_strings::prism_file_header) };
	inline static const auto prism_value_line{ boost::regex(regex_strings::prism_file_value_line) };
	inline static const auto nn_float{ boost::regex(regex_strings::nonnegative_float) };

	inline static const auto gmc_general{ boost::regex(regex_strings::gmc_general) };
	inline static const auto gmc_semantics_definition{ boost::regex(regex_strings::gmc_semantics_definition) };
	inline static const auto gmc_column_name{ boost::regex(regex_strings::gmc_column_name) };
};