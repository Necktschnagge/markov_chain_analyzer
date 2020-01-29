#pragma once

#include "sparse_amtrix.h"

template<class _RationalT, class _IntegerT>
struct mc_analyzer {

	using mc_type = markov_chain<_RationalT, _IntegerT>;
	using set_type = std::unordered_set<_IntegerT>;

	/** @
	*/
	static sparse_matrix target_adjusted_probability_matrix(const mc_type& mc, const set_type& target_states) {
		auto m{ sparse_matrix(mc.size_states(), mc.size_states()) };

		for (auto it = mc.forward_transitions.cbegin(); it != mc.forward_transitions.cend(); ++it) {
			if (target_states.find(it->first) != target_states.cend()) continue;
			for (auto jt = it->second.cbegin(); jt != it->second.cend(); ++jt)
				m(it->first, jt->first) = jt->second->probability;
		}
		return m;
	}

	static void subtract_unity_matrix(sparse_matrix& m) {
		if (m.size_m() != m.size_n()) throw std::invalid_argument("Matrix is not quadratic.");
		for (sparse_matrix::size_t it{ 0 }; it != m.size_m(); ++it) m(it, it) -= 1;
	}

	static std::vector<_RationalT> rewarded_image_vector(const sparse_matrix& target_adjusted_matrix, const mc_type& mc, const std::size_t& reward_selector) {
		if (!(reward_selector < mc.n_edge_decorations)) throw std::invalid_argument("Given markov chain has to few rewards.");
		auto result{ std::vector<_RationalT>(target_adjusted_matrix.size_m(), 0) };
		for (sparse_matrix::size_t state_s{ 0 }; state_s != target_adjusted_matrix.size_m(); ++state_s) {
			result[state_s] = -std::accumulate(target_adjusted_matrix[state_s].cbegin(), target_adjusted_matrix[state_s].cend(), _RationalT(0.0),
				[&](const _RationalT& val, const auto& appendee /*pointing to state "t"*/) {
					try {
						return val + appendee.second /*P_{-> A} */ * mc.forward_transitions.at(state_s).at(appendee.first)->decorations[reward_selector];
					}
					catch (const std::out_of_range&) {
						return val;
					}
				});
		}
		return result;
	}

	/** Stores a new reward function for variance
	*/
	static void calculate_variance_reward(mc_type& mc, std::size_t index_basic_reward, std::size_t index_basic_decoration, std::size_t index_destination_reward){
		if (!(index_basic_reward < mc.n_edge_decorations)) throw std::out_of_range("Basic reward out of range.");
		if (!(index_destination_reward < mc.n_edge_decorations)) throw std::out_of_range("Destination reward out of range.");
		if (!(index_basic_decoration < mc.n_node_decorations)) throw std::out_of_range("Basic decoration out of range.");
		for (auto it{ mc.forward_transitions.begin() }; it != mc.forward_transitions.end(); ++it) {
			for (auto jt{ it->second.begin() }; jt != it->second.end(); ++jt) {
				double factor{ mc.states.at(jt->first).decorations[index_basic_decoration]
					+ jt->second->decorations[index_basic_reward]
					- mc.states.at(it->first).decorations[index_basic_decoration]
				};
				jt->second->decorations[index_destination_reward] = factor * factor;
			}
		}
	}

	static void calculate_covariance_reward(mc_type& mc, std::size_t index_basic_reward_1, std::size_t index_basic_reward_2, std::size_t index_basic_decoration_1, std::size_t index_basic_decoration_2, std::size_t index_destination_reward) {
		if (!(index_basic_reward_1 < mc.n_edge_decorations)) throw std::out_of_range("Basic reward out of range.");
		if (!(index_basic_reward_2 < mc.n_edge_decorations)) throw std::out_of_range("Basic reward out of range.");
		if (!(index_destination_reward < mc.n_edge_decorations)) throw std::out_of_range("Destination reward out of range.");
		if (!(index_basic_decoration_1 < mc.n_node_decorations)) throw std::out_of_range("Basic decoration out of range.");
		if (!(index_basic_decoration_2 < mc.n_node_decorations)) throw std::out_of_range("Basic decoration out of range.");
		for (auto it{ mc.forward_transitions.begin() }; it != mc.forward_transitions.end(); ++it) {
			for (auto jt{ it->second.begin() }; jt != it->second.end(); ++jt) {
				double factor1{
					mc.states.at(jt->first).decorations[index_basic_decoration_1]
					+ jt->second->decorations[index_basic_reward_1]
					- mc.states.at(it->first).decorations[index_basic_decoration_1]
				};
				double factor2{
					mc.states.at(jt->first).decorations[index_basic_decoration_2]
					+ jt->second->decorations[index_basic_reward_2]
					- mc.states.at(it->first).decorations[index_basic_decoration_2]
				};
				jt->second->decorations[index_destination_reward] = factor1 * factor2;
			}
		}
	}
};


/**
  Solves linear system M * x = b
*/
std::vector<double> solve_linear_system(const sparse_matrix& M, const std::vector<double>& b, amgcl::profiler<>* optional_profiler = nullptr) {

	amgcl::profiler<> profiler;
	auto t_total = profiler.scoped_tic("total");

	// Create an AMGCL solver for the problem.
	typedef amgcl::backend::builtin<double> Backend;
	amgcl::make_solver<
		amgcl::amg<
		Backend,
		amgcl::coarsening::aggregation,
		amgcl::relaxation::spai0
		>,
		amgcl::solver::cg<Backend>
	> solve(M);
	///####check different coarsening and relaxations.

	std::cout << solve.precond() << std::endl;

	auto t_solve = profiler.scoped_tic("solve");
	std::vector<double>  x(M.size_n(), 0.0);
	solve(b, x);

	if (optional_profiler) *optional_profiler = profiler;
	return x;
}

