#pragma once

#ifndef SKELETON_MESH_H
#define SKELETON_MESH_H

#include <GL/glew.h> // holds all OpenGL type declarations

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "shader.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

struct Vertex {
	// position
	glm::vec3 Position;
};

struct VertexBoneData {
	// per bone (1-4 weights)
	glm::vec4 weights;
};

class SkeletonMesh {
	public:
		/*  Mesh Data  */
		std::vector<Vertex> vertices;
		std::vector<VertexBoneData> vbd;
		std::vector<unsigned int> indices;
		std::vector<glm::mat4> offsetMatricies;
		unsigned int VAO;

		/*  Functions  */
		// constructor
		SkeletonMesh(std::vector<Vertex> vertices, std::vector<VertexBoneData> vbd, std::vector<unsigned int> indices, std::vector<glm::mat4> offsetMatricies)
		{
			this->vertices = vertices;
			this->vbd = vbd;
			this->indices = indices;
			this->offsetMatricies = offsetMatricies;

			// now that we have all the required data, set the vertex buffers and its attribute pointers.
			setupMesh();
		}

		// render the mesh
		void Draw() 
		{
			// draw mesh
			glBindVertexArray(VAO);
			glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
		}

	private:
		/*  Render data  */
		unsigned int VBO, boneVB, EBO;

		/*  Functions    */
		// initializes all the buffer objects/arrays
		void setupMesh()
		{
			// create buffers/arrays
			glGenVertexArrays(1, &VAO);
			glGenBuffers(1, &boneVB);
			glGenBuffers(1, &VBO);
			glGenBuffers(1, &EBO);

			glBindVertexArray(VAO);
			// load data into vertex buffers
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			// A great thing about structs is that their memory layout is sequential for all its items.
			// The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
			// again translates to 3/2 floats which translates to a byte array.
			glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);  

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

			// set the vertex attribute pointers
			// vertex Positions
			glEnableVertexAttribArray(0);	
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

			glBindBuffer(GL_ARRAY_BUFFER, boneVB);
			glBufferData(GL_ARRAY_BUFFER, vbd.size() * sizeof(VertexBoneData), &vbd[0], GL_STATIC_DRAW);

			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(VertexBoneData), (void*)0);

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);
		}
};
#endif
