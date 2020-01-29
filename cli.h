/**
 * @file cli.h
 *
 * Commandline interface for MC Analyzer.
 *
 */
#pragma once

#include "global_data.h"

#include <fstream>

struct cli_commands {

	using id = global::id;

	/**
		@brief Reinitialized a markov chain. The markov chain is replaced by a fresh one.
		@details Syntax: reset_mc>{id}>{n_state_decorations}>{n_transition_decorations}
		@param id                         id where to locate markov chain
		@param n_state_decorations        number of state decorations (Needed for e.g. expect values, or variances)
		@param n_transition_decorations   number of transition decorations (Needed for storing e.g. rewards)
	*/
	inline static const auto RESET_MC{ "reset_mc" };

	/**
		@brief reads transitions from prism's file format to build up a markov chain.
		@param id id where the markov chain is stored. It must have been initialized before (with reset_mc) and be empty.
		@param file fiel path of the *.tra file to read
	*/
	inline static const auto read_tra{ "read_tra" }; // id, file
	inline static const auto read_gmc{ "read_gmc" }; // id, file
	inline static const auto add_rew{ "add_rew" }; // id, file, reward_index
	inline static const auto read_target{ "read_target" }; // id, file
	inline static const auto read_label{ "read_label" }; // id, file, label_id
	inline static const auto calc_expect{ "calc_expect" }; // mc_id, reward_index, target_id, destination_decoration
	inline static const auto calc_variance{ "calc_variance" }; // mc_id, reward_index, target_id, destination_decoration, expect_decoration, free_reward
	inline static const auto write_gmc{ "write_gmc" }; //##not impl
	inline static const auto write_deco{ "write_deco" }; /// ##not impl
	//inline static //cov calc
	inline static const auto GENERATE_HERMAN{ "generate_herman" }; // id mc, n
};

/**
	@brief Reads commands from stream, performs corresponding actions.
	@param commands stream containing commands, separated by new line ('\n'). Parameters are separated with '>' within one line.
	@param g global struct for storing data
	@exception std::logic_error Maleformed instruction...
	@exception std::invalid_argument Wrong number of parameters.
	@exception std::invalid_argument Could not parse parameter.
*/
inline void cli(std::istream& commands, global& g) {

	using mc_type = markov_chain<double, unsigned long>; // ### what bit width is appropriate for measure purpose???, what for release
	const std::string split_symbol{ ">" };

	commands.unsetf(std::ios_base::skipws); // also read whitespaces
	while (commands.good())
	{
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
				id = std::stoul(items[1]);
				n_state_decoration = std::stoull(items[2]);
				n_transition_decoration = std::stoull(items[3]);
			}
			catch (...) { throw std::invalid_argument("Could not parse parameter."); }
			g.markov_chains[id] = std::make_unique<mc_type>(n_transition_decoration, n_state_decoration);
			continue;
		}

		if (instruction == cli_commands::read_tra) {
			if (items.size() != 3) throw std::invalid_argument("Wrong number of parameters.");
			std::string& file_path = items[2];
			cli_commands::id id{ 0 };
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

		if (instruction == cli_commands::read_gmc) {
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

		if (instruction == cli_commands::add_rew) {
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

		if (instruction == cli_commands::read_target) {
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

		if (instruction == cli_commands::read_label) {
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

		if (instruction == cli_commands::calc_expect) { //  mc_id, reward_index, target_id, destination_decoration
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

		if (instruction == cli_commands::calc_variance) {
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

		if (instruction == cli_commands::GENERATE_HERMAN) { // id mc, n, target_set_id
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

			auto time = generate_herman(*g.markov_chains[mc_id], size, g.target_sets[target_set_id]);
			printDuration(time);
			continue;
		}

		std::cout << "WARNING: Command not recognized:   " << command << "\nDid not match any known instruction key!\n";
	}
}

