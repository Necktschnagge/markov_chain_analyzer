#pragma once


struct global {

	using id = unsigned int;

	std::map<id, std::unique_ptr<markov_chain<double>>> markov_chains;

	std::map<id, std::unique_ptr<std::unordered_set<unsigned long>>> target_sets;

};