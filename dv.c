#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include <vector>
#include <limits.h>
#include <linux/errno.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>

#include "share.h"


void print_dv_help();

int main(int argc, char *argv[]) {
	int hflag = 0;
	int c;
	std::vector<std::string> files;
	char* dumpster_path;
	char current_path[PATH_MAX];
	char buf[PATH_MAX];

	while ((c = getopt(argc, argv, "h")) != -1) {
		switch (c) {
			case 'h':
				hflag++;
				break;
			case '?':
			default:
				std::cerr << "Do not have this option" <<std::endl;
				return -1;
		}
	}

	if (hflag) {
		print_dv_help();
		return 0;
	}

	//get the DUMPSTER enviornment path
	dumpster_path = getenv("DUMPSTER");
	if (dumpster_path == NULL) {
		std::cerr << "DUMPSTER: " <<  strerror(errno) << std::endl;
		return 1;
	}

	if (access(dumpster_path, F_OK | W_OK | R_OK | X_OK) == -1) {
		std::cerr << "DUMPSTER: " <<  strerror(errno) << std::endl;
		return 1;
	}

	if (getcwd(current_path, PATH_MAX) == NULL) {
		std::cerr << "getcwd:" << strerror(errno) << std::endl;
		return -1;
	}

	  //parse filenames into a vector
  for (; optind < argc; optind++) {
  	std::string temp = std::string(argv[optind]);

  	temp.insert(0, "/");
  	temp.insert(0, dumpster_path);
  	

  	if (realpath(temp.c_str(), buf)) {
	  	files.push_back(std::string(buf));
  	}
  	else {
  		std::cerr << "realpath: " <<  temp << strerror(errno) << std::endl;
  	}
  }


  for (auto file : files) {
  	struct stat metadata;
  	if (stat(file.c_str(), &metadata) == -1) {
				std::cerr << file << ": " <<  strerror(errno) << std::endl;
				return -1;
		}

  	send_file_to(file, std::string(current_path));
  }

	/* code */
	return 0;
}

void print_dv_help() {
	std::cout << "-h : display basic usage message\n"
		"file [file ...] : file(s) to be restored\n";
	return;
}