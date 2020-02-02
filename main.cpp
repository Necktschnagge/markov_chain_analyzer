#include "markov_chain.h"
#include "intset.h"
#include "mc_analyzer.h"
#include "herman.h"
#include "mc_calc.h"
#include "cli.h"

static constexpr bool DEBUG_MODE = false;

int main(int argc, char** argv)
{
	global g{}; // contains all global data

	if constexpr (DEBUG_MODE) {
		auto commands{ std::ifstream("R:\\c.txt") };
		cli(commands, g);
		return 0;
	}

	if (argc == 2) {
		auto commands{ std::ifstream(argv[1]) };
		cli(commands, g);
	}
	else {
		cli(std::cin, g);
	}


	std::cout << "Finished.\n";
}
