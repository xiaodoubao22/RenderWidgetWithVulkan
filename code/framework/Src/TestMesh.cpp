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
#include "Utils.h"

namespace std {
template<> struct hash<framework::Vertex3D> {
	size_t operator()(framework::Vertex3D const& vertex) const {
		return (hash<glm::vec3>()(vertex.position) ^ (hash<glm::vec2>()(vertex.texCoord) << 1)) >> 1;
	}
};
}	// namespace std

namespace framework {
TestMesh::TestMesh() {}
TestMesh::~TestMesh() {}

void TestMesh::LoadFromFile(std::string& path)
{
	mVertices.clear();
	mIndexes.clear();

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

void TestMesh::GenerateSphere(float radius, glm::vec3 center, glm::uvec2 gridNum)
{
	mVertices.clear();
	mIndexes.clear();

	int gridNumU = gridNum.x;
	int gridNumV = gridNum.y;

	for (int j = 0; j <= gridNumV; j++) {
		for (int i = 0; i <= gridNumU; i++) {
			float u = static_cast<float>(i) / gridNumU;
			float v = static_cast<float>(j) / gridNumV;

			Vertex3D vertex{};
			vertex.texCoord = { u, 1.0f - v };
			vertex.texCoord *= 2.0;

			float phy = v * consts::FLT_PI;
			float theta = u * consts::FLT_PI * 2;
			vertex.position.x = radius * std::sin(phy) * std::cos(theta);
			vertex.position.y = radius * std::sin(phy) * std::sin(theta);
			vertex.position.z = radius * std::cos(phy);

			vertex.normal = glm::normalize(vertex.position - center);

			mVertices.emplace_back(vertex);
		}
	}

	for (int j = 0; j < gridNumV; j++) {
		for (int i = 0; i < gridNumU; i++) {
			int x = j * (gridNumU + 1) + i;
			int leftUp = x;
			int leftDown = x + gridNumU + 1;
			int rightUp = leftUp + 1;
			int rightDown = leftDown + 1;

			mIndexes.push_back(rightUp);
			mIndexes.push_back(leftUp);
			mIndexes.push_back(rightDown);

			mIndexes.push_back(leftDown);
			mIndexes.push_back(rightDown);
			mIndexes.push_back(leftUp);
		}
	}

}
}   // namespace framework

