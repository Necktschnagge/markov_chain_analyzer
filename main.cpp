#include "markov_chain.h"
#include "intset.h"
#include "mc_analyzer.h"
#include "herman.h"
#include "mc_calc.h"
#include "cli.h"

#include "json.hpp"

static constexpr bool DEBUG_MODE = true;

int main(int argc, char** argv)
{
	global g{}; // contains all global data

	nlohmann::json json;
	json["herman"] = nlohmann::json::array();

	std::cout << json;
	
	if constexpr (DEBUG_MODE) {
		auto commands{ std::ifstream("R:\\c.txt") };
		cli(commands, g, json);
		return 0;
	}

	if (argc == 2) {
		auto commands{ std::ifstream(argv[1]) };
		cli(commands, g, json);
	}
	else {
		cli(std::cin, g, json);
	}


	std::cout << "Finished.\n";
}
