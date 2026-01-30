#pragma once

#include<string>
#include<vector>
#include<deque>
#include<filesystem>
#include<set>
#include<map>
//#include <utility> // For std::pair

namespace fs = std::filesystem;

//std::map<double, std::deque<std::string>> PB_info_Read(std::map<int, std::string>& mp, fs::path& cd, char del);
std::map<double, std::deque<std::string>> PB_info_Read(std::map<int, std::string>& mp, fs::path& cd, std::map<double, std::deque<double>>& t_mp, char del);
std::deque<std::vector<double>> PB_DataR(const fs::path& cd, char del, int boxcar_window, double &PB_t, unsigned int PB_frame);
std::vector<double> ref_corr_factor(const std::deque<std::vector<double>>& data);
void PB_norm_cal(std::deque<std::vector<double>>& PBdata, std::vector<double>& corr_fact,double &PB_t);
std::deque<double> fit_R(const fs::path& cd, char del, std::deque<std::pair<std::string, double>>& lst_excl);
void Fit_pt(fs::path& cd, std::string del, std::deque<std::pair<std::string, double>> file_num, std::deque<std::deque<double>> param,const std::deque<std::string>& header);

