//#include <glutil2.h>
//#include <camera.h>
#include <iostream>

#include <files.hpp>
#include <model.hpp>
#include <cam.hpp>
#include <figures3.h>

#include <vector>
#include <math.h>
#include <stdlib.h>
#include <time.h>

const u32 FSIZE = sizeof(f32);
const u32 ISIZE = sizeof(i32);
const u32 SCR_WIDTH = 1280;
const u32 SCR_HEIGHT = 720;
const f32  ASPECT = (f32)SCR_WIDTH / (f32)SCR_HEIGHT;

glm::vec3 lightColor = glm::vec3(0.98f, 0.85f, 0.3f);

Cam* cam;
bool firstMouse = true;
f32 lastx = SCR_WIDTH / 2.0f;
f32 lasty = SCR_HEIGHT / 2.0f;

f32 deltaTime = 0.0f;
f32 lastFrame = 0.0f;

float *fNoiseSeed2D = nullptr;
float *fPerlinNoise2D = nullptr;
int nOctaveCount = 5;
float fScalingBias = 2.0f;

/**
 * keyboard input processing
 **/
void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		cam->processKeyboard(FORWARD, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		cam->processKeyboard(LEFT, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		cam->processKeyboard(BACKWARD, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		cam->processKeyboard(RIGHT, deltaTime);
	}
}

void mouse_callback(GLFWwindow* window, f64 xpos, f64 ypos) {
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
		cam->movePov(xpos, ypos);
	}
	else {
		cam->stopPov();
	}
}

void scroll_callback(GLFWwindow* window, f64 xoffset, f64 yoffset) {
	cam->processScroll((f32)yoffset);
}

void PerlinNoise2D(u32 nWidth, u32 nHeight, float *fSeed, int nOctaves, float fBias, float *fOutput) {
	for (int x = 0; x < nWidth; x++)
		for (int y = 0; y < nHeight; y++)
		{
			float fNoise = 0.0f;
			float fScaleAcc = 0.0f;
			float fScale = 1.0f;

			for (int o = 0; o < nOctaves; o++)
			{
				int nPitch = nWidth >> o;
				int nSampleX1 = (x / nPitch) * nPitch;
				int nSampleY1 = (y / nPitch) * nPitch;

				int nSampleX2 = (nSampleX1 + nPitch) % nWidth;
				int nSampleY2 = (nSampleY1 + nPitch) % nWidth;

				float fBlendX = (float)(x - nSampleX1) / (float)nPitch;
				float fBlendY = (float)(y - nSampleY1) / (float)nPitch;

				float fSampleT = (1.0f - fBlendX) * fSeed[nSampleY1 * nWidth + nSampleX1] + fBlendX * fSeed[nSampleY1 * nWidth + nSampleX2];
				float fSampleB = (1.0f - fBlendX) * fSeed[nSampleY2 * nWidth + nSampleX1] + fBlendX * fSeed[nSampleY2 * nWidth + nSampleX2];

				fScaleAcc += fScale;
				fNoise += (fBlendY * (fSampleB - fSampleT) + fSampleT) * fScale;
				fScale = fScale / fBias;
			}
			// Scale to seed range
			fOutput[y * nWidth + x] = fNoise / fScaleAcc;
		}

}

i32 main() {
	GLFWwindow* window = glutilInit(3, 3, SCR_WIDTH, SCR_HEIGHT, "Maincra");
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	Files* files = new Files("bin", "resources/textures", "resources/objects");

	Shader* shaderCubes = new Shader(files, "shaderC.vert", "shaderC.frag");
	Shader* shader = new Shader(files, "shader.vert", "shader.frag");

	Cube* cubex = new Cube();
	Model* golem = new Model(files, "golem/iron_golem.obj");
	Model* tree = new Model(files, "ftree/Tree.obj");
	Model* bat = new Model(files, "Bat/Bat.obj");

	srand(time(0));
	u32 n = 60;
	u32 capas = 5;


	fNoiseSeed2D = new float[n * n];
	fPerlinNoise2D = new float[n * n];
	for (int i = 0; i < n * n; i++) {
		fNoiseSeed2D[i] = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 4));//(float)rand() / (float)RAND_MAX;
	}
	PerlinNoise2D(n, n, fNoiseSeed2D, nOctaveCount, fScalingBias, fPerlinNoise2D);


	std::vector<glm::vec3> positions(n*n*capas);
	std::vector<glm::vec3> surface(n*n);

	for (u32 k = 0; k < capas; ++k) {
		for (u32 i = 0; i < n; ++i) {
			for (u32 j = 0; j < n; ++j) {
				f32 x = i - n / 2.0f;
				f32 z = j - n / 2.0f;
				f32 y = (u32)(fPerlinNoise2D[j * n + i] * 16.0f) - 1.0f*k;

				positions[(k*n*n) + i * n + j] = glm::vec3(x, y, z);
			}
		}
	}
	for (u32 i = 0; i < n*n; ++i) {
		surface[i] = positions[i];
	}

	//light position
	glm::vec3 lightPos = glm::vec3(positions[1].x, 20 + positions[1].y, positions[1].z);
	//camera position
	glm::vec3 initPos = glm::vec3(positions[1].x, 20 + positions[1].y, positions[1].z);

	cam = new Cam(initPos);

	u32 vbo, vao, ebo;
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);

	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, cubex->getVSize()*FSIZE,
		cubex->getVertices(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, cubex->getISize()*ISIZE,
		cubex->getIndices(), GL_STATIC_DRAW);

	// posiciones
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, cubex->len(), cubex->skip(0));
	glEnableVertexAttribArray(0);
	// normales
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, cubex->len(), cubex->skip(6));
	glEnableVertexAttribArray(2);
	// textures
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, cubex->len(), cubex->skip(9));
	glEnableVertexAttribArray(3);

	u32 grass = shaderCubes->loadTexture("block2.jpg");
	u32 stone = shaderCubes->loadTexture("stone.jpg");
	u32 sand = shaderCubes->loadTexture("sand.jpg");

	int moveX = 1;
	int moveZ = n;
	bool movement = true;
	float distanciaX = 0.0f; float nextY = 0.0f; float distanciaZ = 0.0f; float moveVert = 0.0f;
	u32 movetimer = 0; u32 maxTime = 20;

	u32 golemPos = rand() % (n*n);
	glm::vec3 golemCoord = glm::vec3(surface[golemPos].x + 0.5f, surface[golemPos].y + 1.1f, surface[golemPos].z + 0.5f);

	u32 treePos = rand() % (n*n);
	glm::vec3 treeCoord = glm::vec3(surface[treePos].x, surface[treePos].y + 0.5f, surface[treePos].z);

	glEnable(GL_DEPTH_TEST);

	while (!glfwWindowShouldClose(window)) {
		f32 currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		glBindTexture(GL_TEXTURE_2D, grass);

		processInput(window);
		glClearColor(0.52f, 0.81f, 0.92f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		shaderCubes->use();

		glm::mat4 proj = glm::perspective(cam->zoom, ASPECT, 0.1f, 100.0f);
		shaderCubes->setMat4("proj", proj);

		glm::mat4 view = glm::mat4(1.0f);
		shaderCubes->setMat4("view", cam->getViewM4());

		lightPos.x = float(n/2) * (cos(currentFrame) + sin(currentFrame));
		lightPos.z = float(n/2) * (cos(currentFrame) - sin(currentFrame));
		shaderCubes->setVec3("xyz", lightPos);
		shaderCubes->setVec3("xyzColor", lightColor);
		shaderCubes->setVec3("xyzView", cam->pos);

		glBindVertexArray(vao);
		u32 contador = 0;
		for (u32 i = 0; i < positions.size(); ++i) {

			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, positions[i]);
			if (positions[i][1] < surface[contador][1] - 2.0f) glBindTexture(GL_TEXTURE_2D, stone);
			else if (positions[i][0] <= 0) { glBindTexture(GL_TEXTURE_2D, sand); }
			else { glBindTexture(GL_TEXTURE_2D, grass); }

			shaderCubes->setMat4("model", model);

			glDrawElements(GL_TRIANGLES, 6 * 6, GL_UNSIGNED_INT, 0);
			contador++;
			if (contador >= n * n) contador = 0;
		}
		
		////////////////////////////////////////////// Objetos //////////////////////////////////////////////
		shader->use();

		shader->setVec3("xyz", lightPos);
		shader->setVec3("xyzColor", lightColor);
		shader->setVec3("xyzView", cam->pos);
		proj = glm::perspective(cam->zoom, ASPECT, 0.1f, 100.0f);
		shader->setMat4("proj", proj);
		shader->setMat4("view", cam->getViewM4());

		glm::mat4 model = glm::mat4(1.0f);

		model = translate(model, golemCoord);
		if (movement) {
			for (u32 i = 0; i < n; i++) {
				if (golemPos + moveX == i * n || golemPos + moveX == (i * n) + n) {
					moveX *= -1;
					break;
				}
			}
			for (u32 i = 0; i < n; i++) {
				if (golemPos + moveZ == i || golemPos + moveZ == (n*n) - n + i) {
					moveZ *= -1;
					break;
				}
			}
			golemPos += moveX + moveZ;
			golemCoord = glm::vec3(surface[golemPos].x + 0.5f, surface[golemPos].y + 1.1f, surface[golemPos].z + 0.5f);
			moveX = rand() % 3 - 1;
			moveZ = 60 * (rand() % 3 - 1);
			distanciaX = (surface[golemPos + moveX + moveZ].x + 0.5f) - (golemCoord.x);
			distanciaZ = (surface[golemPos + moveX + moveZ].z + 0.5f) - (golemCoord.z);
			nextY = surface[golemPos + moveX + moveZ].y + 1.1f;
			movement = false;
		}
		else {
			golemCoord.x += (distanciaX / maxTime);
			golemCoord.z += (distanciaZ / maxTime);
			if (movetimer == maxTime/2) { golemCoord.y = nextY; }
		}
		model = glm::scale(model, glm::vec3(0.08f));
		shader->setMat4("model", model);
		golem->Draw(shader);
		
		/////// Arbol
		model = glm::mat4(1.0f);
		model = translate(model, treeCoord);
		shader->setMat4("model", model);
		tree->Draw(shader);

		/////// Murcielago
		model = glm::mat4(1.0f);
		glm::vec3 batCoord = glm::vec3(cam->pos.x, cam->pos.y, cam->pos.z);
		batCoord.x += float(10) * (cos(currentFrame) + sin(currentFrame));
		batCoord.z += float(10) * (cos(currentFrame) - sin(currentFrame));
		model = translate(model, batCoord);
		shader->setMat4("model", model);
		bat->Draw(shader);

		movetimer += 1;
		if (movetimer >= maxTime) { movement = true; movetimer = 0; }
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ebo);

	delete shaderCubes;
	delete shader;
	delete golem;
	delete tree;
	delete cubex;
	delete cam;
	delete files;

	return 0;
}

/* vim: set tabstop=2:softtabstop=2:shiftwidth=2:noexpandtab */
