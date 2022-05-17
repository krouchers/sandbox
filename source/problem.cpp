#include <problem.h>
#include <iostream>

problem::problem(std::string model_path, std::string problem_text, int answer)
    : _model_path{model_path}, _problem_text{problem_text},
      _mesh{_model_path, "../assets/textures/viking_room.png"},
      _answer{answer}
{
}

mesh &problem::get_mesh()
{
  return _mesh;
}

std::string &problem::get_text()
{
  return _problem_text;
}

int problem::get_answer(){
  return _answer;
}