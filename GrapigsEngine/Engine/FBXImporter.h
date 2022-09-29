#pragma once
/*
 *	Author		: Jina Hyun
 *	Date		: 09/29/22
 *	File Name	: FBXImporter.h
 *	Desc		: Import fbx
 */
#include <fbxsdk.h>	// Fbx variables and functions
#include <vector>	// std::vector
#include <glm/glm.hpp>	// glm

struct Attribute
{
	glm::vec4 position{};
	glm::vec4 vertex_normal{};
	glm::vec4 face_normal{};
};

struct Polygon
{
	std::string name{};
	glm::mat4 transform{ 1 };
	std::vector<Attribute> attributes;
	std::vector<int> childrenIndex;
	int parentIndex = -1;
};

struct Dummy
{
	std::vector<Polygon> polygons;
	int rootIndex;
	std::string name;
};

class FBXImporter
{
public:
	static std::vector<Dummy*> Load(const char* file_path) noexcept;
	static FbxScene* ImportFbx(FbxManager* p_manager, const char* file_path) noexcept;
private:
	static std::vector<Dummy*> Parse(FbxNode* p_root) noexcept;
	static int ParseNode(FbxNode* p_node, int parent, std::vector<Polygon>& polygons) noexcept;
	static std::vector<Attribute> GetAttribute(const FbxMesh* p_mesh) noexcept;
};