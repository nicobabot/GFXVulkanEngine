#include "ModelLoader.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#include <cmath>
#include <filesystem>
#include <limits>
#include <fstream>
#include <stdexcept>
#include <unordered_map>

namespace
{
    // Resolve map_Kd relative to its MTL, including MTLs in subdirectories.
    class MaterialReader : public tinyobj::MaterialReader
    {
    public:
        explicit MaterialReader(const std::filesystem::path& directory) : directory(directory) {}
        std::vector<std::filesystem::path> directories;
        bool operator()(const std::string& id, std::vector<tinyobj::material_t>* materials,
            std::map<std::string, int>* names, std::string* diagnostics) override
        {
            const auto path = (directory / id).lexically_normal();
            std::ifstream stream(path);
            if (!stream)
            {
                if (diagnostics) *diagnostics += "Could not open MTL: " + path.generic_string() + "\n";
                return false;
            }
            tinyobj::LoadMtl(names, materials, &stream, diagnostics);
            directories.resize(materials->size(), path.parent_path());
            return true;
        }
    private:
        std::filesystem::path directory;
    };

    bool IsFinite(const glm::vec3& value)
    {
        return std::isfinite(value.x) && std::isfinite(value.y) && std::isfinite(value.z);
    }

    void CheckIndex(int index, size_t count, const char* attribute, const std::string& path)
    {
        if (index < 0 || static_cast<size_t>(index) >= count)
            throw std::runtime_error("Invalid OBJ " + std::string(attribute) + " index in: " + path);
    }

    glm::vec3 ReadPosition(const tinyobj::attrib_t& attributes, int index, const std::string& path)
    {
        CheckIndex(index, attributes.vertices.size() / 3, "position", path);
        const size_t offset = static_cast<size_t>(index) * 3;
        const glm::vec3 position(attributes.vertices[offset], attributes.vertices[offset + 1], attributes.vertices[offset + 2]);
        if (!IsFinite(position))
            throw std::runtime_error("Non-finite OBJ position in: " + path);
        return position;
    }
}

GfxMeshData GfxLoader::LoadObj(const std::string& path, bool flipV)
{
    if (path.empty())
        throw std::runtime_error("OBJ path must not be empty");
    tinyobj::attrib_t attributes;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> sourceMaterials;
    GfxMeshData mesh;
    std::string diagnostics;
    const auto resolvedPath = std::filesystem::absolute(path);
    MaterialReader materialReader(resolvedPath.parent_path());
    std::ifstream stream(resolvedPath);
    if (!stream)
        throw std::runtime_error("Could not load OBJ '" + path + "'");
    // The installed 1.x parser triangulates using triangle fans. Pre-triangulate
    // complex concave faces in the authoring tool.
    if (!tinyobj::LoadObj(&attributes, &shapes, &sourceMaterials, &diagnostics,
        &stream, &materialReader, true))
        throw std::runtime_error("Could not load OBJ '" + path + "': " + diagnostics);
    mesh.warnings = diagnostics;
    mesh.materials.push_back({ "fallback", glm::vec3(1.0f), {} });
    for (size_t i = 0; i < sourceMaterials.size(); ++i)
    {
        const auto& source = sourceMaterials[i];
        GfxSourceMaterial material;
        material.name = source.name;
        material.baseColor = glm::vec3(source.diffuse[0], source.diffuse[1], source.diffuse[2]);
        if (!IsFinite(material.baseColor) || glm::any(glm::lessThan(material.baseColor, glm::vec3(0))))
            throw std::runtime_error("Invalid MTL diffuse color in: " + path);
        if (!source.diffuse_texname.empty())
            material.baseColorTexture = (materialReader.directories[i] / source.diffuse_texname)
                .lexically_normal().generic_string();
        mesh.materials.push_back(std::move(material));
        if (source.dissolve != 1.0f || !source.alpha_texname.empty())
            mesh.warnings += "Transparency ignored for MTL material: " + source.name + "\n";
    }

    std::unordered_map<Vertex, uint32_t> uniqueVertices;
    size_t generatedNormalTriangles = 0, missingUvCorners = 0, skippedTriangles = 0;
    for (const auto& shape : shapes)
    {
        if (shape.mesh.indices.size() % 3 != 0)
            throw std::runtime_error("OBJ triangulation returned an incomplete triangle: " + path);
        const size_t shapeFirstSubmesh = mesh.submeshes.size();
        for (size_t triangle = 0; triangle < shape.mesh.indices.size(); triangle += 3)
        {
            std::array<Vertex, 3> corners{};
            for (size_t corner = 0; corner < corners.size(); ++corner)
            {
                const auto& sourceIndex = shape.mesh.indices[triangle + corner];
                corners[corner].position = ReadPosition(attributes, sourceIndex.vertex_index, path);
                // Optional -1 indices mean absent attributes. Never dereference them.
                if (sourceIndex.texcoord_index != -1)
                    CheckIndex(sourceIndex.texcoord_index, attributes.texcoords.size() / 2, "UV", path);
                if (sourceIndex.normal_index != -1)
                    CheckIndex(sourceIndex.normal_index, attributes.normals.size() / 3, "normal", path);
            }

            // Double intermediates avoid overflow for large finite source coordinates.
            const glm::dvec3 edge1 = glm::dvec3(corners[1].position) - glm::dvec3(corners[0].position);
            const glm::dvec3 edge2 = glm::dvec3(corners[2].position) - glm::dvec3(corners[0].position);
            const glm::dvec3 cross = glm::cross(edge1, edge2);
            const double areaSquared = glm::dot(cross, cross);
            if (areaSquared == 0.0)
            {
                ++skippedTriangles;
                continue;
            }
            const glm::vec3 faceNormal(cross / std::sqrt(areaSquared));
            const size_t face = triangle / 3;
            const int sourceMaterial = face < shape.mesh.material_ids.size() ? shape.mesh.material_ids[face] : -1;
            if (sourceMaterial < -1 || sourceMaterial >= static_cast<int>(sourceMaterials.size()))
                throw std::runtime_error("Invalid OBJ material index in: " + path);
            const uint32_t slot = static_cast<uint32_t>(sourceMaterial + 1);
            if (mesh.submeshes.size() == shapeFirstSubmesh || mesh.submeshes.back().materialSlot != slot)
                mesh.submeshes.push_back({ static_cast<uint32_t>(mesh.indices.size()), 0, slot, shape.name });
            bool generatedNormal = false;
            for (size_t corner = 0; corner < corners.size(); ++corner)
            {
                const auto& sourceIndex = shape.mesh.indices[triangle + corner];
                Vertex& vertex = corners[corner];
                vertex.color = glm::vec3(1.0f);
                if (sourceIndex.texcoord_index == -1)
                {
                    vertex.texCoord = glm::vec2(0.0f);
                    ++missingUvCorners;
                }
                else
                {
                    const size_t offset = static_cast<size_t>(sourceIndex.texcoord_index) * 2;
                    const float u = attributes.texcoords[offset];
                    const float v = attributes.texcoords[offset + 1];
                    if (!std::isfinite(u) || !std::isfinite(v))
                        throw std::runtime_error("Non-finite OBJ UV in: " + path);
                    vertex.texCoord = glm::vec2(u, flipV ? 1.0f - v : v);
                }

                vertex.normal = faceNormal;
                if (sourceIndex.normal_index != -1)
                {
                    const size_t offset = static_cast<size_t>(sourceIndex.normal_index) * 3;
                    const glm::vec3 normal(attributes.normals[offset], attributes.normals[offset + 1], attributes.normals[offset + 2]);
                    if (!IsFinite(normal))
                        throw std::runtime_error("Non-finite OBJ normal in: " + path);
                    const glm::dvec3 preciseNormal(normal);
                    const double lengthSquared = glm::dot(preciseNormal, preciseNormal);
                    if (lengthSquared > 0.0)
                        vertex.normal = glm::vec3(preciseNormal / std::sqrt(lengthSquared));
                    else
                        generatedNormal = true;
                }
                else
                    generatedNormal = true;

                // Position alone cannot identify a GPU vertex: retain UV seams
                // and hard edges by including the complete Vertex in equality.
                auto found = uniqueVertices.find(vertex);
                if (found == uniqueVertices.end())
                {
                    if (mesh.vertices.size() >= (std::numeric_limits<uint32_t>::max)())
                        throw std::runtime_error("OBJ exceeds 32-bit vertex indexing: " + path);
                    const uint32_t index = static_cast<uint32_t>(mesh.vertices.size());
                    mesh.vertices.push_back(vertex);
                    found = uniqueVertices.emplace(vertex, index).first;
                }
                if (mesh.indices.size() >= (std::numeric_limits<uint32_t>::max)())
                    throw std::runtime_error("OBJ exceeds the supported index count: " + path);
                mesh.indices.push_back(found->second);
                ++mesh.submeshes.back().indexCount;
            }
            if (generatedNormal)
                ++generatedNormalTriangles;
        }
    }
    if (mesh.indices.empty())
        throw std::runtime_error("OBJ contains no non-degenerate triangles: " + path);
    if (!mesh.warnings.empty() && mesh.warnings.back() != '\n')
        mesh.warnings += '\n';
    if (generatedNormalTriangles != 0)
        mesh.warnings += "Generated flat normals for " + std::to_string(generatedNormalTriangles) + " triangles.\n";
    if (missingUvCorners != 0)
        mesh.warnings += "Missing UVs on " + std::to_string(missingUvCorners) + " corners; using (0,0).\n";
    if (skippedTriangles != 0)
        mesh.warnings += "Skipped " + std::to_string(skippedTriangles) + " degenerate triangles.\n";
    return mesh;
}
