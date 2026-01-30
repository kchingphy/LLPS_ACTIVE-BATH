// ConsoleApplication1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <iostream>
//#include "../function/SYS_F_M.h"
// #include "../LLPS_analysis/function/PART_COEFF.h"
#include "PART_COEFF.h"
#include "SYS_F_M.h"
#include "PB_ANALY.h"
// #include "KS.h"

#include <unordered_set>


//#include<OpenCV_img/img_OPENCV.h>
using std::cout;
using std::endl;
using std::string;
using std::deque;
using std::vector;
using std::pair;
using std::map;
using std::set;

/**
* @brief
* @param
* @return
* @warning
*/

/**
 * @brief Extract fit parameter and print in a better format
 * @param DI A DIR class that contains the directory of interested data
 * @param data_mp A pair of maps that stores: 1)avgT:{file#}, 2)avgT:time of file 
 * @param header List that contains the header for the printed file
 * @note
*/
void PB_rearrange(DIR& DI, std::pair <std::map<double, std::deque<double>>, std::map<double, std::deque<std::string>>>&data_mp, const deque<string> &header) {
    cout << "in rearrange" << endl;
    fs::path cd = DI.cid;
    cd = cd / fs::path("Fits");
    cout << cd << endl;
    deque<pair<string,double>> files;    //list contains all pair of <file number, file time>
    deque<deque<double>> fits;  //list contains all parameter
    deque<pair<string, double>> fit_R_sq_exclusion; //list contains excluded fits if R_sq < 0.9
    for (const auto& info_pair : data_mp.second) {  //loop through the map that contains file# info, with avgT as key
        double avgT = info_pair.first;
        for (const string &file_num : info_pair.second) {
            cd = cd / fs::path(file_num+ "_fit.txt");
            deque<double> temp_fit = fit_R(cd,' ',fit_R_sq_exclusion); //read fit parameter from _fit.csv file, store in a list
            temp_fit.push_front(info_pair.first);
            //push data into the desire list
            fits.push_back(temp_fit);
            double fileT = data_mp.first[avgT].front();
            data_mp.first[avgT].pop_front();    //remove file time from map
            std::pair<string, double> file_temp(file_num,fileT);
            files.push_back(file_temp);
            //files.push_back(file_num);
            //cout << avgT << "," << data_mp.first[avgT].front() << endl;
            //Push time data (not avgT) into files array
            
            //files.push_back(fileT);
            dir_up(cd, 1);  //backup 1 layer
        }
    }
    dir_up(cd, 1);
    
    cd = cd / fs::path("fit_param_cat.csv");
    Fit_pt(cd, ",", files, fits, header);
    cout << "File print:" << cd << endl;
    if (fit_R_sq_exclusion.empty()) {
        cout << "No R^2 exclusion" << endl;
    }
    else{
        unsigned int i = 0;
        dir_up(cd, 1);
        cout << "Date: " << cd.parent_path().filename().string() << endl;
        for (const auto& rsq : fit_R_sq_exclusion) {
            cout << i << ":(" << rsq.first << ",R^2 = " << rsq.second << ")" << endl;
            i++;
        }
    }
}



/**
* @brief Normalize PB data for fitting and plotting
* @param DI A DIR class that contains the directory of interested data
* @param extension Extension of datafile, usually is .csv
* @param PB_frame The frame number-1 right after photobleached
* @note
*/

void PB_Normalization(DIR &DI,string &extension, unsigned int PB_frame) {
    cout << DI.cid << endl;
    fs::path date = DI.cid.filename();
    set<string> filename = dir_iter_F_P(DI.cid, R"((\d+-\d+)(_R)?\.csv)");
 
    for (const string& name : filename) {   //for each filename
        fs::path cd_pt = DI.cid;
        dir_up(cd_pt,2);
        
        cd_pt = cd_pt / fs::path("normalized") / date;
        pair<fs::path, fs::path> fileP;
        
        fileP.first = DI.cid / fs::path(name+extension);    //PB data file
        fileP.second = DI.cid / fs::path(name + "_R" + extension);
        double t0_PBafter = 0.0;  //time right after PB happen
        //boxcar average the PB and ref data
        deque<vector<double>> ref_boxcared = PB_DataR(fileP.second, ',', 5, t0_PBafter, PB_frame);
        deque<vector<double>> PB_nor = PB_DataR(fileP.first, ',', 5, t0_PBafter, PB_frame);
        //
        vector<double> corr_fector = ref_corr_factor(ref_boxcared); //obtain correction factor
        PB_norm_cal(PB_nor, corr_fector,t0_PBafter);    //normalize PB data
        vector<string> header = { "time(s)",name }; //header for printed file
        cd_pt = cd_pt / fs::path(name + "_norm.csv");
        Data_print(cd_pt, PB_nor, header, ",");

    }
}

/**
* @brief Read PB data into from a csv file and extract the <T> and the corrsponding file#
* @param DI A DIR class that contains the directory of interested data
* @param tar_lst List of targeted header. Will be OVERWRITE into ordered header list
* @param header_excl List of headers that will be excluded from final printed file
* @return A map with key:<T> and value: a list of file corrspond to <T>
* @warning tar_lst will be overwrite into ordered header list (order follow the data file's header order)
*/
std::pair <std::map<double, std::deque<double>>, std::map<double, std::deque<std::string>>> PB_data_info(DIR &DI, std::deque<string>& tar_lst, std::deque<string>& header_excl) {

    map<int, string> header = F_target(',', DI.cid, tar_lst);  //find the position of selected header
    
    vector<string> header_reorder;
    tar_lst.clear();    //empty all element of tar_lst
    for (const auto& pair : header) {   //reinsert ordered header into tar_lst
        bool push = true;
        for (string &hd_exc:header_excl) {
            if (hd_exc == pair.second) {    //exclude header based on header_excl list
                push = false;
            }
        }
        if (push) {
            tar_lst.push_back(pair.second);
        }
    }
    //cout << "data info get" << endl;
    map<double, std::deque<double>> time_mp;
    cout << "up to here" << endl;
    map<double, std::deque<std::string>> data_mp = PB_info_Read(header, DI.cid,time_mp ,',');
    cout << "PB info read done!" << endl;
    //cout << "info read done" << endl;
    /*for (const auto& pair : time_mp) {
        std::cout << "Key: " << pair.first << " -> Values: ";
        for (const auto& value : pair.second) {
            std::cout << value << " ";
        }
        std::cout << std::endl;
    }*/
    pair<map<double, std::deque<double>>, map<double, std::deque<std::string>>> mp_pair(time_mp, data_mp);

    return mp_pair;
    
}



/**
 * @brief Initialize Partition Coefficient analysis by reading file info and headers
 * @param DI Directory object containing current directory information
 * @param tar_lst List of target headers to extract
 * @param header_excl List of headers to exclude from final selection
 * @param frameLst List that contains frame exclusion information
 * @param cut List that contains the last frame of each file
 * @return Map of file# (for droplet analysis) and corresponding avgT and time (<file#,{avgT,time}>)
 * @note This function reads a .csv file, extract the file# and corresponding average time and time, store in a map. It also update the frameLst and cut list (see PC_info_read).  
 */
static std::map<std::string, std::pair<double, double>> PC_Initial(DIR& DI, std::deque<string>& tar_lst, std::deque<string>& header_excl, std::deque<DP_EXC>& frameLst, std::deque<std::string>& cut) {
    map<string, pair<double, double>> lstInfo;

    std::map<int, std::string> header = F_target(',', DI.cid, tar_lst);  //find the position of selected header
    //vector<string> header_reorder;
    tar_lst.clear();    //empty all element of tar_lst
    
    for (const auto& pair : header) {   //reinsert ordered header into tar_lst
        bool push = true;
        for (string& hd_exc : header_excl) {
            if (hd_exc == pair.second) {    //exclude header based on header_excl list
                push = false;
            }
        }
        if (push) {
            tar_lst.push_back(pair.second);
        }
    }
    
    lstInfo = PC_info_Read(header, DI.cid, ',', frameLst, cut);

    return lstInfo;
}


// int main() {
//     std::cout << "Program started successfully.\n";
//     return 0;
// }
int main()
{
    // std::map<string, string> parent_map = {
    //     {"CDF","Droplet_size_stat"},
    //     {"PB","Droplet_PB_stat"},
    //     {"PC","Droplet_partition-coeff_stat"},
    //     {"PT","Protein"}
    // };
    // string parent, child;
    std::map<string, int> mode_map = {  //select what you are doing, PC == partition coeff
       {"PC",0},
       {"PB",1},
    //    {"KS",2},
       {"TEST",-1}
    };
    fs::path cd = fs::current_path();
    cd = cd.parent_path();
    string filename = "LLPS_analysis_input.txt";
    cd = cd / fs::path(filename);
    cout << "current dir: " << cd << endl;
    Config input = configuration(cd);
    int mode_int = mode_map[input.mode];
   



    
    

    ///Major input
    // string mode = "PC"; //select what you want to analyze
    // string root = "Droplet_stat";    //root is parent
    //string child = "Salt,By_Prod,Urea";
    // string child = "New_Salt,test";
    //string child = "phase/New_Salt";
    // vector<string> date_lst = { "20250422" };

    ///For PB
    // bool PB_correction = false;  //For normalizing PB data ONLY
    // string PBdata_ext = ".csv";
    // unsigned int PB_frame = 17; //frame # after PB occur
    
    ///For CDF
    bool toConcat = false;  //concat data

    
    //string root = parent_map[mode];
    
    //string root = "Z-stack test";

    
    //for Partition_coeff
    //string capture_clause = R"(Z-(\d+)_\d+_Droplet_stats\.csv)";
    string capture_clause_File = R"(Z-(\d+)_\d+_Droplet_stats\.csv)";
    string capture_clause_Frame = R"(Z-\d+_(\d+)_Droplet_stats\.csv)";
    // double std_n = 250; // Number of standard deviations for the threshold
    // int moving_avg_win_size = 5; // Example window size


    //for manual
    // fs::path cd_def("C:\\Users\\kcbb1\\Desktop\\Research\\LLPS\\Analysis\\20241212\\PB-Done\\conversion");


    // mode_int = -1;
    


    switch (mode_int) {

    case 0: {

        DIR data(input.layer, input.parent, input.child);
        cout << "parent: " << data.parent << endl;
        deque<string> header_select = { "File#","Frame","Del_T_avg(min)","Exclude", "Del_T(min)","Exc-element" };    //target which column to extract
        deque<string> header_exclude = { "Exclude", "Exc-element" };   //header that will be excluded from final file 
        deque<string> header_add_front = { "Date" };    //header that will be added to the front to the file file, ORDER PERSERVE
        deque<string> data_header_select = { "Mean","Area","X","Y", "Circ." };
        deque<string> header_pt = { "Date" ,"File#" , "Frame", "Droplet#" ,"Area","Part_Coeff","Circ." };
        // double scale = 6.1688; //pixel per micron
        //deque<PC_Data> result;  //store all results for a specific date
        //bool Fst_pt = true;
        for (const fs::path& sd : data.subdir) {    //for each subdirectory
            cout << "subdir: " << sd << endl;
            for (string& date : input.date_lst) {   //for each date
                data.cid = data.parent / sd / fs::path("file_info") / fs::path(date + "_Stat-info.csv");
                cout << "data path: " << data.cid << endl;
                deque<DP_EXC> Exc_lst;  //list contains excluding selection
                deque<string> Exc_cut;  //list contains last frame of each file, use to determine when to output abnormal appendation
                map<string, pair<double, double>> info_mp = PC_Initial(data, header_select, header_exclude, Exc_lst, Exc_cut);  //obtain file info and header

                ///Debug info print
                // for (auto& pair:info_mp)
                // {
                //     cout << "key: " << pair.first << ", val: (" << pair.second.first << "," << pair.second.second << ")" << endl;
                // }

                // for(auto& ele:Exc_lst){
                //     cout << "Exclude file#: " << ele.FileNum << ", frame: " << ele.Frame << "\t";
                //     if (!ele.element.empty()) {
                //         cout << "  Exclude droplet#: ";
                //         for (string& dp : ele.element) {
                //             cout << dp << ",";
                //         }
                //     }
                //     cout << "\n";
                // }
                ///End debug info print

                
                while (!header_add_front.empty()) {   //add the addition header into FRONT of final header list
                    string header = header_add_front.front();
                    header_add_front.pop_front();
                    header_select.push_front(header);    //append to header_select list from front
                }
                dir_up(data.cid, 2);    //go back to data.parent / sd

                ///Loop through all directory and get all filename
                deque<fs::path> file_lst;
                data.cid = data.cid / fs::path("raw") / date;

                // if (fs::exists(data.cid) && fs::is_directory(data.cid)) {
                //     std::cout << "Directory exists\n";
                // } else {
                //     std::cout << "Directory does not exist\n";
                // }

                dir_iter_F(data.cid, file_lst);
                // for(fs::path& pth : file_lst) {
                //     cout << "item: " << pth << endl;
                // }
                
                ///Function to check if number of last frame match number of files
                auto match_ck = [](const deque<fs::path> &file,const string &capture_clause,deque<string> frame_last) -> int {
                    std::unordered_set<string> file_set;    //a set that store unique file number
                    for(auto &item:file){
                        string F_num = file_name_find(item, "", capture_clause).filename().string();
                        file_set.insert(F_num);
                    }
                    if(file_set.size() != frame_last.size()){
                        std::cerr << "Number of last frame does not match number of files, CHECK!" << endl;
                        cout << "Total file #: " << file_set.size() << ", Total number of last frame: " << frame_last.size() << endl;
                        return 1;
                    }
                    return 0;
                };

                match_ck(file_lst, capture_clause_File, Exc_cut);   //check if number of last frame match number of files
                

                deque<Droplet> max_record;  //list of recorded maximum (intensity)
                deque<Droplet> excluded;    //list of excluded dropelt by range
                string LastFile;    //track the last read file

                ///If directory don't exist, make one
                if (!fs::exists(data.cid / fs::path("Max")) || !fs::is_directory(data.cid / fs::path("Max"))) {
                    //fs::create_directory(dir);
                    cout << "no max directory, making one" << endl;
                    fs::create_directories(data.cid / fs::path("Max")); //create a directory for maximum area list
                }
                if (!fs::exists(data.cid / fs::path("ABN_Append")) || !fs::is_directory(data.cid / fs::path("ABN_Append"))) {
                    //fs::create_directory(dir);
                    cout << "no abnormal appendation directory, making one" << endl;
                    fs::create_directories(data.cid / fs::path("ABN_Append")); //create a directory for abnormal appendation of max_list
                }
                if (!fs::exists(data.cid / fs::path("Exclude_Append")) || !fs::is_directory(data.cid / fs::path("Exclude_Append"))) {
                    //fs::create_directory(dir);
                    cout << "no exclude appendation directory, making one" << endl;
                    fs::create_directories(data.cid / fs::path("Exclude_Append")); //create a directory for abnormal appendation of max_list
                }
                ///End make directory
                ///Clean previous appendation files
                for (const auto& entry : fs::directory_iterator(data.cid / fs::path("ABN_Append"))) {   //remove all abnormal appendation file previous printed
                    if (entry.path().extension() == ".csv") {
                        fs::remove(entry.path());
                    }
                }
                for (const auto& entry : fs::directory_iterator(data.cid / fs::path("Exclude_Append"))) {   //remove all abnormal appendation file previous printed
                    if (entry.path().extension() == ".csv") {
                        fs::remove(entry.path());
                    }
                }
                ///End clean previous appendation files
                
                ///Loop through all file in the directory
                for (fs::path& filename : file_lst) {   //loop each item (each file) in the file list
                    data.cid = data.cid / filename;
                    //string FileN = file_name_find(data.cid, "", R"(Z-(\d+)_\d+_Droplet_stats\.csv)").filename().string();    //extract file#
                    string FileN = file_name_find(data.cid, "", capture_clause_File).filename().string();   //extract file#
                    //string frameS = file_name_find(data.cid, "", R"(Z-\d+_(\d+)_Droplet_stats\.csv)").filename().string();
                    string frameS = file_name_find(data.cid, "", capture_clause_Frame).filename().string(); //extract frame#

                    if (LastFile.empty()) {
                        LastFile = FileN;
                    }

                    if (FileN != LastFile) {    //If is a new file number, print the max_record and clear max list
                        fs::path file_root = data.cid;
                        dir_up(file_root, 1);
                        fs::path fileName_PT = file_name_find(data.cid, LastFile + "_max.csv", R"()").filename();
                        max_lst_Pt(file_root / fs::path("Max") / fileName_PT, max_record, header_pt, date);   //print recorded max list
                        fileName_PT = file_name_find(data.cid, LastFile + "_excluded.csv", R"()").filename();
                        max_lst_Err_Pt(file_root / fs::path("Exclude_Append") / fileName_PT, excluded, header_pt, date);
                        max_record.clear(); //clear max_record
                        excluded.clear();
                        Exc_cut.pop_front();
                        LastFile = FileN;
                    }

                    bool readData = true;   //read raw data file if true
                    deque<int> lst_dpExc; //exclude droplet
                    
                    if (!Exc_lst.empty()) { //if exclusion exist, 2 cases: 1) exclude that frame or 2) exclude certain droplet
                        //cout << "in exc list" << endl;
                        for (DP_EXC& Exc : Exc_lst) {
                            if (Exc.FileNum == FileN && Exc.Frame == frameS) {
                                if (Exc.element.empty()) {  //if not excluding any specific droplet (element), don't read the file
                                    readData = false;
                                    cout << "File: " << FileN << ",Frame: " << frameS << " skipped" << endl;
                                }
                                else {
                                    cout << "File: " << FileN << ",Frame: " << frameS << " skiping elements: ";
                                    for (string const& ele : Exc.element) {
                                        lst_dpExc.push_back(stoi(ele));
                                        cout << ele << ",";
                                    }
                                    cout << endl;
                                }
                            }
                        }
                    }
                    //else {  //print skip file
                    //    cout << "File: " << FileN << ",Frame: " << frameS << " skipped" << endl;
                    //}


                    /*if (!lst_dpExc.empty()) {
                        cout << "exclude: " << FileN << " " << frameS << endl;
                        cout << "droplet: ";
                        for (int& ex_ele : lst_dpExc) {
                            cout << ex_ele << ",";
                        }
                        cout << endl;
                    }
                    else {
                        cout << "Empty exclude list" << endl;
                    }*/


                    if (readData) {
                        map<int, string> header = F_target(',', data.cid, data_header_select);  //find the position of selected header
                        if (max_record.empty()) {
                            PC_FileR(data.cid, max_record, excluded,lst_dpExc, FileN, frameS, header, ',', true, stoi(Exc_cut.front()));
                        }
                        else {
                            PC_FileR(data.cid, max_record, excluded,lst_dpExc, FileN, frameS, header, ',', false, stoi(Exc_cut.front()));
                        }
                    }

                    /*cout << "Read frame: " << frameS << endl;
                    for (Droplet& item : max_record) {
                        cout << "Frame = " << item.Frame << " " << item.area << endl;
                    }*/

                    dir_up(data.cid, 1);    //back to subdir
                }   //end file list loop

                //string FileN = file_name_find(data.cid, "", R"(Z-(\d+)_\d+_Droplet_stats\.csv)").filename().string();
              
                cout << "LastFile: " << LastFile << endl;
                fs::path fileName_PT = file_name_find(data.cid, LastFile + "_max.csv", R"()").filename();
                fileName_PT = data.cid / fs::path("Max") / fileName_PT;
                max_lst_Pt(fileName_PT, max_record, header_pt, date);   //print recorded max list
                fileName_PT = file_name_find(data.cid, LastFile + "_excluded.csv", R"()").filename();
                fileName_PT = data.cid / fs::path("Exclude_Append") / fileName_PT;
                
                max_lst_Err_Pt(fileName_PT, excluded, header_pt, date);   //print recorded max list
                
                //correction


            }   //end date loop



        }   //end subdir loop
        cout << "Partition Coefficient analysis complete!" << endl;


        break;
    }

    case 1: {//FRAP analysis
        cout <<"FRAP Analysis"<< endl;
        DIR data(input.layer, input.parent, input.child);
        cout << data.parent << endl;
        

        for (const fs::path& sd : data.subdir) {    //for each subdirectory (conditions)

           if (input.PB_correction) {   //Normalize and correct FRAP data, run this ONLY if is not normalized and corrected 
               data.cid = data.parent / sd / fs::path("raw");
               cout << "path: " << data.cid << endl;
               for (string& date : input.date_lst)
               {
                   fs::path date_ph = fs::path(date);
                   data.cid = data.cid / date_ph;
                   PB_Normalization(data, input.ext, input.PB_frame_occur - 1);
                   dir_up(data.cid, 1);
               }
            cout << "FRAP correction complete" << endl;
           }
           else {   //Extract fit parameter, MAKE SURE RUN THIS AFTER FITTING
               for (string& date : input.date_lst) {  //for each date
                   deque<string> header_select = { "File#","Del_T_avg(min)","Exclude", "Del_T(min)" };    //target which column to extract
                   deque<string> header_exclude = { "Exclude" };   //header that will be excluded from final file 
                   deque<string> header_add = { "%_recovery","T_recovery(s)" };    //header that will be added to the file file, ORDER PERSERVE
                   data.cid = data.parent / sd / fs::path("file_info") / fs::path(date + "_PB-info.csv");
                   ///obtain PB file info and header
                   std::pair <std::map<double, std::deque<double>>, std::map<double, std::deque<std::string>>> info_mp = PB_data_info(data, header_select, header_exclude);  //OVERWRITE header_select
                   ///
                   cout << "data info get finish" << endl;
                   while (!header_add.empty()) {   //add the addition header into final header list
                       string header = header_add.front();
                       header_add.pop_front();
                       header_select.push_back(header);    //append to header_select list
                   }
                   dir_up(data.cid, 2);    //go back to data.parent / sd
                   data.cid = data.cid / fs::path("normalized") / fs::path(date);
                   PB_rearrange(data, info_mp, header_select);

               }
               cout << "FRAP parameter extraction complete" << endl;

           }
        }

        break;
    }

    case 2: {//KS-test, not done
        // cout << "KS-test Analysis" << endl;
        // try{
        //     DIR data(input.layer, input.parent, input.child);
        //     for (const fs::path& sd : data.subdir) {    //for each subdirectory (conditions)
        //     data.cid = data.parent / sd;
        //     cout << data.cid << endl;
        //     ks_read(data.cid);
        //     }
        // }
        // catch (const std::exception& e) {
        //     std::cerr << "Error: " << e.what() << std::endl;
        // }
        
        break;
    }

    default:
        cout << "test mode" << endl;
        DIR data(input.layer, input.parent, input.child);
        cout << data.parent << endl;
        cout << data.cid << endl;
        cout << "subdir list: " << endl;
        for (const fs::path& sd : data.subdir) {    //for each subdirectory
            cout << sd << endl;
        }
   

        break;

    }   //end switch statement



    return 0;
}

