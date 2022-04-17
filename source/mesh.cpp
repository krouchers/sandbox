#include <mesh.h>
#include <tiny_obj_loader.h>
#include <stdexcept>

mesh::mesh(const std::string model_path, const std::string texture_path)
    : _model_path{model_path}, _texture_path{texture_path}
{
    load_model();
}

void mesh::load_model()
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    std::string warn, err;
    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, _model_path.c_str()))
    {
        throw std::runtime_error("failed to load object");
    }

    for (const auto &shape : shapes)
    {
        for (const auto &index : shape.mesh.indices)
        {
            Vertex vertex;
            vertex.position = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]};

            vertex.texture_coord = {
                attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1]};
            _vertices.push_back(vertex);
            _indices.push_back(_indices.size());
        }
    }
}
std::vector<Vertex> &mesh::get_vertices()
{
    return _vertices;
}
std::vector<uint32_t> &mesh::get_indices()
{
    return _indices;
}

std::string mesh::get_texture_path()
{
    return _texture_path;
}