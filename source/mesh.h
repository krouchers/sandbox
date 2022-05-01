#pragma once
// std
#include <string>
#include <vulkan_context.h>

class mesh
{

    void load_model();
    std::string _model_path;
    std::string _texture_path;

    std::vector<Vertex> _vertices;
    std::vector<uint32_t> _indices;

public:
    mesh(const std::string model_path, const std::string);
    mesh();
    //geters
    std::vector<Vertex> &get_vertices();
    std::vector<uint32_t> &get_indices();
    std::string get_texture_path();
    //
};
