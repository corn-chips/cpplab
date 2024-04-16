// csv-reader.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <vector>


//csv format
// itemA,itemB,itemC,itemD  <<  label line
// abcde,efghi,jklmn,opqrs  <<  data lines
// tuvwx,yzabc,defgh,ijklm
// nopqr,stuvw,xyzab,cdefg

//
// array[0] = 1;
// array[1] = 1;
// array[2] = 1;
// array[3] = 1;
// 
// std::map<std::string, int>
// map["asdfasdf"] = 1
//

std::map<std::string, std::vector<std::string>> parseCSV(const std::string& filename) {

    std::ifstream inputFile(filename);
    if (!inputFile.is_open()) {
        std::cerr << "ERROR OPENING FILE!" << std::endl;
        throw std::runtime_error("");
    }

    //get first line
    std::string line;
    std::getline(inputFile, line);

    //split up the line into each category
    auto parseLine = [](const std::string& line) -> std::vector<std::string> {
        //itemA
        // itemA,itemB,itemC,itemD

        std::vector<std::string> itemList;

        int s = 0;
        bool stringstate = false;
        for (int i = 0; i < line.length(); i++) {
            if (line[i] == '\"') {
                stringstate = !stringstate;
            }
            if (stringstate) continue;
            if (line[i] == ',' || line[i] == '\n') {
                itemList.push_back(line.substr(s, i - s));
                s = i+1;
            }
        }
        return itemList;
    };
    
    auto labelLine = parseLine(line);

    std::map<std::string, std::vector<std::string>> csv;
    for (const std::string& label : labelLine) {
        csv[label] = std::vector<std::string>();
    }

    while (std::getline(inputFile, line)) {
        auto b = parseLine(line);
        int b_idx = 0;
        for (const std::string& label : labelLine) {
            csv[label].push_back(b[b_idx]);
            b_idx++;
        }
    }
    return csv;
}