#include "markov_chain.h"
#include "intset.h"
#include "mc_analyzer.h"
#include "herman.h"
#include "mc_calc.h"
#include "cli.h"

#include "nlohmann/json.hpp"


static constexpr bool DEBUG_MODE = false;

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
		const auto FILE_PATH{ "R:\\c.txt" };
		_Commands_from_file.open(FILE_PATH);
		params.instructions = &_Commands_from_file;
		std::cout << "DEBUG MODE ENABLED: Loading instructions from " << FILE_PATH << "\n";
	}

	if (!params.instructions) params.instructions = &std::cin;
	performance_log = std::move(cli(*params.instructions, g));
	if (params.json_log)  *params.json_log << performance_log;

	std::cout << "Finished.\n";
}
