/**
 * @file mc_calc.h
 *
 * Contains facades for calculate expects / variances / ..  on markov chains.
 *
 */
#pragma once

#include "markov_chain.h"
#include "commands.h"

 /**
	  Calculates expects of accumulated edge rewards along paths until reaching target_set in markov chain.
 */
template <class _MarkovChain, class _IntegralSet>
nlohmann::json calc_expect(_MarkovChain& mc, std::size_t reward_index, const _IntegralSet& target_set, std::size_t decoration_destination_index) {

	using analyzer = mc_analyzer<typename _MarkovChain::rational_type, typename _MarkovChain::integral_type>;

	constexpr unsigned COUNT_TIMESTAMPS{ 7 };
	std::array<decltype(std::chrono::steady_clock::now()), COUNT_TIMESTAMPS> timestamps;

	timestamps[0] = std::chrono::steady_clock::now();
	auto target_probability_matrix{ target_adjusted_probability_matrix(mc, target_set) };
	timestamps[1] = std::chrono::steady_clock::now();
	auto target_probability_matrix_minus_one{ target_probability_matrix };
	timestamps[2] = std::chrono::steady_clock::now();
	target_probability_matrix_minus_one.subtract_unity_matrix();
	timestamps[3] = std::chrono::steady_clock::now();
	auto image_vector{ analyzer::rewarded_image_vector(target_probability_matrix, mc, reward_index) };
	timestamps[4] = std::chrono::steady_clock::now();
	auto result{ solve_linear_system(target_probability_matrix_minus_one, image_vector) };
	timestamps[5] = std::chrono::steady_clock::now();
	mc.set_decoration(result, decoration_destination_index);
	timestamps[6] = std::chrono::steady_clock::now();

	nlohmann::json performance_log;
	static_assert(std::is_same<decltype(timestamps[1] - timestamps[0])::period, std::nano>::value, "Unit is supposed to be nanoseconds.");
	std::array<double, COUNT_TIMESTAMPS - 1> diffs;
	std::transform(timestamps.cbegin(),
		timestamps.cbegin() + (timestamps.size() - 1),
		timestamps.cbegin() + 1,
		diffs.begin(),
		[](auto before, auto after) { return (after - before).count() / 1'000'000.0; }
	);

	performance_log[cli_commands::CALC_EXPECT] = {
		{sc::decoration_index_egde_source, reward_index },
		{sc::decoration_index_node_target, decoration_destination_index },
		{sc::time_create_pto_matrix, diffs[0]},
		{sc::time_copy_pto_matrix, diffs[1]},
		{sc::time_subtract_unity_matrix, diffs[2]},
		{sc::time_calc_image_vector, diffs[3]},
		{sc::time_solve_linear_system, diffs[4]},
		{sc::time_write_decoration_node, diffs[5]},
		{sc::time_total, (timestamps[COUNT_TIMESTAMPS - 1] - timestamps[0]).count() / 1'000'000.0},
		{sc::unit, sc::milliseconds}
	};
	return performance_log;
	//### reuse this code where variances are calculated?
}

/**
	Calculates variances of accumulated edge rewards along paths until reaching target_set in markov chain.
*/
template <class _MarkovChain, class _IntegralSet>
nlohmann::json calc_variance(_MarkovChain& mc, std::size_t reward_index, const _IntegralSet& target_set, std::size_t decoration_destination_index, std::size_t expect_decoration_index, std::size_t free_reward_index) {

	using analyzer = mc_analyzer<typename _MarkovChain::rational_type, typename _MarkovChain::integral_type>;

	constexpr unsigned COUNT_TIMESTAMPS{ 11 };
	std::array<decltype(std::chrono::steady_clock::now()), COUNT_TIMESTAMPS> timestamps;

	timestamps[0] = std::chrono::steady_clock::now();
	auto target_probability_matrix{ target_adjusted_probability_matrix(mc, target_set) };
	timestamps[1] = std::chrono::steady_clock::now();
	auto target_probability_matrix_minus_one{ target_probability_matrix };
	timestamps[2] = std::chrono::steady_clock::now();
	target_probability_matrix_minus_one.subtract_unity_matrix();
	timestamps[3] = std::chrono::steady_clock::now();
	auto image_vector{ analyzer::rewarded_image_vector(target_probability_matrix,mc,reward_index) };
	timestamps[4] = std::chrono::steady_clock::now();
	auto result{ solve_linear_system(target_probability_matrix_minus_one, image_vector) };
	timestamps[5] = std::chrono::steady_clock::now();
	mc.set_decoration(result, expect_decoration_index);
	timestamps[6] = std::chrono::steady_clock::now();
	analyzer::calculate_variance_reward(mc, reward_index, expect_decoration_index, free_reward_index);
	timestamps[7] = std::chrono::steady_clock::now();
	auto image_vector2{ analyzer::rewarded_image_vector(target_probability_matrix, mc, free_reward_index) };
	timestamps[8] = std::chrono::steady_clock::now();
	auto result2{ solve_linear_system(target_probability_matrix_minus_one, image_vector2) };
	timestamps[9] = std::chrono::steady_clock::now();
	mc.set_decoration(result2, decoration_destination_index);
	timestamps[10] = std::chrono::steady_clock::now();

	nlohmann::json performance_log;
	static_assert(std::is_same<decltype(timestamps[1] - timestamps[0])::period, std::nano>::value, "Unit is supposed to be nanoseconds.");
	std::array<double, COUNT_TIMESTAMPS - 1> diffs;
	std::transform(timestamps.cbegin(),
		timestamps.cbegin() + (timestamps.size() - 1),
		timestamps.cbegin() + 1,
		diffs.begin(),
		[](auto before, auto after) { return (after - before).count() / 1'000'000.0; }
	);

	performance_log[cli_commands::CALC_VARIANCE] = {
		{sc::decoration_index_egde_source, reward_index },
		{sc::decoration_index_egde_free, free_reward_index },
		{sc::decoration_index_node_target + sc::_expect, expect_decoration_index },
		{sc::decoration_index_node_target + sc::_variance, decoration_destination_index },
		{sc::time_create_pto_matrix, diffs[0]},
		{sc::time_copy_pto_matrix, diffs[1]},
		{sc::time_subtract_unity_matrix, diffs[2]},
		{sc::time_calc_image_vector + sc::_expect, diffs[3]},
		{sc::time_solve_linear_system + sc::_expect, diffs[4]},
		{sc::time_write_decoration_node + sc::_expect, diffs[5]},
		{sc::time_calc_interim_reward, diffs[6]},
		{sc::time_calc_image_vector + sc::_variance, diffs[7]},
		{sc::time_solve_linear_system + sc::_variance, diffs[8]},
		{sc::time_write_decoration_node + sc::_variance, diffs[9]},
		{sc::time_total, (timestamps[COUNT_TIMESTAMPS - 1] - timestamps[0]).count() / 1'000'000.0},
		{sc::time_solve_linear_system, diffs[4] + diffs[8] },
		{sc::unit, sc::milliseconds}
	};
	return performance_log;
}


/**
	Calculates covariances of accumulated edge rewards along paths until reaching target_set in markov chain.
*/
template <class _MarkovChain, class _IntegralSet>
nlohmann::json calc_covariance(
	_MarkovChain& mc,
	std::size_t reward_index1,
	std::size_t reward_index2,
	const _IntegralSet& target_set,
	std::size_t decoration_destination_index,
	std::size_t expect_decoration_index1,
	std::size_t expect_decoration_index2,
	std::size_t free_reward_index) {

	using analyzer = mc_analyzer<typename _MarkovChain::rational_type, typename _MarkovChain::integral_type>;

	constexpr unsigned COUNT_TIMESTAMPS{ 11 };
	std::array<decltype(std::chrono::steady_clock::now()), COUNT_TIMESTAMPS> timestamps;

	timestamps[0] = std::chrono::steady_clock::now();
	auto target_probability_matrix{ target_adjusted_probability_matrix(mc, target_set) };
	timestamps[1] = std::chrono::steady_clock::now();
	auto target_probability_matrix_minus_one{ target_probability_matrix };
	timestamps[2] = std::chrono::steady_clock::now();
	target_probability_matrix_minus_one.subtract_unity_matrix();
	timestamps[3] = std::chrono::steady_clock::now();
	auto image_vector1{ analyzer::rewarded_image_vector(target_probability_matrix,mc,reward_index1) };
	auto image_vector2{ analyzer::rewarded_image_vector(target_probability_matrix,mc,reward_index2) };
	timestamps[4] = std::chrono::steady_clock::now();
	auto interim1{ solve_linear_system(target_probability_matrix_minus_one, image_vector1) };
	auto interim2{ solve_linear_system(target_probability_matrix_minus_one, image_vector2) };
	timestamps[5] = std::chrono::steady_clock::now();
	mc.set_decoration(interim1, expect_decoration_index1);
	mc.set_decoration(interim2, expect_decoration_index2);
	timestamps[6] = std::chrono::steady_clock::now();
	analyzer::calculate_covariance_reward(
		mc, 
		reward_index1,
		reward_index2,
		expect_decoration_index1,
		expect_decoration_index2,
		free_reward_index);
	timestamps[7] = std::chrono::steady_clock::now();
	auto image_vector_cov{ analyzer::rewarded_image_vector(target_probability_matrix, mc, free_reward_index) };
	timestamps[8] = std::chrono::steady_clock::now();
	auto result{ solve_linear_system(target_probability_matrix_minus_one, image_vector_cov) };
	timestamps[9] = std::chrono::steady_clock::now();
	mc.set_decoration(result, decoration_destination_index);
	timestamps[10] = std::chrono::steady_clock::now();

	nlohmann::json performance_log;
	static_assert(std::is_same<decltype(timestamps[1] - timestamps[0])::period, std::nano>::value, "Unit is supposed to be nanoseconds.");
	std::array<double, COUNT_TIMESTAMPS - 1> diffs;
	std::transform(timestamps.cbegin(),
		timestamps.cbegin() + (timestamps.size() - 1),
		timestamps.cbegin() + 1,
		diffs.begin(),
		[](auto before, auto after) { return (after - before).count() / 1'000'000.0; }
	);

	const auto _1{ std::string("_1") };
	const auto _2{ std::string("_2") };

	performance_log[cli_commands::CALC_VARIANCE] = {
		{sc::decoration_index_egde_source + _1, reward_index1 },
		{sc::decoration_index_egde_source + _2, reward_index2 },
		{sc::decoration_index_egde_free, free_reward_index },
		{sc::decoration_index_node_target + sc::_expect +_1, expect_decoration_index1 },
		{sc::decoration_index_node_target + sc::_expect + _2, expect_decoration_index2 },
		{sc::decoration_index_node_target + sc::_covariance, decoration_destination_index },
		{sc::time_create_pto_matrix, diffs[0]},
		{sc::time_copy_pto_matrix, diffs[1]},
		{sc::time_subtract_unity_matrix, diffs[2]},
		{sc::time_calc_image_vector + sc::_expect, diffs[3]},
		{sc::time_solve_linear_system + sc::_expect, diffs[4]},
		{sc::time_write_decoration_node + sc::_expect, diffs[5]},
		{sc::time_calc_interim_reward, diffs[6]},
		{sc::time_calc_image_vector + sc::_covariance, diffs[7]},
		{sc::time_solve_linear_system + sc::_covariance, diffs[8]},
		{sc::time_write_decoration_node + sc::_covariance, diffs[9]},
		{sc::time_total, (timestamps[COUNT_TIMESTAMPS - 1] - timestamps[0]).count() / 1'000'000.0},
		{sc::time_solve_linear_system, diffs[4] + diffs[8] },
		{sc::unit, sc::milliseconds}
	};
	return performance_log;
}