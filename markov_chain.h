#pragma once

#include <unordered_map>
#include <unordered_set>
#include <istream>
#include "bijective_unordered_map.h"
#include <regex>

#include "regxc.h"
#include <boost/regex.hpp>
#include "loghelper.h"
#include <exception>
#include <chrono>

#include <boost/algorithm/string.hpp>


template <class _ProbabilityType>
class markov_chain
{
public:
	using Reward = _ProbabilityType;
private:
	using regx_it = boost::regex_iterator<std::string::const_iterator>;

	std::size_t n_rewards; // number of edge values
	std::size_t n_decorations; // number of state values

	struct edge {
		edge(const _ProbabilityType& probability, std::size_t n_rewards = 0) : probability(probability), rewards(n_rewards, 0) {}

		_ProbabilityType probability;
		std::vector<_ProbabilityType> rewards;
	};

	struct state {
		state(std::size_t n_decorations) : decorations(n_decorations, 0) {}

		std::vector<_ProbabilityType> decorations;
	};

	std::unordered_map<unsigned long, std::unordered_map<unsigned long, edge*>> forward_transitions; // hardcoded unsigned long###
	std::unordered_map<unsigned long, std::unordered_map<unsigned long, edge*>> inverse_transitions;
	std::unordered_map<unsigned long, state> states;

	template<bool SKIP_ALREADY_INITIALIZED = false>
	inline void init_state(unsigned long id) {
		if (states.find(id) != states.end()) {
			if constexpr (SKIP_ALREADY_INITIALIZED) return;
			else throw std::logic_error("This state has already been initialized.");
		}
		states.emplace(id, n_decorations);
	}

public:

	/** Creates an empty markov chain.
	@details Per default memory will be allocated for given number of reward functions. */
	markov_chain(std::size_t n_rewards, std::size_t n_decorations) : n_rewards(n_rewards), n_decorations(n_decorations) {}

	bool empty() const {
		return forward_transitions.empty();
	}

	std::size_t size_states() const {
		return states.size();
	}

	/** Reads a prism transitions file to build up a markov chain.
	@details Checks whether the markov chain is empty before reading file. */
	void read_transitions_from_prism_file(std::istream& transitions) {
		if (!transitions.good()) throw std::invalid_argument("Bad stream.");
		surround_log("Building markov chain from prism transitions file", [&]() {
			// check for empty
			if (!empty()) throw std::logic_error("Forbidden to read transitions from file if markov chain is not empty.");

			// read input to string
			transitions.unsetf(std::ios_base::skipws); // also recognize new lines and spaces
			const auto input_s{ std::string(std::istream_iterator<char>(transitions),std::istream_iterator<char>()) };

			// check overall file format
			const auto valid_file_format{ boost::regex_match(input_s, regxc::prism_file_format) };
			std::cout << "Check for well-formed file format: " << interprete_bool_n(valid_file_format);
			if (!valid_file_format) throw std::invalid_argument("The file is not well-formed.");

			// find end of header line
			const auto regx_it_prism_header_transitions{ regx_it(input_s.cbegin(),input_s.cend(),regxc::prism_header) };
			const auto exists_header_line{ regx_it_prism_header_transitions != regx_it() };
			if (!exists_header_line) throw std::logic_error("Could not find end of the header line.");
			const std::string::const_iterator transitions_header_begin{ regx_it_prism_header_transitions->operator[](0).first };
			const std::string::const_iterator transitions_header_end{ regx_it_prism_header_transitions->operator[](0).second };

			// store number of states and number of transitions
			// do the same also for the rewards fiel!!!!!#####
			regx_it header_iterator{ regx_it(transitions_header_begin, transitions_header_end, regxc::nonnegative_integer) };
			auto s = std::string(transitions_header_begin, transitions_header_end);
			if (header_iterator == regx_it()) throw std::logic_error("Could not find number of states in header line.");
			unsigned long count_states{ std::stoul(header_iterator->str()) };
			++header_iterator;
			if (header_iterator == regx_it()) throw std::logic_error("Could not find number of transitions in header line.");
			unsigned long count_transitions{ std::stoul(header_iterator->str()) };


			// iterate all lines that define edges of the markov chain
			for (auto it = regx_it(transitions_header_end, input_s.cend(), regxc::prism_value_line); it != regx_it(); ++it) {
				auto jt = regx_it(it->operator[](0).first, it->operator[](0).second, regxc::nn_float);
				const auto wrong_number{ "Found a transition line which does not contain exactly 3 numbers." };

				if (jt == regx_it()) throw std::invalid_argument(wrong_number);
				const unsigned long from{ std::stoul(jt->str(0)) }; //hardcoded type for cast!!!#### // also check that it is an int literal?, same case in rewards function

				++jt;
				if (jt == regx_it()) throw std::invalid_argument(wrong_number);
				const unsigned long to{ std::stoul(jt->str(0)) };

				++jt;
				if (jt == regx_it()) throw std::invalid_argument(wrong_number);
				const double probability{ std::stod(jt->str(0)) };

				++jt;
				if (jt != regx_it()) throw std::invalid_argument(wrong_number);

				if (forward_transitions[from].find(to) != forward_transitions[from].end())
					throw std::invalid_argument("Trying to add a transition that already exists.");

				auto ptr{ new edge(probability, n_rewards) }; //##!! cast prob here!!
				inverse_transitions[to][from] = forward_transitions[from][to] = ptr;

				init_state<true>(from); init_state<true>(to);
			}
			if (empty()) std::cout << "WARNING: Created markov chain is still empty.\n";
			if (count_states != states.size()) std::cout << "WARNING: Number of states in header line is wrong.\n";
			// check also number of transitions.##
			});
	}

	/** Reads a prism rewards file to build up a markov chain.
	@details Checks whether the markov chain has all the transitions that rewards are defined for in given file. */
	void read_rewards_from_prism_file(std::istream& rewards, std::size_t index_of_reward = 0) {
		if (!rewards.good()) throw std::invalid_argument("Bad stream.");
		if (!(index_of_reward < n_rewards)) throw std::logic_error("Markov chain has not enough space for rewards. You need to specify number of rewards at construction. Adding more rewards dynamically is not yet implemented.");
		surround_log("Adding rewards to markov chain, reading from prism rewards file", [&]() {
			// read input to string
			rewards.unsetf(std::ios_base::skipws);
			const auto input_s{ std::string(std::istream_iterator<char>(rewards),std::istream_iterator<char>()) };

			// check overall file format
			const auto valid_file_format{ boost::regex_match(input_s, regxc::prism_file_format) };
			std::cout << "Check for well-formed file format: " << interprete_bool_n(valid_file_format);
			if (!valid_file_format) throw std::invalid_argument("The file is not well-formed.");

			// find end of header line
			const auto regx_it_prism_header_rewards{ regx_it(input_s.cbegin(),input_s.cend(),regxc::prism_header) };
			const auto exists_header_line{ regx_it_prism_header_rewards != regx_it() };
			if (!exists_header_line) throw std::logic_error("Could not find end of the header line.");
			std::string::const_iterator rewards_header_end{ regx_it_prism_header_rewards->operator[](0).second };

			// iterate all lines that define rewards of the markov chain
			for (auto it = regx_it(rewards_header_end, input_s.cend(), regxc::prism_value_line); it != regx_it(); ++it) {
				auto jt = regx_it(it->operator[](0).first, it->operator[](0).second, regxc::nn_float);
				const auto wrong_number{ "Found a reward line which does not contain exactly 3 numbers." };

				if (jt == regx_it()) throw std::invalid_argument(wrong_number);
				const unsigned long from{ std::stoul(jt->str(0)) };

				++jt;
				if (jt == regx_it()) throw std::invalid_argument(wrong_number);
				unsigned long to{ std::stoul(jt->str(0)) };

				++jt;
				if (jt == regx_it()) throw std::invalid_argument(wrong_number);
				double reward{ std::stod(jt->str(0)) };

				++jt;
				if (jt != regx_it()) throw std::invalid_argument(wrong_number);

				if (forward_transitions[from].find(to) == forward_transitions[from].end())
					throw std::invalid_argument("File defines reward for some non-existent edge.");

				if (forward_transitions[from][to]->rewards[index_of_reward])
					std::cout << "WARNING: Reward gets replaced.\n";
				forward_transitions[from][to]->rewards[index_of_reward] = reward; //##!! cast here!!
				// attention: requires each edge to appear once
			}
			});
	}

	void read_from_gmc_file(std::istream& input) {
		/// rename all identifiers here!
		auto& test_file_stream{ input };
		if (!input.good()) throw std::invalid_argument("Bad stream.");
		test_file_stream.unsetf(std::ios_base::skipws); // also recognize new lines and spaces
		surround_log("Building markov chain from gmc file", [&]() {
		
			//rebname::
			const auto test_file_string{ std::string(std::istream_iterator<char>(static_cast<std::istream&>(test_file_stream)),std::istream_iterator<char>()) };
			
			// describing formal "general markov chain"
			const auto gmc_s_new_line{ regex_strings::new_line };
			const auto gmc_s_irgnored_line{ regex_strings::native_ignored };
			auto gmc_general{ regxc::gmc_general };

			// Check for overall format:
			if (!boost::regex_match(test_file_string, regxc::gmc_general))
				throw std::invalid_argument("No valid GENERAL MARKOV CHAIN format");
			std::cout << "General syntax: okay.\n";

			// Locate semantics defintion line:
			const auto regx_it_semantics_definition{
				regx_it(test_file_string.begin(),test_file_string.end(),regxc::gmc_semantics_definition)
			};
			if (std::distance(regx_it_semantics_definition, regx_it()) != 1) throw std::invalid_argument("Syntax error: did not find exactly one semantics defintion.");
			std::string::const_iterator semantics_definition_begin{ regx_it_semantics_definition->operator[](0).first };
			std::string::const_iterator semantics_definition_end{ regx_it_semantics_definition->operator[](0).second };
			
			// Extract column names:
			auto column_names_vector{ std::vector<std::pair<std::string::const_iterator,std::string::const_iterator>>() };
			for (auto col_name_it{ regx_it(semantics_definition_begin, semantics_definition_end, regxc::gmc_column_name) };
				col_name_it != regx_it(); ++col_name_it) {
				column_names_vector.emplace_back(col_name_it->operator[](0).first, col_name_it->operator[](0).second);
			}
			std::cout << "Column names: " << column_names_vector.size() << std::endl; // necessary? <- and next lines:
			for (const auto& pair : column_names_vector) {
				for (auto it = pair.first; it != pair.second; ++it) std::cout << *it;
				std::cout << "\n";
			}
			const auto sfrom{ std::string("$from") };
			const auto sto{ std::string("$to") };
			const auto sprob{ std::string("$prob") };
			
			std::vector<std::string> cnv;
			std::transform(column_names_vector.cbegin(), column_names_vector.cend(), std::back_inserter(cnv), [](auto pair) {return std::string(pair.first, pair.second); });
			
			const auto cfrom{ std::find(cnv.cbegin(), cnv.cend(), sfrom) };
			const auto cto{ std::find(cnv.cbegin(), cnv.cend(), sto) };
			const auto cprob{ std::find(cnv.cbegin(), cnv.cend(), sprob) };
			
			if (cfrom == cnv.cend()) throw std::invalid_argument("No $from column");
			if (cto == cnv.cend()) throw std::invalid_argument("No $to column");
			if (cprob == cnv.cend()) throw std::invalid_argument("No $prob column");

			const std::size_t pfrom = cfrom - cnv.cbegin();
			const std::size_t pto = cto - cnv.cbegin();
			const std::size_t pprob = cprob - cnv.cbegin();
			std::set<std::size_t> ppositions{ pfrom, pto, pprob };

			throw std::logic_error("Not yet fully implemented.");
			// from here on recheck the code!
			
			//check all other lines for being in right format (wellformed body)
			const auto gmc_s_value_format{ std::string(R"([+-]?([0-9]*[.])?[0-9]+)") };
			const auto gmc_s_separator{ std::string(R"(\s*,\s*)") };

			const auto gmc_s_value_definition_line{ [&]() {
				auto result{ std::string() };
				result.reserve(gmc_s_separator.size() * (column_names_vector.size() - 1) + gmc_s_value_format.size() * column_names_vector.size() + 1);
				for (std::size_t i = 0; i < column_names_vector.size() - 1; ++i) (result += gmc_s_value_format) += gmc_s_separator;
				return result += gmc_s_value_format;
			}() };
			
			const auto gmc_value_definition_body{ boost::regex(
				std::string("((") + gmc_s_irgnored_line + "|" + gmc_s_value_definition_line + ")?(" + gmc_s_new_line + "|$))*"
			) };

			std::cout << "Check for wellformed body: " <<
				interprete_bool_n(boost::regex_match(semantics_definition_end, test_file_string.cend(), gmc_value_definition_body));


			//read all lines of body:
			if (!empty()) throw std::logic_error("Forbidden to read transitions from file if markov chain is not empty.");
			
			for (
				auto def_line_it{ regx_it(semantics_definition_end, test_file_string.cend(), boost::regex(gmc_s_value_definition_line)) };
				def_line_it != regx_it();
				++def_line_it) {
				
				// for each line defining some edge
				std::vector<std::string> row_items; 
				const std::string line{ def_line_it->str() };
				boost::split(row_items, line, boost::is_any_of(",")); // split the line at the commata
				if (row_items.size() != cnv.size()) throw std::logic_error("This should be already catched by regex match.");
				for (auto& item : row_items) item = boost::regex_replace(item, boost::regex("\\s"), "");
				
				unsigned long from{ 0 }, to{ 0 };
				_ProbabilityType p{ 0 };
				try {
					from = std::stoull(row_items[pfrom]);
					to = std::stoull(row_items[pto]);
					p = std::stod(row_items[pprob]);

					//create edge:
					auto e = new edge(p,n_rewards);
					
					// check if edge alr4eady exists!!!
					forward_transitions[from][to] = e;
					inverse_transitions[to][from] = e;
					if (states.find(from) == states.cend()) states.emplace(from, n_decorations);
					if (states.find(to) == states.cend()) states.emplace(to, n_decorations);

					//set rewards
					for (std::size_t i = 0; i < cnv.size(); ++i) {
						if (ppositions.find(i) != ppositions.cend()) continue;
						std::size_t reward_index{ std::stoull(cnv[i])};
						double reward{ std::stod(row_items[i]) };
						e->rewards.at(reward_index) = reward;
					}
				}
				catch (...) {
					throw std::invalid_argument("Could not read some parameter");
				}
				
			}
		
			});
	}

	markov_chain& operator=(const markov_chain&) = delete;
	markov_chain(const markov_chain&) = delete;

	~markov_chain() {
		for (const auto& i : forward_transitions) {
			for (const auto& j : i.second) {
				delete j.second;
			}
		}
	}

	template<class _Array>
	void set_decoration(const _Array& source, std::size_t index) {
		if (!(index < n_decorations)) throw std::out_of_range("Not enough decorations defined.");
		for (auto it{ states.begin() }; it != states.end(); ++it)
			it->second.decorations[index] = source[it->first];

	}

	template<class A, class B>
	friend class mc_analyzer;

	friend std::chrono::nanoseconds generate_herman_f(markov_chain<double>& mc, unsigned int size, std::unique_ptr<std::unordered_set<unsigned long>>& target_set);
};

