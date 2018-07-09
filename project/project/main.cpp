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
#include <vector>
#include "model.h"
#include <iostream>
#include <freeglut/freeglut.h>
#include <gltools/GLTools.h>
#include "Particle.h"
#include "cloth.h"
using namespace std;

/** 创建一个粒子类对象 */
//CParticle Snow;
///** 用来设置粒子的属性值 */
//float x, y, z, vx, vy, vz, ax, ay, az, size, lifetime, dec;
//int r, g, b;

/** 初始化雪花粒子 */
//bool InitSnow()
//{
//	for (int i = 0; i < Snow.GetNumOfParticle(); ++i)
//	{
//		///初始化颜色（白色）
//		r = 255;
//		g = 255;
//		b = 255;
//		Snow.SetColor(i, r, g, b);
//
//		///初始化坐标
//		x = 0.1f * (rand() % 50) - 2.5f;
//		y = 2 + 0.1f * (rand() % 2);
//		if ((int)x % 2 == 0)
//			z = rand() % 6;
//		else
//			x = -rand() % 3;
//		Snow.SetPosition(i, x, y, z);
//
//		///初始化速度
//		vx = 0.00001 * (rand() % 100);
//		vy = 0.0000002 * (rand() % 28000);
//		vz = 0;
//		Snow.SetVelocity(i, vx, vy, vz);
//
//		///初始化加速度
//		ax = 0;
//		ay = 0.000005f;
//		az = 0;
//		Snow.SetAcceleration(i, ax, ay, az);
//
//		///初始化生命周期
//		lifetime = 100;
//		Snow.SetLifeTime(i, lifetime);
//
//		///消失速度
//		dec = 0.005 * (rand() % 50);
//		Snow.SetDec(i, dec);
//
//		///初始化大小
//		Snow.SetSize(i, 0.03f);
//	}
//	return true;
//}
/** 更新粒子 */
//void UpdateSnow()
//{
//	/** 更新位置 */
//	x += (vx * 5);
//	y -= vy;
//
//	/** 更新速度 */
//	vy += ay;
//
//	/** 更新生存时间 */
//	lifetime -= dec;
//
//	if (x > 3)
//		x = -2;
//
//	/** 如果粒子消失或生命结束 */
//	if (y <= -1 || lifetime <= 0)
//	{
//		/** 初始化位置 */
//		x = 0.1f * (rand() % 50) - 2.5f;
//		y = 2 + 0.1f * (rand() % 2);
//		if ((int)x % 2 == 0)
//			z = rand() % 6;
//		else
//			z = -rand() % 3;
//
//		/** 初始化速度 */
//		vx = (float)(0.00001 * (rand() % 100));
//		vy = (float)(0.0000002 * (rand() % 28000));
//		vz = 0;
//
//		/** 初始化加速度 */
//		ax = 0;
//		ay = 0.000005f;
//		az = 0;
//		lifetime = 100;
//		dec = 0.005*(rand() % 50);
//	}
//}
//void DrawParticle()
//{
//	/** 绑定纹理 */
//	glBindTexture(GL_TEXTURE_2D, texName[1]);
//
//	for (int i = 0; i<Snow.GetNumOfParticle(); ++i)
//	{
//		/** 获得粒子的所有属性 */
//		Snow.GetAll(i, r, g, b, x, y, z, vx, vy, vz, ax, ay, az, size, lifetime, dec);
//		glLoadIdentity();
//		glTranslatef(0.0f, 0.0f, -6.0f);
//		glColor4ub(r, g, b, 255);
//		glNormal3f(0.0f, 0.0f, 1.0f);   /**< 定义法线方向 */
//										/** 画出粒子 */
//		glBegin(GL_QUADS);
//		glTexCoord2f(0.0f, 0.0f); glVertex3f(x - size, y - size, z);
//		glTexCoord2f(1.0f, 0.0f); glVertex3f(x - size, y + size, z);
//		glTexCoord2f(1.0f, 1.0f); glVertex3f(x + size, y + size, z);
//		glTexCoord2f(0.0f, 1.0f); glVertex3f(x + size, y - size, z);
//		glEnd();
//
//		/** 更新粒子属性 */
//		UpdateSnow();
//		Snow.SetAll(i, r, g, b, x, y, z, vx, vy, vz, ax, ay, az, size, lifetime, dec);
//	}
//	glutPostRedisplay();//发送渲染请求
//}
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
void processInput(GLFWwindow *window);
unsigned int loadTexture(const char *path);
unsigned int loadCubemap(vector<string> faces);

GLFWwindow* initOpenGL();
// settings
const unsigned int SCR_WIDTH = 1280;
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

// lighting
glm::vec3 lightPos(0.0f, 0.0f, 15.0f);
int main()
{
	GLFWwindow* window = initOpenGL();

	Shader textShader("text.vs", "text.fs");
	initText(textShader);

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);


	Shader skyboxShader("./skybox.vs", "./skybox.fs");
	Shader modelShader("./modelshader.vs", "./modelshader.fs");
	Shader clothShader("./cloth.vs", "./cloth.fs");
	Model ourModel("./newbeach/beach_final_test.obj");
	ClothUtil ourCloth = ClothUtil(15);


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

	

	vector<string> faces
	{
		("resources/textures/skybox/right.jpg"),
		("resources/textures/skybox/left.jpg"),
		("resources/textures/skybox/top.jpg"),
		("resources/textures/skybox/bottom.jpg"),
		("resources/textures/skybox/back.jpg"),
		("resources/textures/skybox/front.jpg")
	};
	unsigned int cubemapTexture = loadCubemap(faces);



	skyboxShader.use();
	skyboxShader.setInt("skybox", 0);

	glm::vec3 modelPos = glm::vec3(0, 0, 0);

	// shader configuration
	// --------------------
	modelShader.use();
	modelShader.setInt("material.diffuse", 0);

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

		// display text
		{
			glEnable(GL_CULL_FACE);
			stringstream ss;
			ss << fps;;
			string temp;
			ss >> temp;
			ss.clear();
			string fpsShow = "FPS:  " + temp;
			RenderText(textShader, fpsShow, 25.0f, 25.0f, 1.0f, glm::vec3(0.5, 0.8f, 0.2f));
		}

		//model
		{
			//modelShader.use();

			//glm::mat4 projection = glm::perspective(45.0f, (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
			//glm::mat4 view = camera.GetViewMatrix();
			//modelShader.setMat4("projection", projection);
			//modelShader.setMat4("view", view);

			//glm::mat4 model;
			//model = glm::translate(model, modelPos);
			////model = glm::translate(model, glm::vec3(0.0f, -50.0f, 0.0f));
			////model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
			//modelShader.setMat4("model", model);
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
			glm::mat4 projection = glm::perspective(45.0f, (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
			glm::mat4 view = camera.GetViewMatrix();
			modelShader.setMat4("projection", projection);
			modelShader.setMat4("view", view);

			// world transformation
			glm::mat4 model;
			modelShader.setMat4("model", model);
			ourModel.Draw(modelShader);
		}

		// skybox
		{
			glm::mat4 view = camera.GetViewMatrix();
			glm::mat4 projection = glm::perspective(45.0f, (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
			// draw skybox as last
			glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
			skyboxShader.use();
			view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation from the view matrix
			skyboxShader.setMat4("view", view);
			skyboxShader.setMat4("projection", projection);
			// skybox cube
			glBindVertexArray(skyboxVAO);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
			glDrawArrays(GL_TRIANGLES, 0, 36);
			glBindVertexArray(0);
			glDepthFunc(GL_LESS); // set depth function back to default
		}

		//cloth
		{
			glDisable(GL_CULL_FACE);
			clothShader.use();
			glm::mat4 model;
			model = glm::mat4();
			//model = glm::translate(model, glm::vec3(0, 5, -2));
			model = glm::translate(model, glm::vec3(0, 5, 10));
			model = glm::scale(model, glm::vec3(3, 3, 3));
			glm::mat4 projection = glm::perspective(45.0f, (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
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
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

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
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
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
	glEnable(GL_CULL_FACE);
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