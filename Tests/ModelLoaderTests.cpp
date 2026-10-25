#include "../ModelLoader.h"
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>

static void Require(bool condition, const char* message)
{
    if (!condition)
        throw std::runtime_error(message);
}

class ObjFixtures
{
public:
    ObjFixtures()
    {
        directory = std::filesystem::temp_directory_path() / ("AEngineObjTests-"
            + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        Require(std::filesystem::create_directory(directory), "Could not create OBJ fixture directory");
    }
    ~ObjFixtures()
    {
        std::error_code ignored;
        for (const auto& file : files)
            std::filesystem::remove(file, ignored);
        for (const auto& folder : folders)
            std::filesystem::remove(folder, ignored);
        std::filesystem::remove(directory, ignored);
    }
    std::string Write(const char* name, const std::string& contents)
    {
        const auto file = directory / name;
        if (file.parent_path() != directory && std::filesystem::create_directory(file.parent_path()))
            folders.push_back(file.parent_path());
        files.push_back(file);
        std::ofstream stream(file);
        stream << contents;
        Require(stream.good(), "Could not write OBJ fixture");
        return file.string();
    }
    std::filesystem::path directory;
private:
    std::vector<std::filesystem::path> files;
    std::vector<std::filesystem::path> folders;
};

static void CheckMesh(const GfxMeshData& mesh)
{
    Require(!mesh.vertices.empty() && mesh.indices.size() % 3 == 0, "Invalid triangle mesh result");
    size_t nextIndex = 0;
    for (const auto& submesh : mesh.submeshes)
    {
        Require(submesh.firstIndex == nextIndex && submesh.indexCount > 0 && submesh.indexCount % 3 == 0,
            "Invalid submesh range");
        Require(submesh.materialSlot < mesh.materials.size(), "Invalid material slot");
        nextIndex += submesh.indexCount;
    }
    Require(nextIndex == mesh.indices.size(), "Submeshes do not cover the mesh");
    for (uint32_t index : mesh.indices)
        Require(index < mesh.vertices.size(), "Imported index out of range");
    for (const Vertex& vertex : mesh.vertices)
    {
        Require(std::isfinite(vertex.position.x) && std::isfinite(vertex.position.y)
            && std::isfinite(vertex.position.z), "Non-finite position");
        Require(std::abs(glm::length(vertex.normal) - 1.0f) < 0.0001f, "Normal is not finite and normalized");
        Require(vertex.color == glm::vec3(1.0f), "Material texture should not be tinted by imported vertex colors");
    }
}

static void ExpectFailure(const std::string& path, const std::string& expected)
{
    try { GfxLoader::LoadObj(path); }
    catch (const std::exception& error)
    {
        Require(std::string(error.what()).find(expected) != std::string::npos, "Unexpected OBJ failure diagnostic");
        return;
    }
    throw std::runtime_error("Invalid OBJ was accepted: " + path);
}

int main()
{
    try
    {
        ObjFixtures fixtures;
        const std::string trianglePositions = "v 10 20 30\nv 11 20 30\nv 10 21 30\n";
        const std::string triangle = fixtures.Write("triangle.obj", trianglePositions + "f 1 2 3\n");
        const GfxMeshData flat = GfxLoader::LoadObj(triangle);
        CheckMesh(flat);
        Require(flat.vertices.size() == 3 && flat.indices.size() == 3, "Unexpected triangle topology");
        Require(flat.vertices[0].position == glm::vec3(10, 20, 30), "Import baked a transform into the mesh");
        Require(flat.vertices[0].normal == glm::vec3(0, 0, 1), "Incorrect fallback normal/winding");
        Require(flat.vertices[0].texCoord == glm::vec2(0), "Missing UV fallback is not zero");
        Require(!flat.warnings.empty(), "Missing attributes were not reported");
        const GfxMeshData again = GfxLoader::LoadObj(triangle);
        Require(again.vertices.size() == 3 && again.indices.size() == 3, "Repeated loads accumulated old data");

        const std::string quadAttributes = "v 0 0 0\nv 1 0 0\nv 1 1 0\nv 0 1 0\n"
            "vt 0.1 0.2\nvt 1 0\nvt 1 1\nvt 0 1\nvn 0 0 2\n";
        const std::string quad = fixtures.Write("quad.obj", quadAttributes + "f 1/1/1 2/2/1 3/3/1 4/4/1\n");
        const GfxMeshData flipped = GfxLoader::LoadObj(quad);
        const GfxMeshData unflipped = GfxLoader::LoadObj(quad, false);
        CheckMesh(flipped);
        Require(flipped.vertices.size() == 4 && flipped.indices.size() == 6, "Convex quad was not triangulated/deduplicated");
        Require(std::abs(flipped.vertices[0].texCoord.y - 0.8f) < 0.0001f
            && std::abs(unflipped.vertices[0].texCoord.y - 0.2f) < 0.0001f, "UV flip option failed");

        const auto uvSeam = GfxLoader::LoadObj(fixtures.Write("uvseam.obj", quadAttributes
            + "vt 0.5 0.5\nf 1/1/1 2/2/1 3/3/1\nf 1/5/1 3/3/1 4/4/1\n"));
        Require(uvSeam.vertices.size() == 5, "UV seam was incorrectly merged");
        CheckMesh(uvSeam);
        const auto normalSeam = GfxLoader::LoadObj(fixtures.Write("normalseam.obj", trianglePositions
            + "vn 0 0 1\nvn 0 0 -1\nf 1//1 2//1 3//1\nf 3//2 2//2 1//2\n"));
        Require(normalSeam.vertices.size() == 6, "Different explicit normals were incorrectly merged");
        CheckMesh(normalSeam);
        const auto hardEdge = GfxLoader::LoadObj(fixtures.Write("hardedge.obj",
            "v 0 0 0\nv 1 0 0\nv 0 1 0\nv 0 0 1\nf 1 2 3\nf 1 4 2\n"));
        Require(hardEdge.vertices.size() == 6, "Generated hard-edge normals were incorrectly merged");
        CheckMesh(hardEdge);

        const auto negative = GfxLoader::LoadObj(fixtures.Write("negative.obj", trianglePositions + "f -3 -2 -1\n"));
        CheckMesh(negative);
        Require(negative.vertices[0].position == flat.vertices[0].position, "Negative OBJ indices failed");
        const auto zeroNormals = GfxLoader::LoadObj(fixtures.Write("zero.obj", trianglePositions
            + "vn 0 0 0\nf 1//1 2//1 3//1\n"));
        CheckMesh(zeroNormals);
        const auto multipleShapes = GfxLoader::LoadObj(fixtures.Write("shapes.obj", trianglePositions
            + "o first\nf 1 2 3\no second\nf 1 2 3\n"));
        Require(multipleShapes.indices.size() == 6, "Multiple OBJ shapes were not flattened");
        Require(multipleShapes.submeshes.size() == 2, "Shape boundaries were not preserved");
        fixtures.Write("materials.mtl", "newmtl red\nKd 0.8 0.1 0.2\nmap_Kd color.png\n"
            "newmtl green\nKd 0.1 0.9 0.2\n");
        const auto multi = GfxLoader::LoadObj(fixtures.Write("multi.obj", "mtllib materials.mtl\n"
            + trianglePositions + "f 1 2 3\nusemtl red\nf 1 2 3\nusemtl green\nf 1 2 3\n"
            "usemtl red\nf 1 2 3\n"));
        CheckMesh(multi);
        Require(multi.materials.size() == 3 && multi.submeshes.size() == 4, "Material runs were lost");
        Require(multi.submeshes[0].materialSlot == 0 && multi.submeshes[1].materialSlot == 1
            && multi.submeshes[2].materialSlot == 2 && multi.submeshes[3].materialSlot == 1,
            "Incorrect usemtl/fallback mapping");
        Require(multi.materials[1].baseColor == glm::vec3(0.8f, 0.1f, 0.2f), "Kd was not imported");
        Require(multi.materials[1].baseColorTexture == (fixtures.directory / "color.png").generic_string(),
            "map_Kd path was not resolved");
        Require(multi.vertices.size() == 3, "Material changes duplicated identical vertices");
        const auto adjacent = GfxLoader::LoadObj(fixtures.Write("adjacent.obj", "mtllib materials.mtl\n"
            + trianglePositions + "usemtl red\nf 1 2 3\nf 1 1 2\nf 1 2 3\n"));
        Require(adjacent.submeshes.size() == 1 && adjacent.submeshes[0].indexCount == 6,
            "Adjacent material faces were not combined or degenerate range leaked");
        const auto unknown = GfxLoader::LoadObj(fixtures.Write("unknown.obj", trianglePositions
            + "usemtl nonexistent\nf 1 2 3\n"));
        Require(unknown.submeshes[0].materialSlot == 0, "Unknown material did not select fallback");
        fixtures.Write("nested/surface.mtl", "newmtl nested\nKd 1 1 1\nmap_Kd ../nested_color.png\n");
        const auto nested = GfxLoader::LoadObj(fixtures.Write("nested_material.obj",
            "mtllib nested/surface.mtl\n" + trianglePositions + "usemtl nested\nf 1 2 3\n"));
        Require(nested.materials[1].baseColorTexture == (fixtures.directory / "nested_color.png").generic_string(),
            "Texture path was not relative to its MTL directory");
        const auto panels = GfxLoader::LoadObj("Models/material_test.obj");
        CheckMesh(panels);
        Require(panels.submeshes.size() == 3 && panels.indices.size() == 18 && panels.materials.size() == 4,
            "Material panel asset topology is incorrect");
        const auto degenerate = GfxLoader::LoadObj(fixtures.Write("degenerate.obj", trianglePositions
            + "f 1 2 3\nf 1 1 2\n"));
        Require(degenerate.indices.size() == 3 && !degenerate.warnings.empty(), "Degenerate triangle was not skipped");

        ExpectFailure("", "empty");
        ExpectFailure((fixtures.directory / "missing.obj").string(), "Could not load");
        ExpectFailure(fixtures.Write("empty.obj", "# no geometry\n"), "no non-degenerate");
        ExpectFailure(fixtures.Write("onlydegenerate.obj", trianglePositions + "f 1 1 2\n"), "no non-degenerate");
        ExpectFailure(fixtures.Write("badposition.obj", trianglePositions + "f 1 2 99\n"), "position");
        ExpectFailure(fixtures.Write("baduv.obj", trianglePositions + "vt 0 0\nf 1/99 2/1 3/1\n"), "UV");
        ExpectFailure(fixtures.Write("badnormal.obj", trianglePositions + "vn 0 0 1\nf 1//99 2//1 3//1\n"), "normal");
        ExpectFailure(fixtures.Write("nonfinite.obj", "v 1e40 0 0\nv 0 1 0\nv 0 0 1\nf 1 2 3\n"), "Non-finite");

        const auto room = GfxLoader::LoadObj(MODEL_PATH);
        CheckMesh(room);
        std::cout << "Viking Room: " << room.vertices.size() << " vertices, " << room.indices.size() / 3 << " triangles\n";
        std::cout << "PASS: CPU OBJ import, submesh ranges, MTL slots/Kd/paths, attributes, seams, invalid inputs.\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
}
