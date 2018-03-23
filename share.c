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

void send_file_to(std::string file, std::string dest_path) {
	struct stat metadata;
	int ext;

	char* file_basename = basename((char*)file.c_str());
	std::string dest = std::string(dest_path)
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
		send_file_to_recursively(file, std::string(""), dest_path);
	}
}


void send_file_to_recursively(std::string full_path
																	, std::string relative_parent_path
																	, std::string dest_path) {
	char* base_name = basename((char*)full_path.c_str());
	struct stat metadata;
	struct dirent* dir_metadata;
	DIR* dir_ptr;
	std::string dest = std::string(dest_path)
																			.append("/")
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
																								.erase(0, dest_path.length())
																								.append("/");

	while (dir_metadata = readdir(dir_ptr)) {
		std::string next_full_path = std::string(full_path)
																	.append("/")
																	.append(dir_metadata->d_name);
		if (dot.compare(dir_metadata->d_name) && dotdot.compare(dir_metadata->d_name)) {
			send_file_to_recursively(next_full_path
														, next_relative_parent_path, dest_path);
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

