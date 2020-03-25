

#include "../commands.h"
#include "../string_constants.h"

#include <nlohmann/json.hpp>


#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <iostream>


int main() {

	nlohmann::json j;
	nlohmann::json zipped_leader_sync;
	nlohmann::json zipped_herman;

	constexpr char ae{ '\x84' };
	constexpr char oe{ '\x94' };

	const auto file_path_leader_sync{ [](std::size_t i) { return std::string("../measures/leader_sync/log") + std::to_string(i) + ".json"; } };
	const auto file_path_herman{ [](std::size_t i) { return std::string("../measures/herman/log") + std::to_string(i) + ".json"; } };

	/** 
		@param j should be a JSON list of objects.
		@param string should be a string to search for
		@return iterator to the first object in the list with an attribute {string} or j.cend
	*/
	const auto search_item_containing{ [](const nlohmann::json& j, auto string) -> nlohmann::json::const_iterator {
		for (auto it = j.cbegin(); it != j.cend(); ++it) {
			if (it->contains(string)) return it;
		}
		return j.cend();
	} };

	// zip values of interest:
	for (std::size_t i{ 0 }; i <= 22; ++i) {
		auto json_file{ std::ifstream(file_path_leader_sync(i)) };
		json_file >> j;
		auto it_print_mc = search_item_containing(j, cli_commands::PRINT_MC);
		auto size_node = it_print_mc->operator[](cli_commands::PRINT_MC)[sc::size_nodes];
		auto size_edges = it_print_mc->operator[](cli_commands::PRINT_MC)[sc::size_edges];
		auto it_calc_variance = search_item_containing(j, cli_commands::CALC_VARIANCE);
		auto time_total_calculation = it_calc_variance->operator[](cli_commands::CALC_VARIANCE)[sc::time_total];
		auto time_les_calculation = it_calc_variance->operator[](cli_commands::CALC_VARIANCE)[sc::time_solve_linear_system];
		const auto separator{ ": " };
		/*
		std::cout << sc::size_nodes << separator << size_node;
		std::cout << sc::size_edges << separator << size_edges;
		std::cout << sc::time_total << separator << time_total_calculation;
		std::cout << sc::time_solve_linear_system << separator << time_les_calculation;
		*/
		zipped_leader_sync.push_back({
			{sc::size_nodes, size_node},
			{sc::size_edges, size_edges},
			{sc::time_total, time_total_calculation},
			{sc::time_solve_linear_system, time_les_calculation}
			});
	}

	for (std::size_t i{ 3 }; i < 16; i += 2) {
		auto json_file{ std::ifstream(file_path_herman(i)) };
		json_file >> j;
		auto it_print_mc = search_item_containing(j, cli_commands::PRINT_MC);
		auto size_node = it_print_mc->operator[](cli_commands::PRINT_MC)[sc::size_nodes];
		auto size_edges = it_print_mc->operator[](cli_commands::PRINT_MC)[sc::size_edges];
		auto it_calc_variance = search_item_containing(j, cli_commands::CALC_VARIANCE);
		auto time_total_calculation = it_calc_variance->operator[](cli_commands::CALC_VARIANCE)[sc::time_total];
		auto time_les_calculation = it_calc_variance->operator[](cli_commands::CALC_VARIANCE)[sc::time_solve_linear_system];
		auto it_generate_herman = search_item_containing(j, cli_commands::GENERATE_HERMAN);
		auto herman_size = it_generate_herman->operator[](cli_commands::GENERATE_HERMAN)[sc::size];
		const auto separator{ ": " };
		/*
		std::cout << sc::size_nodes << separator << size_node;
		std::cout << sc::size_edges << separator << size_edges;
		std::cout << sc::time_total << separator << time_total_calculation;
		std::cout << sc::time_solve_linear_system << separator << time_les_calculation;
		*/
		zipped_herman.push_back({
			{sc::size, herman_size},
			{sc::size_nodes, size_node},
			{sc::size_edges, size_edges},
			{sc::time_total, time_total_calculation},
			{sc::time_solve_linear_system, time_les_calculation}
			});
	}


	// print output as required for diagrams:
	//=========================================================================
	//=========================================================================
	std::stringstream edges_of_nodes;
	edges_of_nodes << "\n% Kantenzahl in Abh" << ae << "ngigkeit der Knotenzahl.\n";
	edges_of_nodes << R"(
\begin{figure}
\caption{Kantenzahl in Abh)" << ae << R"(ngigkeit der Knotenzahl}
\label{fig-leader-sync}
\centering
\begin{tikzpicture}
\begin{axis}[
xlabel={Knotenzahl},
ylabel={Kantenzahl},
xmin=10,
xmax=1000000,
xmode=log,
ymin= 10,
ymax=60000000,
ymode=log,
legend pos=north west,
ymajorgrids=true,
grid style=dashed,
]
\path[name path=axis] (axis cs:0,0) -- (axis cs:1,0);

\addplot[only marks,color=blue,mark=square,name path=f]
coordinates {
)";
	for (const auto& item : zipped_leader_sync) {
		edges_of_nodes << "(" << item[sc::size_nodes] << "," << item[sc::size_edges] << ")";
	}
	edges_of_nodes << R"(
};

\addplot[only marks,color=red!50,mark=triangle,]
coordinates {
)";
	for (const auto& item : zipped_herman) {
		edges_of_nodes << "(" << item[sc::size_nodes] << "," << item[sc::size_edges] << ")";
	}
	edges_of_nodes << R"(
};

\addlegendentry{leader\_sync}
\addlegendentry{herman}

\end{axis}
\end{tikzpicture}
\end{figure}
)";
	//=========================================================================
	std::stringstream herman;
	herman << "\n% herman: stuff @ size\n";
	herman << R"(
\begin{figure}
\caption{hermanN}
\label{fig-herman}
\centering
\begin{tikzpicture}
\begin{axis}[
xlabel={N},
ylabel={},
xmin=1,
xmax=18,
ymode=log,
ymin= 0.1,
ymax=10000000000,
xtick={3,5,7,9,11,13,15,17},
legend pos=north west,
ymajorgrids=true,
grid style=dashed,
]
\path[name path=axis] (axis cs:0,0) -- (axis cs:1,0);

\addplot[color=blue,mark=square,name path=f]
coordinates {
)";
	for (const auto& item : zipped_herman) {
		herman << "(" << item[sc::size] << "," << item[sc::time_solve_linear_system] << ")";
	}
	herman << R"(
};

\addplot[color=red!50,mark=square,]
coordinates {
)";
	for (const auto& item : zipped_herman) {
		herman << "(" << item[sc::size] << "," << item[sc::size_edges] << ")";
	}
	herman << R"(
};

\addplot[color=green!50,mark=square,]
coordinates {
)";
	for (const auto& item : zipped_herman) {
		herman << "(" << item[sc::size] << "," << item[sc::size_nodes] << ")";
	}
	herman << R"(
};

\addlegendentry{linear system solving time (ms)}
\addlegendentry{number of edges}
\addlegendentry{number of nodes}

\end{axis}
\end{tikzpicture}
\end{figure}
)";


	//=========================================================================
	std::stringstream les_time_in_edges;
	les_time_in_edges << "\n% time_solve_linear_system @ size_edges\n";
	les_time_in_edges << R"(
\begin{figure}
\caption{Zeitaufwand in Abh)" << "\x84" << R"(ngigkeit der Kantenzahl}
\label{fig-in-edges}
\centering
\begin{tikzpicture}
\begin{axis}[
xlabel={Kantenzahl},
ylabel={Zeit zum L)" << oe << R"(sen des Gleichungssystems (ms) },
xmin=10,
xmax=100000000,
xmode=log,
ymode=log,
ymin= 0.04,
ymax=1000000,
legend pos=north west,
ymajorgrids=true,
grid style=dashed,
]
\path[name path=axis] (axis cs:0,0) -- (axis cs:1,0);

\addplot[ only marks,color=blue,mark=square,name path=f]
coordinates {
)";
	for (const auto& item : zipped_leader_sync) {
		les_time_in_edges << "(" << item[sc::size_edges] << "," << item[sc::time_solve_linear_system] << ")";
	}
	les_time_in_edges << R"(
};

\addplot[only marks,color=red,mark=triangle,name path=f]
coordinates {
)";
	for (const auto& item : zipped_herman) {
		les_time_in_edges << "(" << item[sc::size_edges] << "," << item[sc::time_solve_linear_system] << ")";
	}
	les_time_in_edges << R"(
};

\addlegendentry{leader\_sync}
\addlegendentry{herman}
\end{axis}
\end{tikzpicture}
\end{figure}
)";

	//=========================================================================
	std::stringstream les_time_in_nodes;
	les_time_in_nodes << "\n% time_solve_linear_system @ size_nodes\n";
	les_time_in_nodes << R"(
\begin{figure}
\caption{Zeitaufwand in Abh)" << "\x84" << R"(ngigkeit der Knoten in der \mc{}}
\label{fig-in-nodes}
\centering
\begin{tikzpicture}
\begin{axis}[
xlabel={\#nodes},
ylabel={},
xmin=10,
xmax=1000000,
xmode=log,
ymode=log,
ymin= 0.04,
ymax=1000000,
legend pos=north west,
ymajorgrids=true,
grid style=dashed,
]
\path[name path=axis] (axis cs:0,0) -- (axis cs:1,0);

\addplot[ only marks,color=blue,mark=square,name path=f]
coordinates {
)";
	for (const auto& item : zipped_leader_sync) {
		les_time_in_nodes << "(" << item[sc::size_nodes] << "," << item[sc::time_solve_linear_system] << ")";
	}
	les_time_in_nodes << R"(
};

\addplot[ only marks,color=red,mark=square,name path=f]
coordinates {
)";
	for (const auto& item : zipped_herman) {
		les_time_in_nodes << "(" << item[sc::size_nodes] << "," << item[sc::time_solve_linear_system] << ")";
	}
	les_time_in_nodes << R"(
};

\addlegendentry{leader\_sync: time for solving linear system (ms)}
\addlegendentry{herman: time for solving linear system (ms)}
\end{axis}
\end{tikzpicture}
\end{figure}
)";

	//=========================================================================
	std::stringstream les_time_percentage;
	les_time_percentage << "\n% percentage les_time/total_time @ size_nodes\n";
	les_time_percentage << R"(
\begin{figure}
\caption{Anteil L)" << "\x94" << R"(sung des Gleichungssystem an Gesamtzeitaufwand}
\label{fig-percentage}
\centering
\begin{tikzpicture}
\begin{axis}[
xlabel={Knotenzahl},
ylabel={},
xmin=1,
xmax=300000,
xmode=log,
ymin= 40,
ymax=119,
legend pos=north west,
ymajorgrids=true,
grid style=dashed,
]
\path[name path=axis] (axis cs:0,0) -- (axis cs:1,0);

\addplot[only marks,color=blue,mark=square,name path=f]
coordinates {
)";
	for (const auto& item : zipped_leader_sync) {
		auto x = item[sc::time_solve_linear_system];
		auto y = item[sc::time_total];
		les_time_percentage << "(" << item[sc::size_nodes] << "," << 1.0 * item[sc::time_solve_linear_system].get<double>() / item[sc::time_total].get<double>() * 100.0 << ")";
	}
	les_time_percentage << R"(
};

\addplot[only marks,color=red,mark=triangle,name path=f]
coordinates {
)";
	for (const auto& item : zipped_herman) {
		auto x = item[sc::time_solve_linear_system];
		auto y = item[sc::time_total];
		les_time_percentage << "(" << item[sc::size_nodes] << "," << 1.0 * item[sc::time_solve_linear_system].get<double>() / item[sc::time_total].get<double>() * 100.0 << ")";
	}
	les_time_percentage << R"(
};

\addlegendentry{leader\_sync (in\%)}
\addlegendentry{herman (in\%)}
\end{axis}
\end{tikzpicture}
\end{figure}
)";


	//=========================================================================
	std::stringstream total_time_in_edges;
	total_time_in_edges << "\n% time_solve_linear_system @ size_edges\n";
	total_time_in_edges << R"(
\begin{figure}
\caption{Zeitaufwand in Abh)" << "\x84" << R"(ngigkeit der Kantenzahl}
\label{fig-in-edges}
\centering
\begin{tikzpicture}
\begin{axis}[
xlabel={Kantenzahl},
ylabel={Gesamtzeit zur Varianzermittlung (ms) },
xmin=10,
xmax=100000000,
xmode=log,
ymode=log,
ymin= 0.04,
ymax=1000000,
legend pos=north west,
ymajorgrids=true,
grid style=dashed,
]
\path[name path=axis] (axis cs:0,0) -- (axis cs:1,0);

\addplot[ only marks,color=blue,mark=square,name path=f]
coordinates {
)";
	for (const auto& item : zipped_leader_sync) {
		total_time_in_edges << "(" << item[sc::size_edges] << "," << item[sc::time_total] << ")";
	}
	total_time_in_edges << R"(
};

\addplot[only marks,color=red,mark=triangle,name path=f]
coordinates {
)";
	for (const auto& item : zipped_herman) {
		total_time_in_edges << "(" << item[sc::size_edges] << "," << item[sc::time_total] << ")";
	}
	total_time_in_edges << R"(
};

\addlegendentry{leader\_sync}
\addlegendentry{herman}
\end{axis}
\end{tikzpicture}
\end{figure}
)";
	

	std::vector<std::stringstream*> figures{
		&les_time_percentage
		,&edges_of_nodes
		// ,&herman,  // move into leader_sync
		,&les_time_in_edges // keep as it is
		// ,&total_time_in_edges
		//,&les_time_in_nodes // don't print this anymore.
	};
	std::cout << "\n\n\n%=====================================================";
	std::for_each(figures.cbegin(), figures.cend(), [](const auto& pss) { std::cout << pss->str(); });

	return 0;
}



