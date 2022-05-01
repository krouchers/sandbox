#include <file_stream.h>
#include <vector>

std::vector<std::string> IO::read_text_from_file(const std::string file_name)
{
    std::ifstream file{};
    file.open(file_name);
    std::vector<std::string> ret;
    std::string str;
    if (file.is_open())
    {
        while (!file.eof())
        {
            std::getline(file, str);
            ret.push_back(str);
        }
    }
    else
    {
        std::runtime_error("faile to open file for reading text");
    }
    return ret;
}