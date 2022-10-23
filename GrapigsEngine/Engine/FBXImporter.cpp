/*
 *	Author		: Jina Hyun
 *	Date		: 09/29/22
 *	File Name	: FBXImporter.h
 *	Desc		: Import fbx
 */
#include "FBXImporter.h"

#include <filesystem>	// std::filesystem
#include <iostream>	// std::cerr
#include <map>	// std::map
#include <gl/glew.h>
#include <glm/gtc/matrix_transform.hpp> // transform matrix calculation
#include <sstream>	// strinstream
#include "imgui.h"	// imgui

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

    // Texture Coordinate
	glEnableVertexArrayAttrib(m_vao, 3);
	glVertexArrayAttribFormat(m_vao, 3, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, texture_coordinate));
	glVertexArrayAttribBinding(m_vao, 3, 0);

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
		if (mesh.material.m_p_texture)
			program->SendUniform("u_texture", mesh.material.m_p_texture->Unit());
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
}


/* ParseHelper - end-----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------*/
/* FBXNodePrinter - start -----------------------------------------------------------------------*/

std::string FBXNodePrinter::s_m_filePath{};
std::string FBXNodePrinter::s_m_fileStatus{};
std::vector<int> FBXNodePrinter::s_m_treeHead;
std::vector<FBXNodePrinter::TreeNode> FBXNodePrinter::s_m_tree;
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

void FBXNodePrinter::TreeNode::DisplayNode(const std::vector<TreeNode>& all, const TreeNode& node) noexcept
{
	ImGui::TableNextRow();
	ImGui::TableNextColumn();

	if(node.child.empty())
	{
		static auto flag = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_SpanAvailWidth;

		if(!node.name.empty())
			ImGui::TreeNodeEx(node.name.c_str(), flag);

		ImGui::TableNextColumn();
		{
			ImGui::Unindent(ImGui::GetTreeNodeToLabelSpacing());
			ImGui::TreeNodeEx(node.data_type.c_str(), flag);
			ImGui::Indent(ImGui::GetTreeNodeToLabelSpacing());
		}

		ImGui::TableNextColumn();
		if (node.data.empty())
			ImGui::TextDisabled("--");
		else
		{
			ImGui::Unindent(ImGui::GetTreeNodeToLabelSpacing());
			ImGui::TreeNodeEx(node.data.c_str(), flag);
			ImGui::Indent(ImGui::GetTreeNodeToLabelSpacing());
		}
	}
	else
	{
		bool is_open = false;
		if (node.name.empty())
			ImGui::TableNextColumn();
		else
			is_open = ImGui::TreeNodeEx(node.name.c_str(), ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_OpenOnDoubleClick);

		ImGui::TableNextColumn();
		if (node.data_type.empty())
			ImGui::TextDisabled("--");
		else
			ImGui::TextUnformatted(node.data_type.c_str());
		ImGui::TableNextColumn();
		if (node.data.empty())
			ImGui::TextDisabled("--");
		else
			ImGui::TextUnformatted(node.data.c_str());
		if (is_open)
		{
			for (const auto& c : node.child)
				DisplayNode(all, all[c]);
			ImGui::TreePop();
		}
	}
}

void FBXNodePrinter::SetFileToShowInfo(const std::filesystem::path& path) noexcept
{
	s_m_fileStatus.clear();
	s_m_tree.clear();
	s_m_treeHead.clear();

	s_m_filePath = path.string();
	if (std::filesystem::exists(s_m_filePath) == false)
	{
		s_m_fileStatus = "[" + s_m_filePath + "] does not exist";
		return;
	}
	if (path.extension() != ".fbx")
	{
		s_m_fileStatus = "[" + s_m_filePath + "] is not parseable";
		return;
	}

	FbxManager* p_manager = FbxManager::Create();
	FbxIOSettings* ios = FbxIOSettings::Create(p_manager, IOSROOT);
	p_manager->SetIOSettings(ios);
	FbxImporter * importer = FbxImporter::Create(p_manager, "Importer");
	if (!importer->Initialize(path.string().c_str(), -1, p_manager->GetIOSettings()))
	{
		s_m_fileStatus = "[FBX SDK]: Call to FbxImporter::Initialize() failed.\n";
		s_m_fileStatus += "Error returned" + std::string{ importer->GetStatus().GetErrorString() };
		importer->Destroy();
		p_manager->Destroy();
		return;
	}
	const auto p_scene = FbxScene::Create(p_manager, "New Scene");
	importer->Import(p_scene);
	const auto& p_root = p_scene->GetRootNode();
	for (int i = 0; i < p_root->GetChildCount(); ++i)
	{
		const int head = SetTreeNode(p_root->GetChild(i));
		s_m_treeHead.push_back(head);
	}
	p_scene->Destroy();
	importer->Destroy();
	p_manager->Destroy();
}

void FBXNodePrinter::UpdateGUI() noexcept
{
	ImGui::Begin("FBX FILE NODE STRUCTURE");
	{
		if (s_m_filePath.empty())
			ImGui::Text("Drag and drop file to print fbx nodes!");
		else if (s_m_tree.empty())
			ImGui::Text("%s", s_m_fileStatus.c_str());
		else 
		{
			static ImGuiTableFlags flags = ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody;
			if(ImGui::BeginTable("Nodes", 3, flags))
			{
				const float TEXT_BASE_WIDTH = ImGui::CalcTextSize("A").x;

				ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_NoHide);
				ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, TEXT_BASE_WIDTH * 12.0f);
				ImGui::TableSetupColumn("Data");
				ImGui::TableHeadersRow();

				for (const auto& i : s_m_treeHead)
					TreeNode::DisplayNode(s_m_tree, s_m_tree[i]);
				ImGui::EndTable();
			}
		}
	}
	ImGui::End();
}

int FBXNodePrinter::SetTreeNode(const FbxNode* p_node) noexcept
{
	const auto index = static_cast<int>(s_m_tree.size());
	s_m_tree.push_back(TreeNode{p_node->GetName(), "", ""});
	s_m_tree.push_back(TreeNode{ "", "translation", ParseHelper::ToString(p_node->LclTranslation.Get()) });
	s_m_tree.push_back(TreeNode{ "", "rotation", ParseHelper::ToString(p_node->LclRotation.Get()) });
	s_m_tree.push_back(TreeNode{ "", "scaling", ParseHelper::ToString(p_node->LclScaling.Get()) });

	s_m_tree[index].child.push_back(index + 1);
	s_m_tree[index].child.push_back(index + 2);
	s_m_tree[index].child.push_back(index + 3);

	if(p_node->GetNodeAttributeCount() > 0)
	{
		const auto attrib_index = static_cast<int>(s_m_tree.size());
		s_m_tree.push_back(TreeNode{"Attribute", "", ""});
		s_m_tree[index].child.push_back(attrib_index);
		
		for (int i = 0; i < p_node->GetNodeAttributeCount(); ++i)
		{
			const FbxNodeAttribute* p_attribute = p_node->GetNodeAttributeByIndex(i);
			s_m_tree.push_back(TreeNode{ p_attribute->GetName(), GetAttributeTypeName(p_attribute->GetAttributeType()), ""});
			s_m_tree[attrib_index].child.push_back(i + attrib_index + 1);
		}
	}

	for (int i = 0; i < p_node->GetChildCount(); ++i)
	{
		const int child = SetTreeNode(p_node->GetChild(i));
		s_m_tree[index].child.push_back(child);
	}
	return index;
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

MeshGroup* FBXImporter::Load(const char* file_path) noexcept
{
	const std::filesystem::path path(file_path);
	if (std::filesystem::exists(file_path) == false)
	{
		std::cout << "[FBXImporter]: " << file_path << " does not exist." << std::endl;
		return nullptr;
	}
	if (path.extension() != ".fbx")
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

	auto meshGroup = Parse(pRoot);

	// Destroy the SDK manager and all the other objects is was handling
	p_manager->Destroy();

	return meshGroup;
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

MeshGroup* FBXImporter::Parse(FbxNode* p_root) noexcept
{
	MeshGroup* group = nullptr;
	s_m_meshIndex = -1;
	s_m_verticesCount = 0;

	if (p_root)
	{
		group = new MeshGroup();
		bool is_mesh_exist = false;
		group->m_meshes.resize(1);
		group->m_root = 0;
		group->m_meshes[0].name = "Root";

		for (int i = 0; i < p_root->GetChildCount(); i++)
		{
			is_mesh_exist = true;
			const int top = ParseNode(p_root->GetChild(i), -1, group->m_meshes);
			group->m_meshes[0].children.push_back(top);
			group->m_meshes[top].parent = 0;
		}

		if(is_mesh_exist == false)
		{
			delete group;
			group = nullptr;
		}
		else
		{
			group->m_name = p_root->GetName();
			group->InitBuffers(s_m_meshIndex);
		}
	}

	for (int i = 0; i < group->m_meshes.size(); ++i)
		group->m_meshes[i].index = i;

	return group;
}

int FBXImporter::ParseNode(FbxNode* p_node, int parent, std::vector<Mesh>& meshes) noexcept
{
	const int index = static_cast<int>(meshes.size());
	meshes.emplace_back(Mesh());
	Mesh mesh{};
	mesh.name = p_node->GetName();

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

std::vector<Vertex> FBXImporter::GetVertices(FbxMesh* p_mesh) noexcept
{
	std::vector<glm::vec3> ctrl_pts;
	std::vector<glm::vec4> face_normal;
	std::vector<int> indices;
	std::map<int, glm::vec4> vertex_normal;
	std::vector<glm::vec2> texture_coordinate=ParseHelper::LoadUVInformation(p_mesh);
	int cnt= p_mesh->GetControlPointsCount();
	for (int vert = 0; vert < cnt; ++vert)
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
		attrib.emplace_back(Vertex{ vertex, vertex_normal[indices[i]], face_normal[i], texture_coordinate.size() <= i ? glm::vec2(0) : texture_coordinate[i] });
	}

	return attrib;
}

/* FBXImporter - end ----------------------------------------------------------------------------*/
