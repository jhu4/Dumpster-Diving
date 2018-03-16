#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <unistd.h>

void print_help();
int get_ext(std::string filename);

int main(int argc, char *argv[]) {
	// int fflag = 0, hflag = 0, rflag = 0;
	// int c;
	// std::vector<std::string> files;
	// char* dumpster_path;

	// //get the DUMPSTER enviornment path
	// dumpster_path = getenv("DUMPSTER");

	// while ((c = getopt(argc, argv, "fhr")) != -1) {
	// 	switch (c) {
	// 		case 'f':
	// 			fflag++;
	// 			break;
	// 		case 'h':
	// 			hflag++;
	// 			break;
	// 		case 'r':
	// 			rflag++;
	// 			break;
	// 		case '?':
	// 		default:
	// 			break;
	// 	}
	// }


	// if (hflag) {
	// 	print_help();
	// }
  
 //  //parse filenames into a vector
 //  for (; optind < argc; optind++) {
 //  	files.push_back(std::string(argv[optind]));
 //  }
  
 //  for (auto file : files) {
 //  	rename(file.c_str(), 
 //  		std::string(dumpster_path).append("/").append(file).c_str());
 //  }

	std::cout << get_ext(argv[1]) << std::endl;
	return 0;
}

//get the next extension of a file in the dumpster
int get_ext(std::string filename) {
	filename.insert(0, getenv("DUMPSTER"));

	//file exists
	if (access(filename.c_str(), F_OK) == 0) {
		filename.append(".1");

		char* file = (char*) filename.c_str();
		int last_index = filename.length() - 1;

		while (access(file, F_OK) == 0) {
			file[last_index] += 1;
		}
		return file[last_index] - '0';
	}
	else {
		return 0;
	}
}

void print_help() {
	std::cout <<
		"-h      ask for help manual\n"
		"-f      ignore nonexistent files and arguments, never prompt\n"
		"-r      remove directories and their contents recursively\n";
	exit(1);
}