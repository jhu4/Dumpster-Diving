#ifndef SHARE_H_INCLUDED
#define SHARE_H_INCLUDED

int get_ext(std::string filename);
void send_file_to(std::string file, std::string dumpster_path);
void send_file_to_recursively(std::string full_path
																	, std::string relative_parent_path
																	, std::string dumpster_path);
void copy_file(std::string from, std::string to);
void perserve_metadata(std::string path, struct stat* metadata);
void force_remove(std::string file);

#endif
