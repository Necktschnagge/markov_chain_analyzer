/**
 * @file cli.h
 *
 * Commandline interface for MC Analyzer.
 *
 */
#pragma once

#include "commands.h"
#include "string_constants.h"

#include "json.hpp"

#include <fstream>



/**
	@brief Reads commands from stream, performs corresponding actions.
	@param commands stream containing commands, separated by new line ('\n'). Parameters are separated with '>' within one line.
	@param g global struct for storing data
	@exception std::logic_error Maleformed instruction...
	@exception std::invalid_argument Wrong number of parameters.
	@exception std::invalid_argument Could not parse parameter.
	@exception std::invalid_argument Bad file.
	@exception std::logic_error No markov chain present with given ID.
*/
inline void cli(std::istream& commands, global& g, nlohmann::json& json) {

	using mc_type = global::mc_type;
	const std::string split_symbol{ ">" };
	auto performance_log{ nlohmann::json() };

	commands.unsetf(std::ios_base::skipws); // also read whitespaces
	while (commands.good())
	{
		// Print performance log
		std::cout << "\nPerformance Log JSON so far...\n\n" << performance_log;

		//fetch command
		std::string command{};
		std::getline(commands, command);
		std::cout << "\n\nFetched command: " << command << std::endl;

		//parse command
		std::vector<std::string> items;
		boost::split(items, command, boost::is_any_of(split_symbol));
		if (items.size() == 0) throw std::logic_error(std::string("Maleformed instruction: ") + command);
		std::string& instruction{ items[0] };
		auto doc = make_surround_log("Executing command");

		//execute command
		if (instruction == cli_commands::RESET_MC) {
			if (items.size() != 4) throw std::invalid_argument("Wrong number of parameters.");
			std::size_t n_state_decoration{ 0 }, n_transition_decoration{ 0 };
			cli_commands::id id{ 0 };
			try {
				id = std::stoull(items[1]);
				n_state_decoration = std::stoull(items[2]);
				n_transition_decoration = std::stoull(items[3]);
			}
			catch (...) { throw std::invalid_argument("Could not parse parameter."); }
			g.markov_chains[id] = std::make_unique<mc_type>(n_transition_decoration, n_state_decoration);
			auto&& log{ nlohmann::json() };
			log[cli_commands::RESET_MC] = {
				{ sc::markov_chain_id, id},
				{ sc::number_node_decorations, n_state_decoration},
				{ sc::number_edge_decorations, n_transition_decoration}
			};
			performance_log.push_back(std::move(log));
			continue;
		}

		if (instruction == cli_commands::READ_TRA) {
			if (items.size() != 3) throw std::invalid_argument("Wrong number of parameters.");
			std::string& file_path = items[2];
			cli_commands::id id{ 0 };
			std::ifstream file{};
			try {
				id = std::stoull(items[1]);
			}
			catch (...) { throw std::invalid_argument("Could not parse parameter."); }
			try {
				file.open(file_path);
				if (!file.good()) throw 0;
			}
			catch (...) { throw std::invalid_argument("Bad file."); }
			if (g.markov_chains[id] == nullptr) throw std::logic_error("No markov chain present with given ID.");
			g.markov_chains[id]->read_transitions_from_prism_file(file);
			continue;
		}

		if (instruction == cli_commands::READ_GMC) {
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

		if (instruction == cli_commands::ADD_REW) {
			if (items.size() != 4) throw std::invalid_argument("Wrong number of parameters.");
			std::string& file_path = items[2];
			std::size_t rew_index{ 0 };
			cli_commands::id id{ 0 };
			std::ifstream file{};
			try {
				id = std::stoull(items[1]); //### split try block, like some lines in code before
				file.open(file_path);
				rew_index = std::stoull(items[3]);
			}
			catch (...) { throw std::invalid_argument("Could not parse parameter or open file"); }
			if (g.markov_chains[id] == nullptr) throw std::logic_error("No mc with given ID");
			g.markov_chains[id]->read_rewards_from_prism_file(file, rew_index);
			continue;
		}

		if (instruction == cli_commands::READ_TARGET) {
			if (items.size() != 3) throw std::invalid_argument("Wrong number of parameters.");
			std::string& file_path = items[2];
			cli_commands::id id{ 0 };
			std::ifstream file{};
			try {
				id = std::stoul(items[1]); // ###split
				file.open(file_path);
			}
			catch (...) { throw std::invalid_argument("Could not parse parameter or open file"); }
			g.target_sets[id] = std::make_unique<global::set_type>(
				std::move(int_set<global::int_type>::stointset(file, [](auto s) { return std::stoull(s); }))
				);
			continue;
		}

		if (instruction == cli_commands::READ_LABEL) {
			if (items.size() != 4) throw std::invalid_argument("Wrong number of parameters.");
			std::string& file_path = items[2];
			std::size_t label_id{ 0 };
			cli_commands::id id{ 0 };
			std::ifstream file{};
			try {
				id = std::stoull(items[1]);
				//###split
				file.open(file_path);
				label_id = std::stoull(items[3]);
			}
			catch (...) { throw std::invalid_argument("Could not parse parameter or open file"); }

			g.target_sets[id] = std::make_unique<global::set_type>(
				std::move(int_set<global::int_type>::prismlabeltointset(file, [](auto s) { return std::stoull(s); }, label_id))
				);
			continue;
		}

		if (instruction == cli_commands::CALC_EXPECT) {
			if (items.size() != 5) throw std::invalid_argument("Wrong number of parameters.");
			std::size_t reward_index{ 0 }, destination_decoration{ 0 };
			cli_commands::id mc_id{ 0 }, target_id{ 0 };
			try {
				mc_id = std::stoull(items[1]);
				reward_index = std::stoull(items[2]);
				target_id = std::stoull(items[3]);
				destination_decoration = std::stoull(items[4]);
			}
			catch (...) { throw std::invalid_argument("Could not parse parameter"); }
			if (g.markov_chains[mc_id] == nullptr) throw std::logic_error("No mc with given ID");

			calc_expect(*(g.markov_chains[mc_id]), reward_index, *(g.target_sets[target_id]), destination_decoration);
			continue;
		}

		if (instruction == cli_commands::CALC_VARIANCE) {
			// mc_id, reward_index, target_id, destination_decoration, expect_decoration, free_reward
			if (items.size() != 7) throw std::invalid_argument("Wrong number of parameters.");
			std::size_t expect_decoration{ 0 }, destination_decoration{ 0 }, reward_index{ 0 }, free_reward{ 0 };
			cli_commands::id target_id{ 0 }, mc_id{ 0 };
			try {
				mc_id = std::stoull(items[1]);
				reward_index = std::stoull(items[2]);
				target_id = std::stoull(items[3]);
				destination_decoration = std::stoull(items[4]);
				expect_decoration = std::stoull(items[5]);
				free_reward = std::stoull(items[6]);
			}
			catch (...) { throw std::invalid_argument("Could not parse parameter"); }
			if (g.markov_chains[mc_id] == nullptr) throw std::logic_error("No mc with given ID");
			auto&& log = calc_variance(*(g.markov_chains[mc_id]), reward_index, *(g.target_sets[target_id]), destination_decoration, expect_decoration, free_reward);
			log[cli_commands::CALC_VARIANCE].push_back({ "markov_chain_id", mc_id });
			log[cli_commands::CALC_VARIANCE].push_back({ "target_set_id", target_id });
			performance_log.push_back(std::move(log));
			continue;
		}

		if (instruction == cli_commands::WRITE_DECO) {
			if (items.size() != 3) throw std::invalid_argument("Wrong number of parameters.");
			std::string& file_path = items[2];
			cli_commands::id id{ 0 };
			std::ofstream file{};
			try {
				id = std::stoull(items[1]);
			}
			catch (...) { throw std::invalid_argument("Could not parse parameter."); }
			try {
				file.open(file_path);
				if (!file.good()) throw 0;
			}
			catch (...) { throw std::invalid_argument("Bad file."); }
			if (g.markov_chains[id] == nullptr) throw std::logic_error("No markov chain present with given ID.");
			g.markov_chains[id]->write_edge_decorations(file);
			continue;
		}

		if (instruction == cli_commands::GENERATE_HERMAN) { // id mc, n, target_set_id
			if (items.size() != 4) throw std::invalid_argument("Wrong number of parameters.");
			cli_commands::id mc_id{ 0 }, target_set_id{ 0 };
			unsigned long size{ 0 };
			try {
				mc_id = std::stoull(items[1]);
				size = std::stoul(items[2]);
				target_set_id = std::stoull(items[3]);
			}
			catch (...) { throw std::invalid_argument("Could not parse parameter"); }
			if (g.markov_chains[mc_id] == nullptr) throw std::logic_error("No mc with given ID");

			auto&& log = generate_herman(*g.markov_chains[mc_id], size, g.target_sets[target_set_id]);
			log[cli_commands::GENERATE_HERMAN].push_back({"markov_chain_id", mc_id});
			log[cli_commands::GENERATE_HERMAN].push_back({ "target_set_id", target_set_id });
			performance_log.push_back(std::move(log));
			continue;
		}

		std::cout << "WARNING: Command not recognized:   " << command << "\nDid not match any known instruction key!\n";
	}
	json = std::move(performance_log);
}

