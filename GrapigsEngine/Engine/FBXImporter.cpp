#include "FBXImporter.h"
/*
 *	Author		: Jina Hyun
 *	Date		: 09/29/22
 *	File Name	: FBXImporter.h
 *	Desc		: Import fbx
 */
#include <iostream>	// std::cerr
#include <map>	// std::map
#include <glm/gtc/matrix_transform.hpp> // transform matrix calculation

namespace ParseHelper
{
	glm::mat4 GetTransformMatrix(FbxNode* p_node) noexcept
	{
		const auto& translation = p_node->LclTranslation.Get();
		const auto& rotation = p_node->LclRotation.Get();
		const auto& scaling = p_node->LclScaling.Get();

		glm::mat4 transform{ 1 };
		transform = glm::scale(glm::mat4(1), glm::vec3{ scaling[0], scaling[1], scaling[2] });
		transform = glm::rotate(transform, glm::radians(static_cast<float>(rotation[0])), glm::vec3{ 1, 0, 0 });
		transform = glm::rotate(transform, glm::radians(static_cast<float>(rotation[1])), glm::vec3{ 0, 1, 0 });
		transform = glm::rotate(transform, glm::radians(static_cast<float>(rotation[2])), glm::vec3{ 0, 0, 1 });
		transform = glm::translate(transform, glm::vec3{ translation[0], translation[1], translation[2] });

		return transform;
	}

	glm::vec3 GetVertexNormal(const FbxMesh* p_mesh, int poly, int vert) noexcept
	{
		FbxVector4 normal;
		p_mesh->GetPolygonVertexNormal(poly, vert, normal);
		return glm::vec3{ normal[0], normal[1],normal[2] };
	}
}

std::vector<Dummy*> FBXImporter::Load(const char* file_path) noexcept
{
	FbxManager* p_manager = FbxManager::Create();
	FbxIOSettings* ios = FbxIOSettings::Create(p_manager, IOSROOT);
	p_manager->SetIOSettings(ios);
	const FbxScene* p_scene = ImportFbx(p_manager, file_path);
	FbxNode* pRoot = p_scene->GetRootNode();
	auto dummies = Parse(pRoot);
	p_manager->Destroy();
	return dummies;
}

FbxScene* FBXImporter::ImportFbx(FbxManager* p_manager, const char* file_path) noexcept
{
	FbxImporter* importer = FbxImporter::Create(p_manager, "Importer");
	if (!importer->Initialize(file_path, -1, p_manager->GetIOSettings()))
	{
		std::cerr << "[FBX SDK]: Call to FbxImporter::Initialize() failed." << std::endl;
		std::cerr << "Error returned: " << importer->GetStatus().GetErrorString() << std::endl;
		return nullptr;
	}
	FbxScene* scene = FbxScene::Create(p_manager, "Scene Name");
	importer->Import(scene);
	importer->Destroy();
	return scene;
}

std::vector<Dummy*> FBXImporter::Parse(FbxNode* p_root) noexcept
{
	std::vector<Dummy*> dummies;
	if (p_root)
	{
		for (int i = 0; i < p_root->GetChildCount(); i++)
		{
			Dummy* dummy = new Dummy();
			dummy->rootIndex = ParseNode(p_root->GetChild(i), -1, dummy->polygons);
			dummy->polygons[dummy->rootIndex].transform = glm::mat4{ 1 };
			dummy->name	 = p_root->GetName();
			dummies.push_back(dummy);
		}
	}
	return dummies;
}

int FBXImporter::ParseNode(FbxNode* p_node, int parent, std::vector<Polygon>& polygons) noexcept
{
	const int index = static_cast<int>(polygons.size());
	polygons.emplace_back(Polygon());
	Polygon polygon{};
	polygon.name = p_node->GetName();

	for (int i = 0; i < p_node->GetNodeAttributeCount(); i++)
	{
		if (const FbxNodeAttribute* attribute = p_node->GetNodeAttributeByIndex(i))
		{
			switch (attribute->GetAttributeType())
			{
			case FbxNodeAttribute::eMesh:
			{
				// Read vertex, normal, uv data
				polygon.attributes = GetAttribute(p_node->GetMesh());
			}
			break;
			default:
				break;
			}
		}
	}

	// Read transform data
	polygon.transform = ParseHelper::GetTransformMatrix(p_node);

	// Set parent/children
	polygon.parentIndex = parent;

	// Read transform data
	for (int child = 0; child < p_node->GetChildCount(); ++child)
	{
		const int child_index = ParseNode(p_node->GetChild(child), index, polygons);
		polygon.childrenIndex.push_back(child_index);
	}
	polygons[index] = polygon;
	return index;
}

std::vector<Attribute> FBXImporter::GetAttribute(const FbxMesh* p_mesh) noexcept
{
	std::vector<glm::vec3> ctrl_pts;
	std::vector<glm::vec4> face_normal;
	std::vector<int> indices;
	std::map<int, glm::vec4> vertex_normal;

	for (int vert = 0; vert < p_mesh->GetControlPointsCount(); ++vert)
	{
		const auto& pos = p_mesh->GetControlPointAt(vert);
		ctrl_pts.emplace_back(glm::vec3{ pos[0], pos[1], pos[2] });
	}

	for (int poly = 0; poly < p_mesh->GetPolygonCount(); ++poly)
	{
		const int vert_cnt = p_mesh->GetPolygonSize(poly);

		if (vert_cnt < 3)
		{
			for (int vert = 0; vert < vert_cnt; ++vert)
			{
				const int index = p_mesh->GetPolygonVertex(poly, vert);
				indices.push_back(index);
				face_normal.emplace_back(glm::vec4{ 0 });
				auto find = vertex_normal.find(index);
				if (find == vertex_normal.end())
					vertex_normal[index] = glm::vec4{ ParseHelper::GetVertexNormal(p_mesh, poly, vert), 1 };
				else
					find->second += glm::vec4{ ParseHelper::GetVertexNormal(p_mesh, poly, vert) , 1 };
			}
		}
		else
		{
			// Vertex normal
			for (int vert = 0; vert < vert_cnt; ++vert)
			{
				const int index = p_mesh->GetPolygonVertex(poly, vert);
				auto find = vertex_normal.find(index);
				if (find == vertex_normal.end())
					vertex_normal[index] = glm::vec4{ ParseHelper::GetVertexNormal(p_mesh, poly, vert), 1 };
				else
					find->second += glm::vec4{ ParseHelper::GetVertexNormal(p_mesh, poly, vert) , 1 };
			}

			// Vertex
			const int p0 = p_mesh->GetPolygonVertex(poly, 0);
			int p1 = p_mesh->GetPolygonVertex(poly, 1);
			int p2 = p_mesh->GetPolygonVertex(poly, 2);

			// Face normal
			const glm::vec4& face_norm = glm::normalize(glm::vec4{ glm::cross(ctrl_pts[p1] - ctrl_pts[p0], ctrl_pts[p2] - ctrl_pts[p0]), 0 });

			for (int vert = 2; vert < vert_cnt; ++vert)
			{
				// Vertex
				indices.push_back(p0);
				indices.push_back(p1);
				indices.push_back(p2);

				// Face normal
				face_normal.push_back(face_norm);
				face_normal.push_back(face_norm);
				face_normal.push_back(face_norm);

				p1 = p2;
				p2 = p_mesh->GetPolygonVertex(poly, vert + 1);
			}
		}
	}

	// Calculate Vertex Normal
	for (auto& [key, v] : vertex_normal)
		v = glm::vec4{ v.x / v.w, v.y / v.w, v.z / v.w, 0 };

	std::vector<Attribute> attrib;
	attrib.reserve(indices.size());
	for (int i = 0; i < static_cast<int>(indices.size()); ++i)
	{
		const glm::vec4 vertex{ ctrl_pts[indices[i]], 1 };
		attrib.emplace_back(Attribute{ vertex, vertex_normal[indices[i]], face_normal[i] });
	}

	return attrib;
}
