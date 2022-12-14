/*
 *	Author		: Jina Hyun
 *	Date		: 09/29/22
 *	File Name	: FBXImporter.h
 *	Desc		: Import fbx
 */
#include "FBXImporter.h"

#include <filesystem>		// std::filesystem
#include <iostream>			// std::cerr
#include <map>				// std::map
#include <gl/glew.h>		// gl	
#include <glm/gtc/matrix_transform.hpp> // transform matrix calculation
#include <sstream>			// stringstream

 /* Model - start --------------------------------------------------------------------------------*/

Model::Model(const std::filesystem::path& file_path)
	: m_path(file_path)
{
}

Model::~Model()
{
	Clear();
}

void Model::InitBuffers(int largest_index) noexcept
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

    // Texture Coordinate
	glEnableVertexArrayAttrib(m_vao, 3);
	glVertexArrayAttribFormat(m_vao, 3, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, texture_coordinate));
	glVertexArrayAttribBinding(m_vao, 3, 0);

	glBindVertexArray(0);
}

void Model::Clear() noexcept
{
	if(m_vao > 0)
		glDeleteVertexArrays(1, &m_vao);
	m_vao = 0;
	if(m_vbo > 0)
		glDeleteBuffers(1, &m_vbo);
	m_vbo = 0;
}

void Model::Draw(Primitive primitive, ShaderProgram* program) noexcept
{
	if (m_vao)
	{
		glBindVertexArray(m_vao);
		Draw(primitive, program, m_root, glm::mat4{ 1 });
		glBindVertexArray(0);
	}
}

void Model::Draw(Primitive primitive, ShaderProgram* program, int index, glm::mat4 transform) const noexcept
{
	const auto& mesh = m_meshes[index];
	const auto& vertex = mesh.vertices;

	if (vertex.empty() == false)
	{
		program->SendUniform("u_localToModel", mesh.transform);

		program->SendUniform("u_has_albedo", mesh.material.t_albedo!=nullptr);
		program->SendUniform("u_has_metallic", mesh.material.t_metallic != nullptr);
		program->SendUniform("u_has_roughness", mesh.material.t_roughness != nullptr);
		program->SendUniform("u_has_ao", mesh.material.t_ao != nullptr);
		program->SendUniform("u_has_normalmap", mesh.material.t_normal != nullptr);

		if (mesh.material.t_albedo)
			program->SendUniform("t_albedo", mesh.material.t_albedo->Unit());
		if (mesh.material.t_metallic)
			program->SendUniform("t_metallic", mesh.material.t_metallic->Unit());
	    if (mesh.material.t_roughness)
			program->SendUniform("t_roughness", mesh.material.t_roughness->Unit());
		if (mesh.material.t_ao)
			program->SendUniform("t_ao", mesh.material.t_ao->Unit());
		if (mesh.material.t_normal)
			program->SendUniform("t_normal", mesh.material.t_normal->Unit());

		program->SendUniform("u_metallic", mesh.material.metallic);
		program->SendUniform("u_roughness", mesh.material.roughness);
		program->SendUniform("u_albedo", mesh.material.albedo);
		glNamedBufferSubData(m_vbo, 0, static_cast<GLsizeiptr>(sizeof(Vertex) * vertex.size()), vertex.data());
		glDrawArrays(static_cast<GLenum>(primitive), 0, static_cast<GLsizei>(vertex.size()));
	}

	for (const auto& c : m_meshes[index].children)
	{
		Draw(primitive, program, c, transform * mesh.transform);
	}
}

/* Model - end ----------------------------------------------------------------------------------*/
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
	glm::mat4 GetLocalTransform(const glm::vec3& translation, const glm::vec3& rotation, const glm::vec3& scaling) noexcept
	{
		glm::mat4 transform{ 1 };
		transform = glm::translate(transform, translation);
		transform = glm::rotate(transform, glm::radians(rotation.x), glm::vec3{ 1, 0, 0 });
		transform = glm::rotate(transform, glm::radians(rotation.y), glm::vec3{ 0, 1, 0 });
		transform = glm::rotate(transform, glm::radians(rotation.z), glm::vec3{ 0, 0, 1 });
		transform = glm::scale(transform, scaling);
		return transform;
	}

	glm::vec3 GetVertexNormal(const FbxMesh* p_mesh, int poly, int vert) noexcept
	{
		FbxVector4 normal;
		p_mesh->GetPolygonVertexNormal(poly, vert, normal);
		return glm::vec3{ normal[0], normal[1],normal[2] };
	}
	
    std::vector<glm::vec2> LoadUVInformation(FbxMesh* pMesh)
	{
		std::vector<glm::vec2> texcoords;
		//get all UV set names
		FbxStringList lUVSetNameList;
		pMesh->GetUVSetNames(lUVSetNameList);

		//iterating over all uv sets
		for (int lUVSetIndex = 0; lUVSetIndex < lUVSetNameList.GetCount(); lUVSetIndex++)
		{
			//get lUVSetIndex-th uv set
			const char* lUVSetName = lUVSetNameList.GetStringAt(lUVSetIndex);
			const FbxGeometryElementUV* lUVElement = pMesh->GetElementUV(lUVSetName);

			if (!lUVElement)
				continue;

			// only support mapping mode eByPolygonVertex and eByControlPoint
			if (lUVElement->GetMappingMode() != FbxGeometryElement::eByPolygonVertex &&
				lUVElement->GetMappingMode() != FbxGeometryElement::eByControlPoint)
				return texcoords;

			//index array, where holds the index referenced to the uv data
			const bool lUseIndex = lUVElement->GetReferenceMode() != FbxGeometryElement::eDirect;
			const int lIndexCount = (lUseIndex) ? lUVElement->GetIndexArray().GetCount() : 0;

			//iterating through the data by polygon
			const int lPolyCount = pMesh->GetPolygonCount();
			int lPolyIndexCounter = 0;
			for (int lPolyIndex = 0; lPolyIndex < lPolyCount; ++lPolyIndex)
			{
				// build the max index array that we need to pass into MakePoly
				const int lPolySize = pMesh->GetPolygonSize(lPolyIndex);
				std::vector<glm::vec2> tmp;
				for (int lVertIndex = 0; lVertIndex < lPolySize; ++lVertIndex)
				{

					if (lPolyIndexCounter < lIndexCount)
					{
						FbxVector2 lUVValue;

						//the UV index depends on the reference mode
						int lUVIndex = lUseIndex ? lUVElement->GetIndexArray().GetAt(lPolyIndexCounter) : lPolyIndexCounter;

						lUVValue = lUVElement->GetDirectArray().GetAt(lUVIndex);
						//int key = pMesh->GetPolygonVertex(lPolyIndex, lVertIndex);

						tmp.push_back(glm::vec2(lUVValue[0], lUVValue[1]));
						lPolyIndexCounter++;
					}
				}
				if (lPolySize == 4)
				{
					texcoords.push_back(tmp[0]);
					texcoords.push_back(tmp[1]);
					texcoords.push_back(tmp[2]);
					texcoords.push_back(tmp[0]);
					texcoords.push_back(tmp[2]);
					texcoords.push_back(tmp[3]);
				}
				else if(lPolySize == 3)
				{
					texcoords.push_back(tmp[0]);
					texcoords.push_back(tmp[1]);
					texcoords.push_back(tmp[2]);

				}
			}
		}
		return texcoords;
	}
	template <typename T>
	std::string ToString(const T a_value, const int n = 2)
	{
		std::ostringstream out;
		out.precision(n);
		out << std::fixed << a_value;
		return out.str();
	}

	std::string ToString(const FbxDouble3& vec3)
	{
		return "(" + ToString(vec3.mData[0]) + ", " + ToString(vec3.mData[1]) + ", " + ToString(vec3.mData[2]) + ")";
	}

	glm::mat4 ToMat4(FbxAMatrix mat)
	{
		glm::mat4 r;
		for(int row = 0; row < 4; ++row)
		{
			for (int col = 0; col < 4; ++col)
				r[row][col] = static_cast<float>(mat[row][col]);
		}
		return r;
	}

	glm::vec3 ToVec3(FbxDouble3 vec)
	{
		return glm::vec3{ vec[0], vec[1], vec[2] };
	}
}


/* ParseHelper - end-----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------*/
/* FBXImporter - start --------------------------------------------------------------------------*/

std::filesystem::path FBXImporter::s_path{ "" };
std::size_t FBXImporter::s_m_verticesCount = 0;
int FBXImporter::s_m_meshIndex = -1;
glm::vec3 FBXImporter::max{ std::numeric_limits<float>::min() };
glm::vec3 FBXImporter::min{ std::numeric_limits<float>::max() };
glm::vec4 FBXImporter::sum{ 0};
glm::mat4 FBXImporter::globalTransform{ 1 };

Model* FBXImporter::Load(const char* file_path) noexcept
{
	s_path = std::filesystem::path{file_path};
	if (std::filesystem::exists(file_path) == false)
	{
		std::cout << "[FBXImporter]: " << file_path << " does not exist." << std::endl;
		return nullptr;
	}
	if (s_path.extension() != ".fbx")
	{
		std::cout << "[FBXImporter]: Unable to parse " << file_path << std::endl;
		return nullptr;
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
	//FBXNodePrinter::Print(pRoot);

	auto model = Parse(pRoot);

	// Destroy the SDK manager and all the other objects is was handling
	p_manager->Destroy();

	return model;
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

Model* FBXImporter::Parse(FbxNode* p_root) noexcept
{
	Model* model = nullptr;
	s_m_meshIndex = -1;
	s_m_verticesCount = 0;
	max = glm::vec3{ std::numeric_limits<float>::min() };
	min = glm::vec3{ std::numeric_limits<float>::max() };
	sum = glm::vec4{ 0 };
	globalTransform = glm::mat4{ 1 };

	if (p_root)
	{
		model = new Model(s_path);
		bool is_mesh_exist = false;
		model->m_meshes.resize(1);
		model->m_root = 0;
		model->m_meshes[0].name = "Root";

		for (int i = 0; i < p_root->GetChildCount(); i++)
		{
			is_mesh_exist = true;
			const int top = ParseNode(p_root->GetChild(i), -1, model->m_meshes);
			model->m_meshes[0].children.push_back(top);
			model->m_meshes[top].parent = 0;
		}

		if(is_mesh_exist == false)
		{
			delete model;
			model = nullptr;
		}
		else
		{
			model->m_name = p_root->GetName();
			model->InitBuffers(s_m_meshIndex);
		}
	}

	// Normalize vertex position
	const glm::vec3 center{ sum.x / sum.w, sum.y / sum.w, sum.z / sum.w };
	const glm::vec3 size = abs(max - min);
	const float scale = 1.f / std::max(size.x, std::max(size.y, size.z));

	int index = 0;
	for (auto& m : model->m_meshes)
	{
		m.index = index++;
		m.translation -= center;
		m.translation *= scale;
		m.scaling *= scale;
		m.transform = ParseHelper::GetLocalTransform(m.translation, m.rotation, m.scaling);
	}
	model->m_meshes.front().transform = glm::mat4{ 1 };

	return model;
}

int FBXImporter::ParseNode(FbxNode* p_node, int parent, std::vector<Mesh>& meshes) noexcept
{
	const int index = static_cast<int>(meshes.size());
	meshes.emplace_back(Mesh());
	Mesh mesh{};
	mesh.name = p_node->GetName();
	globalTransform = ParseHelper::ToMat4(p_node->EvaluateGlobalTransform());

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

	mesh.transform = ParseHelper::ToMat4(p_node->EvaluateLocalTransform());
	mesh.translation = ParseHelper::ToVec3(p_node->LclTranslation);
	mesh.rotation = ParseHelper::ToVec3(p_node->LclRotation);
	mesh.scaling = ParseHelper::ToVec3(p_node->LclScaling);

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

std::vector<Vertex> FBXImporter::GetVertices(FbxMesh* p_mesh) noexcept
{
	std::vector<glm::vec3> ctrl_pts;
	std::vector<glm::vec4> face_normal;
	std::vector<int> indices;
	std::map<int, glm::vec4> vertex_normal;
	std::vector<glm::vec2> texture_coordinate=ParseHelper::LoadUVInformation(p_mesh);

	for (int vert = 0; vert < p_mesh->GetControlPointsCount(); ++vert)
	{
		const auto& pos = p_mesh->GetControlPointAt(vert);
		const glm::vec3 vertex{ pos[0], pos[1], pos[2] };
		ctrl_pts.emplace_back(vertex);
		const auto global = globalTransform * glm::vec4{ vertex, 1 };
		SetRange(global.x, global.y, global.z);
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
		attrib.emplace_back(Vertex{ vertex, vertex_normal[indices[i]], face_normal[i], texture_coordinate.size() <= i ? glm::vec2(0) : texture_coordinate[i] });
	}

	return attrib;
}

void FBXImporter::SetRange(float x, float y, float z) noexcept
{
	if (x < min.x)
		min.x = x;
	if (x > max.x)
		max.x = x;
	if (y < min.y)
		min.y = y;
	if (y > max.y)
		max.y = y;
	if (z < min.z)
		min.z = z;
	if (z > max.z)
		max.z = z;
	sum.x += x;
	sum.y += y;
	sum.z += z;
	sum.w += 1;
}

/* FBXImporter - end ----------------------------------------------------------------------------*/
