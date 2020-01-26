// grosser_beleg.cpp : Diese Datei enthält die Funktion "main". Hier beginnt und endet die Ausführung des Programms.
//

#include "markov_chain.h"
#include "intset.h"
#include "mc_analyzer.h"
#include "herman.h"
#include "mc_calc.h"

#include <iostream>
#include <regex>
#include <fstream>
#include <string>
#include <iostream>
#include <chrono>

#include <boost/algorithm/string.hpp>


// run CI,
// run doxygen ####
// implement graph approach
// enable CLI
// measure time.
// include spdlog
// 

struct global {
	std::map<unsigned int, std::unique_ptr<markov_chain<double>>> markov_chains;

	std::map<unsigned int, std::unique_ptr<std::unordered_set<unsigned long>>> target_sets;
};





void cli(std::istream& commands, global& g) {
	const auto reset{ "reset" }; // id, n_dec, n_rew
	const auto read_tra{ "read_tra" }; // id, file
	const auto read_gmc{ "read_gmc" }; // id, file
	const auto add_rew{ "add_rew" }; // id, file, reward_index
	const auto read_target{ "read_target" }; // id, file
	const auto read_label{ "read_label" }; // id, file, label_id
	const auto calc_expect{ "calc_expect" }; // mc_id, reward_index, target_id, destination_decoration
	const auto calc_variance{ "calc_variance" }; // mc_id, reward_index, target_id, destination_decoration, expect_decoration, free_reward
	const auto write_gmc{ "write_gmc" }; //##not impl
	const auto write_deco{ "write_deco" }; /// ##not impl
	//cov calc
	const auto GENERATE_HERMAN{ "generate_herman" }; // id mc, n


	commands.unsetf(std::ios_base::skipws);
	while (commands.good())
	{
		//fetch command
		std::string command{};
		std::getline(commands, command);
		std::cout << "Fetched command: " << command << std::endl;

		//parse command
		std::vector<std::string> items;
		boost::split(items, command, boost::is_any_of(">"));
		if (items.size() == 0) throw std::logic_error(std::string("Maleformed instruction: ") + command);

		//execute command
		std::string& instruction{ items[0] };
		if (instruction == reset) { // create fresh markov chain
			if (items.size() != 4) throw std::invalid_argument("Wrong number of parameters.");
			std::size_t n_dec{ 0 }, n_rew{ 0 }, id{ 0 };
			try {
				id = std::stoul(items[1]);
				n_dec = std::stoull(items[2]);
				n_rew = std::stoull(items[3]);
			}
			catch (...) { throw std::invalid_argument("Could not parse parameter"); }
			g.markov_chains[id] = std::make_unique<markov_chain<double>>(n_rew, n_dec); 
			continue;
		}

		if (instruction == read_tra) {
			if (items.size() != 3) throw std::invalid_argument("Wrong number of parameters.");
			std::string& file_path = items[2];
			std::size_t id{ 0 };
			std::ifstream file{};
			try {
				id = std::stoul(items[1]);
				file.open(file_path);
			}
			catch (...) { throw std::invalid_argument("Could not parse parameter or open file"); }
			if (g.markov_chains[id] == nullptr) throw std::logic_error("No mc with given ID");
			g.markov_chains[id]->read_transitions_from_prism_file(file);
			continue;
		}

		if (instruction == read_gmc) {
			if (items.size() != 3) throw std::invalid_argument("Wrong number of parameters.");
			std::string& file_path = items[2];
			std::size_t id{ 0 };
			std::ifstream file{};
			try {
				id = std::stoul(items[1]);
				file.open(file_path);
			}
			catch (...) { throw std::invalid_argument("Could not parse parameter or open file"); }
			if (g.markov_chains[id] == nullptr) throw std::logic_error("No mc with given ID");
			g.markov_chains[id]->read_from_gmc_file(file);
			continue;
		}

		if (instruction == add_rew) {
			if (items.size() != 4) throw std::invalid_argument("Wrong number of parameters.");
			std::string& file_path = items[2];
			std::size_t id{ 0 }, rew_index{ 0 };
			std::ifstream file{};
			try {
				id = std::stoul(items[1]);
				file.open(file_path);
				rew_index = std::stoull(items[3]);
			}
			catch (...) { throw std::invalid_argument("Could not parse parameter or open file"); }
			if (g.markov_chains[id] == nullptr) throw std::logic_error("No mc with given ID");
			g.markov_chains[id]->read_rewards_from_prism_file(file, rew_index);
			continue;
		}

		if (instruction == read_target) {
			if (items.size() != 3) throw std::invalid_argument("Wrong number of parameters.");
			std::string& file_path = items[2];
			std::size_t id{ 0 };
			std::ifstream file{};
			try {
				id = std::stoul(items[1]);
				file.open(file_path);
			}
			catch (...) { throw std::invalid_argument("Could not parse parameter or open file"); }
			g.target_sets[id] = std::make_unique<std::unordered_set<unsigned long>>(
				std::move(int_set<unsigned long>::stointset(file, [](auto s) { return std::stoul(s); }))
				);
			continue;
		}

		if (instruction == read_label) {
			if (items.size() != 4) throw std::invalid_argument("Wrong number of parameters.");
			std::string& file_path = items[2];
			std::size_t id{ 0 }, label_id{ 0 };
			std::ifstream file{};
			try {
				id = std::stoul(items[1]);
				file.open(file_path);
				label_id = std::stoull(items[3]);
			}
			catch (...) { throw std::invalid_argument("Could not parse parameter or open file"); }

			throw std::logic_error("not implemented: read prism label file to build up a target set.");
			continue;
		}

		if (instruction == calc_expect) { //  mc_id, reward_index, target_id, destination_decoration
			if (items.size() != 5) throw std::invalid_argument("Wrong number of parameters.");
			std::size_t destination_decoration{ 0 }, mc_id{ 0 };
			unsigned long reward_index{ 0 }, target_id{ 0 };
			try {
				mc_id = std::stoul(items[1]);
				reward_index = std::stoull(items[2]);
				target_id = std::stoul(items[3]);
				destination_decoration = std::stoull(items[4]);
			}
			catch (...) { throw std::invalid_argument("Could not parse parameter"); }
			if (g.markov_chains[mc_id] == nullptr) throw std::logic_error("No mc with given ID");

			calc_expect_f(*(g.markov_chains[mc_id]), reward_index, *(g.target_sets[target_id]), destination_decoration);
			continue;
		}

		if (instruction == calc_variance) {
			// mc_id, reward_index, target_id, destination_decoration, expect_decoration, free_reward
			if (items.size() != 7) throw std::invalid_argument("Wrong number of parameters.");
			std::size_t expect_decoration{ 0 }, destination_decoration{ 0 }, mc_id{ 0 };
			unsigned long reward_index{ 0 }, target_id{ 0 }, free_reward{ 0 };
			try {
				mc_id = std::stoul(items[1]);
				reward_index = std::stoull(items[2]);
				target_id = std::stoul(items[3]);
				destination_decoration = std::stoull(items[4]);
				expect_decoration = std::stoull(items[5]);
				free_reward = std::stoull(items[6]);
			}
			catch (...) { throw std::invalid_argument("Could not parse parameter"); }
			if (g.markov_chains[mc_id] == nullptr) throw std::logic_error("No mc with given ID");
			calc_variance_f(*(g.markov_chains[mc_id]), reward_index, *(g.target_sets[target_id]), destination_decoration, expect_decoration, free_reward);
			continue;
		}

		if (instruction == GENERATE_HERMAN) { // id mc, n, target_set_id
			if (items.size() != 4) throw std::invalid_argument("Wrong number of parameters.");
			std::size_t mc_id{ 0 }, target_set_id{ 0 };
			unsigned long size{ 0 };
			try {
				mc_id = std::stoull(items[1]);
				size = std::stoul(items[2]);
				target_set_id = std::stoull(items[3]);
			}
			catch (...) { throw std::invalid_argument("Could not parse parameter"); }
			if (g.markov_chains[mc_id] == nullptr) throw std::logic_error("No mc with given ID");
		
			auto time = generate_herman(*g.markov_chains[mc_id], size,g.target_sets[target_set_id]);
			printDuration(time);
			continue;
		}

		std::cout << "WARNING: Command not recognized:   " << command << "\n";
	}
}


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
