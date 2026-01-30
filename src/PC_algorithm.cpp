// #include "../function/PART_COEFF.h"
#include "PART_COEFF.h"
#include <iostream>
#include <fstream>
// #include <vector>
#include <unordered_map>
#include <cmath>

using std::deque;
using std::vector;
using std::cout;
using std::cerr;
using std::endl;
using std::map;
using std::string;
using std::pair;








/**
// * @brief Read a data file and store into a map
// * @param mp_col Select which column to read based on a given map
// * @param cd Data file location
// * @param t_mp Map. This will get updated to stores time of the file, using avg_T as key: <avg_T:{time data...}>
// * @param del delimiter for the type of file
// *
// * @note mp_col stores the position (count from 0) of the selected headed as key and the header itself as value
//*/


/**
 * @brief Read a data file and store into a map
 * @param mp_col Select which column to read based on a given map
 * @param cd Data file location
 * @param del delimiter for the type of file
 * @param frameLst List that contains frame exclusion information
 * @param lst_cut List that contains the last frame of each file
 * @return Map of file# (for droplet analysis) and corresponding avgT and time (<file#,{avgT,time}>)
 * @note This function reads a .csv file. The mp_col stores the position (count from 0) of the selected headed as key and the header itself as value.
 *       It extract the file# and corresponding average time and time, store in a map. It also update the frameLst and cut list.
 *       The header is hardcoded to look for "File#", "Del_T_avg(min)", "Del_T(min)", "Exclude", and "Exc-element". Make sure these headers exist in the file.
 *       Any empty line in the file will break the reading process with a warning message.
 * 
 */
std::map<std::string, std::pair<double,double>>  PC_info_Read(std::map<int, std::string>& mp_col, fs::path& cd, char del, std::deque<DP_EXC> &frameLst,std::deque<std::string> &lst_cut) {

    std::ifstream data(cd);
    map<string, pair<double, double>>  lst;   //store <file#,{<avgT,time>}>

    if (!data.is_open()) {
        cerr << "File concatenation fail, data file " << cd << " fail to open!" << endl;
        return lst; //return empty map
    }
    string line;
    std::getline(data, line);	//skip header
    string file_last, frame_last;
    while (std::getline(data, line)) {  //loop through each line
        if(line.empty()) {
            cerr << "Empty line detected, CHECK file." << endl;
            lst["Error"] = pair<double,double>(-1.0,-1.0);
            break;   //skip empty line
        }
        int acc = 0;
        std::stringstream ss(line);
        string word;
        string fileS;
        DP_EXC record;
        double avgT = 0.0;
        double time = 0.0;
        bool push = false;  //true if the current item is to be pushed into frameLst
        
        while (std::getline(ss, word, del)) { //extract indivdual element from each line
            if (mp_col.count(acc) != 0) {	//if key exist proceed to read the following
                if (!word.empty())
                {
                    if (mp_col[acc] == "Exclude") {	//push into the exclusion list if file is excluded
                        if (word == "Y") {
                            push = true;
                        }
                    }
                    if (mp_col[acc] == "File#") {
                        fileS = word;
                        record.FileNum = word;
                        if (file_last.empty()) {
                            file_last = word;
                        }
                    }
                    if (mp_col[acc] == "Frame") {
                        //frame = word;
                        record.Frame = word;
                    }
                    if (mp_col[acc] == "Del_T_avg(min)") {

                        avgT = stod(word);
                    }
                    if (mp_col[acc] == "Del_T(min)") {

                        time = stod(word);
                    }
                    if (mp_col[acc] == "Exc-element") { //store excluded element into a list
                        string capture;
                        std::stringstream element(word);
                        while (std::getline(element, capture, '_')) {
                            record.element.push_back(capture);
                        }
                    }
                }
            }
            acc++;
        }
        if (file_last != record.FileNum) {
            lst_cut.push_back(frame_last);
        }
        file_last = record.FileNum;
        frame_last = record.Frame;
        
       
        if (push) {	//store excluded list data
            frameLst.push_back(record);
        }
        lst[fileS].first = avgT;
        lst[fileS].second = time;
    }   //end file loop
    lst_cut.push_back(frame_last);

    data.close();
    return lst;
}

static void exception_pt(fs::path const& cd, const Droplet& dp, std::deque<std::string> const& header, std::ios_base::openmode mode) {
    FIL data(cd, mode);
    if (mode == std::ios::out) {
        for (string const& head : header) {
            data << head << ",";
        }
        data << endl;
    }

    double PC = dp.mean / dp.bk_avg;
    data << dp.Fnum << "," << dp.Frame << "," << dp.NumDp << "," << dp.area << "," << PC << "," << dp.circ << endl;
}


static void Find_maxR(Droplet &current, std::deque<Droplet>& lst_max, double rc,bool pt_flag, bool fst_frame, fs::path cdPT) {
    double pixel_max = 2048.0;
    double pix_cut = 45.0;
    bool y_range = current.Y <= pixel_max - pix_cut && current.Y >= pix_cut;
    bool x_range = current.X <= pixel_max - pix_cut && current.X >= pix_cut;
    if (!y_range || !x_range) {
        return;
    }

    if (!fst_frame) { //compare the current frame droplet to max list if max list is NOT empty
        bool push = true;  //true if lst_max doesn't contain the current droplet
        double dr;
        for (Droplet &dpmax : lst_max) {
            dr = sqrt(pow(current.X - dpmax.X, 2) + pow(current.Y - dpmax.Y, 2));
            /*if (current.Frame == "025" && current.NumDp == 86) {
                cout << "frame: " << dpmax.Frame << ", Dp_num = " << dpmax.NumDp << " dr = " << dr << endl;
            }*/
            if (dr <= rc) { //if drift distance is less than or equal to cutoff
                double dA = dpmax.area - current.area;
                if (dA < 0) {  //current frame dropelt is larger, update max list
                    dpmax = current; 
                }
                else {  //update max list position so it recoreds the latest position for next comparison
                    dpmax.X = current.X;
                    dpmax.Y = current.Y;
                }
                push = false;   //droplet found, don't push
                break;  //droplet found, end loop
            }
        }
        if (push) { //current droplet not found in lst_max, push current droplet into lst_max, print exception if apply
            lst_max.push_back(current); //push current droplet into lst_max
            if (pt_flag) {  //if pt_flag is ture, print the appended droplet
                dir_up(cdPT, 1);
                string prefix = current.Fnum;
                prefix = "Z-" + prefix + "_ABN-append.csv";
                cdPT = cdPT / fs::path("ABN_Append") / fs::path(prefix);    //filename: 
                deque<string> header = { "File#" , "Frame", "Droplet#" ,"Area","Part_Coeff","Circ."};
                if (!fs::exists(cdPT) || !fs::is_regular_file(cdPT)) {  //start a file and write
                    exception_pt(cdPT, current, header, std::ios::out); 
                }
                else {  //append to existing file
                    exception_pt(cdPT, current, header, std::ios::app); 
                }
                cout << "Exception push" << endl;
            }
        }
    }
    else {  //First frame, push all dropelt into max list 
        lst_max.push_back(current);
    }
        
}

static bool Find_maxR_Int(Droplet& current, std::deque<Droplet>& lst_max, double rc, bool pt_flag, bool fst_frame, fs::path cdPT, std::deque<Droplet>& lst_exclude) {
    double pixel_max = 2048.0;
    double pix_cut = 45.0;
    bool y_range = current.Y <= pixel_max - pix_cut && current.Y >= pix_cut;
    bool x_range = current.X <= pixel_max - pix_cut && current.X >= pix_cut;
    // if (current.Fnum == "029" && current.Frame == "031" && current.NumDp == 72) {
    //     cout << "dy = " << current.Y - (pixel_max - pix_cut) << endl;
    //     cout << "y = " << current.Y << endl;
    // }
    bool inrange = false;
    if (!y_range || !x_range) {
        // cout << "Droplet out of range: Frame " << current.Frame << ", Droplet# " << current.NumDp << ", position (" << current.X << "," << current.Y << ")" << endl;
        /*if (current.Fnum == "029")
        {
            cout << "029 exception detected: " << current.Frame << ", position" << current.X << "," << current.Y << endl;
        }*/
        if (!lst_exclude.empty()) {
            bool push = true;  //true if lst_max doesn't contain the current droplet
            double dr;
            for (Droplet& dpmax : lst_exclude) {
                dr = sqrt(pow(current.X - dpmax.X, 2) + pow(current.Y - dpmax.Y, 2));
                /*if (current.Frame == "025" && current.NumDp == 86) {
                    cout << "frame: " << dpmax.Frame << ", Dp_num = " << dpmax.NumDp << " dr = " << dr << endl;
                }*/
                if (dr <= rc) { //if drift distance is less than or equal to cutoff
                    //double dA = dpmax.area - current.area;
                    double dM = dpmax.mean - current.mean;
                    if (dM < 0) {  //current frame dropelt is brighter, update max list
                        dpmax = current;
                    }
                    else {  //update max list position so it recoreds the latest position for next comparison
                        dpmax.X = current.X;
                        dpmax.Y = current.Y;
                    }
                    push = false;   //droplet found, don't push
                    break;  //droplet found, end loop
                }
            }
            if (push) { //current droplet not found in lst_max, push current droplet into lst_max, print exception if apply
                lst_exclude.push_back(current); //push current droplet into lst_max
                if (pt_flag) {  //if pt_flag is ture, print the appended droplet
                    dir_up(cdPT, 1);
                    string prefix = current.Fnum;
                    prefix = "Z-" + prefix + "_RGNExc-append.csv";
                    cdPT = cdPT / fs::path("Exclude_Append") / fs::path(prefix);    //filename: 
                    deque<string> header = { "File#" , "Frame", "Droplet#" ,"Area","Part_Coeff","Circ." };
                    if (!fs::exists(cdPT) || !fs::is_regular_file(cdPT)) {  //start a file and write
                        exception_pt(cdPT, current, header, std::ios::out);
                    }
                    else {  //append to existing file
                        exception_pt(cdPT, current, header, std::ios::app);
                    }
                    cout << "Exclude exception push" << endl;
                }
            }
        }
        else {  //If list is empty push droplet into the list 
            lst_exclude.push_back(current);
        }
        return inrange;
    }
    else {
        inrange = true;
    }

    if (!fst_frame) { //compare the current frame droplet to max list if max list is NOT empty
        bool push = true;  //true if lst_max doesn't contain the current droplet
        double dr;
        for (Droplet& dpmax : lst_max) {
            dr = sqrt(pow(current.X - dpmax.X, 2) + pow(current.Y - dpmax.Y, 2));
            /*if (current.Frame == "025" && current.NumDp == 86) {
                cout << "frame: " << dpmax.Frame << ", Dp_num = " << dpmax.NumDp << " dr = " << dr << endl;
            }*/
            if (dr <= rc) { //if drift distance is less than or equal to cutoff
                //double dA = dpmax.area - current.area;
                double dM = dpmax.mean - current.mean;
                if (dM < 0) {  //current frame dropelt is brighter, update max list
                    dpmax = current;
                }
                else {  //update max list position so it recoreds the latest position for next comparison
                    dpmax.X = current.X;
                    dpmax.Y = current.Y;
                }
                push = false;   //droplet found, don't push
                break;  //droplet found, end loop
            }
        }
        if (push) { //current droplet not found in lst_max, push current droplet into lst_max, print exception if apply
            lst_max.push_back(current); //push current droplet into lst_max
            if (pt_flag) {  //if pt_flag is ture, print the appended droplet
                dir_up(cdPT, 1);
                string prefix = current.Fnum;
                prefix = "Z-" + prefix + "_ABN-append.csv";
                cdPT = cdPT / fs::path("ABN_Append") / fs::path(prefix);    //filename: 
                deque<string> header = { "File#" , "Frame", "Droplet#" ,"Area","Part_Coeff","Circ." };
                if (!fs::exists(cdPT) || !fs::is_regular_file(cdPT)) {  //start a file and write
                    exception_pt(cdPT, current, header, std::ios::out);
                }
                else {  //append to existing file
                    exception_pt(cdPT, current, header, std::ios::app);
                }
                cout << "Exception push" << endl;
            }
        }
    }
    else {  //First frame, push all dropelt into max list 
        lst_max.push_back(current);
    }
    return inrange;
}

static void benchmark(Droplet& current, std::deque<Droplet>& lst_max,int const &pos, bool fst_frame) {
    //cout << "pos in " << pos << endl;
    if (!fst_frame) {
        //cout << "current area: " << current.area << endl;
        double dr = sqrt(pow(current.X - lst_max[pos].X, 2) + pow(current.Y - lst_max[pos].Y, 2));
        cout << "droplet: " << pos + 1 << " dr = " << dr << " (X,Y) " << current.X << "," << current.Y << " (X0,Y0) " << lst_max[pos].X << "," << lst_max[pos].Y << endl;
        lst_max[pos] = current;
    }
    else {  //push current frame dropelt into max list if max list is empty
        lst_max.push_back(current);
    }
    
}

void overlap_ck(Droplet& current, std::deque<Droplet>& lst_exclude, double rc) {
    for (Droplet& lst : lst_exclude) {
        double dr = sqrt(pow(current.X - lst.X, 2) + pow(current.Y - lst.Y, 2));
        if (dr <= rc) {
            lst.print = false;  //overlap with the other list, don't count as exclude
        }
    }
}


int PC_FileR(fs::path const &cd, std::deque<Droplet>& lst_max,std::deque<Droplet>&lst_RGEexclude ,std::deque<int>& lst_Exc, std::string const& FileN, std::string const& FrameS, std::map<int, std::string>& mp_col, char const &del, bool Ffst,int frame_cut) {
    double dr = 37.4;   //max distance allow (in pixel) for same droplet determination
    std::ifstream data(cd);
    if (!data.is_open()) {
        cerr << "File concatenation fail, data file " << cd << " fail to open!" << endl;
        return 1; //return 1 if file fail to open
    }
    frame_cut = floor(frame_cut / 2.0);   //half of maximum frame
    string line;
    std::getline(data, line);	//skip header
    //2nd line is background
    double bkG = 0.0;
    std::getline(data, line);	//skip header
    std::stringstream ss(line);
    string word;
    int acc = 0;
    while (std::getline(ss, word, del)) {
        if (mp_col[acc] == "Mean" && !word.empty()) {
            bkG = stod(word);
        }
        acc++;
    }
    //end background detection
    int dpNum = 1;  //droplet number
    
    while (std::getline(data, line)) {  //loop through each line(each droplet)
        bool skipline = false;
        if (!lst_Exc.empty()) {
            for (int const& dpSkip : lst_Exc) {
                if (dpNum == dpSkip) {
                    cout << "SKIPPING-frame: " << FrameS << " dp# " << dpNum << endl;
                    skipline = true;    //skip this droplet if is listed as exclude
                }
            }
        }
        acc = 0;
        ss.clear();
        ss.str(line);
        Droplet dpRead;
        if (!skipline) {    //if droplet is not excluded, read data
            while (std::getline(ss, word, del)) {
                if (mp_col.count(acc) != 0) {	//if key exist proceed to read the following
                    if (!word.empty()) {
                        if (mp_col[acc] == "Area") {
                            dpRead.area = stod(word);
                        }
                        if (mp_col[acc] == "Mean") {
                            dpRead.mean = stod(word);
                        }
                        if (mp_col[acc] == "X") {
                            dpRead.X = stod(word);
                        }
                        if (mp_col[acc] == "Y") {
                            dpRead.Y = stod(word);
                        }
                        if (mp_col[acc] == "Circ.") {
                            dpRead.circ = stod(word);
                        }
                    }
                }
                acc++;
            }
            //cout << dpNum << " " << dpRead.area << endl;
            dpRead.Fnum = FileN;
            dpRead.Frame = FrameS;
            
            dpRead.NumDp = dpNum;
            dpRead.bk_avg = bkG;
            dpRead.NumDp = dpNum;
            bool ABN_pt = false;    //boolian for printing abnormal appdendation of max_list

            if (stoi(FrameS) >= frame_cut) {   //frame cutoff for abnormal print
                ABN_pt = true;
            }
            //dpRead.bk_avg = bkG;
            //int pos = dpNum - 1;
            //cout << "pos before: " << pos << endl;
            //benchmark(dpRead, lst_max, dpNum-1,Ffst);

            //Find_maxR(dpRead, lst_max, 15.0, ABN_pt, Ffst, cd);
            
            bool inrange = Find_maxR_Int(dpRead, lst_max, dr, ABN_pt, Ffst, cd,lst_RGEexclude);
            if (inrange) {
                // overlap_ck(dpRead, lst_RGEexclude, dr); 
            }
            dpNum++;
        }
        else {
            dpNum++;
        }
    }
    data.close();
    return 0;
}


void max_lst_Pt(fs::path const& cd, std::deque<Droplet>& lst_max, std::deque<std::string> const &header, std::string const & date) {
    FIL data(cd,std::ios::out);
    cout << "print path: " << cd << endl;
    for (string const& head : header) {
        data << head << ",";
    }
    data << endl;
    for (Droplet& dp : lst_max) {
        double PC = dp.mean / dp.bk_avg;
        data << date << "," << dp.Fnum << "," << dp.Frame << "," << dp.NumDp <<"," << dp.area << "," << PC << "," << dp.circ << endl;
    }
}

void max_lst_Err_Pt(fs::path const& cd, std::deque<Droplet>& lst_max, std::deque<std::string> const& header, std::string const& date) {
    FIL data(cd, std::ios::out);
    cout << "print path: " << cd << endl;
    for (string const& head : header) {
        data << head << ",";
    }
    data << endl;
    // cout << "Excluded droplet list size: " << lst_max.size() << endl;
    for (Droplet& dp : lst_max) {
        if (dp.print) {
            double PC = dp.mean / dp.bk_avg;
            // cout << "Excluded droplet printed: File# " << dp.Fnum << ", Frame " << dp.Frame << ", Droplet# " << dp.NumDp << endl;
            data << date << "," << dp.Fnum << "," << dp.Frame << "," << dp.NumDp << "," << dp.area << "," << PC << "," << dp.circ << endl;
        }
        
    }
}
