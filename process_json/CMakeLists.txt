cmake_minimum_required(VERSION 3.10)
project(Process_json VERSION 1.0)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS False)


add_executable(Process_json
	main.cpp
	
	)


include_directories(SYSTEM ../extern/json/include)
