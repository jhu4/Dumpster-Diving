#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <unistd.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/errno.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>

void print_help();
void print_stat(std::string path);
int get_ext(std::string filename);
void to_dumpster(std::string file, std::string dumpster_path);
void to_dumpster_recursively(std::string full_path
																	, std::string relative_parent_path
																	, std::string dumpster_path);

int main(int argc, char *argv[]) {
	int fflag = 0, hflag = 0, rflag = 0;
	int c;
	std::vector<std::string> files;
	char* dumpster_path;
	char* current_path;

	//get the DUMPSTER enviornment path
	dumpster_path = getenv("DUMPSTER");
	getcwd(current_path, 0);
	
	if (dumpster_path == NULL) {
		std::cerr << "DUMPSTER: " <<  strerror(errno) << std::endl;
		return 1;
	}

	if (access(dumpster_path, F_OK | W_OK | R_OK | X_OK) == -1) {
		std::cerr << "DUMPSTER: " <<  strerror(errno) << std::endl;
		return 1;
	} 

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
				perror("Do not have this option");
				break;
		}
	}

	if (hflag) {
		print_help();
	}
  
  //parse filenames into a vector
  for (; optind < argc; optind++) {
  	std::string temp = std::string(argv[optind]);

  	//if the path is not an absolute path
  	// turn it into an absolute path
  	if(temp.front() != '/') {
  		temp.insert(0, current_path);
  	}

  	std::cout << temp << std::endl;
  	files.push_back(temp);
  }
  
  for (auto file : files) {
  	to_dumpster(file, std::string(dumpster_path).append("/"));
  }

	return 0;
}


void to_dumpster(std::string file, std::string dumpster_path) {
	struct stat metadata;

	if (stat(file.c_str(), &metadata) == -1) {
		std::cerr << file << ": " <<  strerror(errno) << std::endl;
		return;
	}

	char* file_basename = basename((char*)file.c_str());
	print_stat(file);

	int i = rename(file.c_str(), std::string(dumpster_path)
  															.append("/")
  															.append(file_basename)
  															.c_str());

	if (i == -1) {
		std::cerr << file << ": " <<  strerror(errno) << std::endl;
		to_dumpster_recursively(file, std::string(""), dumpster_path);
	}
}


void to_dumpster_recursively(std::string full_path
																	, std::string relative_parent_path
																	, std::string dumpster_path) {
	char* base_name = basename((char*)full_path.c_str());
	struct stat metadata;
	struct dirent* dir_metadata;
	DIR* dir_ptr;
	std::string next_relative_parent_path = std::string(relative_parent_path)
																					.append(base_name)
																					.append("/");


	if (stat(full_path.c_str(), &metadata) == -1) {
		std::cerr << full_path << ": " <<  strerror(errno) << std::endl;
		return;
	}

	//if is not a directory
	if(!S_ISDIR(metadata.st_mode)) {
		std::cerr << full_path << " is not a directory" << std::endl;
		return;
	}


	if ((dir_ptr = opendir(full_path.c_str())) == NULL) {
		std::cerr << full_path << ": " <<  strerror(errno) << std::endl;
		return;
	}

	//make a directory in the dumpster
	std::string relative_path = std::string(dumpster_path)
																				.append(relative_parent_path)
																				.append(base_name);
	if (mkdir(relative_path.c_str(), umask(0)) == -1) {
		std::cerr << "In mkdir\t" << relative_path << ": " <<  strerror(errno) << std::endl;
		return;
	}
	while ((dir_metadata = readdir(dir_ptr)) != NULL) {
		std::string next_full_path = std::string(full_path)
																	.append("/")
																	.append(dir_metadata->d_name);
		
		to_dumpster_recursively(next_full_path
														, next_relative_parent_path, dumpster_path);
	}
	
	closedir(dir_ptr);	
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

void print_stat(std::string path) {
	struct stat tmp;

	int i;
	if ((i = stat(path.c_str(), &tmp)) == -1) {
		std::cout << "stat() error code on " << path.c_str() << std::endl;
		return;
	}

	std::cout << tmp.st_dev << std::endl;
	std::cout << tmp.st_ino << std::endl;
	std::cout << tmp.st_mode << std::endl;
	std::cout << tmp.st_nlink << std::endl;
	std::cout << tmp.st_uid << std::endl;
	std::cout << tmp.st_gid << std::endl;
	std::cout << tmp.st_rdev << std::endl;
	std::cout << tmp.st_size << std::endl;
	std::cout << tmp.st_blksize << std::endl;
	std::cout << tmp.st_blocks << std::endl;
	std::cout << tmp.st_atime << std::endl;
	std::cout << tmp.st_mtime << std::endl;
	std::cout << tmp.st_ctime << std::endl;

}

void print_help() {
	std::cout <<
		"-h      display a basic help and usage message\n"
		"-f      force a complete remove, do not move to dumpster\n"
		"-r      remove directories and their contents recursively\n";
	exit(1);
}