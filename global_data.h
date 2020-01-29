#pragma once


struct global {

	std::map<unsigned int, std::unique_ptr<markov_chain<double>>> markov_chains;

	std::map<unsigned int, std::unique_ptr<std::unordered_set<unsigned long>>> target_sets;

};