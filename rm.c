#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <unistd.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>

void print_help();
void print_stat(std::string path);
bool same_partition(char* one, char* another);
int get_ext(std::string filename);

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
		perror("DUMPSTER does not exist!");
		return 1;
	}

	if (access(dumpster_path, F_OK | W_OK | R_OK | X_OK) == -1) {
		perror("DUMPSTER does not have the right permission!");
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

  	if(temp.front() != '/') {
  		temp.insert(0, current_path);
  	}

  	std::cout << temp << std::endl;
  	files.push_back(temp);
  }
  
  for (auto file : files) {
  	char* file_basename = basename((char*)file.c_str());
  	print_stat(file);
  	rename(file.c_str(), 
  		std::string(dumpster_path).append("/").append(file_basename).c_str());
  }

	return 0;
}

bool same_partition(char* path1, char* path2) {
	struct stat* temp1;
	struct stat* temp2;

	stat(path1, temp1);
	stat(path2, temp2);

	return temp1->st_dev == temp2->st_dev; 
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
	struct stat* tmp;

	int i;
	if ((i = stat(path.c_str(), tmp)) == -1) {
		std::cout << "stat() error code" << std::endl;
		return;
	}

	std::cout << tmp->st_dev << std::endl;
	std::cout << tmp->st_ino << std::endl;
	std::cout << tmp->st_mode << std::endl;
	std::cout << tmp->st_nlink << std::endl;
	std::cout << tmp->st_uid << std::endl;
	std::cout << tmp->st_gid << std::endl;
	std::cout << tmp->st_rdev << std::endl;
	std::cout << tmp->st_size << std::endl;
	std::cout << tmp->st_blksize << std::endl;
	std::cout << tmp->st_blocks << std::endl;
	std::cout << tmp->st_atime << std::endl;
	std::cout << tmp->st_mtime << std::endl;
	std::cout << tmp->st_ctime << std::endl;

}

void print_help() {
	std::cout <<
		"-h      ask for help manual\n"
		"-f      ignore nonexistent files and arguments, never prompt\n"
		"-r      remove directories and their contents recursively\n";
	exit(1);
}