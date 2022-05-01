#pragma once
// std
#include <string>
#include <fstream>
#include <stdexcept>
#include <vector>

class IO
{
public:
    static std::vector<std::string> read_text_from_file(const std::string file_name);
};