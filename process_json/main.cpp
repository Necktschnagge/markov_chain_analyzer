

#include "../commands.h"
#include "../string_constants.h"

#include <nlohmann/json.hpp>


#include <fstream>
#include <string>
#include <iostream>


int main() {

	nlohmann::json j;
	nlohmann::json zipped;

	const auto file_path{ [](std::size_t i) { return std::string("../measures/leader_snyc/log") + std::to_string(i) + ".json"; } };

	const auto get_element_containing{ [](const nlohmann::json& j, auto string) -> nlohmann::json::const_iterator {
		for (auto it = j.cbegin(); it != j.cend(); ++it) {
			if (it->contains(string)) return it;
		}
	} };

	// zip values of interest:
	for (std::size_t i{ 0 }; i <= 22; ++i) {
		auto json_file{ std::ifstream(file_path(i)) };
		json_file >> j;
		auto it_print_mc = get_element_containing(j, cli_commands::PRINT_MC);
		auto size_node = it_print_mc->operator[](cli_commands::PRINT_MC)[sc::size_nodes];
		auto size_edges = it_print_mc->operator[](cli_commands::PRINT_MC)[sc::size_edges];
		auto it_calc_variance = get_element_containing(j, cli_commands::CALC_VARIANCE);
		auto time_total_calculation = it_calc_variance->operator[](cli_commands::CALC_VARIANCE)[sc::time_total];
		auto time_les_calculation = it_calc_variance->operator[](cli_commands::CALC_VARIANCE)[sc::time_solve_linear_system];
		const auto separator{ ": " };
		std::cout << sc::size_nodes << separator << size_node;
		std::cout << sc::size_edges << separator << size_edges;
		std::cout << sc::time_total << separator << time_total_calculation;
		std::cout << sc::time_solve_linear_system << separator << time_les_calculation;
		zipped.push_back({
			{sc::size_nodes, size_node},
			{sc::size_edges, size_edges},
			{sc::time_total, time_total_calculation},
			{sc::time_solve_linear_system, time_les_calculation}
			});
	}

	// print output as required for diagrams:
	std::cout << "\n size_edges @ size_nodes\n";
	for (const auto& item : zipped) {
		std::cout << "(" << item[sc::size_nodes] << "," << item[sc::size_edges] << ")";
	}

	std::cout << "\n total_time @ size_edges\n";
	for (const auto& item : zipped) {
		std::cout << "(" << item[sc::size_edges] << "," << item[sc::time_total] << ")";
	}
	std::cout << "\n time_solve_linear_system @ size_edges\n";
	for (const auto& item : zipped) {
		std::cout << "(" << item[sc::size_edges] << "," << item[sc::time_solve_linear_system] << ")";
	}
	std::cout << "\n total_time @ size_nodes\n";
	for (const auto& item : zipped) {
		std::cout << "(" << item[sc::size_nodes] << "," << item[sc::time_total] << ")";
	}
	std::cout << "\n time_solve_linear_system @ size_nodes\n";
	for (const auto& item : zipped) {
		std::cout << "(" << item[sc::size_nodes] << "," << item[sc::time_solve_linear_system] << ")";
	}

	std::cout << "\n percentage les_time/total_time @ size_nodes\n";
	for (const auto& item : zipped) {
		auto x = item[sc::time_solve_linear_system];
		auto y = item[sc::time_total];
		std::cout << "(" << item[sc::size_nodes] << "," << 1.0 * item[sc::time_solve_linear_system].get<double>() / item[sc::time_total].get<double>() * 100.0 << ")";
	}

	return 0;
}



