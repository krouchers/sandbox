#pragma once
#include <file_system_interface.h>

class stream;

class application_file_system : public file_system
{
public:
    application_file_system(const char *root);
    stream *open(std::string &&file_path) final;

    void close(stream *stream) final;
    std::vector<std::string> read_text_from_file(const std::string file_name);

    ~application_file_system() final;

private:
    std::string _root_path;
};

class application_stream : public stream
{
public:
    application_stream(FILE *file_stream);
    ~application_stream() final;
    std::string read() final;
    size_t tell() final;
    bool seek(size_t pos) final;

private:
    FILE *_file;
};