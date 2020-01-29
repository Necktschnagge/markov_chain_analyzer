#pragma once

#include "markov_chain.h"


template <class _MCType>
void calc_expect_f(_MCType& mc, std::size_t reward_index, const std::unordered_set<unsigned long>& target_set, std::size_t decoration_destination_index) {

	using analyzer = mc_analyzer<typename _MCType::rational_type,typename _MCType::integral_type>;

	auto target_probability_matrix{ analyzer::target_adjusted_probability_matrix(mc, target_set) };
	auto target_probability_matrix_minus_one{ target_probability_matrix };
	analyzer::subtract_unity_matrix(target_probability_matrix_minus_one);

	auto image_vector{ analyzer::rewarded_image_vector(target_probability_matrix,mc,reward_index) };
	auto result{ solve_linear_system(target_probability_matrix_minus_one, image_vector) };
	mc.set_decoration(result, decoration_destination_index);
}

template <class _MCType>
void calc_variance_f(_MCType& mc, std::size_t reward_index, const std::unordered_set<unsigned long>& target_set, std::size_t decoration_destination_index, std::size_t expect_decoration_index, std::size_t free_reward_index) {

	using analyzer = mc_analyzer<typename _MCType::rational_type, typename _MCType::integral_type>;

	std::vector<decltype(std::chrono::steady_clock::now())> timestamps;
	std::vector<std::string> names;

	timestamps.reserve(20);
	names.reserve(20);
	auto inserter = std::back_inserter(timestamps);
	auto add_name = std::back_inserter(names);
	
	*inserter++ = std::chrono::steady_clock::now();

	auto target_probability_matrix{ analyzer::target_adjusted_probability_matrix(mc, target_set) };
	*inserter++ = std::chrono::steady_clock::now();
	*add_name++ = "Create P -> target";

	auto target_probability_matrix_minus_one{ target_probability_matrix };
	*inserter++ = std::chrono::steady_clock::now();
	*add_name++ = "Copy P -> target";

	analyzer::subtract_unity_matrix(target_probability_matrix_minus_one);
	*inserter++ = std::chrono::steady_clock::now();
	*add_name++ = "Subtract I";

	auto image_vector{ analyzer::rewarded_image_vector(target_probability_matrix,mc,reward_index) };
	*inserter++ = std::chrono::steady_clock::now();
	*add_name++ = "Calculate image vector (expect)";

	auto result{ solve_linear_system(target_probability_matrix_minus_one, image_vector) };
	*inserter++ = std::chrono::steady_clock::now();
	*add_name++ = "Solve expect values";

	mc.set_decoration(result, expect_decoration_index);
	*inserter++ = std::chrono::steady_clock::now();
	*add_name++ = "Write decorations (expect)";

	analyzer::calculate_variance_reward(mc, reward_index, expect_decoration_index, free_reward_index);
	*inserter++ = std::chrono::steady_clock::now();
	*add_name++ = "Calculate modified reward";

	auto image_vector2{ analyzer::rewarded_image_vector(target_probability_matrix, mc, free_reward_index) };
	*inserter++ = std::chrono::steady_clock::now();
	*add_name++ = "Calculate image vector (variance)";

	auto result2{ solve_linear_system(target_probability_matrix_minus_one, image_vector2) };
	*inserter++ = std::chrono::steady_clock::now();
	*add_name++ = "Solve variances";

	mc.set_decoration(result2, decoration_destination_index);
	*inserter++ = std::chrono::steady_clock::now();
	*add_name++ = "Write decorations (variance)";
	
	std::vector<std::chrono::nanoseconds> differences;
	std::transform(timestamps.cbegin(),
		timestamps.cbegin() + (timestamps.size() - 1),
		timestamps.cbegin() + 1,
		std::back_inserter(differences),
		[](auto before, auto after) -> std::chrono::nanoseconds {
			return after - before;
		});

	std::cout << "variance time measures:\n\n";
	for (int i = 0; i < differences.size(); ++i) {
		std::cout << std::setw(15) << differences[i].count() << "   :IN STEP:   " << names[i] << "\n";
	}
	
}
