#include "TestMesh.h"

#include <unordered_map>

#ifndef TINYOBJLOADER_IMPLEMENTATION
#define TINYOBJLOADER_IMPLEMENTATION
#endif // !TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#ifndef GLM_ENABLE_EXPERIMENTAL
#define GLM_ENABLE_EXPERIMENTAL
#endif // !GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/hash.hpp"

#include "Log.h"

namespace std {
template<> struct hash<render::Vertex3D> {
	size_t operator()(render::Vertex3D const& vertex) const {
		return (hash<glm::vec3>()(vertex.position) ^ (hash<glm::vec2>()(vertex.texCoord) << 1)) >> 1;
	}
};
}	// namespace std

namespace render {
TestMesh::TestMesh() {}
TestMesh::~TestMesh() {}

bool TestMesh::LoadFromFile(std::string& path)
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str())) {
		throw std::runtime_error(warn + err);
	}

	LOGI("attrib.vertices = %d", attrib.vertices.size());

	std::unordered_map<Vertex3D, uint32_t> uniqueVertices{};

	for (auto& shape : shapes) {
		for (const auto& index : shape.mesh.indices) {
			Vertex3D vertex{};
			vertex.position = {
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};

			vertex.texCoord = {
				attrib.texcoords[2 * index.texcoord_index + 0],
				attrib.texcoords[2 * index.texcoord_index + 1]
			};

			if (uniqueVertices.count(vertex) == 0) {
				uniqueVertices[vertex] = static_cast<uint32_t>(mVertices.size());
				mVertices.push_back(vertex);
			}
			mIndexes.push_back(uniqueVertices[vertex]);
		}
	}

	LOGI("mVertices.size() = %d", mVertices.size());
}
}   // namespace render

