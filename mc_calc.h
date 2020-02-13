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
		{"reward_index", reward_index },
		{"variance_state_decoration_index", decoration_destination_index },
		{"source_edge_decoration_index", reward_index },
		{"create_P_target", diffs[0]},
		{"copy_P_target", diffs[1]},
		{"subtract_unity_matrix", diffs[2]},
		{"calc_image_vector_for_expect", diffs[3]},
		{"solve_linear_system_expect", diffs[4]},
		{"write_decorations_expect", diffs[5]},
		{"total_time", (timestamps[COUNT_TIMESTAMPS-1] - timestamps[0]).count() / 1'000'000.0},
		{"linear_system_solve_time", diffs[4] },
		{"unit", "milliseconds"}
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
		{"reward_index", reward_index },
		{"interim_result_edge_decoration_index", free_reward_index },
		{"expect_state_decoration_index", expect_decoration_index },
		{"variance_state_decoration_index", decoration_destination_index },
		{"source_edge_decoration_index", reward_index },
		{"create_P_target", diffs[0]},
		{"copy_P_target", diffs[1]},
		{"subtract_unity_matrix", diffs[2]},
		{"calc_image_vector_for_expect", diffs[3]},
		{"solve_linear_system_expect", diffs[4]},
		{"write_decorations_expect", diffs[5]},
		{"calc_interim_reward", diffs[6]},
		{"calc_image_vector_for_variance", diffs[7]},
		{"solve_linear_system_variance", diffs[8]},
		{"write_decorations_variance", diffs[9]},
		{"total_time", (timestamps[COUNT_TIMESTAMPS - 1] - timestamps[0]).count() / 1'000'000.0},
		{"linear_system_solve_time", diffs[4] + diffs[8] },
		{"unit", "milliseconds"}
	};
	return performance_log;
}
