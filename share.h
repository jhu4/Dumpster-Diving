#ifndef SHARE_H_INCLUDED
#define SHARE_H_INCLUDED

void print_stat(std::string path);
int get_ext(std::string filename);
void to_dumpster(std::string file, std::string dumpster_path);
void to_dumpster_recursively(std::string full_path
																	, std::string relative_parent_path
																	, std::string dumpster_path);
void copy_file(std::string from, std::string to);
void perserve_metadata(std::string path, struct stat* metadata);
void force_remove(std::string file);

#endif
