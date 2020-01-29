#pragma once


struct global {

	using id = unsigned int;
	using int_type = unsigned long; //###??
	using rational_type = double;

	std::map<id, std::unique_ptr<markov_chain<double,int_type>>> markov_chains;

	std::map<id, std::unique_ptr<std::unordered_set<int_type>>> target_sets;

};