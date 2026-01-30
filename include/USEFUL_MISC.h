#pragma once
#include <iostream>
#include <vector>
#include <deque>
#include <string>
#include <unordered_map>

/**
 * @brief Print a 1-D container to the console, using range based forloop 
 * @param container Any container that is 1-D and can be used in a range based forloop
 * @note Print with the position of the container: position,element. Each element printed in a line
*/
template <typename Container>   //Template type deduction
void cont_pt(const Container& container) {
    std::cout << "printing list" << std::endl;
    unsigned int i = 0;
    for (const auto& ele : container) {
        std::cout << i << "," << ele << std::endl;
        i++;
    }
    std::cout << "list done printing" << std::endl;
}

struct Config
{
    int mode;
    std::string parent;
    std::string child;
    unsigned int layer;
    std::vector<std::string> date_lst;
    unsigned int PB_frame;
};



void input_read(const std::string &file, std::unordered_map<std::string,std::string> &config);
// void input_read(const std::string &file, std::string &parent, std::string &child, unsigned int &layer, int &mode, std::vector<std::string> &date_lst, unsigned int &PB_frame);



