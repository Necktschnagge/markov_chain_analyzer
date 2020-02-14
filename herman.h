/**
 * @file herman.h
 *
 * Generating Markov chain for herman's selfstabilisation algorithm.
 *
 */
#pragma once

#include "markov_chain.h"
#include "string_constants.h"

#include "json.hpp"


#include <chrono>


 /**
	 @brief Takes markov chain object and creates transitions for herman's self-stabilizing algorithm.
	 @details Uses edge decoration (reward) at index 0 to store unique costs of 1 for each transition.
	 @param mc The markov chain object to modify, must be \a markov_chain::empty().
	 @param size The size of the herman problem for which the states and transitions will be created. Must be odd and (_Integers << size) must not exceed limits of _Integers.
	 @exception std::invalid_argument Markov chain must be empty.
	 @exception std::invalid_argument Size of herman must be odd.
	 @exception std::invalid_argument Size of herman is too big for storing all states in _Integers type.
	 @exception std::invalid_argument Reward 0 needed for costs.
 */
template<class _Rationals, class _Integers, class _Set, bool disable_checks = false>
nlohmann::json generate_herman(markov_chain<_Rationals, _Integers>& mc, const _Integers& size, std::unique_ptr<_Set>& target_set) {
	//### get rid of unique ptr ? // -> then make unique must be replace set must be cleared. (just drop it or use inserter iterator???)
	auto  performance_log{ nlohmann::json() };
	std::array<std::chrono::steady_clock::time_point, 3> timestamps;

	using mc_type = markov_chain<_Rationals, _Integers>;
	const _Integers INT1{ 1 };

	// Compile-time checks:
	static_assert(std::is_same<typename _Set::value_type, _Integers>::value,
		"typename _Set does not agree with typename _Integers. Values in _Set must be of type _Integers");

	timestamps[0] = std::chrono::steady_clock::now();

	// Runtime checks:
	if constexpr (!disable_checks) {
		const bool size_too_big = !
			[](const _Integers& size) { // ~checks if shift does not exceed type _Integers
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
		if (mc.n_edge_decorations == 0) throw std::invalid_argument("Reward 0 needed for costs.");
	}

	timestamps[1] = std::chrono::steady_clock::now();

	// Do the actual calculation:

	// Initialize all states:
	for (_Integers state{ 0 }; (state >> size) == 0; ++state) {
		mc.init_state(state);
	}

	const auto target_state_enumerator{ [&](
		const _Integers& herman_size,
		auto tell /* Callback function that is used to lof final states: tell(const _Integers&) */
		) -> void {
			const auto helper = [&](
				const _Integers& max_pos /* max bit position that can be 1*/,
				_Integers node /* containing bits until pos */,
				const _Integers& pos /* position tobe considered whether set 0 or 1 */,
				bool option /* true iff there is still the option to not alter the bit */,
				bool last_bit /* previous bit */,
				auto tell /* function to log a final state */,
				auto self /* ptr to function itself */
				) -> void {
					{ // dont use option (to not alter the bit)
						bool this_bit{ !last_bit };
						const _Integers node2 = node | ((1ULL << pos) * this_bit);
						if (pos == max_pos) {
							if (!option) tell(
								static_cast<typename std::add_const<decltype(node2)>::type> // just ensure function call on const reference or copy.
								(node2));
						}
						else self(max_pos, node2, pos + 1, option, this_bit, tell, self);
					}
					if (option) { // use option (to not alter the bit)
						const bool& this_bit{ last_bit };
						node |= (1ULL << pos) * this_bit;
						if (pos == max_pos) tell(static_cast<typename std::add_const<decltype(node)>::type>(node)); // do not need to check option because option was used in current step.
						else self(max_pos, node, pos + 1, false, this_bit, tell, self);
					}
			};
			for (auto most_significant_bit : { true, false })
				helper(herman_size - 1, 0, 0, true, most_significant_bit, tell, helper);
	} };

	// Save target states:
	target_set = std::make_unique<_Set>();
	target_state_enumerator(size, [&](const _Integers& value) { target_set->insert(value); });

	// For all states create outgoing edges...
	for (_Integers state{ 0 }; state < (INT1 << size); ++state) {
		_Integers next_state{ 0 };
		std::vector<_Integers> non_deterministic_positions{};
		non_deterministic_positions.reserve(size);

		// List all non-det positions and set deterministic ones of nex_state:
		for (_Integers pos{ 0 }; pos < size; ++pos) {
			// bits in from state:
			bool current_bit = state & (INT1 << pos);
			bool next_bit = state & (INT1 << (pos + 1) % size);

			bool is_det{ current_bit != next_bit };
			if (is_det) next_state |= _Integers(next_bit) << pos;
			else non_deterministic_positions.push_back(pos);
		}

		_Rationals unique_outgoing_edge_probability = 1.0 / (INT1 << non_deterministic_positions.size());

		// For all outgoing edges:
		for (_Integers bits_for_nondet_positions{ 0 };
			bits_for_nondet_positions < INT1 << non_deterministic_positions.size();
			++bits_for_nondet_positions)
		{
			// Map bits to the non_deterministic_positions:
			_Integers next_state_copy{ next_state };
			for (_Integers pos_it{ 0 }; pos_it < non_deterministic_positions.size(); ++pos_it) {
				bool bit_to_map = (bits_for_nondet_positions & INT1 << pos_it);
				next_state_copy |= _Integers(bit_to_map) << non_deterministic_positions[pos_it];
			}
			// Create edge:
			const auto pEdge{ new typename mc_type::edge(unique_outgoing_edge_probability, mc.n_edge_decorations) };
			mc.forward_transitions[state][next_state_copy] = pEdge;
			mc.inverse_transitions[next_state_copy][state] = pEdge;
			pEdge->decorations[0] = _Rationals(1);
		}
	}
	
	timestamps[2] = std::chrono::steady_clock::now();

	static_assert(std::is_same<decltype(timestamps[1] - timestamps[0])::period, std::nano>::value, "Unit is supposed to be nanoseconds.");
	performance_log[cli_commands::GENERATE_HERMAN] = {
		{sc::size, size },
		{sc::size_nodes, mc.size_states() },
		{sc::size_edges, mc.size_edges()},
		{sc::time_run_checks, 1.0 * (timestamps[1] - timestamps[0]).count()/1'000'000.0 },
		{sc::time_run_generator, 1.0 * (timestamps[2] - timestamps[1]).count()/1'000'000.0 },
		{sc::time_total,  1.0 * (timestamps[2] - timestamps[0]).count() / 1'000'000.0},
		{sc::unit, sc::milliseconds}
	};

	return performance_log;
}
