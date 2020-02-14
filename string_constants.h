#pragma once

#include <string>

struct sc {

	inline static const auto markov_chain_id{ std::string("mc_id") };
	inline static const auto target_set_id{ std::string("ts_id") };
	inline static const auto number_node_decorations{ std::string("n_node_deco") };
	inline static const auto number_edge_decorations{ std::string("n_edge_deco") };
	inline static const auto file_path{ std::string("file_path") };
	inline static const auto decoration_index{ std::string("deco_index") };
	inline static const auto decoration_index_node_target{ std::string("deco_index_node_target") };
	inline static const auto decoration_index_egde_source{ std::string("deco_index_egde_source") };
	inline static const auto decoration_index_egde_free{ std::string("deco_index_egde_free") };
	inline static const auto prism_label_id{ std::string("prism_label_id") };
	inline static const auto size{ std::string("size") };
	inline static const auto size_nodes{ std::string("size_nodes") };
	inline static const auto size_edges{ std::string("size_edges") };
	inline static const auto time_run_checks{ std::string("time_run_checks") };
	inline static const auto time_run_generator{ std::string("time_run_generator") };
	inline static const auto time_total{ std::string("time_total") };
	inline static const auto unit{ std::string("unit") };
	inline static const auto milliseconds{ std::string("milliseconds") };
	inline static const auto time_create_pto_matrix{ std::string("time_create_pto_matrix") };
	inline static const auto time_copy_pto_matrix{ std::string("time_copy_pto_matrix") };
	inline static const auto time_subtract_unity_matrix{ std::string("time_subtract_unity_matrix") };
	inline static const auto time_calc_image_vector{ std::string("time_calc_image_vector") };
	inline static const auto time_solve_linear_system{ std::string("time_solve_linear_system") };
	inline static const auto time_write_decoration_node{ std::string("time_write_decoration_node") };
	inline static const auto time_calc_interim_reward{ std::string("time_calc_interim_reward") };
	inline static const auto _expect{ std::string("_expect") };
	inline static const auto _variance{ std::string("_variance") };
	inline static const auto _covariance{ std::string("_covariance") };


};
