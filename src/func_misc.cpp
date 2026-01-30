// #include <iostream>
// #include <fstream>
// #include "SYS_F_M.h"
// #include "USEFUL_MISC.h"
// #include <regex>


// using std::string;
// using std::cerr;
// using std::cout;
// using std::endl;


// Config configuration(const std::string &filename){
//     Config cfg;

//     std::ifstream data(filename);
//     if (!data.is_open()) {
//         cerr << "File concatenation fail, data file " << "data.txt" << " fail to open!" << endl;
//         return;
//     }

//     // std::unordered_map<string,string> config;
//     std::regex kv_pattern (R"(^\s*([A-Za-z0-9_]+)\s*:\s*(.+)\s*$)"); //pattern to capture key=value pairs

//     string line;
//     while (std::getline(data, line)) {  //loop through each line
//         if(find(line,R"(^\s*#)") || line.empty()){
//             continue;   //skip comment line (lines starting with #)
//         }
//         std::smatch match;
//         if (std::regex_match(line, match, kv_pattern)) {
//             std::string key = match.str(1);;
//             std::string value = match.str(2);
//             // if–else chain to assign values
//             if (key == "mode")
//                 cfg.mode = std::stoi(value);
//             else if (key == "parent")
//                 cfg.parent = value;
//             else if (key == "child")
//                 cfg.child = value;
//             else if (key == "date_file")
//                 // cfg.date_file = value;
//             else if (key == "file_ext")
//                 cfg.file_ext = value;
//         }    
        
//     }
//     // cout << "mode: " <<config["mode"] << endl;
//     data.close();   


//     return cfg;
// }

// void input_read(const std::string &file, std::unordered_map<string,string> &config) {
//     std::ifstream data(file);
//     if (!data.is_open()) {
//         cerr << "File concatenation fail, data file " << "data.txt" << " fail to open!" << endl;
//         return;
//     }

//     // std::unordered_map<string,string> config;
//     string kv_pattern = R"(^\s*([A-Za-z0-9_]+)\s*:\s*(.+)\s*$)"; //pattern to capture key=value pairs

//     string line;
//     while (std::getline(data, line)) {  //loop through each line
//         if(find(line,R"(^\s*#)") || line.empty()){
//             continue;   //skip comment line (lines starting with #)
//         }
//         std::smatch match = capture(line, kv_pattern);
//         for(size_t i = 1; i < match.size(); ++i) {  //store captured groups into map
//             config[match.str(1)] = match.str(2);
//         }
        
//     }
//     // cout << "mode: " <<config["mode"] << endl;
//     data.close();    
// }


