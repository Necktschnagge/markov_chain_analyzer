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
		@param input the stream to read from
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

	/**
		@brief Reads labels from prism's file format and stores them into an \a std::unordered_set<_IntegerT>
		@details Searches for all states that are labeled with \a label_id and put them into the returned set.
		@param input the stream to read from
		@param string_to_int_conversion function to convert each single state id from string representation (\a std::string) to integer (\a _IntegerT)
		@param label_id label to consider
	*/
	template<class T>
	inline static std::unordered_set<_IntegerT> prismlabeltointset(std::istream& input, T&& string_to_int_conversion, const std::size_t& label_id) {
		using regex_iterator = boost::regex_iterator<std::string::const_iterator>;
		auto result{ std::unordered_set<_IntegerT>() };
		input.unsetf(std::ios_base::skipws); // also read whitespaces
		const auto input_string{ std::string(std::istream_iterator<char>(input),std::istream_iterator<char>()) };

		const auto lable_file_line{ boost::regex(regex_strings::nonnegative_integer + R"(:.*?(\r\n|\r|\n|$))" ) };

		for (
			auto it = regex_iterator(input_string.cbegin(), input_string.cend(), lable_file_line);
			it != regex_iterator();
			++it
			) {
			const auto line = it->str();
			const auto colon_pos = line.find(':');
			if (colon_pos == line.npos) throw std::bad_exception();
			for (
				auto it = regex_iterator(line.cbegin() + colon_pos, line.cend(), regxc::nonnegative_integer);
				it != regex_iterator();
				++it
				) {
				if (std::stoull(it->str()) == label_id)
					result.insert(string_to_int_conversion(std::string(line.cbegin(), line.cbegin() + colon_pos)));
			}
		}
		return result;
	}
};