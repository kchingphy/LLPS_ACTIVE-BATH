#pragma once
#include "SYS_F_M.h"
#include <deque>
#include <string>
#include <utility> // Include this for std::pair
#include <map>

struct Droplet
{
	std::string Fnum;	//file number
	std::string Frame;
	int NumDp;	//droplet number
	double T;	//time
	double avgT;	//average time
	double mean;
	double bk_avg;
	double X, Y, area;	
	double circ;	//circulity
	bool print = true;
};

struct DP_EXC
{
	std::string FileNum;
	std::string Frame;
	std::deque<std::string> element;	//droplet# for exclusion
};

//std::map<std::string, std::deque<Droplet>>  PC_info_Read(std::map<int, std::string>& mp_col, fs::path& cd, char del);
std::map<std::string, std::pair<double, double>>  PC_info_Read(std::map<int, std::string>& mp_col, fs::path& cd, char del, std::deque<DP_EXC>& frameLst, std::deque<std::string>& lst_cut);
void max_lst_Pt(fs::path const& cd, std::deque<Droplet>& lst_max, std::deque<std::string> const& header, std::string const& date);
void max_lst_Err_Pt(fs::path const& cd, std::deque<Droplet>& lst_max, std::deque<std::string> const& header, std::string const& date);
int PC_FileR(fs::path const& cd, std::deque<Droplet>& lst_max, std::deque<Droplet>& lst_RGEexclude, std::deque<int>& lst_Exc, std::string const& FileN, std::string const& FrameS, std::map<int, std::string>& mp_col, char const& del, bool Ffst, int frame_cut);
