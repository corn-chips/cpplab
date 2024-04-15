#pragma once
#include <map>
#include <vector>
#include <string>

std::map<std::string, std::vector<std::string>> parseCSV(const std::string& filename);