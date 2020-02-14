#pragma once

#include <string>

struct sc {

	inline static const auto markov_chain_id{ std::string("mc_id") };
	inline static const auto target_set_id{ std::string("ts_id") };
	inline static const auto number_node_decorations{ std::string("n_node_deco") };
	inline static const auto number_edge_decorations{ std::string("n_edge_deco") };
	inline static const auto file_path{ std::string("file_path") };
	inline static const auto decoration_index{ std::string("deco_index") };
	inline static const auto prism_label_id{ std::string("prism_label_id") };
	inline static const auto size{ std::string("size") };
	inline static const auto size_nodes{ std::string("size_nodes") };
	inline static const auto size_edges{ std::string("size_edges") };
	inline static const auto time_run_checks{ std::string("time_run_checks") };
	inline static const auto time_run_generator{ std::string("time_run_generator") };
	inline static const auto time_total{ std::string("time_total") };
	inline static const auto unit{ std::string("unit") };
	inline static const auto milliseconds{ std::string("milliseconds") };

};