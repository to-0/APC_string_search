#include <iostream>
#include <fstream>
#include <vector>
#include <stdint.h>
#include <string>
#define N 8000
struct Arguments {
	std::ifstream in;
	std::string pattern{""};
	uint32_t n{0};
	std::size_t pattern_length{0};
	std::vector<uint64_t> rows_columns; // for how many rows do I need to remember number of columns for each row, if the pattern is multi-row
	unsigned int pattern_new_lines{0};
	size_t chars_before_first_new_line{ 0 };
};
Arguments argum;
int calc_pattern_hash() {
	int h = 0;
	for (size_t i = 0; i < argum.pattern_length; i++) {
		h += int(argum.pattern[i]);
	}
	return h;
}
struct Position {
	uint64_t column{ 0 };
	uint64_t row { 0 };
	uint64_t distance{0}; // distance from the file beginning
};
Position calculate_beginning_position(Position current, int overall_distance,Position &beginning) {
	//std::cout << "Calculate position\n";
	//std::cout << "Argum rows columns size: " << argum.rows_columns.size() << "\n";
	if (argum.pattern_new_lines != 0) { //multi line pattern
		beginning.column = argum.rows_columns[0] - argum.chars_before_first_new_line-1;//
		beginning.row = current.row - argum.pattern_new_lines; //because we calculate the position before moving row
	}
	else { //one line pattern
		beginning.column = current.column - argum.pattern_length;
		beginning.row = current.row;
	}
	beginning.distance = overall_distance - static_cast<uint64_t>(argum.pattern_length)+1;
	//std::cout << "Prezili sme\n";
	return beginning;
}

int check_patterns(std::vector<char> buffer, size_t start_index)  {
	size_t i{ 0 };
	size_t j{ start_index };
	//std::cout << "Check pattern\n";
	while (i < argum.pattern_length) {
		if (buffer[j] != argum.pattern[i]) {
			return 0;
		}
		i++;
		j = (j + 1) % argum.pattern_length;
	}
	return 1;
}
int char_lookup() {
	std::vector<char> buffer;
	char file_buffer[N];
	char c{' '};
	int hash{0};
	Position current_position;
	Position last_position;
	Position beginning_position;
	int last_left{-1}; //if last position had left neighbour in the distance of N, 0 if false 1 if Truef
	int pattern_hash = calc_pattern_hash();
	//buffer.resize(argum.pattern_length);
	size_t first_index{ 0 };
	size_t insert_index{ 0 };
	uint32_t overall_distance{ 0 };
	while (true) {
		argum.in.read(file_buffer, N);
		auto gc = argum.in.gcount();
		//std::cout << "Precital som tolkoto znakov do velkeho bufra " << gc << "\n";
		if (argum.in.gcount() > 0) {
			for (auto i = 0; i < gc;i++) {
				c = file_buffer[i];
				if (int(c) > 127 || int(c) <= 0) { //check if we were able to read something or if we read valid char 
					std::cerr << "Incorrect input";
					return 1;
				}
				//moving 
				if (c == '\n') {
					//push the number of columns of this row #TOTO POZRIET CI DOBRE ROBIM
					argum.rows_columns.insert(argum.rows_columns.end(), current_position.column + 1);
					//std::cout << "Pridal som do argum rows columns teraz " << argum.rows_columns.size() << "\n";
					//if we exceed the length, remove the first element
					if (argum.rows_columns.size() > argum.pattern_new_lines) {
						//std::cout << "Mazem argum.rows.columns o velkosti " << argum.rows_columns.size() << "\n";
						argum.rows_columns.erase(argum.rows_columns.begin());
					}
					current_position.row += 1;
					current_position.column = 0;
				}
				else {
					current_position.column += 1;
				}
				//buffer[insert_index] = c;
				//std::cout << "First " << first_index << "Inserted " << insert_index << '\n';
				hash += int(c);
				//std::cout << "Current position :" << current_position.row << " " << current_position.column << '\n';
				//std::cout << "Buffer size " << buffer.size() << '\n';
				if (buffer.size() >= argum.pattern_length) {
					hash -= int(buffer[first_index]); //recalculate hash
					buffer[insert_index] = c;
				}
				else {
					buffer.insert(buffer.end(), c);
				}
				//std::cout << "Tu som\n";
				first_index = (first_index + 1) % argum.pattern_length;
				if (hash == pattern_hash && buffer.size() == argum.pattern_length && argum.rows_columns.size() == argum.pattern_new_lines) {
					//std::cout << "\nHash match\n";
					calculate_beginning_position(current_position, overall_distance, beginning_position);
					//check if patterns match
					if (check_patterns(buffer, first_index) == 1) {
						if (last_left == -1) { //this is the first pattern found so there can't be neighbour from the left
							last_left = 0;
						}
						else { // we can look to the left
							if (beginning_position.distance - last_position.distance <= argum.n) { // check the distance between current and last to the left
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
					}
				}
				overall_distance++;
				

				insert_index = (insert_index + 1) % argum.pattern_length;
			}
		}
		if (argum.in.eof()) {
			//std::cout << "EOF\n";
			return 0;
		}
	}
	return 0;
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

	argum.in.open(input_path);
	if (!argum.in.is_open()) {
		return 1;
	}
	argum.pattern = std::string(argv[2]);
	//argum.pattern = "\na";
	if(argum.pattern.empty()) {
		return 1;
	}
	argum.pattern_length = argum.pattern.length();
	if (argum.pattern_length >= 256) {
		return 1;
	}
	int new_lines{ 0 };
	argum.chars_before_first_new_line = 0;
	for (size_t i = 0; i < argum.pattern_length; i++) {
		if (argum.pattern[i] == '\n') {
			if (new_lines == 0) {
				argum.chars_before_first_new_line = i;
			}
			new_lines += 1;
		}
	}
	argum.pattern_new_lines = new_lines;
	long long a{0};
	try {
		a = std::stoll(std::string(argv[3]));
	}
	catch (std::exception& e) {
		return 1;
	}
	
	if (a < 0 || a > UINT32_MAX) {
		return 1;
	}
	argum.n = static_cast<uint32_t>(a);
	if (argum.n == 0) {
		return 1;
	}
	return char_lookup();
}
