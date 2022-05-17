#include "IO.h"
#include <vector>
#include <fstream>
#include <algorithm>
#include <iostream>

std::vector<std::string> application_file_system::read_text_from_file(const std::string file_name)
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

application_file_system::application_file_system(const char *root_path)
    : _root_path{root_path}
{
    std::replace(_root_path.begin(), _root_path.end(), '\\', '/');
    _root_path.erase(_root_path.find("sandbox") + std::string("sandbox").size());
    if (_root_path.rfind('/') != _root_path.size() - 1)
    {
        _root_path += '/';
    }
}

application_file_system::~application_file_system()
{
}

stream *application_file_system::open(std::string &&file_path)
{
    if (file_path.find(_root_path) == std::string ::npos)
    {
        file_path = _root_path + file_path;
    }
    FILE *file = nullptr;
    file = fopen(file_path.c_str(), "rb");
    if (file == nullptr)
        return nullptr;
    return new application_stream(file);
}

application_stream::application_stream(FILE *file_stream) : _file{file_stream}
{
}

std::string application_stream::read()
{
    fseek(_file, 0, SEEK_END);
    auto size = ftell(_file);
    std::vector<char> text(size);
    fseek(_file, 0, SEEK_SET);
    fread(text.data(), sizeof(char), size, _file);
    return std::string(text.data(), text.data() + size);
}

size_t application_stream::tell()
{
    return ftell(_file);
}

bool application_stream::seek(size_t pos)
{
    return (fseek(_file, pos, SEEK_SET) == 0);
}

application_stream::~application_stream()
{
    fclose(_file);
}

void application_file_system::close(stream *stream)
{
    delete stream;
}
