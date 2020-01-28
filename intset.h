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

template<class _IntType>
struct int_set {
	template<class T>
	inline static std::unordered_set<_IntType> stointset(std::istream& input, T&& c) {
		using regex_iterator = boost::regex_iterator<std::string::const_iterator>;
		auto result{ std::unordered_set<_IntType>() };
		input.unsetf(std::ios_base::skipws);
		const auto input_string{ std::string(std::istream_iterator<char>(input),std::istream_iterator<char>()) };

		for (auto it = regex_iterator(input_string.cbegin(), input_string.cend(), regxc::nonnegative_integer);
			it != regex_iterator();
			++it) {
			result.insert(c(it->str()));
		}
		return result;
	}
};