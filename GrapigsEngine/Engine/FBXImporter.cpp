#include "FBXImporter.h"
/*
 *	Author		: Jina Hyun
 *	Date		: 09/29/22
 *	File Name	: FBXImporter.h
 *	Desc		: Import fbx
 */
#include <filesystem>	// std::filesystem
#include <iostream>	// std::cerr
#include <map>	// std::map
#include <gl/glew.h>
#include <glm/gtc/matrix_transform.hpp> // transform matrix calculation
#include <fbxsdk/scene/shading/fbxsurfacematerial.h>
MeshGroup::~MeshGroup()
{
	Clear();
}

void MeshGroup::InitBuffers(int largest_index) noexcept
{
	if (largest_index < 0)
		return;

	if (m_vbo == 0)
		glCreateBuffers(1, &m_vbo);
	glNamedBufferStorage(m_vbo, static_cast<GLsizeiptr>(sizeof(Vertex) * m_meshes[largest_index].vertices.size()), nullptr, GL_DYNAMIC_STORAGE_BIT);

	if (m_vao == 0)
		glCreateVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	// Vertex Position
	glEnableVertexArrayAttrib(m_vao, 0);
	glVertexArrayVertexBuffer(m_vao, 0, m_vbo, 0, sizeof(Vertex));
	glVertexArrayAttribFormat(m_vao, 0, 4, GL_FLOAT, GL_FALSE, offsetof(Vertex, position));
	glVertexArrayAttribBinding(m_vao, 0, 0);

	// Vertex Normal
	glEnableVertexArrayAttrib(m_vao, 1);
	glVertexArrayAttribFormat(m_vao, 1, 4, GL_FLOAT, GL_FALSE, offsetof(Vertex, vertex_normal));
	glVertexArrayAttribBinding(m_vao, 1, 0);

	// Face Normal
	glEnableVertexArrayAttrib(m_vao, 2);
	glVertexArrayAttribFormat(m_vao, 2, 4, GL_FLOAT, GL_FALSE, offsetof(Vertex, face_normal));
	glVertexArrayAttribBinding(m_vao, 2, 0);

	glBindVertexArray(0);
}

void MeshGroup::Clear() noexcept
{
	if(m_vao > 0)
		glDeleteVertexArrays(1, &m_vao);
	m_vao = 0;
	if(m_vbo > 0)
		glDeleteBuffers(1, &m_vbo);
	m_vbo = 0;
}

void MeshGroup::Draw(Primitive primitive, ShaderProgram* program) noexcept
{
	if (m_vao)
	{
		glBindVertexArray(m_vao);
		Draw(primitive, program, m_root, glm::mat4{ 1 });
		glBindVertexArray(0);
	}
}

void MeshGroup::Draw(Primitive primitive, ShaderProgram* program, int index, glm::mat4 transform) const noexcept
{
	const auto& mesh = m_meshes[index];
	const auto& vertex = mesh.vertices;

	if (vertex.empty() == false)
	{
		program->SendUniform("u_localToModel", mesh.transform);

		program->SendUniform("o_ambient", mesh.material.ambient);
		program->SendUniform("o_diffuse", mesh.material.diffuse);
		program->SendUniform("o_specular", mesh.material.specular);

		glNamedBufferSubData(m_vbo, 0, static_cast<GLsizeiptr>(sizeof(Vertex) * vertex.size()), vertex.data());
		glDrawArrays(static_cast<GLenum>(primitive), 0, static_cast<GLsizei>(vertex.size()));
	}

	for (const auto& c : m_meshes[index].children)
	{
		Draw(primitive, program, c, transform * mesh.transform);
	}
}

/* MeshGroup - end ------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------*/
/* ParseHelper - start --------------------------------------------------------------------------*/

std::ostream& operator<<(std::ostream& os, const glm::vec3& v)
{
	return os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
}

std::ostream& operator<<(std::ostream& os, const FbxDouble3& val)
{
	return os << "(" << val[0] << ", " << val[1] << ", " << val[2] << ")";
}

namespace ParseHelper
{
	glm::mat4 GetTransformMatrix(FbxNode* p_node) noexcept
	{
		const auto& translation = p_node->LclTranslation.Get();
		const auto& rotation = p_node->LclRotation.Get();
		const auto& scaling = p_node->LclScaling.Get();

		glm::mat4 transform{ 1 };
		transform = glm::translate(transform, glm::vec3{ translation[0], translation[1], translation[2] } * 0.01f);
		transform = glm::rotate(transform, glm::radians(static_cast<float>(rotation[0])), glm::vec3{ 1, 0, 0 });
		transform = glm::rotate(transform, glm::radians(static_cast<float>(rotation[1])), glm::vec3{ 0, 1, 0 });
		transform = glm::rotate(transform, glm::radians(static_cast<float>(rotation[2])), glm::vec3{ 0, 0, 1 });
		transform = glm::scale(transform, glm::vec3{ scaling[0], scaling[1], scaling[2] }  * 0.01f);

		return transform;
	}

	glm::vec3 GetVertexNormal(const FbxMesh* p_mesh, int poly, int vert) noexcept
	{
		FbxVector4 normal;
		p_mesh->GetPolygonVertexNormal(poly, vert, normal);
		return glm::vec3{ normal[0], normal[1],normal[2] };
	}
}


/* ParseHelper - end-----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------*/
/* FBXNodePrinter - start -----------------------------------------------------------------------*/


int FBXNodePrinter::s_tab = 0;

constexpr std::string FBXNodePrinter::GetAttributeTypeName(const FbxNodeAttribute::EType type) noexcept
{
	switch (type)
	{
	case FbxNodeAttribute::eUnknown: return "unidentified";
	case FbxNodeAttribute::eNull: return "null";
	case FbxNodeAttribute::eMarker: return "marker";
	case FbxNodeAttribute::eSkeleton: return "skeleton";
	case FbxNodeAttribute::eMesh: return "mesh";
	case FbxNodeAttribute::eNurbs: return "nurbs";
	case FbxNodeAttribute::ePatch: return "patch";
	case FbxNodeAttribute::eCamera: return "camera";
	case FbxNodeAttribute::eCameraStereo: return "stereo";
	case FbxNodeAttribute::eCameraSwitcher: return "camera switcher";
	case FbxNodeAttribute::eLight: return "light";
	case FbxNodeAttribute::eOpticalReference: return "optical reference";
	case FbxNodeAttribute::eOpticalMarker: return "marker";
	case FbxNodeAttribute::eNurbsCurve: return "nurbs curve";
	case FbxNodeAttribute::eTrimNurbsSurface: return "trim nurbs surface";
	case FbxNodeAttribute::eBoundary: return "boundary";
	case FbxNodeAttribute::eNurbsSurface: return "nurbs surface";
	case FbxNodeAttribute::eShape: return "shape";
	case FbxNodeAttribute::eLODGroup: return "lodgroup";
	case FbxNodeAttribute::eSubDiv: return "suddiv";
	case FbxNodeAttribute::eCachedEffect: return "cached effect";
	case FbxNodeAttribute::eLine: return "line";
	}
	return "unknown";
}

void FBXNodePrinter::Print(const FbxNode* p_root) noexcept
{
	s_tab = 0;
	if (p_root)
	{
		for (int i = 0; i < p_root->GetChildCount(); i++)
			PrintNode(p_root->GetChild(i));
	}
}

void FBXNodePrinter::PrintAttribute(const FbxNodeAttribute* p_attribute) noexcept
{
	if (!p_attribute)
		return;
	PrintTabs();
	std::cout << "<attribute type=" << GetAttributeTypeName(p_attribute->GetAttributeType())
		<< " name=" << p_attribute->GetName() << ">" << std::endl;
}

void FBXNodePrinter::PrintNode(const FbxNode* p_node) noexcept
{
	// Print the contents of the node
	PrintTabs();
	std::cout << "<node-name=" << p_node->GetName()
		<< " translation=" << p_node->LclTranslation.Get()
		<< " rotation=" << p_node->LclRotation.Get()
		<< " scaling=" << p_node->LclScaling.Get() << ">" << std::endl;
	s_tab++;

	for (int i = 0; i < p_node->GetNodeAttributeCount(); i++)
		PrintAttribute(p_node->GetNodeAttributeByIndex(i));

	for (int j = 0; j < p_node->GetChildCount(); j++)
		PrintNode(p_node->GetChild(j));

	s_tab--;
	PrintTabs();
	std::cout << "</node>\n" << std::endl;
}

void FBXNodePrinter::PrintTabs() noexcept
{
	for (int i = 0; i < s_tab; ++i)
		std::cout << "\t";
}

/* FBXNodePrinter - end -------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------*/
/* FBXImporter - start --------------------------------------------------------------------------*/

std::size_t FBXImporter::s_m_verticesCount = 0;
int FBXImporter::s_m_meshIndex = -1;

std::vector<MeshGroup*> FBXImporter::Load(const char* file_path) noexcept
{
	const std::filesystem::path path(file_path);
	if (std::filesystem::exists(file_path) == false)
	{
		std::cout << "[FBXImporter]: " << file_path << " does not exist." << std::endl;
		return std::vector<MeshGroup*>{};
	}
	if (path.extension() != ".fbx")
	{
		std::cout << "[FBXImporter]: Unable to parse " << file_path << std::endl;
		return std::vector<MeshGroup*>{};
	}

	// Initialize the SDK manager
	FbxManager* p_manager = FbxManager::Create();

	// Create the IO settings object
	FbxIOSettings* ios = FbxIOSettings::Create(p_manager, IOSROOT);
	p_manager->SetIOSettings(ios);

	const FbxScene* p_scene = ImportFbx(p_manager, file_path);

	// Print the node of the scene and their attributes recursively
	// Note that we are not printing the root node because it should not contain any attributes
	FbxNode* pRoot = p_scene->GetRootNode();
	FBXNodePrinter::Print(pRoot);

	auto mesh_groups = Parse(pRoot);

	// Destroy the SDK manager and all the other objects is was handling
	p_manager->Destroy();

	return mesh_groups;
}

FbxScene* FBXImporter::ImportFbx(FbxManager* p_manager, const char* file_path) noexcept
{
	// Create an importer using the SDK manager
	FbxImporter* importer = FbxImporter::Create(p_manager, "Importer");
	if (!importer->Initialize(file_path, -1, p_manager->GetIOSettings()))
	{
		std::cerr << "[FBX SDK]: Call to FbxImporter::Initialize() failed." << std::endl;
		std::cerr << "Error returned: " << importer->GetStatus().GetErrorString() << std::endl;
		return nullptr;
	}
	// Create a new scene so that it can be populated by the imported file
	FbxScene* scene = FbxScene::Create(p_manager, "New Scene");
	// Import the contents of the file into scene
	importer->Import(scene);
	// The file is imported, so get rid of the importer
	importer->Destroy();
	return scene;
}

std::vector<MeshGroup*> FBXImporter::Parse(FbxNode* p_root) noexcept
{
	std::vector<MeshGroup*> mesh_groups;
	if (p_root)
	{
		for (int i = 0; i < p_root->GetChildCount(); i++)
		{
			s_m_meshIndex = -1;
			s_m_verticesCount = 0;

			MeshGroup* mesh_group = new MeshGroup();
			mesh_group->m_root = ParseNode(p_root->GetChild(i), -1, mesh_group->m_meshes);
			mesh_group->m_name = p_root->GetName();
			mesh_group->InitBuffers(s_m_meshIndex);
			mesh_groups.push_back(mesh_group);
		}
	}
	return mesh_groups;
}

int FBXImporter::ParseNode(FbxNode* p_node, int parent, std::vector<Mesh>& meshes) noexcept
{
	const int index = static_cast<int>(meshes.size());
	meshes.emplace_back(Mesh());
	Mesh mesh{};
	mesh.name = p_node->GetName();
	// TODO: Need to find applicable fbx file to test this.
	//int mcount = p_node->GetSrcObjectCount<FbxSurfaceMaterial>();
	//for (int i = 0; i < mcount; i++)
	//{
	//	FbxSurfaceMaterial* material = p_node->GetSrcObject<FbxSurfaceMaterial>(i);
	//	if (material)
	//	{
	//		// This only gets the material of type sDiffuse, you probably need to traverse all Standard Material Property by its name to get all possible textures.
	//		FbxProperty prop = material->FindProperty(FbxSurfaceMaterial::sDiffuse);

	//		// Check if it's layeredtextures
	//		int layered_texture_count = prop.GetSrcObjectCount<FbxLayeredTexture>();
	//		if (layered_texture_count > 0)
	//		{
	//			for (int j = 0; j < layered_texture_count; j++)
	//			{
	//				FbxLayeredTexture* layered_texture = FbxCast<FbxLayeredTexture>(prop.GetSrcObject<FbxLayeredTexture>(j));
	//				int lcount = layered_texture->GetSrcObjectCount<FbxTexture>();
	//				for (int k = 0; k < lcount; k++)
	//				{
	//					FbxTexture* texture = FbxCast<FbxTexture>(layered_texture->GetSrcObject<FbxTexture>(k));
	//					// Then, you can get all the properties of the texture, include its name
	//					const char* texture_name = texture->GetName();
	//					texture_name;
	//				}
	//			}
	//		}
	//		else
	//		{
	//			// Directly get textures
	//			int texture_count = prop.GetSrcObjectCount<FbxTexture>();
	//			for (int j = 0; j < texture_count; j++)
	//			{
	//				const FbxTexture* texture = FbxCast<FbxTexture>(prop.GetSrcObject<FbxTexture>(j));
	//				// Then, you can get all the properties of the texture, include its name
	//				const char* texture_name = texture->GetName();
	//				texture_name;
	//			}
	//		}
	//	}

	//}
	for (int i = 0; i < p_node->GetNodeAttributeCount(); i++)
	{
		if (const FbxNodeAttribute* attribute = p_node->GetNodeAttributeByIndex(i))
		{
			switch (attribute->GetAttributeType())
			{
			case FbxNodeAttribute::eMesh:
			{
				// Read vertex, normal, uv data
				mesh.vertices = GetVertices(p_node->GetMesh());
				if(mesh.vertices.size() > s_m_verticesCount)
				{
					s_m_verticesCount = mesh.vertices.size();
					s_m_meshIndex = index;
				}
			}

			break;
			default:
				break;
			}
		}
	}

	mesh.transform = ParseHelper::GetTransformMatrix(p_node);
	// Set parent/children
	mesh.parent = parent;
	// Read transform data
	for (int child = 0; child < p_node->GetChildCount(); ++child)
	{
		const int child_index = ParseNode(p_node->GetChild(child), index, meshes);
		mesh.children.push_back(child_index);
	}

	meshes[index] = mesh;
	return index;
}

std::vector<Vertex> FBXImporter::GetVertices(const FbxMesh* p_mesh) noexcept
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

	std::vector<Vertex> attrib;
	attrib.reserve(indices.size());
	for (int i = 0; i < static_cast<int>(indices.size()); ++i)
	{
		const glm::vec4 vertex{ ctrl_pts[indices[i]], 1 };
		attrib.emplace_back(Vertex{ vertex, vertex_normal[indices[i]], face_normal[i] });
	}

	return attrib;
}

/* FBXImporter - end ----------------------------------------------------------------------------*/
