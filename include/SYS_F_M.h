#pragma once
#include <filesystem>
#include <string>
#include <fstream>
#include <deque>
#include <map>
#include <set>
#include <vector>
#include <regex>


namespace fs = std::filesystem;


/**
 * @class DIR
 * @brief Class for storing path other than the current path
 *
 * Stores a root dierectory, and n layers within the root
 * Also store a list of additional directory, this must added manually
 * @warning The member function is defined in sys_mani.cpp
 */
class DIR {
public:
	fs::path parent;

	std::deque<fs::path> M_dir;
	std::deque<fs::path> subdir;
	fs::path cid;	//currently interested directory
	/*deque<fs::path> subdir;
	* std::deque<fs::path> subdir;
	deque<fs::path> file;
	deque<fs::path> del_term;*/

	// Constructor for PATH_INFO, initialize root
	DIR(int layer, std::string &path_new, const std::string& subdir_lst);

	// Turn a list of string into path object, store at subdir 
	//void str_dk_path(deque<string>& dk,bool pop);

	//
	
};



/**
 * @class FIL
 * @brief Class for opening/creating file
 * 
 * This class provides method to open/create/write a file
 * @warning The member function is defined in file_mani.cpp 
 */
class FIL {

private:
	std::fstream file;
public:
	/**
	 * @brief Constructor for initializing the file
	 * @param F_name name of the file for write/read
	 * @param mode std::ios::(choose_mode), choose_mode depends on user --> write: out,read: in, append data: app
	 * @note see more mode example online
	 * 
	 */
	FIL(std::string& F_name, std::ios_base::openmode mode);
	FIL(const char* F_name, std::ios_base::openmode mode);
	FIL(fs::path& F_name, std::ios_base::openmode mode);
	FIL(const fs::path& F_name, std::ios_base::openmode mode);
	
	/**
	 * @brief Member function for defining << operator
	 * 
	 * @return *this, allows function to return the object FIL, thus allow chaining 
	 */
	FIL& operator<<(std::string& str)
	{
		file << str;
		return *this;	//return the reference of FIL, allow chaining
	}
	FIL& operator<<(const std::string& str)
	{
		file << str;
		return *this;
	}
	FIL& operator<<(const char* str)
	{
		file << str;
		return *this;
	}
	FIL& operator<<(double input)
	{
		file << input;
		return *this;
	}
	
	FIL& operator<<(int input)
	{
		file << input;
		return *this;
	}
	FIL& operator<<(std::ostream& (*manip)(std::ostream&))
	{
		manip(file);
		return *this;
	}



	/**
	 * @brief Member function for counting number of column in a file
	 * @param del delimiter of the file
	 * @return number of column
	 * @see file_mani.cpp -> fil_col
	 */
	//int fil_col(const char del);

	//Destructor
	~FIL();
};

/*
* Reqire file: file_mani.cpp 
* @see For detail, see the designated function
*/

void File_concat(fs::path file_final, std::deque<fs::path>& file_lst, fs::path data_path, const std::string& patt);
std::map<int, std::string> F_target(const char del, const fs::path& ifile_name, const std::deque<std::string>& tar_lst);
fs::path file_name_find(fs::path cd, const std::string &addition,const std::string &patt);
std::string str_find(std::string& str, const std::string& addition, const std::string& patt);	//find a part of the string
std::deque<fs::path> find_file(const fs::path& root, const std::string& extension);	//Find all match regular file under root directory
void Data_print(fs::path& cd, const std::deque<std::vector<double>>& data, std::vector<std::string>& header, std::string del);
void txtTocsv(fs::path& cd, char del);
std::deque<std::deque<double>> DataR_col(fs::path& cd, char del);
bool find(const std::string& word, const std::string& patt);
std::smatch capture(const std::string& word, const std::string& patt);

//std::deque<std::deque<double>> DataR_col(fs::path& cd, std::vector<std::string>& header, char del);


//void lst_pt(std::vector<double>& lst);
/*
* Reqire file: sys_mani.cpp
* @see For detail, see the designated function
*/


struct Config
{
    std::string mode;
    std::string parent;
    std::string child;
    unsigned int layer;
	std::string ext;
    std::vector<std::string> date_lst;
    unsigned int PB_frame_occur;
	std::string date_file_test;
	bool PB_correction;
};
Config configuration(const fs::path &filename);

void dir_up(fs::path& p, int layer);
void dir_iter_F(fs::path& dir_p,std::deque<fs::path> &dk);
std::set<std::string> dir_iter_F_P(fs::path& dir_p, const std::string& patt);
//bool dir_iter(fs::path& dir_p, unordered_map<fs::path, deque<fs::path>>& ump);

/**
 * @brief Concate a path and a string and return as a path object
 * @param p the desire path
 * @param str a given string
 * 
 * @return A concatenated path object
*/
inline fs::path path_str_Conc(const fs::path& p, const std::string& str) {
	fs::path temp_path = str;
	fs::path path_final = p / temp_path;
	path_final = path_final.lexically_normal();
	return path_final;
}

std::deque<fs::path> File_path(fs::path F_name);
