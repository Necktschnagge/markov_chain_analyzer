cmake_minimum_required(VERSION 3.10)
project(MC_Analyzer VERSION 1.0)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS False)


set(Boost_USE_STATIC_LIBS ON)

#FIND_PACKAGE(amgcl)
FIND_PACKAGE( Boost 1.54.0 COMPONENTS regex REQUIRED )
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )
include_directories(../amgcl)


add_executable(MC_Analyzer
	main.cpp
	
	regxc.h
	#regxc.cpp
	
	loghelper.h
	markov_chain.h
	)
#add_executable(MC_Analyzer )

target_include_directories(MC_Analyzer PUBLIC "${PROJECT_BINARY_DIR}")
#configure_file(ProjectConfig.h.in ProjectConfig.h)


TARGET_LINK_LIBRARIES(MC_Analyzer LINK_PUBLIC ${Boost_LIBRARIES} )
#target_link_libraries(MC_Analyzer Boost::regex)