#pragma once


#include <string>
#include <boost/regex.hpp>

class regex_strings {
public:
	const static std::string nonnegative_integer;
	const static std::string nonnegative_float;
	
	const static std::string spaces0;
	const static std::string spaces1;
	const static std::string new_line;
	
	const static std::string prism_file_header;
	const static std::string prism_file_value_line;
	const static std::string prism_file_format;
	
	const static std::string gmc_column_name;
	const static std::string native_ignored;
	const static std::string gmc_semantics_definition; // use gmc column name here! 	// use with ECMAScript mode -> not anymore, removed ^ and $
	
	
	const static std::string gmc_general;
	

};

class regxc {
public:
	const static boost::regex nonnegative_integer;
	const static boost::regex prism_file_format;
	const static boost::regex prism_header;
	const static boost::regex prism_value_line;
	const static boost::regex nn_float;
	
	const static boost::regex gmc_general;
	const static boost::regex gmc_semantics_definition;
	const static boost::regex gmc_column_name;
};
