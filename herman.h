/**
 * @file herman.h
 *
 * Generating Markov chain for herman's selfstabilisation algorithm.
 *
 */
#pragma once

#include "markov_chain.h"

#include <chrono>


/**
	@brief Takes markov chain object and creates transitions for herman's self-stabilizing algorithm.
	@details Uses edge decoration (reward) at index 0 to store unique costs of 1 for each transition.
	@param mc The markov chain object to modify, must be \a markov_chain::empty().
	@param size The size of the herman problem for which the states and transitions will be created. Must be odd and (_Integers << size) must not exceed limits of _Integers.
	@exception std::invalid_argument Markov chain must be empty.
	@exception std::invalid_argument Size of herman must be odd.
	@exception std::invalid_argument Size of herman is too big for storing all states in _Integers type.
	// @excpetion std::invalid_argument Decoration 0 needed for target set. ###
	@exception std::invalid_argument Reward 0 needed for costs.
*/
template<class _Rationals, class _Integers, class _Set, bool disable_checks = false>
std::chrono::nanoseconds generate_herman(markov_chain<_Rationals, _Integers>& mc, const _Integers& size, std::unique_ptr<_Set>& target_set) {
	//### get rid of unique ptr

	// Compile-time checks:
	static_assert(std::is_same<typename _Set::value_type,_Integers>::value,
		"typename _Set does not agree with typename _Integers. Values in _Set must be of type _Integers");

	// Runtime checks:
	if constexpr (!disable_checks) {
		const bool size_too_big = !
			[](const _Integers& size) { // ~checks if shift does not exceed type _Integers
			// ! _Integers is the type which is actually used ??
			// ! std::size_t is the natural upperbound from architecture ??
			_Integers shifted{ 1 };
			for (_Integers i{ 0 }; i != size; ++i) {
				const _Integers next{ shifted << 1 };
				if (
					!(next > shifted) ||
					!(next == shifted + shifted)
					) return false;
				shifted = next;
			}
			return true;
		}
		(size);

		// Check constraints:
		if (!mc.empty()) throw std::invalid_argument("Markov chain must be empty.");
		if (!(size % 2)) throw std::invalid_argument("Size of herman must be odd.");
		if (size_too_big) throw std::invalid_argument("Size of herman is too big for storing all states in _Integers type.");
		// if (mc.n_node_decorations == 0) throw std::invalid_argument("Decoration 0 needed for target set."); ###
		if (mc.n_edge_decorations == 0) throw std::invalid_argument("Reward 0 needed for costs.");
	}

	// Store timepoint:
	std::chrono::steady_clock::time_point before_ns{ std::chrono::steady_clock::now() };

	// Do the actual calculation:

	// Initialize all states
	for (_Integers state{ 0 }; (state >> size) == 0; ++state) {
		mc.init_state(state);
	}

	// Enumerates all states that are target:
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
		//mc.states.at(value).decorations[0] = 1;
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
			const auto pEdge{ new markov_chain<double>::edge(prob, mc.n_edge_decorations) };
			mc.forward_transitions[state][c_next_state] = pEdge;
			mc.inverse_transitions[c_next_state][state] = pEdge;
			pEdge->decorations[0] = 1;
		}
	}
	auto after_ns{ std::chrono::steady_clock::now() };
	auto dif = after_ns - before_ns;
	return dif;
}
