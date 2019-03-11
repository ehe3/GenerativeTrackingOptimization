#pragma once

#ifndef SKELETON_MODEL_H
#define SKELETON_MODEL_H

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stb_image.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "SkeletonMesh.h"
#include "shader.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>

class SkeletonModel 
{
	public:
		/* Dummy constructor */
		SkeletonModel() {}
		
		/*  Model Data */
		std::vector<SkeletonMesh> meshes;
		std::string directory;

		/*  Functions   */
		// constructor, expects a filepath to a 3D model.
		SkeletonModel(std::string const &path)
		{
			loadModel(path);
		}

		// draws the model, and thus all its meshes
		void Draw()
		{
			//meshes[1].Draw(shader);
			for(unsigned int i = 0; i < meshes.size(); i++)
				meshes[i].Draw();
		}

	private:
		/*  Functions   */
		// loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
		void loadModel(std::string const &path)
		{
			// read file via ASSIMP
			Assimp::Importer importer;
			const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
			// check for errors
			if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
			{
				std::cerr << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
				return;
			}

			// retrieve the directory path of the filepath
			directory = path.substr(0, path.find_last_of('/'));

			// process ASSIMP's root node recursively
			processNode(scene->mRootNode, scene);
		}

		// processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
		void processNode(aiNode *node, const aiScene *scene)
		{
			// process each mesh located at the current node
			for(unsigned int i = 0; i < node->mNumMeshes; i++)
			{
				// the node object only contains indices to index the actual objects in the scene. 
				// the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
				aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
				// only take the mesh with all the bones (for some reason there are duplicates in the .dae file)
				if (mesh->mNumBones == 4)
				{
					meshes.push_back(processMesh(mesh, scene));
				}
			}
			// after we've processed all of the meshes (if any) we then recursively process each of the children nodes
			for(unsigned int i = 0; i < node->mNumChildren; i++)
			{
				processNode(node->mChildren[i], scene);
			}

		}

		inline glm::mat4 aiMatrix4x4ToGlm(const aiMatrix4x4* from)
		{
		    glm::mat4 to;
		
		    to[0][0] = (GLfloat)from->a1; to[0][1] = (GLfloat)from->b1;  to[0][2] = (GLfloat)from->c1; to[0][3] = (GLfloat)from->d1;
		    to[1][0] = (GLfloat)from->a2; to[1][1] = (GLfloat)from->b2;  to[1][2] = (GLfloat)from->c2; to[1][3] = (GLfloat)from->d2;
		    to[2][0] = (GLfloat)from->a3; to[2][1] = (GLfloat)from->b3;  to[2][2] = (GLfloat)from->c3; to[2][3] = (GLfloat)from->d3;
		    to[3][0] = (GLfloat)from->a4; to[3][1] = (GLfloat)from->b4;  to[3][2] = (GLfloat)from->c4; to[3][3] = (GLfloat)from->d4;
		
		    return to;
		}
		
		SkeletonMesh processMesh(aiMesh *mesh, const aiScene *scene)
		{
			// data to fill
			std::vector<Vertex> vertices;
			std::vector<VertexBoneData> vbd;
			std::vector<unsigned int> indices;
			std::vector<glm::mat4> offsetMatricies;
			//vector<glm::mat4> binfo;

			// Walk through each of the mesh's vertices
			for(unsigned int i = 0; i < mesh->mNumVertices; i++)
			{
				Vertex vertex;
				glm::vec3 vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
				// positions
				vector.x = mesh->mVertices[i].x;
				vector.y = mesh->mVertices[i].y;
				vector.z = mesh->mVertices[i].z;
				vertex.Position = vector;
				vertices.push_back(vertex);
			}
			// now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
			for(unsigned int i = 0; i < mesh->mNumFaces; i++)
			{
				aiFace face = mesh->mFaces[i];
				// retrieve all indices of the face and store them in the indices vector
				for(unsigned int j = 0; j < face.mNumIndices; j++)
					indices.push_back(face.mIndices[j]);
			}

			// updating bone information
			vbd.resize(vertices.size());
			for(unsigned int i = 0; i < mesh->mNumBones; i++)
			{
				aiMatrix4x4 offsetMatrix = mesh->mBones[i]->mOffsetMatrix;
				offsetMatricies.push_back(aiMatrix4x4ToGlm(&offsetMatrix));
				for (unsigned int j = 0; j < mesh->mBones[i]->mNumWeights; j++)
				{
					int vertexID = mesh->mBones[i]->mWeights[j].mVertexId;
					float weight = mesh->mBones[i]->mWeights[j].mWeight;
					vbd[vertexID].weights[i] = weight;
				}
			}

			return SkeletonMesh(vertices, vbd, indices, offsetMatricies);
		}
		
};
#endif
