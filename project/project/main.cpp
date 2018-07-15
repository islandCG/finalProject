#include <ft2build.h>
#include FT_FREETYPE_H
#include <glad/glad.h>
#include <glfw3.h>
#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include<sstream>
#include "shader_m.h"
#include "camera.h"
#include "model.h"
#include <iostream>
#include <freeglut/freeglut.h>
#include <gltools/GLTools.h>
#include "cloth.h"
#include "particle.h"
using namespace std;

/// Holds all state information relevant to a character as loaded using FreeType
struct Character {
	GLuint TextureID;   // ID handle of the glyph texture
	glm::ivec2 Size;    // Size of glyph
	glm::ivec2 Bearing;  // Offset from baseline to left/top of glyph
	GLuint Advance;    // Horizontal offset to advance to next glyph
};
map<GLchar, Character> Characters;
GLuint VAO, VBO;

void initText(Shader textShader);
void RenderText(Shader &shader, std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color);

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mod);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
unsigned int loadTexture(const char *path);
unsigned int loadCubemap(vector<string> faces);

GLFWwindow* initOpenGL();
// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// camera
Camera camera(glm::vec3(22.0f, 10.0f, 77.0f));
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;
float fps = 0.0f;

//dayornight
bool dnFlag = false;
bool firstChange = false;

// lighting
glm::vec3 lightPos(10.0f, 25.0f, 10.0f);


// Oceam
// ---------------

#define START_X		-4.0
#define START_Z		-2.5
#define START_Y		10
#define LENGTH_X	0.1
#define LENGTH_Z	0.1

#define HEIGHT_SCALE	1.6

#define WAVE_COUNT		6

#define STRIP_COUNT		80
#define STRIP_LENGTH	50
#define DATA_LENGTH		STRIP_LENGTH*2*(STRIP_COUNT-1)

static GLfloat pt_strip[STRIP_COUNT*STRIP_LENGTH*3] = {0};
static GLfloat pt_normal[STRIP_COUNT*STRIP_LENGTH*3] = {0};
static GLfloat vertex_data[DATA_LENGTH*3] = {0};
static GLfloat normal_data[DATA_LENGTH*3] = {0};


//wave_length, wave_height, wave_dir, wave_speed, wave_start.x, wave_start.y
static const GLfloat wave_para[6][6] = {
	{	1.6,	0.12,	0.9,	0.06,	0.0,	0.0	},
	{	1.3,	0.1,	1.14,	0.09,	0.0,	0.0	},
	{	0.2,	0.01,	0.8,	0.08,	0.0,	0.0	},
	{	0.18,	0.008,	1.05,	0.1,	0.0,	0.0	},
	{	0.23,	0.005,	1.15,	0.09,	0.0,	0.0	},
	{	0.12,	0.003,	0.97,	0.14,	0.0,	0.0	}
};

static const GLfloat gerstner_pt_a[22] = {
	0.0,0.0, 41.8,1.4, 77.5,5.2, 107.6,10.9,
	132.4,17.7, 152.3,25.0, 167.9,32.4, 179.8,39.2,
	188.6,44.8, 195.0,48.5, 200.0,50.0
};
static const GLfloat gerstner_pt_b[22] = {
	0.0,0.0, 27.7,1.4, 52.9,5.2, 75.9,10.8,
	97.2,17.6, 116.8,25.0, 135.1,32.4, 152.4,39.2,
	168.8,44.8, 184.6,48.5, 200.0,50.0
};
static const GLint gerstner_sort[6] = {
	0, 0, 1, 1, 1, 1
};

static struct {
	GLuint vertex_buffer, normal_buffer;
	GLuint vertex_shader, fragment_shader, program;
	GLuint diffuse_texture, normal_texture;

	struct {
		GLint diffuse_texture, normal_texture;
	} uniforms;

	struct {
		GLint position;
		GLint normal;
	} attributes;
} names;


static struct {
	GLfloat time;
	GLfloat wave_length[WAVE_COUNT],
		wave_height[WAVE_COUNT],
		wave_dir[WAVE_COUNT],
		wave_speed[WAVE_COUNT],
		wave_start[WAVE_COUNT*2];
} values;


static void calcuGerstnerGradient(const GLfloat *, GLfloat *);
static void initWave(void);
static float gerstnerY(float, float, float, const GLfloat *);
static int normalizeF(float *, float *, int);
static void calcuWave(void);

// Oceam end
// ----------------

int main()
{
	GLFWwindow* window = initOpenGL();

	Shader textShader("text.vs", "text.fs");
	initText(textShader);

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE); // Enabled by default on some drivers, but not all so always enable to make sure


	Shader skyboxShader("./skybox.vs", "./skybox.fs");
	Shader depthShader("./depthshader.vs", "./depthshader.fs");
	Shader modelShader("./modelshader.vs", "./modelshader.fs");
	Shader clothShader("./cloth.vs", "./cloth.fs");
	Shader oceanShader("./oceanshader.vs", "./oceanshader.fs");
    unsigned int waterTexture = loadTexture("./resources/textures/water.jpeg");

	Model ourModel("./newbeach/beach_final_test.obj");
	Model flowerModel("./flower/flower.obj");
	ClothUtil ourCloth = ClothUtil(15);
	ParticleSystem ourParticle = ParticleSystem(300, glm::vec3(0, -0.98, 0), glm::vec3(-10, 10, -20));


	float skyboxVertices[] = {
		// positions          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f,  1.0f
	};
	// skybox VAO
	unsigned int skyboxVAO, skyboxVBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	// ocean vao
    unsigned int oceanPosVBO, oceanNorVBO, oceanVAO;
    glGenVertexArrays(1, &oceanVAO);
    glGenBuffers(1, &oceanPosVBO);
    // 链接顶点数组对象
    glBindVertexArray(oceanVAO);
    glBindBuffer(GL_ARRAY_BUFFER, oceanPosVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data, GL_STATIC_DRAW);
    // 连接顶点着属性
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &oceanNorVBO);
    glBindBuffer(GL_ARRAY_BUFFER, oceanNorVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(normal_data), normal_data, GL_STATIC_DRAW);
    // 连接顶点着属性
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);

	vector<string> facesDay
	{
		("resources/textures/skybox/front.jpg"),
		("resources/textures/skybox/back.jpg"),
		("resources/textures/skybox/top.jpg"),
		("resources/textures/skybox/bottom.jpg"),
		("resources/textures/skybox/right.jpg"),
		("resources/textures/skybox/left.jpg")
	};
	unsigned int cubemapTextureDay = loadCubemap(facesDay);

	vector<string> facesNight
	{
		("resources/textures/skybox1/front.jpg"),
		("resources/textures/skybox1/back.jpg"),
		("resources/textures/skybox1/top.jpg"),
		("resources/textures/skybox1/bottom.jpg"),
		("resources/textures/skybox1/right.jpg"),
		("resources/textures/skybox1/left.jpg")
	};
	unsigned int cubemapTextureNight = loadCubemap(facesNight);

	skyboxShader.use();
	skyboxShader.setInt("skybox", 0);
	
    oceanShader.use();
    oceanShader.setInt("diffuseTexture", 0);
    initWave();
    calcuWave();

	// Depth Map
	// --------------------
	GLuint depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);

	const GLuint SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

	GLuint depthMap;
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	GLfloat borderColor[] = { 1.0, 1.0, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Model Config
	// --------------------
	glm::vec3 modelPos = glm::vec3(0, 0, 0);

	// shader configuration
	// --------------------
	modelShader.use();
	modelShader.setInt("material.diffuse", 0);
	modelShader.setInt("shadowMap", 1);

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// per-frame time logic
		// --------------------
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		fps = 1 / deltaTime;
		

		// input
		// -----
		processInput(window);
		//按q向上移动镜头
		if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
			modelPos.y -= 25 * deltaTime;
		}
		//按e向下移动镜头
		if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
			modelPos.y += 25 * deltaTime;
		}
		// render
		// ------
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		// 1. Render depth of scene to texture (from light's perspective)
		// - Get light projection/view matrix.
		glm::mat4 lightProjection, lightView;
		glm::mat4 lightSpaceMatrix;
		GLfloat near_plane = 1.0f, far_plane = 1000.0f;
		//lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
		lightProjection = glm::perspective(45.0f, (float)SCR_WIDTH / (float)SCR_HEIGHT, near_plane, far_plane);
		lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
		lightSpaceMatrix = lightProjection * lightView;

		// - render scene from light's point of view
		depthShader.use();
		depthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
			glClear(GL_DEPTH_BUFFER_BIT);
			glm::mat4 model;
			depthShader.setMat4("model", model);
			ourModel.Draw(depthShader);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// display text
		{
			stringstream ss;
			ss << fps;;
			string temp;
			ss >> temp;
			ss.clear();
			string fpsShow = "FPS:  " + temp;
			RenderText(textShader, fpsShow, 25.0f, 25.0f, 1.0f, glm::vec3(0.5, 0.8f, 0.2f));
		}

		// Reset viewport
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// 2. 像往常一样渲染场景，但这次使用深度贴图
		//model
		{
			glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			modelShader.use();
			//modelShader.setInt("shadowMap", 1);
			//modelShader.setVec3("light.position", lightPos);
			//modelShader.setVec3("viewPos", camera.Position);

			// light properties
			modelShader.setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
			modelShader.setVec3("light.diffuse", 0.5f, 0.5f, 0.5f);
			modelShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);

			// material properties
			// modelShader.setVec3("material.specular", 0.5f, 0.5f, 0.5f);
			modelShader.setVec3("material.specular", 0.1f, 0.1f, 0.1f);
			// modelShader.setFloat("material.shininess", 64.0f);
			modelShader.setFloat("material.shininess", 16.0f);

			// view/projection transformations
			glm::mat4 projection = glm::perspective(45.0f, (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
			glm::mat4 view = camera.GetViewMatrix();
			modelShader.setMat4("projection", projection);
			modelShader.setMat4("view", view);

			modelShader.setVec3("viewPos", camera.Position);
			modelShader.setVec3("light.position", lightPos);
			modelShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, depthMap);
			// world transformation
			glm::mat4 model;
			modelShader.setMat4("model", model);
			ourModel.Draw(modelShader);
		}

		// display text
		{
			stringstream ss;
			ss << fps;;
			string temp;
			ss >> temp;
			ss.clear();
			string fpsShow = "FPS:  " + temp;
			RenderText(textShader, fpsShow, 25.0f, 25.0f, 1.0f, glm::vec3(0.5, 0.8f, 0.2f));
		}

		// skybox
		{
			if (dnFlag) {
				lightPos.y = sin(glfwGetTime() / 3.0f) * 10.0f;
				//cout << "y  " << lightPos.y << endl;
				lightPos.x = cos(glfwGetTime() / 3.0f) * 10.0f;
			}
			else {
				if (firstChange) {
					lightPos = glm::vec3((10.0f, 25.0f, 10.0f));
					firstChange = false;
				}
			}
			//lightPos.y = sin(glfwGetTime() / 3.0f) * 10.0f;
			////cout << "y  " << lightPos.y << endl;
			//lightPos.x = cos(glfwGetTime() / 3.0f) * 10.0f;
			glm::mat4 view = camera.GetViewMatrix();
			glm::mat4 projection = glm::perspective(45.0f, (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
			// draw skybox as last
			glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
			skyboxShader.use();
			view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation from the view matrix
			skyboxShader.setMat4("view", view);
			skyboxShader.setMat4("projection", projection);
			// skybox cube
			glBindVertexArray(skyboxVAO);
			glActiveTexture(GL_TEXTURE0);
			if (lightPos.y >= 0.0f) {
				glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTextureDay);
			}    else {
				glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTextureNight);
			}
			glDrawArrays(GL_TRIANGLES, 0, 36);
			glBindVertexArray(0);
			glDepthFunc(GL_LESS); // set depth function back to default
		}

		// ocean
		{
			// draw the ocean
			// -------------
			oceanShader.use();
			oceanShader.setVec3("lightColor",  1.0f, 1.0f, 1.0f);
			oceanShader.setVec3("lightPos", lightPos);
			oceanShader.setVec3("viewPos", camera.Position);

			glm::mat4 projection;
			glm::mat4 view;
			glm::mat4 model;
			projection = glm::perspective(45.0f, (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
			view = camera.GetViewMatrix();
			model = glm::translate(model, glm::vec3(15, -10.5, 12));
			// model = glm::scale(model, glm::vec3(3, 3, 3));
			oceanShader.setMat4("projection", projection);
			oceanShader.setMat4("view", view);
			oceanShader.setMat4("model", model);
			oceanShader.setFloat("time", values.time);
			// glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, waterTexture);

			values.time += 0.05;
			calcuWave();

			// Draw
			glBindVertexArray(oceanVAO);
			glGenBuffers(1, &oceanPosVBO);
			glBindBuffer(GL_ARRAY_BUFFER, oceanPosVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data, GL_STATIC_DRAW);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);

			glGenBuffers(1, &oceanNorVBO);
			glBindBuffer(GL_ARRAY_BUFFER, oceanNorVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(normal_data), normal_data, GL_STATIC_DRAW);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(1);

			for(int c=0; c<(STRIP_COUNT-1); c++)
				glDrawArrays(GL_TRIANGLE_STRIP, STRIP_LENGTH*2*c, STRIP_LENGTH*2);
			// glDrawArrays(GL_TRIANGLE_STRIP, 0, STRIP_LENGTH);
		}

		//cloth
		{
			clothShader.use();
			glm::mat4 model;
			model = glm::mat4();
			//model = glm::translate(model, glm::vec3(0, 5, -2));
			model = glm::translate(model, glm::vec3(0, 5, 10));
			model = glm::scale(model, glm::vec3(3, 3, 3));
			glm::mat4 projection = glm::perspective(45.0f, (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
			glm::mat4 view = camera.GetViewMatrix();
			clothShader.setVec3("objectColor", glm::vec3(1.0f, 1.0f, 1.0f));
			clothShader.setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
			clothShader.setMat4("projection", projection);
			clothShader.setMat4("view", view);
			clothShader.setMat4("model", model);
			clothShader.setVec3("lightPos", lightPos);
			clothShader.setVec3("viewPos", camera.Position);
			clothShader.setFloat("ambientStrength", 0.1);
			clothShader.setFloat("diffStrength", 0.8);
			clothShader.setFloat("specularStrength", 1);
			clothShader.setInt("shiny", 32);
			ourCloth.ClothSimulating(clothShader, deltaTime, 0.098, 0.5, 0.5, glm::vec3(2, 0, 1));
		}

		//particles
		{
			modelShader.use();
			modelShader.setVec3("light.position", lightPos);
			modelShader.setVec3("viewPos", camera.Position);

			// light properties
			modelShader.setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
			modelShader.setVec3("light.diffuse", 0.5f, 0.5f, 0.5f);
			modelShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);

			// material properties
			modelShader.setVec3("material.specular", 0.5f, 0.5f, 0.5f);
			modelShader.setFloat("material.shininess", 64.0f);

			// view/projection transformations
			glm::mat4 projection = glm::perspective(45.0f, (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
			glm::mat4 view = camera.GetViewMatrix();
			modelShader.setMat4("projection", projection);
			modelShader.setMat4("view", view);
			ourParticle.simulate(deltaTime);
			for (int i = 0; i < ourParticle.particles.size(); i++) {
				//modelShader.setFloat("alpha", ourParticle.particles[i].color.a);
				glm::mat4 model = ourParticle.particles[i].model;
				modelShader.setMat4("model", model);
				flowerModel.Draw(modelShader);
			}
			
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------
	glDeleteVertexArrays(1, &skyboxVAO);
	glDeleteBuffers(1, &skyboxVAO);
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1, &oceanVAO);
	glDeleteBuffers(1, &oceanPosVBO);
	glDeleteBuffers(1, &oceanNorVBO);

	glfwTerminate();
	return 0;
}

GLFWwindow* initOpenGL() {
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "finalProject", NULL, NULL);
	if (window == NULL)
	{
		cout << "Failed to create GLFW window" << endl;
		glfwTerminate();
		return 0;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		cout << "Failed to initialize GLAD" << endl;
		return 0;
	}
	return window;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
		lightPos.y += 1.0f;
		cout << lightPos.x << "    " << lightPos.y << "    " << lightPos.z << endl;
	}

	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		lightPos.y -= 1.0f;
		cout << lightPos.x << "    " << lightPos.y << "    " << lightPos.z << endl;
	}

	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
		lightPos.z += 1.0f;
		cout << lightPos.x << "    " << lightPos.y << "    " << lightPos.z << endl;
	}

	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		lightPos.z -= 1.0f;
		cout << lightPos.x << "    " << lightPos.y << "    " << lightPos.z << endl;
	}
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mod) {
	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
		dnFlag = !dnFlag;
		firstChange = true;
	}
	//if (key == GLFW_KEY_W && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
	//	camera.ProcessKeyboard(FORWARD, deltaTime);
	//}
	//if (key == GLFW_KEY_S && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
	//	camera.ProcessKeyboard(BACKWARD, deltaTime);
	//}
}

// glfw: whenever the window Size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const * path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		cout << "Texture failed to load at path: " << path << endl;
		stbi_image_free(data);
	}

	return textureID;
}

// loads a cubemap texture from 6 individual texture faces
// order:
// +X (right)
// -X (left)
// +Y (top)
// -Y (bottom)
// +Z (front) 
// -Z (back)
// -------------------------------------------------------
unsigned int loadCubemap(vector<string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			cout << "Cubemap texture failed to load at path: " << faces[i] << endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}

void RenderText(Shader &shader, string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color)
{
	// Activate corresponding render state	
	shader.use();
	shader.setVec3("textColor", color.x, color.y, color.z);
	//glUniform3f(glGetUniformLocation(shader.Program, "textColor"), color.x, color.y, color.z);
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(VAO);

	// Iterate through all characters
	std::string::const_iterator c;
	for (c = text.begin(); c != text.end(); c++)
	{
		Character ch = Characters[*c];

		GLfloat xpos = x + ch.Bearing.x * scale;
		GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

		GLfloat w = ch.Size.x * scale;
		GLfloat h = ch.Size.y * scale;
		// Update VBO for each character
		GLfloat vertices[6][4] = {
			{ xpos,     ypos + h,   0.0, 0.0 },
		{ xpos,     ypos,       0.0, 1.0 },
		{ xpos + w, ypos,       1.0, 1.0 },

		{ xpos,     ypos + h,   0.0, 0.0 },
		{ xpos + w, ypos,       1.0, 1.0 },
		{ xpos + w, ypos + h,   1.0, 0.0 }
		};
		// Render glyph texture over quad
		glBindTexture(GL_TEXTURE_2D, ch.TextureID);
		// Update content of VBO memory
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // Be sure to use glBufferSubData and not glBufferData

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		// Render quad
		glDrawArrays(GL_TRIANGLES, 0, 6);
		// Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void initText(Shader textShader) {
	// Set OpenGL options
	//glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Compile and setup the shader

	glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(SCR_WIDTH), 0.0f, static_cast<GLfloat>(SCR_HEIGHT));
	textShader.use();
	textShader.setMat4("projection", projection);

	// FreeType
	FT_Library ft;
	// All functions return a value different than 0 whenever an error occurred
	if (FT_Init_FreeType(&ft))
		cout << "ERROR::FREETYPE: Could not init FreeType Library" << endl;

	// Load font as face
	FT_Face face;
	if (FT_New_Face(ft, "C:/Windows/Fonts/Arial.ttf", 0, &face))
		cout << "ERROR::FREETYPE: Failed to load font" << endl;

	// 设置字体宽度和高度宽度设置为0根据高度动态计算
	FT_Set_Pixel_Sizes(face, 0, 48);

	// Disable byte-alignment restriction
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// Load first 128 characters of ASCII set
	for (GLubyte c = 0; c < 128; c++)
	{
		// Load character glyph 
		if (FT_Load_Char(face, c, FT_LOAD_RENDER))
		{
			cout << "ERROR::FREETYTPE: Failed to load Glyph" << endl;
			continue;
		}
		// Generate texture
		GLuint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RED,
			face->glyph->bitmap.width,
			face->glyph->bitmap.rows,
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			face->glyph->bitmap.buffer
		);
		// Set texture options
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// Now store character for later use
		Character character = {
			texture,
			glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
			glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
			face->glyph->advance.x
		};
		Characters.insert(std::pair<GLchar, Character>(c, character));
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	// Destroy FreeType once we're finished
	FT_Done_Face(face);
	FT_Done_FreeType(ft);


	// Configure VAO/VBO for texture quads
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}



static void initWave(void)
{
	//Initialize values{}
	values.time = 0.0;
	for(int w=0; w<WAVE_COUNT; w++)
	{
		values.wave_length[w] = wave_para[w][0];
		values.wave_height[w] = wave_para[w][1];
		values.wave_dir[w] = wave_para[w][2];
		values.wave_speed[w] = wave_para[w][3];
		values.wave_start[w*2] = wave_para[w][4];
		values.wave_start[w*2+1] = wave_para[w][5];
	}

	//Initialize pt_strip[]
	int index=0;
	for(int i=0; i<STRIP_COUNT; i++)
	{
		for(int j=0; j<STRIP_LENGTH; j++)
		{
			pt_strip[index] = START_X + i*LENGTH_X;
			pt_strip[index+2] = START_Z + j*LENGTH_Z;
			index += 3;
		}
	}
}

static float gerstnerY(float w_length, float w_height, float x_in, const GLfloat gerstner[22])
{
	x_in = x_in * 400.0 / w_length;

	while(x_in < 0.0)
		x_in += 400.0;
	while(x_in > 400.0)
		x_in -= 400.0;
	if(x_in > 200.0)
		x_in = 400.0 - x_in;

	int i = 0;
	float yScale = w_height/50.0;
	while(i<18 && (x_in<gerstner[i] || x_in>=gerstner[i+2]))
		i+=2;
	if(x_in == gerstner[i])
		return gerstner[i+1] * yScale;
	if(x_in > gerstner[i])
		return ((gerstner[i+3]-gerstner[i+1]) * (x_in-gerstner[i]) / (gerstner[i+2]-gerstner[i]) + gerstner[i+3]) * yScale;
    return 0;
}

static int normalizeF(float in[], float out[], int count)
{
	int t=0;
	float l = 0.0;

	if(count <= 0.0){
		printf("normalizeF(): Number of dimensions should be larger than zero.\n");
		return 1;
	}
	while(t<count && in[t]<0.0000001 && in[t]>-0.0000001){
		t++;
	}
	if(t == count){
		printf("normalizeF(): The input vector is too small.\n");
		return 1;
	}
	for(t=0; t<count; t++)
		l += in[t] * in[t];
	if(l < 0.0000001){
		l = 0.0;
		for(t=0; t<count; t++)
			in[t] *= 10000.0;
		for(t=0; t<count; t++)
			l += in[t] * in[t];
	}
	l = sqrt(l);
	for(t=0; t<count; t++)
		out[t] /= l;

	return 0;
}


static void calcuWave(void)
{
	//Calculate pt_strip[z], poly_normal[] and pt_normal[]
	int index=0;
	float d, wave;
	for(int i=0; i<STRIP_COUNT; i++)
	{
		for(int j=0; j<STRIP_LENGTH; j++)
		{
			wave = 0.0;
			for(int w=0; w<WAVE_COUNT; w++){
				d = (pt_strip[index] - values.wave_start[w*2] + (pt_strip[index+2] - values.wave_start[w*2+1]) * tan(values.wave_dir[w])) * cos(values.wave_dir[w]);
				if(gerstner_sort[w] == 1){
					wave += values.wave_height[w] - gerstnerY(values.wave_length[w], values.wave_height[w], d + values.wave_speed[w] * values.time, gerstner_pt_a);
				}else{
					wave += values.wave_height[w] - gerstnerY(values.wave_length[w], values.wave_height[w], d + values.wave_speed[w] * values.time, gerstner_pt_b);
				}
			}
			pt_strip[index+1] = START_Y + wave*HEIGHT_SCALE;
			index += 3;
		}
	}

	index = 0;
	for(int i=0; i<STRIP_COUNT; i++)
	{
		for(int j=0; j<STRIP_LENGTH; j++)
		{
			int p0 = index-STRIP_LENGTH*3, p1 = index+3, p2 = index+STRIP_LENGTH*3, p3 = index-3;
			float xa, ya, za, xb, yb, zb;
			if(i > 0){
				if(j > 0){
					xa = pt_strip[p0] - pt_strip[index], ya = pt_strip[p0+1] - pt_strip[index+1], za = pt_strip[p0+2] - pt_strip[index+2];
					xb = pt_strip[p3] - pt_strip[index], yb = pt_strip[p3+1] - pt_strip[index+1], zb = pt_strip[p3+2] - pt_strip[index+2];
					pt_normal[index] += ya*zb-yb*za;
					pt_normal[index+1] += xb*za-xa*zb;
					pt_normal[index+2] += xa*yb-xb*ya;
				}
				if(j < STRIP_LENGTH-1){
					xa = pt_strip[p1] - pt_strip[index], ya = pt_strip[p1+1] - pt_strip[index+1], za = pt_strip[p1+2] - pt_strip[index+2];
					xb = pt_strip[p0] - pt_strip[index], yb = pt_strip[p0+1] - pt_strip[index+1], zb = pt_strip[p0+2] - pt_strip[index+2];
					pt_normal[index] += ya*zb-yb*za;
					pt_normal[index+1] += xb*za-xa*zb;
					pt_normal[index+2] += xa*yb-xb*ya;
				}
			}
			if(i < STRIP_COUNT-1){
				if(j > 0){
					xa = pt_strip[p3] - pt_strip[index], ya = pt_strip[p3+1] - pt_strip[index+1], za = pt_strip[p3+2] - pt_strip[index+2];
					xb = pt_strip[p2] - pt_strip[index], yb = pt_strip[p2+1] - pt_strip[index+1], zb = pt_strip[p2+2] - pt_strip[index+2];
					pt_normal[index] += ya*zb-yb*za;
					pt_normal[index+1] += xb*za-xa*zb;
					pt_normal[index+2] += xa*yb-xb*ya;
				}
				if(j < STRIP_LENGTH-1){
					xa = pt_strip[p2] - pt_strip[index], ya = pt_strip[p2+1] - pt_strip[index+1], za = pt_strip[p2+2] - pt_strip[index+2];
					xb = pt_strip[p1] - pt_strip[index], yb = pt_strip[p1+1] - pt_strip[index+1], zb = pt_strip[p1+2] - pt_strip[index+2];
					pt_normal[index] += ya*zb-yb*za;
					pt_normal[index+1] += xb*za-xa*zb;
					pt_normal[index+2] += xa*yb-xb*ya;
				}
			}
			if(normalizeF(&pt_normal[index], &pt_normal[index], 3))
				printf("%d\t%d\n", index/3/STRIP_LENGTH, (index/3)%STRIP_LENGTH);

			index += 3;
		}
	}

	//Calculate vertex_data[] according to pt_strip[], and normal_data[] according to pt_normal[]
	int pt;
	for(int c=0; c<(STRIP_COUNT-1); c++)
	{
		for(int l=0; l<2*STRIP_LENGTH; l++)
		{
			if(l%2 == 1){
				pt = c*STRIP_LENGTH + l/2;
			}else{
				pt = c*STRIP_LENGTH + l/2 + STRIP_LENGTH;
			}
      index = STRIP_LENGTH*2*c+l;
			for(int i=0; i<3; i++){
				vertex_data[index*3+i] = pt_strip[pt*3+i];
				normal_data[index*3+i] = pt_normal[pt*3+i];
			}
		}
	}
}

