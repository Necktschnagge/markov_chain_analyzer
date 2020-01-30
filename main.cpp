#include "markov_chain.h"
#include "intset.h"
#include "mc_analyzer.h"
#include "herman.h"
#include "mc_calc.h"
#include "cli.h"


int main(int argc, char** argv)
{
	global g{}; // contains all global data

	std::cout << "number of arguments: " << argc << "\n";
	std::cout << "Number okay: " << interprete_bool_n(argc >= 4);
	//if (!(argc >= 4)) return 1;###
	auto commands = std::ifstream("R:\\c.txt");
	cli(commands, g);

	std::cout << "Hello World!\n";
}
