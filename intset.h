/**
 * @file intset.h
 *
 * Utility to read set of Integers from file.
 *
 */
#pragma once

#include "regxc.h"

#include <unordered_set>
#include <istream>
#include <functional>

/**
	@brief Utilities to read set of integers from file.
	@tparam _IntegerT integer type to read
*/
template<class _IntegerT>
struct int_set {

	/**
		@brief Reads a stream, extracts the integers and stores them into an \a std::unordered_set<_IntegerT>
		@param string_to_int_conversion function to convert each single integer from string representation (\a std::string) to integer (\a _IntegerT)
	*/
	template<class T>
	inline static std::unordered_set<_IntegerT> stointset(std::istream& input, T&& string_to_int_conversion) {
		using regex_iterator = boost::regex_iterator<std::string::const_iterator>;
		auto result{ std::unordered_set<_IntegerT>() };
		input.unsetf(std::ios_base::skipws); // also read whitespaces
		const auto input_string{ std::string(std::istream_iterator<char>(input),std::istream_iterator<char>()) };

		for (
			auto it = regex_iterator(input_string.cbegin(), input_string.cend(), regxc::nonnegative_integer);
			it != regex_iterator();
			++it
			) {
			result.insert(string_to_int_conversion(it->str()));
		}
		return result;
	}
};