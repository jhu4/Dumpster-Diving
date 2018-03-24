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

int main(int argc, char *argv[]) {
	int mflag = 0, hflag = 0;
	int c;
	char buf[PATH_MAX];
	std::string src;
	std::string dest = std::string(argv[2]);
	struct timeval start;
	struct timeval end;


	while ((c = getopt(argc, argv, "mh")) != -1) {
		switch (c) {
			case 'h':
				hflag++;
				break;
			case 'm':
				mflag++;
				break;
			case '?':
			default:
				std::cerr << "Do not have this option" <<std::endl;
				return -1;
		}
	}

	src = std::string(argv[optind++]);
	dest = std::string(argv[optind++]);

	gettimeofday(&start, NULL);

	if (mflag) {
		send_file_to_recursively(src, std::string(""), dest);	
		sync();
	}
	else {
		for (int i = 0; i < 100; i++) {
			if(rename(src.c_str(), dest.c_str()) == -1) {
				std::cerr << strerror(errno) << std::endl;
			}
			sync();
			if(rename(dest.c_str(), src.c_str()) == -1) {
				std::cerr << strerror(errno) << std::endl;
			}
			sync();
		}
	}
	gettimeofday(&end, NULL);

	if (mflag) {
		std::cout << ((end.tv_sec - start.tv_sec) * 1000 + (float)(end.tv_usec - start.tv_usec) / 1000) << std::endl;
	}
	else {
		std::cout << (float)((end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec)) / 200000 << std::endl;
	}
	return 0;
}