#include "markov_chain.h"
#include "intset.h"
#include "mc_analyzer.h"
#include "herman.h"
#include "mc_calc.h"
#include "cli.h"

#include "nlohmann/json.hpp"

static constexpr bool DEBUG_MODE = false;
static constexpr bool COMPRESSED_JSON_MODE = false;

struct cli_params {

	inline static const auto __instructions{ std::string("--instructions") };
	inline static const auto __json_log{ std::string("--json-log") };

	std::istream* instructions{ nullptr };
	std::ostream* json_log{ nullptr };
	char* instructions_param{ nullptr };
	char* json_log_param{ nullptr };
};

int main(int argc, char** argv)
{
	global g{}; // contains all global data
	cli_params params; // commandline parameters / run configuration
	std::ifstream _Commands_from_file;
	std::ofstream _Json_log_file;

	nlohmann::json performance_log;

	std::vector<bool> recognized_params(argc, false);
	recognized_params[0] = true;
	auto get_param = [&](std::size_t i) { return std::string(argv[i]); };

	// parse params: look for known tokens
	for (std::size_t i{ 1 }; i < argc; ++i) {
		if (get_param(i) == cli_params::__instructions) {
			if (recognized_params[i] || !(i + 1 < argc)) {
				std::cerr << "PARAM ERROR: " << get_param(i);
				exit(1);
			}
			recognized_params[i] = true;
			recognized_params[i + 1] = true;
			params.instructions_param = argv[i + 1];
		}
		if (get_param(i) == cli_params::__json_log) {
			if (recognized_params[i] || !(i + 1 < argc)) {
				std::cerr << "PARAM ERROR: " << get_param(i);
				exit(1);
			}
			recognized_params[i] = true;
			recognized_params[i + 1] = true;
			params.json_log_param = argv[i + 1];
		}
	}

	// check for unrecognized tokens:
	if (!std::accumulate(recognized_params.cbegin(), recognized_params.cend(), true, [](auto l, auto r) { return l && r; })) {
		std::cout << "\nWARNING:   There were unrecognized parameters:\n";
		for (std::size_t i{ 0 }; i < recognized_params.size(); ++i) {
			if (!recognized_params[i])
				std::cout << "   " << get_param(i) << "\n";
		}
		std::cout << std::endl;
	};

	// apply configuration of parameters
	if (params.instructions_param) {
		_Commands_from_file.open(params.instructions_param);
		params.instructions = &_Commands_from_file;
	}
	if (params.json_log_param) {
		_Json_log_file.open(params.json_log_param);
		params.json_log = &_Json_log_file;
	}
	if constexpr (DEBUG_MODE) {
		_Commands_from_file.open("R:\\c.txt");
		params.instructions = &_Commands_from_file;
	}

	if (!params.instructions) params.instructions = &std::cin;
	performance_log = std::move(cli(*params.instructions, g));
	if (params.json_log)  *params.json_log << performance_log;

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
