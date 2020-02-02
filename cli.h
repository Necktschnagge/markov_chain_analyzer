/**
 * @file cli.h
 *
 * Commandline interface for MC Analyzer.
 *
 */
#pragma once

#include "global_data.h"

#include "json.hpp"

#include <fstream>

 /**
	 @brief Contains all instruction key to use MC Analyzer form command line. Additionally syntax information are given how to use the instructions.
 */
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
		@brief Reads transitions from prism's file format to build up a markov chain.
		@details Syntax: read_tra>{id}>{file}
		@param id id where the markov chain is stored. It must have been initialized before (with reset_mc) and be empty.
		@param file file path of the *.tra file to read
	*/
	inline static const auto READ_TRA{ "read_tra" }; // id, file

	/**
		@brief Reads a markov chain from general markov chain (gmc) format, a file format introduced with this tool.
		@details Syntax: read_gmc>{id}>{file}
		@param id id where the markov chain is stored. It must have been initialized before (with reset_mc) and be empty.
		@param file file path of the *.gmc file to read
	*/
	inline static const auto READ_GMC{ "read_gmc" };

	/**
		@brief Reads edge decorations (rewards) from prism's trew file format. Needs an initialized non-empty markov chain thatt already has a transition for each reward defined in file.
		@details Syntax: add_rew>{id}>{file}>{decoration_index}
		@param id id where the markov chain is stored. It must have been initialized before (with reset_mc) and be empty.
		@param file file path of the *.trew file to read
		@param decoration_index index where to store the reward values into each edge.
	*/
	inline static const auto ADD_REW{ "add_rew" };

	/**
		@brief Reads a set of integers from a file to store it as target set for expect / variance / cobvaraiance calculation
		@details Syntax: read_target>{id}>{file}
		@param id id where the target set is stored. Previous target set located at given id will be overwritten.
		@param file file path of the file to read
	*/
	inline static const auto READ_TARGET{ "read_target" };

	/**
		@brief Reads a prism state label file in order to recognize a set of target states
		@details Syntax: read_label>{id}>{file}>{label_id}
		@param id id where the target set is stored. Previous target set located at given id will be overwritten.
		@param file file path of the file to read
	*/
	inline static const auto READ_LABEL{ "read_label" };

	/**
		@brief Calculates for each state of the markov chain the expect of accumulated transition decoration (rewards) until reaching the first state in target set.
		@details Syntax: calc_expect>{mc_id}>{transition_decoration_index}>{target_set_id}>{state_decoration_index}
		@param mc_id id where the markov chain is stored.
		@param transition_decoration_index Index of transition decorations (rewards) for that the expect should be calculated
		@param target_set_id Id to find the set of goal states.
		@param state_decoration_index Index of state decorations where the expect should be stored.
	*/
	inline static const auto CALC_EXPECT{ "calc_expect" };

	/**
		@brief Calculates for each state of the markov chain the variance of accumulated transition decoration (rewards) until reaching the first state in target set.
		@details Syntax: calc_variance>{mc_id}>{transition_decoration_index}>{target_set_id}>{state_decoration_index}>{state_decoration_expects_index}>{free_transition_decoration}
		@param mc_id id where the markov chain is stored.
		@param transition_decoration_index Index of transition decorations (rewards) for that the variance should be calculated
		@param target_set_id Id to find the set of goal states.
		@param state_decoration_index Index of state decorations where the variances should be stored.
		@param state_decoration_expects_index Index of state decorations where the expects should be stored. (Note: the algorithm needs to calculate also expects in order to be able to claculate variances.)
		@param state_decoration_index Index for storing interim results as transition decorations. Choose a free index that contains values which can be overwritten.
	*/
	inline static const auto CALC_VARIANCE{ "calc_variance" };

	//inline static const auto write_gmc{ "write_gmc" }; //##not implemented

	/**
		@brief Writes state decorations of a markov chain into a file.
		@details Syntax: write_state_decorations>{mc_id}>{file}
		@param mc_id Id of the markov chian which should be written to file.
		@param file File path where the result should be stored.
	*/
	inline static const auto WRITE_DECO{ "write_state_decorations" };

	/**
		@brief Generates transitions for Herman's self-stabilizing algorithm and sets all edge decorations at index 0 to 1.
		@details Syntax: generate_herman>{id}>{herman_size}>{target_set_id} It also prints the time that has been spent.
		@param id id where the markov chain is stored. It must have been initialized before (with reset_mc) and be empty.
		@param herman_size Number of the processes in ring architecture for herman instance. Must be odd.
		@param target_set_id Id of the target set that will be created by the algorithm to store the goal states corresponding to the created markov chain.
	*/
	inline static const auto GENERATE_HERMAN{ "generate_herman" }; // id mc, n, targetset id

	//inline static //cov calc
	// delte mc , delete target set #####
};

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
		std::cout << "performance_log.json so far...\n\n" << performance_log;

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
			calc_variance(*(g.markov_chains[mc_id]), reward_index, *(g.target_sets[target_id]), destination_decoration, expect_decoration, free_reward);
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
			log["generate_herman"].push_back({"markov_chain_id", mc_id});
			log["generate_herman"].push_back({ "target_set_id", target_set_id });
			performance_log.push_back(std::move(log));
			continue;
		}

		std::cout << "WARNING: Command not recognized:   " << command << "\nDid not match any known instruction key!\n";
	}
}

