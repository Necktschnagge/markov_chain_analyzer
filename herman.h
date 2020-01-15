#pragma once

#include "markov_chain.h"

#include <chrono>


std::chrono::nanoseconds generate_herman_f(markov_chain<double>& mc, unsigned int size, std::unique_ptr<std::unordered_set<unsigned long>>& target_set) {
	if (!(size % 2)) throw std::invalid_argument("size of herman must be odd");
	if (size > 63) throw std::invalid_argument("size of herman must not be greater than 63");
	if (mc.n_decorations == 0) throw std::invalid_argument("decoration 0 needed for target set");
	if (mc.n_rewards == 0) throw std::invalid_argument("reward 0 needed for costs.");

	std::chrono::steady_clock::time_point before_ns{ std::chrono::steady_clock::now() };

	// conflict with state enumeration type here ### unsigned long vs std::size_t:
	for (std::size_t state{ 0 }; (state >> size) == 0; ++state) {
		mc.states.emplace(state, mc.n_decorations);
	}

	// enumerates all states that are target
	const auto enumerator = [&](unsigned int herman_size, auto tell) -> void {
		const auto helper = [&](unsigned int max_pos, uint64_t node, unsigned int pos, bool option, bool last_bit, auto tell, auto self) -> void {
			{ // dont use option (to not alter the bit)
				///debug:
				bool check = node < 8;
				bool this_bit{ !last_bit };
				uint64_t node2 = node | ((1ULL << pos) * this_bit);
				if (pos == max_pos) {
					if (!option) tell(node2);
				}
				else self(max_pos, node2, pos + 1, option, this_bit, tell, self);
			}
			if (option) { // use option
				bool check = node < 8;
				const bool& this_bit{ last_bit };
				node |= (1ULL << pos) * this_bit;
				bool check2 = node < 8;
				if (pos == max_pos) {
					tell(node); // do not check option because option was used in current step.
				}
				else self(max_pos, node, pos + 1, false, this_bit, tell, self);
			}
		};
		for (auto x : { true, false }) helper(herman_size - 1, 0, 0, true, x, tell, helper);
	};

	// save target states
	target_set = std::make_unique<std::unordered_set<unsigned long>>();
	enumerator(size, [&](uint64_t value) {
		mc.states.at(value).decorations[0] = 1;
		target_set->insert(value);
		});

	// for all states create outgoing edges:
	for (std::size_t state{ 0 }; state < (1uLL << size); ++state) {
		std::size_t next_state{ 0 };
		std::vector<unsigned int> non_deterministic_positions{};
		non_deterministic_positions.reserve(size);

		// list all non-det positions and set deterministic ones of nex_state:
		for (unsigned int pos{ 0 }; pos < size; ++pos) {
			// bits in from state:
			bool current_bit = state & (1uLL << pos);
			bool next_bit = state & (1uLL << (pos + 1) % size);

			bool is_det{ current_bit != next_bit };
			if (is_det) next_state |= static_cast<std::size_t>(next_bit) << pos;
			else non_deterministic_positions.push_back(pos);
		}
		// probability of going to next state
		double prob = 1.0 / (1ull << non_deterministic_positions.size());

		// for all outgoing edges:
		for (auto i = 0uLL; i < 1ull << non_deterministic_positions.size(); ++i) {
			//map bits of i to the non_deterministic_positions:
			std::size_t c_next_state{ next_state };
			for (std::size_t pos_it = 0; pos_it < non_deterministic_positions.size(); ++pos_it) {
				bool bit_to_map = (i & 1ull << pos_it);
				c_next_state |= static_cast<unsigned long long>(bit_to_map) << non_deterministic_positions[pos_it];
			}
			// create edge:
			const auto pEdge{ new markov_chain<double>::edge(prob, mc.n_rewards) };
			mc.forward_transitions[state][c_next_state] = pEdge;
			mc.inverse_transitions[c_next_state][state] = pEdge;
			pEdge->rewards[0] = 1;
		}
	}
	auto after_ns{ std::chrono::steady_clock::now() };
	auto dif = after_ns - before_ns;
	return dif;
}
