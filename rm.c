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


void print_help();
void print_stat(std::string path);
int get_ext(std::string filename);
void to_dumpster(std::string file, std::string dumpster_path);
void to_dumpster_recursively(std::string full_path
																	, std::string relative_parent_path
																	, std::string dumpster_path);
void copy_file(std::string from, std::string to);
void perserve_metadata(std::string path, struct stat* metadata);
void force_remove(std::string file);

int main(int argc, char *argv[]) {
	int fflag = 0, hflag = 0, rflag = 0;
	int c;
	std::vector<std::string> files;
	char* dumpster_path;
	char current_path[PATH_MAX];
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
				perror("Do not have this option");
				break;
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

  	//if the path is not an absolute path
  	// turn it into an absolute path
  	if(temp.front() != '/') {
  		temp.insert(0, "/");
  		temp.insert(0, current_path);
  	}

  	if (realpath(temp.c_str(), buf)) {
	  	std::cout << buf << std::endl;
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

		if (!rflag && S_ISDIR(metadata.st_mode)) {
			std::cerr << "rm: cannot remove, " << file << " is a directory"
								<<std::endl;	
		}
		else if (fflag) {
  		force_remove(file);
  	}
  	else {
  		to_dumpster(file, std::string(dumpster_path).append("/"));	
  	}
  }
  

	return 0;
}


void force_remove(std::string file) {
	struct stat metadata;
	struct dirent* dir_metadata;
	DIR* dir_ptr;
	std::string dot = std::string(".");
	std::string dotdot = std::string("..");

	if (stat(file.c_str(), &metadata) == -1) {
		std::cerr << file << ": " <<  strerror(errno) << std::endl;
		return;
	}

		//if is not a directory, which means it is a file
	if (!S_ISDIR(metadata.st_mode)) {
		unlink(file.c_str());
		return;
	}


	if ((dir_ptr = opendir(file.c_str())) == NULL) {
		std::cerr << file << ": " <<  strerror(errno) << std::endl;
		return;
	}

	while (dir_metadata = readdir(dir_ptr)) {
		std::string next_file = std::string(file)
																	.append("/")
																	.append(dir_metadata->d_name);
		if (dot.compare(dir_metadata->d_name) && dotdot.compare(dir_metadata->d_name)) {
			force_remove(next_file);
		}
	}

	rmdir(file.c_str());
	return;
}

void to_dumpster(std::string file, std::string dumpster_path) {
	struct stat metadata;
	int ext;

	char* file_basename = basename((char*)file.c_str());
	std::string dest = std::string(dumpster_path)
  															.append("/")
  															.append(file_basename)
  															.c_str();

	if (stat(file.c_str(), &metadata) == -1) {
		std::cerr << file << ": " <<  strerror(errno) << std::endl;
		return;
	}

	if ((ext = get_ext(dest)) == -1) {
		std::cerr << "capacity of " << file_basename << " reaches maximum" << std::endl;
		return;
	}
	else if (ext > 0) {
		dest.append(".").append(std::to_string(ext));
	}

	int i = rename(file.c_str(), dest.c_str());

	if (i == -1) {
		// std::cerr << file << ": " <<  strerror(errno) << std::endl;
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
	std::string dest = std::string(dumpster_path)
																			.append(relative_parent_path)
																			.append(base_name);

	std::string dot = std::string(".");
	std::string dotdot = std::string("..");
	int ext;

	if (stat(full_path.c_str(), &metadata) == -1) {
		std::cerr << full_path << ": " <<  strerror(errno) << std::endl;
		return;
	}

	if ((ext = get_ext(dest)) == -1) {
		std::cerr << "capacity of " << base_name << " reaches maximum" << std::endl;
		return;
	}
	else if (ext > 0) {
		dest.append(".").append(std::to_string(ext));
	}

	//if is not a directory
	if (!S_ISDIR(metadata.st_mode)) {
		copy_file(full_path, dest);
		perserve_metadata(dest, &metadata);
		unlink(full_path.c_str());
		return;
	}


	if ((dir_ptr = opendir(full_path.c_str())) == NULL) {
		std::cerr << full_path << ": " <<  strerror(errno) << std::endl;
		return;
	}

	//make a directory in the dumpster
	if (mkdir(dest.c_str(), umask(0)) == -1) {
		std::cerr << "In mkdir\t" << dest << ": " <<  strerror(errno) << std::endl;
		return;
	}
	
	//change the mode to open to every one to prevent permission deny
	if (chmod(dest.c_str(), S_IRWXU | S_IRWXG | S_IRWXO) == -1) {
		std::cerr << "In chmod\t" << dest << ": " <<  strerror(errno) << std::endl;
		return;
	}

	std::string next_relative_parent_path = std::string(dest)
																								.erase(0, dumpster_path.length())
																								.append("/");

	while (dir_metadata = readdir(dir_ptr)) {
		std::string next_full_path = std::string(full_path)
																	.append("/")
																	.append(dir_metadata->d_name);
		if (dot.compare(dir_metadata->d_name) && dotdot.compare(dir_metadata->d_name)) {
			to_dumpster_recursively(next_full_path
														, next_relative_parent_path, dumpster_path);
		}
	}
	
	perserve_metadata(dest, &metadata);
	closedir(dir_ptr);
	rmdir(full_path.c_str());
}



void perserve_metadata(std::string path, struct stat* metadata) {
	if (chmod(path.c_str(), metadata->st_mode) == -1) {
		std::cerr << "In chmod\t" << path << ": " <<  strerror(errno) << std::endl;
		return;
	}

	struct timeval perserve_time[2];
	perserve_time[0].tv_sec = metadata->st_atim.tv_sec;
	perserve_time[0].tv_usec = metadata->st_atim.tv_nsec / 1000;
	perserve_time[1].tv_sec = metadata->st_mtim.tv_sec;
	perserve_time[1].tv_usec = metadata->st_mtim.tv_nsec / 1000;
	
	if (utimes(path.c_str(), perserve_time) == -1) {
		std::cerr << "In utime\t" << path << ": " <<  strerror(errno) << std::endl;
		return;
	} 
}

void copy_file(std::string from, std::string to) {
	char buf[1024];
	size_t size;

  int src = open(from.c_str(), O_RDONLY, 0);
  int dest = open(to.c_str(), O_WRONLY | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO) ;

  while ((size = read(src, buf, 1024)) > 0) {
      write(dest, buf, size);
  }

  close(src);
  close(dest);
}


//add the next extension of a file in the dumpster
//return next ext number if successfully added, -1 if next extension is > 9
int get_ext(std::string filename) {
	//file exists
	if (access(filename.c_str(), F_OK) == 0) {
		filename.append("..");

		
		int last_index = filename.length() - 1;

		for (int i = 1; i < 10; i++) {
			filename[last_index] = i + '0';
			//if this file does not exist
			if (access(filename.c_str(), F_OK) == -1) {
				return i;
			}
		}
		return -1;
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
	std::cout << tmp.st_mode << "\t--permission" << std::endl;
	std::cout << tmp.st_nlink << std::endl;
	std::cout << tmp.st_uid << std::endl;
	std::cout << tmp.st_gid << std::endl;
	std::cout << tmp.st_rdev << std::endl;
	std::cout << tmp.st_size << std::endl;
	std::cout << tmp.st_blksize << std::endl;
	std::cout << tmp.st_blocks << std::endl;
	std::cout << tmp.st_atime << "\t--access time" << std::endl;
	std::cout << tmp.st_mtime << "\t--modify time" << std::endl;
	std::cout << tmp.st_ctime << std::endl;

}

void print_help() {
	std::cout <<
		"-h      display a basic help and usage message\n"
		"-f      force a complete remove, do not move to dumpster\n"
		"-r      remove directories and their contents recursively\n";
	exit(1);
}