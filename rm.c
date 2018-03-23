#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <unistd.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>
 #include <sys/time.h>
#include <linux/errno.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <utime.h>
#include <limits.h>
#include "share.h"

void print_help();


int main(int argc, char *argv[]) {
	int fflag = 0, hflag = 0, rflag = 0;
	int c;
	std::vector<std::string> files;
	char* dumpster_path;
	char buf[PATH_MAX];

	while ((c = getopt(argc, argv, "fhr")) != -1) {
		switch (c) {
			case 'f':
				fflag++;
				break;
			case 'h':
				hflag++;
				break;
			case 'r':
				rflag++;
				break;
			case '?':
			default:
				std::cerr << "Do not have this option" <<std::endl;
				return -1;
		}
	}

	if (hflag) {
		print_help();
		return 0;
	}
  
	//get the DUMPSTER enviornment path
	dumpster_path = getenv("DUMPSTER");
	if (dumpster_path == NULL) {
		std::cerr << "DUMPSTER: " <<  strerror(errno) << std::endl;
		return -1;
	}

	if (access(dumpster_path, F_OK | W_OK | R_OK | X_OK) == -1) {
		std::cerr << "DUMPSTER: " <<  strerror(errno) << std::endl;
		return -1;
	}

  //parse filenames into a vector
  for (; optind < argc; optind++) {
  	std::string temp = std::string(argv[optind]);

  	if (realpath(temp.c_str(), buf)) {
	  	files.push_back(std::string(buf));
  	}
  	else {
  		std::cerr << "realpath: " <<  temp << strerror(errno) << std::endl;
  		return -1;
  	}
  }

  for (auto file : files) {
  	struct stat metadata;
  	if (stat(file.c_str(), &metadata) == -1) {
				std::cerr << file << ": " <<  strerror(errno) << std::endl;
				return -1;
		}

		if (!rflag && S_ISDIR(metadata.st_mode)) {
			std::cerr << "rm: cannot remove, " << file << " is a directory"
								<<std::endl;	
		}
		else if (fflag) {
  		force_remove(file);
  	}
  	else {
  		send_file_to(file, std::string(dumpster_path));	
  	}
  }
  

	return 0;
}

void print_help() {
	std::cout <<
		"-h      display a basic help and usage message\n"
		"-f      force a complete remove, do not move to dumpster\n"
		"-r      remove directories and their contents recursively\n"
		"file [file ...] – one or more file(s) to be removed\n";
	return;
}