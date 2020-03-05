/**
 * @file cli.h
 *
 * Commandline interface for MC Analyzer.
 *
 */
#pragma once

#include "commands.h"
#include "string_constants.h"
#include "global_data.h"

#include "nlohmann/json.hpp"

#include <fstream>



class failed_instruction : public std::runtime_error {
public:
	using _Mybase = std::runtime_error;

	explicit failed_instruction(const std::string& _Message) : _Mybase(_Message.c_str()) {}

	explicit failed_instruction(const char* _Message) : _Mybase(_Message) {}

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

	@return Logs as \a nlohmann::json containing qualitative description of what happenned as well as quantitative performance measures.
*/
inline nlohmann::json cli(std::istream& commands, global& g) {

	using mc_type = global::mc_type;
	const std::string split_symbol{ ">" };
	auto performance_log{ nlohmann::json() };

	commands.unsetf(std::ios_base::skipws); // also read whitespaces
	while (commands.good())
	{
		// Print performance log
		std::cout << "\nPerformance Log JSON so far...\n\n" << performance_log << "\n\n";

		//fetch command
		std::string command{};
		std::getline(commands, command);
		std::cout << "\n\nFetched command: " << command << std::endl;

		if (boost::regex_match(command, boost::regex(R"(\s*)"))) {
			std::cout << "Recognized empty line. Skipping ..." << std::endl;
			continue;
		}
		try {
			//parse command
			std::vector<std::string> items;
			boost::split(items, command, boost::is_any_of(split_symbol));
			if (items.size() == 0) throw failed_instruction(std::string("Maleformed instruction: ") + command);
			std::string& instruction{ items[0] };
			auto doc = make_surround_log("Executing command");

			//execute command
			if (instruction == cli_commands::RESET_MC) {
				if (items.size() != 4) throw failed_instruction("Wrong number of parameters.");
				std::size_t n_state_decoration{ 0 }, n_transition_decoration{ 0 };
				global::id id{ 0 };
				try {
					id = std::stoull(items[1]);
					n_state_decoration = std::stoull(items[2]);
					n_transition_decoration = std::stoull(items[3]);
				}
				catch (...) { throw failed_instruction("Could not parse parameter."); }
				g.markov_chains[id] = std::make_unique<mc_type>(n_transition_decoration, n_state_decoration);
				performance_log.push_back({
						{instruction,
							{
								{ sc::markov_chain_id, id},
								{ sc::number_node_decorations, n_state_decoration},
								{ sc::number_edge_decorations, n_transition_decoration}
							}
						}
					});
				continue;
			}

			if (instruction == cli_commands::READ_TRA) {
				if (items.size() != 3) throw failed_instruction("Wrong number of parameters.");
				std::string& file_path = items[2];
				global::id id{ 0 };
				std::ifstream file{};
				try {
					id = std::stoull(items[1]);
				}
				catch (...) { throw failed_instruction("Could not parse parameter."); }
				file.open(file_path);
				if (!file.good()) throw failed_instruction("Could not open file.");
				if (g.markov_chains[id] == nullptr) throw failed_instruction("No markov chain present with given ID.");
				g.markov_chains[id]->read_transitions_from_prism_file(file);
				performance_log.push_back({
						{instruction,
							{
								{ sc::markov_chain_id, id},
								{ sc::file_path, file_path}
							}
						}
					});
				continue;
			}

			if (instruction == cli_commands::READ_GMC) {
				if (items.size() != 3) throw failed_instruction("Wrong number of parameters.");
				std::string& file_path = items[2];
				std::size_t id{ 0 };
				std::ifstream file{};
				file.open(file_path);
				try {
					id = std::stoul(items[1]);
				}
				catch (...) { throw failed_instruction("Could not parse parameter"); }
				if (!file.good()) throw failed_instruction("Could not open file.");
				if (g.markov_chains[id] == nullptr) throw failed_instruction("No mc with given ID");
				g.markov_chains[id]->read_from_gmc_file(file);
				performance_log.push_back({
						{instruction,
							{
								{ sc::markov_chain_id, id},
								{ sc::file_path, file_path}
							}
						}
					});
				continue;
			}

			if (instruction == cli_commands::ADD_REW) {
				if (items.size() != 4) throw failed_instruction("Wrong number of parameters.");
				std::string& file_path = items[2];
				std::size_t rew_index{ 0 };
				global::id id{ 0 };
				std::ifstream file{};
				file.open(file_path);
				try {
					id = std::stoull(items[1]);
					rew_index = std::stoull(items[3]);
				}
				catch (...) { throw failed_instruction("Could not parse parameter."); }
				if (!file.good()) throw failed_instruction("Could not open file.");
				if (g.markov_chains[id] == nullptr) throw failed_instruction("No mc with given ID");
				g.markov_chains[id]->read_rewards_from_prism_file(file, rew_index);
				performance_log.push_back({
						{instruction,
							{
								{ sc::markov_chain_id, id},
								{ sc::decoration_index, rew_index },
								{ sc::file_path, file_path}
							}
						}
					});
				continue;
			}

			if (instruction == cli_commands::READ_TARGET) {
				if (items.size() != 3) throw failed_instruction("Wrong number of parameters.");
				std::string& file_path = items[2];
				global::id id{ 0 };
				std::ifstream file{};
				file.open(file_path);
				try {
					id = std::stoul(items[1]);
				}
				catch (...) { throw failed_instruction("Could not parse parameter."); }
				if (!file.good()) throw failed_instruction("Could not open file.");
				g.target_sets[id] = std::make_unique<global::set_type>(
					std::move(int_set<global::int_type>::stointset(file, [](auto s) { return std::stoull(s); }))
					);
				performance_log.push_back({
						{instruction,
							{
								{ sc::target_set_id, id},
								{ sc::file_path, file_path}
							}
						}
					});
				continue;
			}

			if (instruction == cli_commands::READ_LABEL) {
				if (items.size() != 4) throw failed_instruction("Wrong number of parameters.");
				std::string& file_path = items[2];
				std::size_t label_id{ 0 };
				global::id id{ 0 };
				std::ifstream file{};
				file.open(file_path);
				try {
					id = std::stoull(items[1]);
					label_id = std::stoull(items[3]);
				}
				catch (...) { throw failed_instruction("Could not parse parameter."); }
				if (!file.good()) throw failed_instruction("Could not open file.");
				g.target_sets[id] = std::make_unique<global::set_type>(
					std::move(int_set<global::int_type>::prismlabeltointset(file, [](auto s) { return std::stoull(s); }, label_id))
					);
				performance_log.push_back({
						{instruction,
							{
								{ sc::target_set_id, id },
								{ sc::file_path, file_path },
								{ sc::prism_label_id, label_id }
							}
						}
					});
				continue;
			}

			if (instruction == cli_commands::CALC_EXPECT) {
				if (items.size() != 5) throw failed_instruction("Wrong number of parameters.");
				std::size_t reward_index{ 0 }, destination_decoration{ 0 };
				global::id mc_id{ 0 }, target_id{ 0 };
				try {
					mc_id = std::stoull(items[1]);
					reward_index = std::stoull(items[2]);
					target_id = std::stoull(items[3]);
					destination_decoration = std::stoull(items[4]);
				}
				catch (...) { throw failed_instruction("Could not parse parameter"); }
				if (g.markov_chains[mc_id] == nullptr) throw failed_instruction("No mc with given ID");

				auto&& log = calc_expect(*(g.markov_chains[mc_id]), reward_index, *(g.target_sets[target_id]), destination_decoration);
				log[instruction].push_back({ sc::markov_chain_id, mc_id });
				log[instruction].push_back({ sc::target_set_id, target_id });
				performance_log.push_back(std::move(log));
				continue;
			}

			if (instruction == cli_commands::CALC_VARIANCE) {
				// mc_id, reward_index, target_id, destination_decoration, expect_decoration, free_reward
				if (items.size() != 7) throw failed_instruction("Wrong number of parameters.");
				std::size_t expect_decoration{ 0 }, destination_decoration{ 0 }, reward_index{ 0 }, free_reward{ 0 };
				global::id target_id{ 0 }, mc_id{ 0 };
				try {
					mc_id = std::stoull(items[1]);
					reward_index = std::stoull(items[2]);
					target_id = std::stoull(items[3]);
					destination_decoration = std::stoull(items[4]);
					expect_decoration = std::stoull(items[5]);
					free_reward = std::stoull(items[6]);
				}
				catch (...) { throw failed_instruction("Could not parse parameter"); }
				if (g.markov_chains[mc_id] == nullptr) throw failed_instruction("No mc with given ID");
				auto&& log = calc_variance(*(g.markov_chains[mc_id]), reward_index, *(g.target_sets[target_id]), destination_decoration, expect_decoration, free_reward);
				log[instruction].push_back({ sc::markov_chain_id, mc_id });
				log[instruction].push_back({ sc::target_set_id, target_id });
				performance_log.push_back(std::move(log));
				continue;
			}

			if (instruction == cli_commands::CALC_COVARIANCE) {
				/* Syntax: calc_covariance
					>{mc_id}
					>{edge_decoration_1}
					>{edge_decoration_2}
					>{target_set_id}
					>{state_decoration_index}
					>{state_decoration_expects_index1}
					>{state_decoration_expects_index2}
					>{free_transition_decoration}
				*/
				if (items.size() != 9) throw failed_instruction("Wrong number of parameters.");
				std::size_t state_decoration_expects_index1{ 0 },
					state_decoration_expects_index2{ 0 },
					destination_decoration{ 0 },
					edge_decoration_1{ 0 },
					edge_decoration_2{ 0 },
					free_reward{ 0 };
				global::id target_set_id{ 0 },
					mc_id{ 0 };
				try {
					mc_id = std::stoull(items[1]);
					edge_decoration_1 = std::stoull(items[2]);
					edge_decoration_2 = std::stoull(items[3]);
					target_set_id = std::stoull(items[4]);
					destination_decoration = std::stoull(items[5]);
					state_decoration_expects_index1 = std::stoull(items[6]);
					state_decoration_expects_index2 = std::stoull(items[7]);
					free_reward = std::stoull(items[8]);
				}
				catch (...) { throw failed_instruction("Could not parse parameter"); }
				if (g.markov_chains[mc_id] == nullptr) throw failed_instruction("No mc with given ID");
				auto&& log = calc_covariance(
					*(g.markov_chains[mc_id]),
					edge_decoration_1,
					edge_decoration_2,
					*(g.target_sets[target_set_id]),
					destination_decoration,
					state_decoration_expects_index1,
					state_decoration_expects_index2,
					free_reward);
				log[instruction].push_back({ sc::markov_chain_id, mc_id });
				log[instruction].push_back({ sc::target_set_id, target_set_id });
				performance_log.push_back(std::move(log));
				continue;
			}

			if (instruction == cli_commands::WRITE_DECO) {
				if (items.size() != 3) throw failed_instruction("Wrong number of parameters.");
				std::string& file_path = items[2];
				global::id id{ 0 };
				std::ofstream file{};
				try {
					id = std::stoull(items[1]);
				}
				catch (...) { throw failed_instruction("Could not parse parameter."); }
				file.open(file_path);
				if (!file.good()) { throw failed_instruction("Bad file."); }
				if (g.markov_chains[id] == nullptr) throw failed_instruction("No markov chain present with given ID.");
				g.markov_chains[id]->write_edge_decorations(file);
				performance_log.push_back({
						{instruction,
							{
								{ sc::markov_chain_id, id },
								{ sc::file_path, file_path }
							}
						}
					});
				continue;
			}

			if (instruction == cli_commands::GENERATE_HERMAN) { // id mc, n, target_set_id
				if (items.size() != 4) throw failed_instruction("Wrong number of parameters.");
				global::id mc_id{ 0 }, target_set_id{ 0 };
				unsigned long size{ 0 };
				try {
					mc_id = std::stoull(items[1]);
					size = std::stoul(items[2]);
					target_set_id = std::stoull(items[3]);
				}
				catch (...) { throw failed_instruction("Could not parse parameter"); }
				if (g.markov_chains[mc_id] == nullptr) throw failed_instruction("No mc with given ID");

				auto&& log = generate_herman(*g.markov_chains[mc_id], size, g.target_sets[target_set_id]);
				log[instruction].push_back({ sc::markov_chain_id, mc_id });
				log[instruction].push_back({ sc::target_set_id, target_set_id });
				log[instruction].push_back({ sc::size, size });
				performance_log.push_back(std::move(log));
				continue;
			}

			if (instruction == cli_commands::DELETE_MC) {
				if (items.size() != 2) throw failed_instruction("Wrong number of parameters.");
				global::id id{ 0 };
				try {
					id = std::stoull(items[1]);
				}
				catch (...) { throw failed_instruction("Could not parse parameter"); }
				if (g.markov_chains[id] == nullptr) throw failed_instruction("No mc with given ID");
				g.markov_chains.erase(id);
				performance_log.push_back({
						{instruction,
							{
								{ sc::markov_chain_id, id }
							}
						}
					});
				continue;
			}

			if (instruction == cli_commands::DELETE_TS) {
				if (items.size() != 2) throw failed_instruction("Wrong number of parameters.");
				global::id id{ 0 };
				try {
					id = std::stoull(items[1]);
				}
				catch (...) { throw failed_instruction("Could not parse parameter"); }
				if (g.target_sets[id] == nullptr) throw failed_instruction("No mc with given ID");
				g.target_sets.erase(id);
				performance_log.push_back({
						{instruction,
							{
								{ sc::target_set_id, id }
							}
						}
					});
				continue;
			}

			if (instruction == cli_commands::PRINT_MC) {
				if (items.size() != 2) throw failed_instruction("Wrong number of parameters.");
				global::id id{ 0 };
				try {
					id = std::stoull(items[1]);
				}
				catch (...) { throw failed_instruction("Could not parse parameter"); }
				if (g.markov_chains[id] == nullptr) throw std::logic_error("No mc with given ID");
				performance_log.push_back({
						{instruction,
							{
								{ sc::markov_chain_id, id },
								{ sc::size_nodes, g.markov_chains[id]->size_states() },
								{ sc::size_edges, g.markov_chains[id]->size_edges() }
							}
						}
					});
				continue;
			}

			std::cout << "WARNING: Command not recognized:   " << command << "\nDid not match any known instruction key!\n";
		}
		catch (const failed_instruction & e) {
			std::cout << "ERROR:   failed_instruction:  " << e.what() << "\n";
		}

	}
	return performance_log;
}

