#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include "share.h"

int main(int argc, char *argv[]) {

	int hflag = 0;
	int c;

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

	/* code */
	return 0;
}