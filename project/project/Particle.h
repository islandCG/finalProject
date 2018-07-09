#pragma once
#ifndef PARTICLE_H
#define PARTICLE_H

#include <glfw3.h>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>
#include <glm\glm.hpp>
#include <vector>
#include <cmath>
#include <time.h>
#include <stb_image.h>
#include "shader_m.h"
#include "model.h"
using namespace std;


struct particle {
	glm::vec3 pos;
	glm::vec3 vel;
	glm::vec3 acc;
	//glm::vec4 color;
	glm::mat4 model;
	float age;
	float life;
	float size;
};

class ParticleSystem {
public:
	ParticleSystem();
	ParticleSystem(int count, glm::vec3 force, glm::vec3 pos);
	void init();
	void simulate(float dt);
	void aging(float dt);
	void applyForce();
	void updatePos(float dt);
	void render();
	vector<particle> particles;
private:
	unsigned int VAO, VBO, EBO;
	int pCount; //number of particle
	glm::vec3 initPos;
	glm::vec3 pForce; // particle's force 
};

ParticleSystem::ParticleSystem() {

}

ParticleSystem::ParticleSystem(int count, glm::vec3 force, glm::vec3 pos) {
	this->pCount = count;
	this->pForce = force;
	this->initPos = pos;
	init();
}

void ParticleSystem::init() {
	srand(unsigned(time(NULL)));
	//glm::vec4 color[3] = { glm::vec4(0, 0, 1, 1), glm::vec4(1, 0, 1, 1), glm::vec4(1, 1, 0, 1) };
	for (int i = 0; i < pCount; i++) {
		particle tmp;
		tmp.pos = initPos;
		tmp.vel = glm::vec3(rand() % 100 - 50, rand() % 30 - 30, rand() % 20);
		//tmp.vel = glm::vec3(rand() % 50 - 25, rand() % 50 - 25, 0);
		tmp.acc = glm::vec3(0, 0, 0);
		//tmp.color = color[rand() % 2];
		tmp.age = 0;
		tmp.life = 0.5 + (rand() % 25);
		//tmp.life = 10;
		tmp.size = 0.3f;
		particles.push_back(tmp);
	}

	//float vertices[] = {
	//	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
	//	0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
	//	0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	//	0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	//	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
	//	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

	//	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	//	0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	//	0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
	//	0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
	//	-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
	//	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

	//	-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	//	-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	//	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	//	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	//	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	//	-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

	//	0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	//	0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	//	0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	//	0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	//	0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	//	0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

	//	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	//	0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
	//	0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	//	0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	//	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	//	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

	//	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
	//	0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	//	0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	//	0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	//	-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
	//	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f
	//};
	//glGenVertexArrays(1, &VAO);
	//glBindVertexArray(VAO);
	//glGenBuffers(1, &VBO);
	//glBindBuffer(GL_ARRAY_BUFFER, VBO);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	//glEnableVertexAttribArray(0);
	////glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	////glEnableVertexAttribArray(1);
	////glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	////glEnableVertexAttribArray(2);
	//glBindVertexArray(0);
	//glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void ParticleSystem::simulate(float dt) {
	aging(dt);
	applyForce();
	updatePos(dt);
	render();
}

void ParticleSystem::aging(float dt) {
	for (int i = 0; i < pCount; i++) {
		particles[i].age += dt;
		if (particles[i].age > particles[i].life) {
			particles[i].pos = initPos;
			particles[i].age = 0;
			particles[i].vel = glm::vec3(rand() % 100 - 50, rand() % 30 - 30, rand() % 20);
			//particles[i].vel = glm::vec3(rand() % 50 - 25, rand() % 50 - 25, 0);
			particles[i].acc = glm::vec3(0, 0, 0);
		}
	}
}

void ParticleSystem::applyForce() {
	for (int i = 0; i < pCount; i++) {
		particles[i].acc = pForce;
	}
}

void ParticleSystem::updatePos(float dt) {
	for (int i = 0; i < pCount; i++) {
		particles[i].pos = particles[i].pos + particles[i].vel * dt * float(0.1);
		particles[i].vel = particles[i].vel + particles[i].acc * dt;
	}
}

void ParticleSystem::render() {
	for (int i = 0; i < pCount; i++) {
		//particles[i].color.a = 1 - particles[i].age / particles[i].life;
		glm::mat4 model;
		model = glm::translate(model, particles[i].pos);
		model = glm::scale(model, glm::vec3(0.5,0.5,0.5));
		particles[i].model = model;
	}
}

#endif // !PARTICLE_H

