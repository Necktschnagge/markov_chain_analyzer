#pragma once

#include "markov_chain.h"


struct global {

	using id = unsigned int;
	using int_type = unsigned long; //###??  // ### what bit width is appropriate for measure purpose???, what for release
	using rational_type = double;
	using mc_type = markov_chain<rational_type, int_type>;
	using set_type = std::unordered_set<int_type>;

	std::map<id, std::unique_ptr<mc_type>> markov_chains;
	std::map<id, std::unique_ptr<set_type>> target_sets;

};