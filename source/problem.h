#pragma once
#include <string>
#include <mesh.h>

class problem
{
    std::string _model_path;
    std::string _problem_text;
    mesh _mesh;
    int _answer;

public:
    problem(std::string, std::string, int);
    //geters
    mesh &get_mesh();
    int get_answer();
    std::string &get_text();
    //
};