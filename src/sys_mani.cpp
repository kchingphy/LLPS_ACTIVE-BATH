// #include "../function/SYS_F_M.h"
#include "SYS_F_M.h"
#include <iostream>
#include <sstream>
#include <regex>


using sst = std::stringstream;
using std::cerr;
using std::cout;
using std::endl;
using std::string;
using std::deque;
using std::regex;
using std::set;


/**
 * @brief Constructor for PATH_INFO
 * @param layer How many n-th layers (going up) from current directory
 * @param new_path Rest of the path from the n-th layer
 *
 * @note This will initialize root
*/
DIR::DIR(int layer, string& path_new,const string &subdir_lst) {
	fs::path cd = fs::current_path();
	std::stringstream ss(subdir_lst);
	string item;
	while (std::getline(ss, item, ',')) {
		fs::path subD = fs::path(item);
		subdir.push_back(subD);
	}
	dir_up(cd, layer);
	parent = path_str_Conc(cd, path_new);
}

/**
 * @brief Turns a list of string into path object and store at desire class object
 * @param dk Deque of string that represents the subdirectory
 * @param pop Pop the list of string as it get store into subdir
 * @param mode Flag for which class object to store to data. 's'=subdir, 'd'=del_term
 *
 * @note This will initialize 
*/
//void DIR::str_dk_path(deque<string>& dk,bool pop) {
//	for (string& ele : dk) {
//		fs::path temp = (ele);
//		if (mode == 's') {
//			//subdir.push_back(temp);
//		}
//		else if (mode == 'a') {
//			add_dir.push_back(temp);
//		}
//		if (pop) {
//			dk.pop_front();
//		}
//	}
//}


/**
 * @brief From a given directory, go up of the directory for a give number of layers
 * @param path
*/
void dir_up(fs::path& p, int layer){
	while (layer > 0) {
		p /= "..";
		--layer;
	}
	p = p.lexically_normal();
}

/**
 * @brief iterate through a directory to obtain a list of regular file
 * @param dir_p the directory path
 * @param dk deque that stores the list of regular files
*/
void dir_iter_F(fs::path& dir_p, std::deque<fs::path>& dk){
	bool no_add = true;	//check if at lease one file is found
	if (fs::exists(dir_p) && fs::is_directory(dir_p)){	//check if directory exist and if is a directory
		for (const fs::directory_entry& entry : fs::directory_iterator(dir_p)) {	//iterate the directory
			if (entry.is_regular_file()) {	//only extract regular file
				dk.push_back(entry.path().filename());
				no_add = false;
			}
		}
	}
	else{
		cerr << "The given path is not a directory or Does Not Exist" << endl;
		cerr << "path: " << dir_p << endl;
	}
	if (no_add) {
		cout << "No regular file found" << endl;
	}
}


/**
 * @brief iterate through a directory to obtain a list of regular file name or partical name 
 * @param dir_p the directory path
 * R"((\d+)_R\.csv)"
 * @return A list of extracted string, NOT PATH OBJECT
*/
std::set<std::string> dir_iter_F_P(fs::path& dir_p, const std::string& patt) {
	bool no_add = true; // Check if at least one file is found
	regex pattern(patt);

	std::smatch match;
	set<string> lst;
	if (fs::exists(dir_p) && fs::is_directory(dir_p)) { // Check if directory exists and if it is a directory
		for (const fs::directory_entry& entry : fs::directory_iterator(dir_p)) { // Iterate the directory
			if (entry.is_regular_file()) { // Only extract regular files
				std::string filename = entry.path().filename().string(); // Convert filename to string
				if (std::regex_search(filename, match, pattern)) {
					string extr_name = match.str(1); // Extract the matched digit
					lst.insert(extr_name);
				}

				no_add = false;
			}
		}
	}
	else {
		std::cerr << "The given path is not a directory or does not exist" << std::endl;
		std::cerr << "Path: " << dir_p << std::endl;
	}

	if (no_add) {
		std::cout << "No regular file found" << std::endl;
	}
	return lst;
}


/**
 * @brief iterate through a directory to obtain a list of subdirectory
 * @param dir_p the directory path
 * @param dk deque that stores the list of regular files
 * 
 * @return no_dir True if no ssubdirectory is detected under a directory
*/

//bool dir_iter(fs::path& dir_p, unordered_map<fs::path, deque<fs::path>> &ump) {
//	bool dir = false;	//true if at lease one directory is found
//	deque<fs::path> dk_mt;	//empty deque for value place holder
//	if (!ump.empty()) {
//		cerr << "The given map is NOT empty, key collision may occur! Process continue!" << endl;
//	}
//	if (fs::exists(dir_p) && fs::is_directory(dir_p)) {	//check if directory exist and if is a directory
//		ump.emplace(dir_p, dk_mt);
//		for (const fs::directory_entry& entry : fs::directory_iterator(dir_p)) {	//iterate the directory
//			if (entry.is_directory()) {	//only extract directory
//				dir = true;
//				ump[dir_p].push_back(entry);
//			}
//		}
//	}
//	else {
//		std::cerr << "The given path is not a directory or Does Not Exist" << std::endl;
//	}
//	if (dir) {
//		cout << "No directory found, reach to bottom" << endl;
//	}
//	return dir;
//}
//

/**
 * @brief Read paths from a data file and store in a deque
 * @param F_name Name of the data file 
 * 
 * @return A deque with path info
*/
deque<fs::path> File_path(fs::path F_name) {
	std::ifstream data(F_name);
	deque<fs::path> p_lst;
	if (!data.is_open()) {
		cerr << "Read path fail, data file " << F_name << " fail to open!" << endl;
	}
	string line;
	while (std::getline(data,line)) {
		p_lst.emplace_back(fs::path(line));
	}
	cout << "Path input done!" << endl;
	return p_lst;
}

/**
 * @brief Loop through all directory under root and find all matched regular file and stroe the path into a deque 
 * @param F_name Name of the data file
 *
 * @return A deque with path info
*/
std::deque<fs::path> find_file(const fs::path &root,const std::string &extension) {
	deque<fs::path> lst;	//list that stores all matched files
	for (const auto& entry : fs::recursive_directory_iterator(root)) {
		if (entry.is_regular_file() && entry.path().extension() == extension) {
			lst.push_back(entry.path());
		}
	}
	return lst;
}

