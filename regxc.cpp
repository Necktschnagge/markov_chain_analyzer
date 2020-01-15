#include "regxc.h"


const std::string regex_strings::nonnegative_integer{ std::string(R"(([1-9][0-9]*|0))") };
const std::string regex_strings::nonnegative_float{ std::string(R"(([0-9]*\.)?[0-9]+)") };
					
const std::string regex_strings::spaces0{ std::string(R"(\s*)") };
const std::string regex_strings::spaces1{ std::string(R"(\s+)") };
const std::string regex_strings::new_line{ std::string(R"((\r\n|\r|\n))") };
				
const std::string regex_strings::prism_file_header{ spaces0 + nonnegative_integer + spaces1 + nonnegative_integer + spaces0 + new_line };
const std::string regex_strings::prism_file_value_line{ spaces0 + nonnegative_integer + spaces1 + nonnegative_integer + spaces1 + nonnegative_float + spaces0 };
const std::string regex_strings::prism_file_format{ prism_file_header + "((\\s*|" + prism_file_value_line + ")(" + new_line + "|$))*" };
					
const std::string regex_strings::gmc_column_name{ std::string(R"(\$\w+)") };
const std::string regex_strings::native_ignored{ std::string(R"(\#.*)") };
const std::string regex_strings::gmc_semantics_definition{ std::string(R"(\$\w+(,\$\w+)*)") }; // use gmc column name here! 	// use with ECMAScript mode -> not anymore, removed ^ and $


const std::string regex_strings::gmc_general{ std::string("^(") +native_ignored + new_line + ")*" + gmc_semantics_definition + "(" + new_line + ".*)*$" };


const boost::regex regxc::nonnegative_integer{ boost::regex(regex_strings::nonnegative_integer) };
const boost::regex regxc::prism_file_format{ boost::regex(regex_strings::prism_file_format) };
const boost::regex regxc::prism_header{ boost::regex(regex_strings::prism_file_header) };
const boost::regex regxc::prism_value_line{ boost::regex(regex_strings::prism_file_value_line) };
const boost::regex regxc::nn_float{ boost::regex(regex_strings::nonnegative_float) };
const boost::regex regxc::gmc_general{ boost::regex(regex_strings::gmc_general) };
const boost::regex regxc::gmc_semantics_definition{ boost::regex(regex_strings::gmc_semantics_definition) };
const boost::regex regxc::gmc_column_name{ boost::regex(regex_strings::gmc_column_name) };