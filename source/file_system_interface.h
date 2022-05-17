#pragma once
// std
#include <string>
#include <vector>

class stream;

class file_system
{
public:
    virtual ~file_system() {};

    virtual stream *open(std::string &&file_path) = 0;
    virtual void close(stream *stream) = 0;
};

class stream
{
public:
    virtual ~stream() {};

    virtual std::string read() = 0;

    virtual size_t tell() = 0;
    virtual bool seek(size_t pos) = 0;
};