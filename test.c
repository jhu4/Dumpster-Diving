#include <stdio.h>
#include <stdlib.h>
#include <iostream>
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
#include <sys/time.h>

#include "share.h"


int main(int argc, char *argv[]) {
	char buf[PATH_MAX];
	std::string src;
	std::string dest = std::string(argv[2]);
	struct timeval start;
	struct timeval end;

	if (realpath(argv[1], buf)) {
	  src = std::string(buf);
  }
  	else {
		std::cerr << "realpath: " <<  argv[1] << strerror(errno) << std::endl;
		return -1;
	}
	gettimeofday(&start, NULL);
	send_file_to(src, dest);	
	gettimeofday(&end, NULL);


	std::cout << ((end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec)) / 1000 << std::endl;
	return 0;
}