/**
 * @file mc_analyzer.h
 *
 * Contains utilities for calculations on markov chains.
 *
 */
#pragma once

#include "sparse_matrix.h"

#include "Eigen/src/SparseCore/SparseMatrix.h"


template <class _MatrixType>
class analyzer_t {
public:

	using matrix_type = _MatrixType;
	/**
		@brief Returns a sparse matrix that contains transition probabilities such that matrix[from][to] is the probability of the transition \a from --> \a to, except for states \a from in \target_states, there the value is set to zero (i.e. no entry in sparse matrix).
	*/
	template <class mc_type, class set_type>
	inline static matrix_type target_adjusted_probability_matrix(const mc_type& mc, const set_type& target_states);
};

template <>
template <class mc_type, class set_type>
inline sparse_matrix analyzer_t<sparse_matrix>::target_adjusted_probability_matrix(const mc_type& mc, const set_type& target_states) {
	static_assert(std::is_same<typename mc_type::integral_type, typename set_type::value_type>::value, "Value type of set must equal integral type of markov chain.");
	auto m{ sparse_matrix(mc.size_states(), mc.size_states()) };
	for (auto it = mc.forward_transitions.cbegin(); it != mc.forward_transitions.cend(); ++it) {
		if (target_states.find(it->first) != target_states.cend()) continue;
		for (auto jt = it->second.cbegin(); jt != it->second.cend(); ++jt)
			m(it->first, jt->first) = jt->second->probability; //###assumes that states are enumerated from 0 ... to n-1
	}
	return m;
} //### could also be matrix class member function.

template <>
template <class mc_type, class set_type>
inline Eigen::SparseMatrix<double> analyzer_t<Eigen::SparseMatrix<double>>::target_adjusted_probability_matrix(const mc_type& mc, const set_type& target_states) {
	static_assert(std::is_same<typename mc_type::integral_type, typename set_type::value_type>::value, "Value type of set must equal integral type of markov chain.");
	auto m{ Eigen::SparseMatrix<double>(mc.size_states(), mc.size_states()) };
	std::vector<Eigen::Triplet<double>> triplet_list;
	for (auto it = mc.forward_transitions.cbegin(); it != mc.forward_transitions.cend(); ++it) {
		if (target_states.find(it->first) != target_states.cend()) continue;
		for (auto jt = it->second.cbegin(); jt != it->second.cend(); ++jt)
			triplet_list.emplace_back(it->first, jt->first, jt->second->probability);
	}
	m.setFromTriplets(triplet_list.cbegin(), triplet_list.cend());
	return m;
} //### could also be matrix class member function.

/**
	@brief Contains static functions for analyzing and calculating values from markov chains.
*/
template<class _RationalT, class _IntegerT>
struct mc_analyzer {

	using mc_type = markov_chain<_RationalT, _IntegerT>;
	using set_type = std::unordered_set<_IntegerT>;

	
	/**
		@brief Calculates image vector in linear system of equations for calculation of expects.
	*/
	static std::vector<_RationalT> rewarded_image_vector(const sparse_matrix& target_adjusted_matrix, const mc_type& mc, const std::size_t& reward_selector) {
		if (!(reward_selector < mc.n_edge_decorations)) throw std::invalid_argument("Given markov chain has too few rewards.");
		auto result{ std::vector<_RationalT>(_size(target_adjusted_matrix), 0) };
		for (std::size_t state_s{ 0 }; state_s != _size(target_adjusted_matrix); ++state_s) {
			result[state_s] = -std::accumulate(target_adjusted_matrix[state_s].cbegin(), target_adjusted_matrix[state_s].cend(), _RationalT(0.0),
				[&](const _RationalT& val, const auto& appendee /*pointing to state "t"*/) {
					try {
						return val + appendee.second /*P_{-> A} */ * mc.forward_transitions.at(state_s).at(appendee.first)->decorations[reward_selector];
					}
					catch (const std::out_of_range&) {
						return val; //## throw error here?
					}
				});
		}
		return result;
	}

	/**
		@brief Calculates image vector in linear system of equations for calculation of expects.
	*/
	static Eigen::VectorXd rewarded_image_vector(const Eigen::SparseMatrix<double>& target_adjusted_matrix, const mc_type& mc, const std::size_t& reward_selector) {
		if (!(reward_selector < mc.n_edge_decorations)) throw std::invalid_argument("Given markov chain has too few rewards.");
		const auto size{ _size(target_adjusted_matrix) };
		auto result{ Eigen::VectorXd(size) };
		for (decltype(_size(target_adjusted_matrix)) i{ 0 }; i < size; ++i) result[i] = 0;
		for (int k = 0; k < target_adjusted_matrix.outerSize(); ++k)
			for (Eigen::SparseMatrix<double>::InnerIterator it(target_adjusted_matrix, k); it; ++it)
			{
				result[it.row()] -= it.value() * mc.forward_transitions.at(it.row()).at(it.col())->decorations[reward_selector];
			}
		return result;
	}

	/**
		Stores a new composed reward function for variance as edge decorations in the markoch chain mc.
	*/
	static void calculate_variance_reward(mc_type& mc, std::size_t index_basic_reward, std::size_t index_basic_decoration, std::size_t index_destination_reward) {
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

	/**
		Stores a new composed reward function for covariance as edge decorations in the markoch chain mc.
	*/
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




