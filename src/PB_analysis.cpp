#include "PB_ANALY.h"
#include "sys_F_M.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <regex>

using std::vector;
using std::deque;
using std::string;
using std::map;
using std::cerr;
using std::endl;
using std::cout;
using std::pair;





/**
 * @brief Read a data file and store the row into a deque
 * @param mp Select which column to read based on a given map
 * @param cd Data file location
 * @param t_mp Map. This will get updated to stores time of the file, using avg_T as key: <avg_T:{time data...}>
 * @param del delimiter for the type of file
 *
 * @note mp stores the position (count from 0) of the selected headed as key and the header itself as value
 * @notte t_mp will be updated
*/
std::map<double, std::deque<std::string>> PB_info_Read(std::map<int, std::string>&mp, fs::path & cd, map<double, std::deque<double>>& t_mp,char del) {

	std::ifstream data(cd);
	//fs::path fname = file_name_find(cd, "_excluded.csv", R"((\d+(\.\d+)?_min_concat)\.csv)");
	map<double, deque<string>> lst;
	if (!data.is_open()) {
		cerr << "File concatenation fail, data file " << cd << " fail to open!" << endl;
	}
	string line;

	std::getline(data, line);	//skip header

	while (std::getline(data, line)) {
		int acc = 0;
		std::stringstream ss(line);
		string word;
		string fileS;
		double avgT;
		double time;
		bool push = true;
		//cout << "line: " << line << endl;
		while (std::getline(ss, word, del)) { //extract indivdual element from each line
			if (mp.count(acc) != 0) {	//if key exist, then the column is the targeted column
				//cout << mp[acc] << ":" << word << endl;
				if (mp[acc] == "Exclude") {	//don't push into the map if file is excluded
					if (word == "Y") {
						push = false;
					}
				}
				if (mp[acc] == "File#") {
					fileS = word;
					cout << "file#: " << word << endl;
				}
				if (mp[acc] == "Del_T_avg(min)") {
					avgT = stod(word);
				}
				if (mp[acc] == "Del_T(min)") {
					time = stod(word);
				}
			}
			acc++;
		}
		if (push) {	//store data if not excluded
			lst[avgT].push_back(fileS);
			t_mp[avgT].push_back(time);
		}
	}
	data.close();
	return lst;
}





///**
// * @brief Read a data file and store the row into a deque
// * @param mp Select which column to read based on a given map
// * @param cd Data file location
// * @param del delimiter for the type of file
// *
// * @note map stores the position (count from 0) of the selected headed as key and the header itself as value
//*/
//std::map<double,std::deque<std::string>> PB_info_Read(std::map<int, std::string>& mp, fs::path& cd, char del) {
//
//	std::ifstream data(cd);
//	//fs::path fname = file_name_find(cd, "_excluded.csv", R"((\d+(\.\d+)?_min_concat)\.csv)");
//	map<double, deque<string>> lst;
//	if (!data.is_open()) {
//		cerr << "File concatenation fail, data file " << cd << " fail to open!" << endl;
//	}
//	string line;
//
//	std::getline(data, line);	//skip header
//
//	while (std::getline(data, line)) {
//		int acc = 0;
//		std::stringstream ss(line);
//		string word;
//		string fileS;
//		double avgT;
//
//		bool push = true;
//		while (std::getline(ss, word, del)) { //extract indivdual element from each line
//			if (mp.count(acc) != 0) {	//if key exist, then the column is the targeted column
//				if (mp[acc] == "Exclude") {	//don't push into the map if file is excluded
//					if (word == "Y") {
//						push = false;
//					}
//				}
//				if (mp[acc] == "File#") {
//					fileS = word;
//				}
//				if (mp[acc] == "Del_T_avg(min)"){
//					avgT = stod(word);
//				}
//			}
//			acc++;
//		}
//		if (push) {	//store data if not excluded
//			lst[avgT].push_back(fileS);
//		}
//	}
//	data.close();
//	return lst;
//}


/**
 * @brief Find the boxcar avarage of the 2nd column in a deque. Then return the averaged data with the 1st column
 * @param data List of data for average
 * @param window_size size of the box car average
 *
 * return A deque of average data data
 * @note Data must be two column, where 1st column is time and second column is mean pixel value
*/
std::deque<std::vector<double>> boxcar_average(const std::deque<std::vector<double>>& data, size_t window_size) {
	deque<vector<double>> averages;
	size_t half_window = window_size / 2;

	for (size_t i = half_window; i < data.size() - half_window; ++i) {
		double sum = 0.0;
		for (size_t j = i - half_window; j <= i + half_window; ++j) {
			sum += data[j][1];
		}
		double average = sum / window_size;
		averages.push_back({ data[i][0], average });
	}

	return averages;
}

/**
 * @brief Read the ref drople data (boxcared) and obtain the correction factor for the PB droplet.
 * @param data List that stores the ref droplet pixel value
 * return A list of correction factor
 * @note Data must be two column, where 1st column is time and second column is mean pixel value
*/
std::vector<double> ref_corr_factor(const std::deque<std::vector<double>>& data) {
	double first_val = data.front().back();
	vector<double> corr_factor;
	corr_factor.reserve(data.size());
	for (vector<double> row : data) {
		double bg_val = row[1];	//background value
		corr_factor.push_back(first_val/bg_val);	
	}
	return corr_factor;
}


/**
 * @brief Read a data file and store into a deque. Each row is represent by a vector within the deque. Then boxcar average the data.
 * @param cd Data file location
 * @param del delimiter for the type of file
 * @param box_car_window Size of the box car average
 * return A list of boxcared data
 * @note Data must be two column, where 1st column is time and second column is mean pixel value
*/
std::deque<std::vector<double>> PB_DataR(const fs::path& cd ,char del, int boxcar_window, double &PB_t, unsigned int PB_frame) {
	deque<vector<double>> PB_Data;
	std::ifstream data(cd);
	if (!data.is_open()) {
		cerr << "Data file " << cd << " Fail to open!" << endl;
	}
	string line;
	std::getline(data, line);	//skip header

	while (std::getline(data, line)) {
		int acc = 0;
		std::stringstream ss(line);
		string word;
		vector<double> temp;	//temporarily store readed values
		while (std::getline(ss, word, del)) { //extract indivdual element from each line
			temp.push_back(stod(word));
		}
		PB_Data.push_back(temp);
	}
	data.close();

	PB_t = PB_Data[PB_frame].front();	//time right after photobleached, for time shift
	deque<vector<double>> PB_data_avg = boxcar_average(PB_Data, boxcar_window);
	return PB_data_avg;
}

/**
 * @brief Find the minimum of a column, given the row is store in a list and the whole is store in another list
 * @param PBdata List that stores the data
 * @param corr_fact List of correction factor

 * @note the deque PBdata will be replace
*/
double Find_min_col(std::deque<std::vector<double>>& data, unsigned int index) {

	vector<double> col;
	col.reserve(data.size());

	for (vector<double> &row:data) {	//extract corrsponding column into col
		col.emplace_back(row[index]);
	}
	// Find the minimum element
	auto min_it = std::min_element(col.begin(), col.end());

	/*if (min_it != col.end()) {
		std::cout << "The minimum value is: " << *min_it << std::endl;
	}
	else {
		std::cout << "The vector is empty." << std::endl;
	}*/
	return *min_it;
}



/**
 * @brief Obtain the corrected and normalized PB value and REPLACE the normalized value with the old value
 * @param PBdata List that stores the time and boxcared PB data
 * @param corr_fact List of correction factor

 * @note the deque PBdata will be replace
*/
void PB_norm_cal(std::deque<std::vector<double>>& PBdata, std::vector<double> &corr_fact, double &PB_t) {
	
	vector<vector<double>> PB_norm;
	PB_norm.reserve(PBdata.size());
	for (unsigned int i = 0; i < PBdata.size(); i++) {
		vector<double> ith_data = PBdata.front();
		PBdata.pop_front();
		ith_data.back() = ith_data.back() * corr_fact[i];
		PBdata.push_back(ith_data);
	}
	double PB_min = Find_min_col(PBdata, 1);
	//normalizing PB data
	double PB_fst_min = PBdata.front().back()-PB_min;	//first element of PB data (t=0) subtract the minimum of the data

	for (unsigned int i = 0; i < PBdata.size(); i++) {
		vector<double> ith_data = PBdata.front();
		PBdata.pop_front();
		ith_data.back() = (ith_data.back() - PB_min) / PB_fst_min;	//normalization
		ith_data.front() = ith_data.front() - PB_t;	//shift time
		PBdata.push_back(ith_data);
	}

}

/**
 * @brief Read fit parameter from a text file and obtain the corrsponding fit
 * @param cd Path of the fit data file
 * @param del Delimitar of fit data file

 * @note 
*/
std::deque<double> fit_R(const fs::path& cd, char del, std::deque<std::pair<std::string,double>> &lst_excl) {
	std::ifstream data(cd);
	if (!data.is_open()) {
		cerr << "File concatenation fail, data file " << cd << " fail to open!" << endl;
	}
	string line;
	std::getline(data, line);	//skip first line
	bool endfile = false;
	deque<double> fit_param;
	while (std::getline(data, line)) {
		if (fit_param.size() == 2) {
			endfile = true;
		}
		int acc = 0;
		std::stringstream ss(line);
		string word;
		
		while (std::getline(ss, word, del)) {
			
			if (acc == 2 && fit_param.size() < 2) {
				double val = stod(word);
				fit_param.push_back(val);
				
			}
			if (acc == 2 && endfile) {
				double Rsq = stod(word);
				string filename = cd.filename().string();
				if (filename == "032-1") {
					cout << filename << ", R^2 = " << Rsq << endl;
				}
				if (Rsq < 0.9) {	//record files that have R^2 less than 0.9
					cout << "R^2 issue: " << filename << "," << Rsq << endl;
					pair<string, double> temp_rsq(filename, Rsq);
					lst_excl.push_back(temp_rsq);
				}
			}
			acc++;
		}
		if (endfile) {
			break;
		}
	}
	data.close();
	return fit_param;

}

/**
 * @brief Print fit parameter to a file
 * @param cd Path to print datafile
 * @param del Delimitar of fit data file

 * @note
*/
void Fit_pt(fs::path& cd, std::string del, std::deque<std::pair<std::string, double>> file_num, std::deque<std::deque<double>> param,const std::deque<std::string>& header) {
	FIL file(cd, std::ios::out);
	if (!header.empty()) {
		for (const string& ele : header) {
			file << ele << del;
		}
		file << endl;
	}
	for (unsigned int i = 0; i < file_num.size(); i++) {
		file << file_num[i].first << del << file_num[i].second << del << param[i][0] << del << param[i][1] << del << param[i][2] << endl;
		cout << "Fit parameter for file: " << file_num[i].first << " done print" << endl;
	}
	
	
}


