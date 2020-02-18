/**
 * @file commands.h
 *
 * Lists the instruction keywords that may be used from the commandline interface.
 *
 */

#pragma once

#include "global_data.h"


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
		@param free_transition_decoration Index for storing interim results as transition decorations. Choose a free index that contains values which can be overwritten.
	*/
	inline static const auto CALC_VARIANCE{ "calc_variance" };

	/**
		@brief Calculates for each state of the markov chain the covariance of accumulated transition decoration (rewards) until reaching the first state in target set.
		@details Syntax: calc_covariance>{mc_id}>{edge_decoration_1}>{edge_decoration_2}>{target_set_id}>{state_decoration_index}>{state_decoration_expects_index1}>{state_decoration_expects_index2}>{free_transition_decoration}
		@param mc_id id where the markov chain is stored.
		@param edge_decoration_1 Index of transition decorations (1. reward function) for that the variance should be calculated
		@param edge_decoration_2 Index of transition decorations (2. reward function) for that the variance should be calculated
		@param target_set_id Id to find the set of goal states.
		@param state_decoration_index Index of state decorations where the variances should be stored.
		@param state_decoration_expects_index1 Index of state decorations where the expects for the 1. reward function should be stored. (Note: the algorithm needs to calculate also expects in order to be able to claculate variances.)
		@param state_decoration_expects_index2 Index of state decorations where the expects for the 2. reward function should be stored. (Note: the algorithm needs to calculate also expects in order to be able to claculate variances.)
		@param free_transition_decoration Index for storing interim results as transition decorations. Choose a free index that contains values which can be overwritten.
	*/
	inline static const auto CALC_COVARIANCE{ "calc_covariance" };

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
		@details Syntax: generate_herman>{id}>{herman_size}>{target_set_id}
		@param id id where the markov chain is stored. It must have been initialized before (with reset_mc) and be empty.
		@param herman_size Number of the processes in ring architecture for herman instance. Must be odd.
		@param target_set_id Id of the target set that will be created by the algorithm to store the goal states corresponding to the created markov chain.
	*/
	inline static const auto GENERATE_HERMAN{ "generate_herman" }; // id mc, n, targetset id

	/**
		@brief Deletes a markov chain.
		@details Syntax: del_mc>{id}
		@param id Id of the markov chain to delete.
	*/
	inline static const auto DELETE_MC{ "del_mc" }; // id mc, n, targetset id

	/**
		@brief Deletes a target set.
		@details Syntax: del_ts>{id}
		@param id Id of the target set to delete.
	*/
	inline static const auto DELETE_TS{ "del_ts" }; // id mc, n, targetset id
	
	/**
		@brief Writes characteristic values about the markov chain into json log, e.g. number of states, number of transitions
		@details Syntax: print_mc>{id}
		@param id Id of the markov chain.
	*/
	inline static const auto PRINT_MC{ "print_mc" }; // id mc, n, targetset id

};
