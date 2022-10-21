
#include <iostream>
#include <fstream>
#include <vector>
struct Arguments {
	std::ifstream in;
	std::string pattern{""};
	uint32_t n{0};
	std::size_t pattern_length{0};
	std::vector<uint64_t> rows_columns; // for how many rows do I need to remember number of columns for each row, if the pattern is multi-row
	unsigned int pattern_new_lines{0};
	size_t chars_before_first_new_line{ 0 };
};
int calc_pattern_hash(Arguments &arguments) {
	int h = 0;
	for (size_t i = 0; i < arguments.pattern_length; i++) {
		h += int(arguments.pattern[i]);
	}
	return h;
}
struct Position {
	uint64_t column{ 0 };
	uint64_t row { 0 };
	uint64_t distance{0}; // distance from the file beginning
};
Position calculate_beginning_position(Position current, int overall_distance,Arguments &arguments,Position &beginning) {
	if (arguments.pattern_new_lines != 0) { //multi line pattern
		beginning.column = arguments.rows_columns[0] - arguments.chars_before_first_new_line;//-1 because we count new line as it still being in the columns
		beginning.row = current.row - arguments.pattern_new_lines;
	}
	else { //one line pattern
		beginning.column = current.column - arguments.pattern_length+1;
		beginning.row = current.row;
	}
	beginning.distance = overall_distance - static_cast<uint64_t>(arguments.pattern_length) + 1;
	return beginning;
}

int check_patterns(std::vector<char> buffer, Arguments &arguments) {
	for (size_t i = 0; i < buffer.size(); i++) {
		if (buffer[i] != arguments.pattern[i]) {
			return 0; //they match
		}
	}
	return 1;
}
int char_lookup(Arguments &arguments) {
	std::vector<char> buffer;
	char c{' '};
	int hash{0};
	Position current_position;
	Position last_position;
	Position beginning_position;
	int last_left{-1}; //if last position had left neighbour in the distance of N, 0 if false 1 if True
	std::vector<char> hold_buffer; // if we find pattern but there is no pattern in the distance n to the left - we use this to check the right side
	int pattern_hash = calc_pattern_hash(arguments);
	uint32_t overall_distance{ 0 };
	while(true) {
		arguments.in.get(c); //read character
		if (arguments.in.gcount() == 0 || int(c) > 127 || int(c)<0) { //check if we were able to read something or if we read valid char 
			return 1;
		}
		buffer.insert(buffer.end(), c);
		hash += int(c);
		if (buffer.size() > arguments.pattern_length) {
			hash -= int(buffer[0]); //recalculate hash
			buffer.erase(buffer.begin()); //delete the first element
		}
		if (hash == pattern_hash) {
			calculate_beginning_position(current_position, overall_distance, arguments, beginning_position);
			//check if patterns match
			if (check_patterns(buffer, arguments) == 1) {
				if (last_left == -1) { //this is the first pattern found so there can't be neighbour from the left
					last_left = 0;
				}
				else { // we can look to the left
					if (beginning_position.distance - last_position.distance <= arguments.n) { // check the distance between current and last to the left
						if (last_left == 0) { //if the last pattern didn't have a neighbour to the left, we now found out that the last pattern has a neighbour to the right in the distance of N
							std::cout << last_position.row << ' ' << last_position.column << '\n';
						}
						std::cout << beginning_position.row << ' ' << beginning_position.column << '\n'; //write it out
						last_left = 1;
					}
					else { //check the right side
						last_left = 0;
					}
				}
				last_position = beginning_position; //last position of the pattern to the left
				//last_position.row = beginning_position.row;
				//last_position.column = beginning_position.column;
				//last_position.distance = beginning_position.distance;
			}
		}
		if (arguments.in.eof()) { //check if we are not at the end of the file, maybe move this to the end of the loop
			return 0;
		}
		overall_distance++;
		//moving 
		if (c == '\n') {
			//push the number of columns of this row 
			arguments.rows_columns.insert(arguments.rows_columns.begin(), current_position.column+1); 
			//if we exceed the length, remove the first element
			if (arguments.rows_columns.size() > arguments.pattern_new_lines) {
				arguments.rows_columns.erase(arguments.rows_columns.begin());
			}
			current_position.row += 1;
			current_position.column = 0;
		}
		else {
			current_position.column += 1;
		}
	}
}



int main(int argc, char *argv[])
{
	if (argc == 1) {
		return 1; //error
	}
	if (argc > 4) { //more arguments than needed
		return 1;
	}
	std::string input_path = std::string(argv[1]);
	Arguments argum;
	argum.in.open(input_path);
	if (!argum.in.is_open()) {
		return 1;
	}
	//argum.pattern = std::string(argv[2]);
	argum.pattern = "a\na";
	if(argum.pattern.empty()) {
		return 1;
	}
	argum.pattern_length = argum.pattern.length();
	int new_lines{ 0 };
	for (size_t i = 0; i < argum.pattern_length; i++) {
		if (argum.pattern[i] == '\n') {
			if (new_lines == 0) {
				argum.chars_before_first_new_line = i;
			}
			new_lines += 1;
		}
	}
	argum.pattern_new_lines = new_lines;

	argum.n = std::atoi(argv[3]);
	if (argum.n == 0) {
		return 1;
	}
	return char_lookup(argum);
}
