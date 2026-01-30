// #include "../function/SYS_F_M.h"
#include "SYS_F_M.h"
#include <iostream>
#include <sstream>
#include <vector>


using std::cerr;
using std::cout;
using std::endl;
using std::regex;
using std::map;
using std::string;
using std::deque;
using std::vector;

FIL::FIL(string& F_name, std::ios_base::openmode mode){
	file.open(F_name, mode);
	if (!file.is_open()){
		cerr << "File" << F_name << " open FAIL!" << endl;
	}
}



FIL::FIL(const char* F_name, std::ios_base::openmode mode){
	file.open(F_name, mode);
	if (!file.is_open()){
		cerr << "File" << F_name << " open FAIL!" << endl;
	}
}

FIL::FIL(fs::path& F_name, std::ios_base::openmode mode){
	file.open(F_name, mode);
	if (!file.is_open()){
		cerr << "File" << F_name << " open FAIL!" << endl;
	}
}

FIL::FIL(const fs::path& F_name, std::ios_base::openmode mode) {
	file.open(F_name, mode);
	if (!file.is_open()) {
		cerr << "File" << F_name << " open FAIL!" << endl;
	}
}

FIL::~FIL(){
	if (file.is_open()){
		file.close();
	}
}


/**
 * @brief convert txt file to csv file
 * @param cd Data file location
 * @param del delimiter for the type of file
 *
 * @note 
*/
void txtTocsv(fs::path& cd, char del) {
	std::ifstream file;
	deque<vector<double>> store;	//store data
	file.open(cd);
	if (!file.is_open()) {
		cerr << "File concatenation fail, data file " << cd << " fail to open!" << endl;
	}
	deque<string> val;
	string line;
	std::getline(file, line);	//skip first line
	bool endfile = false;
	deque<double> fit_param;
	while (std::getline(file, line)) {
		vector<double> lineR;
		std::stringstream ss(line);
		string word;
		while (std::getline(ss, word, del)) {
			if (!word.empty()) {	//ignore empty word
				lineR.push_back(stod(word));
			}
		}
		store.push_back(lineR);
	}
	file.close();

	///Delete this later
	fs::path name_extr = file_name_find(cd, "", R"(PB_(\d+-\d+)\.txt)");
	string name_mod = name_extr.filename().string();
	vector<string> header = { "time(s)",name_mod };
	name_mod = name_mod + "_norm.csv";
	dir_up(name_extr, 1);
	name_extr = name_extr / fs::path(name_mod);
	cout << name_extr << endl;

	Data_print(name_extr, store, header, ",");
}


/**
 * @brief Read a data file and store the data in a 2D list, by column
 * @param cd Printed data file location
 * @param data A list that stores the data for print
 * @param del delimiter for the type of file
 *
 * @Warning It assume the file have a header
*/

std::deque<std::deque<double>> DataR_col(fs::path& cd, char del) {
	std::ifstream file;
	deque<deque<double>> store;	//store data
	file.open(cd);
	if (!file.is_open()) {
		cerr << "Fail to find column, data file " << cd << " fail to open!" << endl;
	}

	deque<string> val;
	string line;
	string word;
	std::getline(file, line);	//skip first line

	bool isFirst = true;
	while (std::getline(file, line)) {
		deque<double> lineR;	//store each line
		std::stringstream ss(line);
		//cout << line << endl;
		while (std::getline(ss, word, del)) {	//obtain each column in a row
			lineR.push_back(stod(word));
		}
		if (isFirst) {
			while (!lineR.empty()) {	//first element
				deque<double> col_val = { lineR.front() };
				store.push_back(col_val);
				lineR.pop_front();
				isFirst = false;
			}
		}
		else {
			for (unsigned int i = 0; i < lineR.size(); i++) {
				double col_val = lineR[i];
				store[i].push_back(col_val);
				//cout << "pop:" << col_val << endl;
			}
		}
		//cout << "size: " << store[0].size() << "," << store[1].size() << endl;
	}
	file.close();

	/*for (unsigned int i = 0; i < store.front().size(); i++) {
		cout << store[0][i] << "," << store[1][i] << endl;
	}*/

	return store;
}



/**
 * @brief Read a data file and store the data in a 2D list, by column
 * @param cd Printed data file location
 * @param data A list that stores the data for print
 * @param del delimiter for the type of file
 *
 * @Warning It assume the file have a header
*/

std::deque<std::deque<double>> DataR_col(fs::path& cd, std::vector<std::string> &header, char del) {
	std::ifstream file;
	deque<deque<double>> store;	//store data
	file.open(cd);
	if (!file.is_open()) {
		cerr << "File concatenation fail, data file " << cd << " fail to open!" << endl;
	}
	deque<string> val;
	string line;
	std::stringstream ss(line);
	string word;
	std::getline(file, line);	//Store header
	while (std::getline(ss, word, del)) {
		header.push_back(word);
	}

	bool endfile = false;
	while (std::getline(file, line)) {
		deque<double> lineR;	//store each line
		
		while (std::getline(ss, word, del)) {	//obtain each column in a row
			lineR.push_back(stod(word));
		}
		if (store.empty()) {
			while (!lineR.empty()) {	//first element
				deque<double> col_val = { lineR.front() };
				store.push_back(col_val);
				lineR.pop_front();
			}
		}
		else {
			for (unsigned int i = 0; i < lineR.size(); i++) {
				double col_val = lineR.front();
				store[i].push_back(col_val);
				lineR.pop_front();
			}
		}
	}
	file.close();

	for (auto& ele : store) {
		cout << ele[0] << "," << ele[1] << endl;
	}

	
	return store;
}





/**
 * @brief Print a deque<vector> data into a file
 * @param cd Printed data file location
 * @param data A list that stores the data for print
 * @param del delimiter for the type of file
 *
 * @note data must be in deque<vector>> form
*/
void Data_print(fs::path& cd, const std::deque<std::vector<double>>& data,std::vector<std::string> &header, std::string del) {
	FIL file(cd,std::ios::out);
	
	if (!header.empty()) {
		for (const string &ele:header) {
			file << ele << del;
		}
		file << endl;
	}
	for (const vector<double>& row : data) {
		for (const double& val : row) {
			file << val << del;
		}
		file << endl;
	}
	cout << cd.filename() << " done!" << endl;
}

//int FIL::fil_col(const char del){
//	/**
//	* @brief Find the number of column in a file (usually text file) by reading the first line from the file.
//	* 
//	* @param del Delimiter of the file
//	* @return Number of column. If return anything less than 0 it means there is an error!
//	*/
//	int acc = -1;
//	if (!file.is_open()){	//return -1 if file is not open
//		cerr << "File not open!" << endl;
//		return acc;
//	}
//	else{
//		acc++;
//		string line, word;
//		getline(file, line);
//		std::stringstream ss(line);
//		while (getline(ss, word, del)){
//			acc++;
//		}
//		return acc;
//	}
//}

/**
	* @brief Concat all data files into one new file
	*
	* @param file_final Path+name of the concatenated file
	* @return 
	*/

void File_concat(fs::path file_final, deque<fs::path>& file_lst, fs::path data_path, const string& patt){
	FIL Conc_data(file_final, std::ios_base::out);
	fs::path path_ck = file_final;
	dir_up(file_final, 1);
	bool isFirstfile = true;
	bool isFirstline = true;
	//regex pattern(R"((\d+))");	//extract pattern, \d+ = 1 or more digit
	regex pattern(patt);
	while (!file_lst.empty()){
		
		fs::path data_p = data_path / file_lst.front();
		std::ifstream data(data_p);
		if (!data.is_open()) {
			cerr << "File concatenation fail, data file " << file_lst.front() << " fail to open!" << endl;
			break;
		}
		else if (fs::equivalent(path_ck,data_p)) {
			cerr << "Concatenated data file overlapped with concatenate list, end process! Check if raw data is in correct path" << endl;
			break;
		}
		else {
			string line;
			string F_name = file_lst.front().filename().string();
			if (!isFirstfile) {
				getline(data, line);
			}
			if (isFirstfile && isFirstline) {	//save the header for the file file, ignore for rest
				getline(data, line);
				line = line + ",File #";
				Conc_data << line << endl;
				isFirstline = false;
			}
			while (getline(data, line)) {
				std::smatch match;
				string F_name_extr;
				if (std::regex_search(F_name,match,pattern)) {
					F_name_extr = match.str(1);	//extract the match digit
				}
				line = line + "," + F_name_extr;
				Conc_data << line << endl;
			}
			isFirstfile = false;
			file_lst.pop_front();
		}
		data.close();
		
	}
}

/**
* @brief Find the target columns in a data file (text/csv file) by reading the first line from the file.
*
* @param del Delimiter of the file
* @param ifile_name Data file name
* @param tar_lst A list of targeted header
* @return A map that points to the corrsponding column, given the position (count starts at 0)
*/
map<int,string> F_target(const char del,const fs::path &ifile_name, const deque<string> &tar_lst) {
	std::ifstream iFile(ifile_name);

	map<int, string> Hlst;	//stores targeted header w/respect to the file column number
	int acc = -1;
	if (!iFile.is_open()) {	
		cerr << ifile_name.filename() << " file not open! Fail to extract header-column #" << endl;
		return Hlst;	//return empty map if error occurs
	}
	else {
		acc++;
		string line, word;
		getline(iFile, line);
		std::stringstream ss(line);
		while (getline(ss, word, del)) {
			for (const string &tar:tar_lst) {	//loop through target list, find the match
				if (tar == word) {
					Hlst[acc] = word;
					continue;
				}
			}
			acc++;
		}
	}
	iFile.close();
	return Hlst;
}

/**
 * @brief Find the corrsponding filename from a path, add addition to the end
 * @param cd Data file path
 * @param addition A string that is added to the end of the filename
 * @return A path with the filename + addition
 * 
 * @note This does not modify cd
*/
fs::path file_name_find(fs::path cd,const string &addition,const string &patt) {
	string name = cd.filename().string();
	/*if (addition.empty()) {
		cout << "filename: " << name << endl;
	}*/
	std::regex pattern(patt);	
	std::smatch matches;
	fs::path path_F;
	dir_up(cd, 1);
	if (std::regex_search(name, matches, pattern)) {
		string extract = matches.str(1) + addition;
		/*if (addition.empty()) {
			cout << "extracted: " << extract << endl;
		}*/
		//cout << "extracted: " << extract << endl;
		path_F = cd / fs::path(extract);
	}
	return path_F;
}

/**
 * @brief Find the corrsponding part of a string given a string and a pattern, add addition to the end
 * @param str String that you want to extracted from
 * @param addition A string that is added to the end of the extracted string
 * @return A string that is extracted string + addition
*/
std::string str_find(std::string& str, const std::string& addition, const std::string& patt) {
	std::regex pattern(patt);
	std::smatch matches;
	string extract;
	if (std::regex_search(str, matches, pattern)) {
		extract = matches.str(1) + addition;
	}
	return extract;
}

/**
 * @brief Check if a word partial match a given pattern
 * @param word The string to be checked
 * @param patt The pattern to be matched
 * @return true if match, false if not
 * @note This use regex to match the pattern
 */
bool find(const std::string& word, const std::string& patt){
	regex pattern(patt);
	bool found = std::regex_search(word,pattern);
	return found;
}

std::smatch capture(const std::string& word, const std::string& patt){
	regex pattern(patt);
	std::smatch matches;
	std::regex_match(word, matches, pattern);
	return matches;
}


/** 
 * @brief Read configuration file and store the data into Config struct
 * @param filename Configuration file path
 * @return A Config struct that contains all configuration data
*/
Config configuration(const fs::path &filename){
    Config cfg;

    std::ifstream data(filename);
    if (!data.is_open()) {
        cerr << "File concatenation fail, data file " << filename << " fail to open!" << endl;
		cfg.parent = "Fail to read config file";
        return cfg;
    }

    // std::unordered_map<string,string> config;
    std::regex kv_pattern(R"(^\s*([A-Za-z0-9_]+)\s*:\s*(.*?)\s*(?:#.*)?$)"); //pattern to capture words: words #comment in files

    string line;
    while (std::getline(data, line)) {  //loop through each line
        if(find(line,R"(^\s*#)") || line.empty()){
            continue;   //skip comment line (lines starting with #)
        }
        std::smatch match;
        if (std::regex_match(line, match, kv_pattern)) {
            std::string key = match.str(1);;
            std::string value = match.str(2);
            // if–else chain to assign values
            if (key == "mode"){
				cfg.mode = value;
			}
            else if (key == "parent")
			{
				cfg.parent = value;
			}
            else if (key == "child")
			{
				cfg.child = value;
			}
            else if (key == "date_file")
			{
				auto date_capture = [](Config &cfg, const std::string &value){
					std::regex date_pattern(R"(\d+)");
					std::sregex_iterator begin(value.begin(), value.end(), date_pattern);
					std::sregex_iterator end;
					for (std::sregex_iterator i = begin; i != end; ++i) {
						cfg.date_lst.push_back(i->str());
					}
				};
				date_capture(cfg, value);
				cfg.date_file_test = value;
			}
            else if (key == "file_ext")
			{
				cfg.ext = value;
			}
			else if (key == "layer")
			{
				cfg.layer = std::stoi(value);
			}
			else if (key == "PB_frame_occur")
			{
				cfg.PB_frame_occur = std::stoi(value);
			}
			else if (key == "PB_correction")
			{
				if (value == "true" || value == "True") {
					cfg.PB_correction = true;
				}
				else {
					cfg.PB_correction = false;
				}
			}

		}
               
    }    
        // cout << "mode: " <<config["mode"] << endl;
    data.close();   
	return cfg;
}
    


    
