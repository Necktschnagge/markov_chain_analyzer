#pragma once

#include "regxc.h"
#include "loghelper.h"

#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>

#include <unordered_map>
#include <unordered_set>
#include <istream>
#include <exception>
#include <chrono>
#include <sstream>

/**
	@brief Represents a morkov chain by storing edges with probabilities, with the possibility to store edge and state decorations.
*/
template <class _RationalT, class _IntegralT>
class markov_chain
{
public:
	/// @brief Type to store probabilities, edge-wise decorations (rewards) and state-wise decorations.
	using rational_type = _RationalT;
	/// @brief Type to enumerate states.
	using integral_type = _IntegralT;

private:

	/// @brief This is the regex iterator for internal use. Always use boost::regex rather than  std::regex.
	using regex_iterator = boost::regex_iterator<std::string::const_iterator>;

	/// @brief Structure for storing one edge (markov chain transition).
	struct edge {
		_RationalT probability;
		std::vector<_RationalT> decorations;

		edge(const _RationalT& probability, const std::size_t& n_edge_decorations) : probability(probability), decorations(n_edge_decorations, 0) {}
	};

	/// @brief Structure for storing one node (markov chain state).
	struct node {
		std::vector<_RationalT> decorations;

		node(const std::size_t& n_node_decorations) : decorations(n_node_decorations, 0) {}
	};

	/** @brief Number of decoration values of rational_type for each edge (i.e. markov chain transition).
		@details For achieving better access performance vectors in all edges are requried to have this size.
	*/
	std::size_t n_edge_decorations;
	/** @brief Number of decoration values of rational_type for each node (i.e. markov chain state).
		@details For achieving better access performance vectors in all nodes are requried to have this size.
	*/
	std::size_t n_node_decorations;

	/**
		@brief Map to access transitions partitioned after first node.
		@details Use semantics \a forward_transitions[from][to] to get a pointer to the desired edge.
		When adding new edges, it is required to keep \a forward_transitions and \a inverse_transitions consistent.
	*/
	std::unordered_map<_IntegralT, std::unordered_map<_IntegralT, edge*>> forward_transitions;
	/**
		@brief Map to access transitions partitioned after second node.
		@details Use semantics \a forward_transitions[to][from] to get a pointer to the desired edge.
		When adding new edges, it is required to keep \a forward_transitions and \a inverse_transitions consistent.
	*/
	std::unordered_map<_IntegralT, std::unordered_map<_IntegralT, edge*>> inverse_transitions;

	/// @brief Maps state id to associated node object containing decorations.
	std::unordered_map<_IntegralT, node> states;

	/**
		@brief Initializes a state with a decoration vector containing zeros.
		@details The node's decoration vector uses size defined by \a n_node_decorations.
		If the state with given ID is already intialized, nothing happens.
	*/
	inline void init_state(const _IntegralT& id) {
		states.emplace(id, n_node_decorations); // emplace does nothing if id already exists
	}

public:

	/**
		@brief Creates an empty markov chain.
		@details Memory will be allocated for given number of node (state) decorations and edge (transition) decorations.
	*/
	markov_chain(const std::size_t& n_edge_decorations, const std::size_t& n_node_decorations) noexcept :
		n_edge_decorations(n_edge_decorations),
		n_node_decorations(n_node_decorations),
		forward_transitions(),
		inverse_transitions(),
		states()
	{
	}

	/// @brief Returns true if and only if there are no states and no transitions.
	bool empty() const noexcept {
		// Consistency implies that both transition operands are the same:
		return forward_transitions.empty() && inverse_transitions.empty() && states.empty();
	}

	/// @brief Returns the number of states in the markov chain.
	auto size_states() const noexcept -> decltype(states.size()) {
		return states.size();
	}

	/**
		@brief Reads a prism transitions file to build up a markov chain.
		@details The markov chain must be empty before reading file.
		@exception std::invalid_argument Bad stream.
		@exception std::logic_error Forbidden to read transitions from file if markov chain is not empty.
		@exception std::invalid_argument The input is maleformed.
		@exception std::logic_error Could not find end of the header line.
		@expetion std::logic_error Could not find number of states in header line.
		@exception std::logic_error Could not find number of transitions in header line.
		@details some excpetions not yet documented. ###
	*/
	void read_transitions_from_prism_file(std::istream& transitions) {
		auto d = make_surround_log("Building markov chain from prism transitions file");
		if (!transitions.good()) throw std::invalid_argument("Bad stream.");
		if (!empty()) throw std::logic_error("Forbidden to read transitions from file if markov chain is not empty.");

		// Read the whole input into a string
		transitions.unsetf(std::ios_base::skipws); // also recognize new lines and spaces
		const auto input_s{ std::string(std::istream_iterator<char>(transitions),std::istream_iterator<char>()) };

		// check overall file format
		const auto valid_file_format{ boost::regex_match(input_s, regxc::prism_file_format) };
		std::cout << "Check for well-formed file format: " << interprete_bool_n(valid_file_format);
		if (!valid_file_format) throw std::invalid_argument("The input is maleformed.");

		// find end of header line
		const auto regx_it_prism_header_transitions{ regex_iterator(input_s.cbegin(),input_s.cend(),regxc::prism_header) };
		const auto exists_header_line{ regx_it_prism_header_transitions != regex_iterator() };
		if (!exists_header_line) throw std::logic_error("Could not find end of the header line.");
		const std::string::const_iterator transitions_header_begin{ regx_it_prism_header_transitions->operator[](0).first };
		const std::string::const_iterator transitions_header_end{ regx_it_prism_header_transitions->operator[](0).second };

		// store number of states and number of transitions
		regex_iterator header_iterator{ regex_iterator(transitions_header_begin, transitions_header_end, regxc::nonnegative_integer) };
		auto s = std::string(transitions_header_begin, transitions_header_end);
		if (header_iterator == regex_iterator()) throw std::logic_error("Could not find number of states in header line.");
		const auto n_states{ std::stoull(header_iterator->str()) };
		++header_iterator;
		if (header_iterator == regex_iterator()) throw std::logic_error("Could not find number of transitions in header line.");
		const auto n_transitions{ std::stoull(header_iterator->str()) };
		//using c = std::remove_const<decltype(n_transitions)>::type;
		std::size_t count_transitions{ 0 }; //### use type c!

		// iterate all lines that define edges of the markov chain
		for (auto it = regex_iterator(transitions_header_end, input_s.cend(), regxc::prism_value_line); it != regex_iterator(); ++it) {
			auto jt = regex_iterator(it->operator[](0).first, it->operator[](0).second, regxc::nn_float);
			const auto wrong_number{ "Found a transition line which does not contain exactly 3 numbers." };

			if (jt == regex_iterator()) throw std::invalid_argument(wrong_number);
			const auto from{ std::stoull(jt->str(0)) };

			++jt;
			if (jt == regex_iterator()) throw std::invalid_argument(wrong_number);
			const auto to{ std::stoull(jt->str(0)) };

			++jt;
			if (jt == regex_iterator()) throw std::invalid_argument(wrong_number);
			const double probability{ std::stod(jt->str(0)) };

			++jt;
			if (jt != regex_iterator()) throw std::invalid_argument(wrong_number);

			if (forward_transitions[from].find(to) != forward_transitions[from].end())
				throw std::invalid_argument("Trying to add a transition that already exists.");

			//## cast prob here!! We need custom striung to int/double in case someone chooses custom types.
			auto ptr{ new edge(probability, n_edge_decorations) };
			inverse_transitions[to][from] = forward_transitions[from][to] = ptr;
			++count_transitions;

			init_state(from);
			init_state(to);
		}
		if (empty()) std::cout << "WARNING: Created markov chain is still empty.\n";
		if (n_states != states.size())
			std::cout << "WARNING: Number of states in header line is wrong.\n";
		if (n_transitions != count_transitions)
			std::cout << "WARNING: Number of transitions in header line is wrong.\n";
	}

	/**
		@brief Reads a prism rewards file to build up a markov chain.
		@details Checks whether the markov chain has all the transitions that rewards are defined for in given file.
		@details Not all exceptions deocumented yet ###
	*/
	void read_rewards_from_prism_file(std::istream& rewards, const std::size_t& index_of_reward = 0) {
		if (!rewards.good()) throw std::invalid_argument("Bad stream.");
		if (!(index_of_reward < n_edge_decorations)) throw std::logic_error("Markov chain has not enough space for rewards. You need to specify number of rewards at construction. Adding more rewards dynamically is not yet implemented.");
		auto d = make_surround_log("Adding rewards to markov chain, reading from prism rewards file");
		// read input to string
		rewards.unsetf(std::ios_base::skipws);
		const auto input_s{ std::string(std::istream_iterator<char>(rewards),std::istream_iterator<char>()) };

		// check overall file format
		const auto valid_file_format{ boost::regex_match(input_s, regxc::prism_file_format) };
		std::cout << "Check for well-formed file format: " << interprete_bool_n(valid_file_format);
		if (!valid_file_format) throw std::invalid_argument("The file is not well-formed.");

		// find end of header line
		const auto regx_it_prism_header_rewards{ regex_iterator(input_s.cbegin(),input_s.cend(),regxc::prism_header) };
		const auto exists_header_line{ regx_it_prism_header_rewards != regex_iterator() };
		if (!exists_header_line) throw std::logic_error("Could not find end of the header line.");
		std::string::const_iterator rewards_header_end{ regx_it_prism_header_rewards->operator[](0).second };

		// iterate all lines that define rewards of the markov chain
		for (auto it = regex_iterator(rewards_header_end, input_s.cend(), regxc::prism_value_line); it != regex_iterator(); ++it) {
			auto jt = regex_iterator(it->operator[](0).first, it->operator[](0).second, regxc::nn_float);
			const auto wrong_number{ "Found a reward line which does not contain exactly 3 numbers." };

			if (jt == regex_iterator()) throw std::invalid_argument(wrong_number);
			const unsigned long from{ std::stoul(jt->str(0)) };

			++jt;
			if (jt == regex_iterator()) throw std::invalid_argument(wrong_number);
			unsigned long to{ std::stoul(jt->str(0)) };

			++jt;
			if (jt == regex_iterator()) throw std::invalid_argument(wrong_number);
			double reward{ std::stod(jt->str(0)) };

			++jt;
			if (jt != regex_iterator()) throw std::invalid_argument(wrong_number);

			if (forward_transitions[from].find(to) == forward_transitions[from].end())
				throw std::invalid_argument("File defines reward for some non-existent edge.");

			if (forward_transitions[from][to]->decorations[index_of_reward])
				std::cout << "WARNING: Reward gets replaced.\n";
			forward_transitions[from][to]->decorations[index_of_reward] = reward; //##!! cast here!!
			// attention: requires each edge to appear once
		}
	}

	//### sanity check for probabilitiy functions

	void read_from_gmc_file(std::istream& input) {
		/// rename all identifiers here!
		auto& test_file_stream{ input };
		if (!input.good()) throw std::invalid_argument("Bad stream.");
		test_file_stream.unsetf(std::ios_base::skipws); // also recognize new lines and spaces
		auto d = make_surround_log("Building markov chain from gmc file");

		//rename::
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
			regex_iterator(test_file_string.begin(),test_file_string.end(),regxc::gmc_semantics_definition)
		};
		if (std::distance(regx_it_semantics_definition, regex_iterator()) != 1) throw std::invalid_argument("Syntax error: did not find exactly one semantics defintion.");
		std::string::const_iterator semantics_definition_begin{ regx_it_semantics_definition->operator[](0).first };
		std::string::const_iterator semantics_definition_end{ regx_it_semantics_definition->operator[](0).second };

		// Extract column names:
		auto column_names_vector{ std::vector<std::pair<std::string::const_iterator,std::string::const_iterator>>() };
		for (auto col_name_it{ regex_iterator(semantics_definition_begin, semantics_definition_end, regxc::gmc_column_name) };
			col_name_it != regex_iterator(); ++col_name_it) {
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
			auto def_line_it{ regex_iterator(semantics_definition_end, test_file_string.cend(), boost::regex(gmc_s_value_definition_line)) };
			def_line_it != regex_iterator();
			++def_line_it) {

			// for each line defining some edge
			std::vector<std::string> row_items;
			const std::string line{ def_line_it->str() };
			boost::split(row_items, line, boost::is_any_of(",")); // split the line at the commata
			if (row_items.size() != cnv.size()) throw std::logic_error("This should be already catched by regex match.");
			for (auto& item : row_items) item = boost::regex_replace(item, boost::regex("\\s"), "");

			unsigned long from{ 0 }, to{ 0 };
			_RationalT p{ 0 };
			try {
				from = std::stoull(row_items[pfrom]);
				to = std::stoull(row_items[pto]);
				p = std::stod(row_items[pprob]);

				//create edge:
				auto e = new edge(p, n_edge_decorations);

				// check if edge alr4eady exists!!!
				forward_transitions[from][to] = e;
				inverse_transitions[to][from] = e;
				if (states.find(from) == states.cend()) states.emplace(from, n_node_decorations);
				if (states.find(to) == states.cend()) states.emplace(to, n_node_decorations);

				//set rewards
				for (std::size_t i = 0; i < cnv.size(); ++i) {
					if (ppositions.find(i) != ppositions.cend()) continue;
					std::size_t reward_index{ std::stoull(cnv[i]) };
					double reward{ std::stod(row_items[i]) };
					e->decorations.at(reward_index) = reward;
				}
			}
			catch (...) {
				throw std::invalid_argument("Could not read some parameter");
			}

		}
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
		if (!(index < n_node_decorations)) throw std::out_of_range("Not enough decorations defined.");
		for (auto it{ states.begin() }; it != states.end(); ++it)
			it->second.decorations[index] = source[it->first];

	}

	template<class, class>
	friend class mc_analyzer;

	template<class _Rationals, class _Integers, class _Set, bool>
	friend std::chrono::nanoseconds generate_herman(markov_chain<_Rationals, _Integers>& mc, const _Integers& size, std::unique_ptr<_Set>& target_set);
};

