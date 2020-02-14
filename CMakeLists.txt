cmake_minimum_required(VERSION 3.10)
project(MC_Analyzer VERSION 1.0)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS False)


set(Boost_USE_STATIC_LIBS ON)

add_executable(MC_Analyzer
	main.cpp
	
	#headers -> so that IDEs like Visual Studio will find them
	cli.h
	commands.h
	global_data.h
	herman.h
	intset.h
	loghelper.h
	markov_chain.h
	mc_analyzer.h
	mc_calc.h
	regxc.h
	sparse_matrix.h
	string_constants.h
	)

#FIND_PACKAGE(amgcl)
FIND_PACKAGE( Boost 1.54.0 COMPONENTS regex REQUIRED )
INCLUDE_DIRECTORIES(SYSTEM ${Boost_INCLUDE_DIR} )
include_directories(../amgcl)
TARGET_LINK_LIBRARIES(MC_Analyzer LINK_PUBLIC ${Boost_LIBRARIES} )

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT MC_Analyzer)#Set Visualo Studio start-up project, so that one can directly run the debugger.

#add_executable(MC_Analyzer )

#target_include_directories(MC_Analyzer PUBLIC "${PROJECT_BINARY_DIR}")
#configure_file(ProjectConfig.h.in ProjectConfig.h)
#target_link_libraries(MC_Analyzer Boost::regex)