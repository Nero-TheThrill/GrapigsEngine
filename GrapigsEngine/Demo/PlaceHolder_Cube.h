#pragma once
#include "ResourceManager.h"

float cube_vertices[] = {
	// front
	-1.0, -1.0,  1.0,
	 1.0, -1.0,  1.0,
	 1.0,  1.0,  1.0,
	-1.0,  1.0,  1.0,
	// back
	-1.0, -1.0, -1.0,
	 1.0, -1.0, -1.0,
	 1.0,  1.0, -1.0,
	-1.0,  1.0, -1.0
};

unsigned cube_elements[] = {
	// front
	0, 1, 2,
	2, 3, 0,
	// right
	1, 5, 6,
	6, 2, 1,
	// back
	7, 6, 5,
	5, 4, 7,
	// left
	4, 0, 3,
	3, 7, 4,
	// bottom
	4, 5, 1,
	1, 0, 4,
	// top
	3, 2, 6,
	6, 7, 3
};

//inline Mesh* GetCubeMesh()
//{
//	Mesh* mesh = new Mesh();
//
//	glGenVertexArrays(1, &mesh->VAO);
//	glBindVertexArray(mesh->VAO);
//	glGenBuffers(NUM_VBO, mesh->VBO);
//
//	mesh->name = "Cube";
//	for(int i = 0; i < 24; i+=3)
//		mesh->positions.push_back({ cube_vertices[i], cube_vertices[i + 1], cube_vertices[i + 2] });
//	for(int i = 0; i < 36; ++i)
//		mesh->indices.push_back(cube_elements[i]);
//	mesh->tag = 1;
//	return mesh;
//}