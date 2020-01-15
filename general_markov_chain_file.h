#pragma once

#include <vector>
#include <string>


class general_markov_chain_file
{
	const std::string from{ "$from" };
	const std::string to{ "$to" };
	const std::string prob{ "$probability" };

	std::vector<std::string> column_names;

public:
	general_markov_chain_file();
	~general_markov_chain_file();
};

