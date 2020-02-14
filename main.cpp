#include "markov_chain.h"
#include "intset.h"
#include "mc_analyzer.h"
#include "herman.h"
#include "mc_calc.h"
#include "cli.h"

#include "json.hpp"

static constexpr bool DEBUG_MODE = false;
static constexpr bool COMPRESSED_JSON_MODE = true;

int main(int argc, char** argv)
{
	global g{}; // contains all global data

	nlohmann::json performance_log;

	if constexpr (DEBUG_MODE) {
		auto commands{ std::ifstream("R:\\c.txt") };
		performance_log = std::move(cli(commands, g));
	}
	else {

		if (argc == 2) {
			auto commands{ std::ifstream(argv[1]) };
			performance_log = std::move(cli(commands, g));
		}
		else {
			performance_log = std::move(cli(std::cin, g));
		}
	}

	/* prepare diagram output */
	if constexpr (COMPRESSED_JSON_MODE) {
		auto compressed{ nlohmann::json() };
		int pos = -1;

		for (auto it = performance_log.cbegin(); it != performance_log.cend(); ++it)
		{
			std::cout << "\n\n" << *it;

			if (it->contains(cli_commands::GENERATE_HERMAN)) {
				++pos;
				const auto& herman{ it->operator[](cli_commands::GENERATE_HERMAN) };
				compressed[pos]["herman_size"] = herman["size"];
				for (const auto& pair : herman.items()) compressed[pos][pair.key()] = pair.value();
			}
			if (it->contains(cli_commands::CALC_VARIANCE)) {
				const auto& variance{ it->operator[](cli_commands::CALC_VARIANCE) };
				for (const auto& pair : variance.items()) compressed[pos][pair.key()] = pair.value();
			}
		}
		std::cout << "\n\n\n\n" << compressed << "\n\n";

		std::stringstream total_time_of_herman_size;
		std::stringstream les_solving_of_herman_size;

		std::stringstream total_time_of_edges;
		std::stringstream les_solving_of_edges;

		std::stringstream les_solving_of_herman_relative;

		std::stringstream states_of_herman_size;
		std::stringstream edges_of_herman_size;


		for (const auto& measure : compressed) {

			total_time_of_herman_size << "(" << measure["size"] << "," << (measure["total_time"].operator double() + measure["generator"].operator double()) << ")";
			les_solving_of_herman_size << "(" << measure["size"] << "," << (measure["linear_system_solve_time"].operator double()) << ")";

			total_time_of_edges << "(" << measure["#edges"] << "," << (measure["total_time"].operator double() + measure["generator"].operator double()) << ")";
			les_solving_of_edges << "(" << measure["#edges"] << "," << (measure["linear_system_solve_time"]) << ")";

			les_solving_of_herman_relative << "(" << measure["size"] << "," << measure["linear_system_solve_time"].operator double() * 100.0 / (measure["total_time"].operator double() + measure["generator"].operator double()) << ")";

			states_of_herman_size << "(" << measure["size"] << "," << (measure["#nodes"]) << ")";
			edges_of_herman_size << "(" << measure["size"] << "," << (measure["#edges"]) << ")";


		}

		std::cout << "total_time_of_herman_size\n" << total_time_of_herman_size.str() << "\n";
		std::cout << "les_solving_of_herman_size\n" << les_solving_of_herman_size.str() << "\n";
		
		std::cout << "total_time_of_edges\n" << total_time_of_edges.str() << "\n";
		std::cout << "les_solving_of_edges\n" << les_solving_of_edges.str() << "\n";

		std::cout << "states_of_herman_size\n" << states_of_herman_size.str() << "\n";
		std::cout << "edges_of_herman_size\n" << edges_of_herman_size.str() << "\n";
		
		
		std::cout << "les_solving_of_herman_relative\n" << les_solving_of_herman_relative.str() << "\n";
	}

	std::cout << "Finished.\n";
}
