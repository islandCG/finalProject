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
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mod);
void processInput(GLFWwindow *window);
unsigned int loadTexture(const char *path);
unsigned int loadCubemap(vector<string> faces);

GLFWwindow* initOpenGL();
// settings
const unsigned int SCR_WIDTH = 1080;
const unsigned int SCR_HEIGHT = 720;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
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
	Model ourModel("./newbeach/beach_final_test.obj");
	Model flowerModel("./flower/flower.obj");
	ClothUtil ourCloth = ClothUtil(15);
	ParticleSystem ourParticle = ParticleSystem(300, glm::vec3(0, -0.98, 0), glm::vec3(-10, 10, -15) + glm::vec3(-22, -12, -77));


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
		if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
			modelPos.y -= 25 * deltaTime;
		}
		//按e向下移动镜头
		if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
			modelPos.y += 25 * deltaTime;
		}
		if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
			modelPos.x -= 25 * deltaTime;
		}
		if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS) {
			modelPos.x += 25 * deltaTime;
			cout << 3 << endl;
		}
		if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) {
			modelPos.z -= 25 * deltaTime;
		}
		if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) {
			modelPos.z += 25 * deltaTime;
		}
		// render
		// ------
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		// 1. Render depth of scene to texture (from light's perspective)
		// - Get light projection/view matrix.
		glm::mat4 lightProjection, lightView;
		glm::mat4 lightSpaceMatrix;
		GLfloat near_plane = 1.0f, far_plane = 100.0f;
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
			model = glm::translate(model, glm::vec3(-22, -12, -77));
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

		//cloth
		{
			clothShader.use();
			glm::mat4 model;
			model = glm::mat4();
			//model = glm::translate(model, glm::vec3(0, 5, -2));
			model = glm::translate(model, glm::vec3(0, 5, 10) + glm::vec3(-22, -12, -77));
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
	//glDeleteVertexArrays(1, &cubeVAO);
	glDeleteVertexArrays(1, &skyboxVAO);
	//glDeleteBuffers(1, &cubeVBO);
	glDeleteBuffers(1, &skyboxVAO);

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
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetKeyCallback(window, key_callback);

	// tell GLFW to capture our mouse
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

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