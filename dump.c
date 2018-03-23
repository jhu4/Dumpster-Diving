#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include <linux/errno.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <limits.h>

#include "share.h"


void print_dump_help();

int main(int argc, char *argv[]) {
	int hflag = 0;
	int c;
	char* dumpster_path;
	struct stat metadata;
	char real_dumpster_path[PATH_MAX];

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
		print_dump_help();
		return 0;
	}

	//get the DUMPSTER enviornment path
	dumpster_path = getenv("DUMPSTER");
	if (dumpster_path == NULL) {
		std::cerr << "DUMPSTER: " <<  strerror(errno) << std::endl;
		return -1;
	}

	if (!realpath(dumpster_path, real_dumpster_path)) {
		std::cerr << "realpath: " <<  dumpster_path << strerror(errno) << std::endl;
		return -1;
	}

	if (stat(real_dumpster_path, &metadata) == -1) {
		std::cerr << "DUMPSTER: " <<  strerror(errno) << std::endl;
		return -1;
	}

	force_remove(real_dumpster_path);

	if (mkdir(real_dumpster_path, umask(0)) == -1) {
		std::cerr << "In mkdir\t" << real_dumpster_path << ": " <<  strerror(errno) << std::endl;
		return -1;
	}

	perserve_metadata(std::string(real_dumpster_path), &metadata);
	return 0;
}


void print_dump_help(){
	std::cout << "-h : display basic usage message\n"
		"This method removes all files in the dumpster\n";
}