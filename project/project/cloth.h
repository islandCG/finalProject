#pragma once
#ifndef CLOTH_SIMULATION
#define CLOTH_SIMULATION

#include <glfw3.h>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>
#include <glm\glm.hpp>
#include <vector>
#include <cmath>
#include <stb_image.h>
#include "shader_m.h"

struct ClothVertex {
	glm::vec3 vPos;
	glm::vec3 vNor;
	glm::vec3 vVel;
	glm::vec2 vTex;
	float mass;
	glm::vec3 Fspring;
	glm::vec3 Fgravity;
	glm::vec3 Fdamping;
	glm::vec3 Fviscous;
	glm::vec3 Ffuse; // force fusion
};

class ClothUtil {
public:
	ClothUtil(int clothResolution); // initialization
	~ClothUtil(); // free the space for safety
	void ClothSimulating(Shader shader, float deltaTime, float cg, float cd, float cv, glm::vec3 ufluid); // ClothSimulation main entrance
	void ProcessInput(GLFWwindow* window);
private:
	unsigned int VAO, VBO, EBO; //render data
	unsigned int clothTexture; //texture
	float textureStride; //length of step
	vector<unsigned int> indices;
	int clothResolution; // number of Vertex per line/row
	float width;
	vector<ClothVertex> cVers; // vertices set vector
							   // some global params
	float restLen[3]; // structural, shear, and flexion
	float stiff[3]; // structural, shear, and flexion
	float Cg; // gravity coefficient
	float Cd; // damping coefficient
	float Cv; // viscous coefficient
	glm::vec3 Ufluid; // viscous coefficient
private:
	void CreateClothVertex(); // create all vertex and top-left is (-0.5, -0.5)
	void UpdateVertexPosition(float deltaTime); // first part of ClothSimulating()
	void RenderClothPlane(Shader shader); // second part of ClothSimulating()
	glm::vec3 CalNormal(int i, int j); // Calculate normal of a vertex
	glm::vec3 CalSpringForce(int i, int j); // Calculate SpringForce of a vertex
	glm::vec3 sfBetweenTwo(int index1, int index2, int type); //Calculate spring force between two vertex
	glm::vec3 CalGravityForce(int i, int j); // Calculate GravityForce of a vertex
	glm::vec3 CalDampingForce(int i, int j); // Calculate DampingForce of a vertex
	glm::vec3 CalViscousForce(int i, int j); // Calculate ViscousForce of a vertex
	glm::vec3 CalForceFusion(ClothVertex vertex); // Calculate force fusion
												  //glm::vec3 CalSpringForceStruct(int i, int j); // Structural Springs part of CalSpringForce()
												  //glm::vec3 CalSpringForceShear(int i, int j); // Shear Springs part of CalSpringForce()
												  //glm::vec3 CalSpringForceFlexion(int i, int j); // Flexion Springs part of CalSpringForce()
	void CalRestlen(float length);//calculate restLen
	int judgeBound(int i, int j, int index); //judge boundary for calculate normal and spring force
	void AddManualForce(); // Add manual force to the cloth
};

ClothUtil::ClothUtil(int clothResolution) {
	this->clothResolution = clothResolution;
	textureStride = 1.0 / (clothResolution - 1);
	//initial restlen
	//width = 1;
	CalRestlen(0.05);
	//initial stiff
	stiff[0] = 40;
	stiff[1] = 30;
	stiff[2] = 20;
	//intial cofficient
	//Cg = 0.98;
	//Cd = 0.5;
	//Cv = 0.5;
	//Ufluid = glm::vec3(0, 0, 5);
	//initial vertex
	CreateClothVertex();
}

ClothUtil::~ClothUtil() {
	cVers.clear();
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
}

void ClothUtil::ClothSimulating(Shader shader, float deltaTime, float cg, float cd, float cv, glm::vec3 ufluid) {
	Cg = cg;
	Cd = cd;
	Cv = cv;
	Ufluid = ufluid;
	if (deltaTime >= 0.5) {
		deltaTime = 0.02;
	}
	UpdateVertexPosition(deltaTime);
	RenderClothPlane(shader);
}

void ClothUtil::CreateClothVertex() {
	for (int i = 0; i < clothResolution; i++) {
		for (int j = 0; j < clothResolution; j++) {
			ClothVertex vtemp;
			vtemp.vPos = glm::vec3(-0.5 + j * restLen[0], 0.5 - i * restLen[0], 0);
			vtemp.vNor = glm::vec3(0, 0, 1);
			vtemp.vVel = glm::vec3(0, 0, 0);
			vtemp.vTex = glm::vec2(j * textureStride, 1 - i * textureStride);
			vtemp.mass = 1;
			vtemp.Fspring = glm::vec3(0, 0, 0);
			vtemp.Fgravity = glm::vec3(0, 0, 0);
			vtemp.Fdamping = glm::vec3(0, 0, 0);
			vtemp.Fviscous = glm::vec3(0, 0, 0);
			vtemp.Ffuse = glm::vec3(0, 0, 0);
			cVers.push_back(vtemp);
		}
	}
	//initial indices      
	for (int i = 0; i < clothResolution - 1; i++) {
		for (int j = 0; j < clothResolution - 1; j++) {
			int temp = i * clothResolution + j;
			int indicesTemp[6] = { temp, temp + 1, temp + clothResolution + 1, temp, temp + clothResolution, temp + clothResolution + 1 };
			indices.insert(indices.end(), indicesTemp, indicesTemp + 6);
		}
	}
	//initial VAO, VBO, EBO
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, cVers.size() * sizeof(ClothVertex), &cVers[0], GL_STREAM_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
	//顶点位置和法向量
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ClothVertex), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(ClothVertex), (void*)offsetof(ClothVertex, vNor));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(ClothVertex), (void*)offsetof(ClothVertex, vTex));
	glBindVertexArray(0);

	//initial texture
	glGenTextures(1, &clothTexture);
	glBindTexture(GL_TEXTURE_2D, clothTexture);
	// 为当前绑定的纹理对象设置环绕、过滤方式
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// 加载并生成纹理
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true);
	unsigned char *data = stbi_load("./resources/textures/flag.jpg", &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_set_flip_vertically_on_load(false);
	stbi_image_free(data);
}

void ClothUtil::UpdateVertexPosition(float deltaTime) {
	for (int i = 0; i < clothResolution; i++) {
		for (int j = 0; j < clothResolution; j++) {
			int index = i * clothResolution + j;
			cVers[index].vNor = CalNormal(i, j);
			cVers[index].Fspring = CalSpringForce(i, j);
			cVers[index].Fgravity = CalGravityForce(i, j);
			cVers[index].Fdamping = CalDampingForce(i, j);
			cVers[index].Fviscous = CalViscousForce(i, j);
		}
	}
	for (int i = 0; i < clothResolution; i++) {
		for (int j = 0; j < clothResolution; j++) {
			int index = i * clothResolution + j;
			cVers[index].Ffuse = CalForceFusion(cVers[index]);
			cVers[index].vVel = cVers[index].vVel + cVers[index].Ffuse / cVers[index].mass * deltaTime;
			if (j != 0) {
				cVers[index].vPos = cVers[index].vPos + cVers[index].vVel * deltaTime;
			}
			//cVers[index].vNor = CalNormal(i, j);
		}
	}
	//修改VBO数据
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, cVers.size() * sizeof(ClothVertex), &cVers[0], GL_STREAM_DRAW);//顶点位置和法向量
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ClothVertex), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(ClothVertex), (void*)offsetof(ClothVertex, vNor));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(ClothVertex), (void*)offsetof(ClothVertex, vTex));
	glBindVertexArray(0);
}

void ClothUtil::RenderClothPlane(Shader shader) {
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, clothTexture);
	glBindVertexArray(VAO);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	glActiveTexture(GL_TEXTURE0);
}

glm::vec3 ClothUtil::CalNormal(int i, int j) {
	glm::vec3 norm = glm::vec3(0, 0, 0);
	int index = i * clothResolution + j;
	norm += glm::cross(cVers[judgeBound(i - 1, j, index)].vPos - cVers[index].vPos, cVers[judgeBound(i - 1, j - 1, index)].vPos - cVers[index].vPos);
	norm += glm::cross(cVers[judgeBound(i - 1, j - 1, index)].vPos - cVers[index].vPos, cVers[judgeBound(i, j - 1, index)].vPos - cVers[index].vPos);
	norm += glm::cross(cVers[judgeBound(i, j - 1, index)].vPos - cVers[index].vPos, cVers[judgeBound(i + 1, j, index)].vPos - cVers[index].vPos);
	norm += glm::cross(cVers[judgeBound(i + 1, j, index)].vPos - cVers[index].vPos, cVers[judgeBound(i + 1, j + 1, index)].vPos - cVers[index].vPos);
	norm += glm::cross(cVers[judgeBound(i + 1, j + 1, index)].vPos - cVers[index].vPos, cVers[judgeBound(i, j + 1, index)].vPos - cVers[index].vPos);
	norm += glm::cross(cVers[judgeBound(i, j + 1, index)].vPos - cVers[index].vPos, cVers[judgeBound(i - 1, j, index)].vPos - cVers[index].vPos);
	norm = glm::normalize(norm);
	return norm;
}

int ClothUtil::judgeBound(int i, int j, int index) {
	if (i < 0 || i >= clothResolution) {
		return index;
	}
	if (j < 0 || j >= clothResolution) {
		return index;
	}
	int a = i * clothResolution + j;
	return i * clothResolution + j;
}

glm::vec3 ClothUtil::CalSpringForce(int i, int j) {
	glm::vec3 force = glm::vec3(0, 0, 0);
	int index = i * clothResolution + j;
	force += sfBetweenTwo(judgeBound(i - 1, j, index), index, 0);
	force += sfBetweenTwo(judgeBound(i + 1, j, index), index, 0);
	force += sfBetweenTwo(judgeBound(i, j - 1, index), index, 0);
	force += sfBetweenTwo(judgeBound(i, j + 1, index), index, 0);
	force += sfBetweenTwo(judgeBound(i - 1, j - 1, index), index, 1);
	force += sfBetweenTwo(judgeBound(i - 1, j + 1, index), index, 1);
	force += sfBetweenTwo(judgeBound(i + 1, j - 1, index), index, 1);
	force += sfBetweenTwo(judgeBound(i + 1, j + 1, index), index, 1);
	force += sfBetweenTwo(judgeBound(i - 2, j, index), index, 2);
	force += sfBetweenTwo(judgeBound(i + 2, j, index), index, 2);
	force += sfBetweenTwo(judgeBound(i, j - 2, index), index, 2);
	force += sfBetweenTwo(judgeBound(i, j + 2, index), index, 2);
	return force;
}

glm::vec3 ClothUtil::sfBetweenTwo(int index1, int index2, int type) {
	float distance = glm::length(cVers[index2].vPos - cVers[index1].vPos);
	if (index1 == index2) {
		return glm::vec3(0, 0, 0);
	}
	glm::vec3 b = stiff[type] * (restLen[type] - distance) * glm::normalize(cVers[index2].vPos - cVers[index1].vPos);
	//return stiff[type] * (restLen[type] - distance) * glm::normalize(cVers[index2].vPos - cVers[index1].vPos);
	return stiff[type] * (restLen[type] - distance) / distance * (cVers[index2].vPos - cVers[index1].vPos);
}

glm::vec3 ClothUtil::CalGravityForce(int i, int j) {
	int index = i * clothResolution + j;
	return cVers[index].mass * glm::vec3(0, -Cg, 0);
}

glm::vec3 ClothUtil::CalDampingForce(int i, int j) {
	int index = i * clothResolution + j;
	return -Cd * cVers[index].vVel;
}

glm::vec3 ClothUtil::CalViscousForce(int i, int j) {
	int index = i * clothResolution + j;
	return Cv * cVers[index].vNor * (Ufluid - cVers[index].vVel) * cVers[index].vNor;
}

glm::vec3 ClothUtil::CalForceFusion(ClothVertex vertex) {
	return vertex.Fspring + vertex.Fgravity + vertex.Fdamping + vertex.Fviscous;
	//return vertex.Fspring;
}

void ClothUtil::CalRestlen(float length) {
	restLen[0] = length;
	restLen[1] = sqrt(2) * length;
	restLen[2] = 2 * length;
}

#endif // !CLOTH_SIMULATION
