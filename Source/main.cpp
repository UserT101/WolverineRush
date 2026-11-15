#pragma once
// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include <iostream>
#include <cmath>
#include <string>
#include <stdexcept>
#include <windows.h>
#include <gdiplus.h>

#pragma comment(lib, "gdiplus.lib")


// Include GLEW and GLFW
#include "glew/glew.h"
#include "glfw/glfw3.h"



// Include GLM
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

// Include our helper function for loading shaders
#include "Shaders/Utils/shaderLoader.hpp"

// Global variables
#include "Globals/globals.hpp"

struct Obstacle
{
	float x;
	float y;
	float size;
	bool nextObstacleSpawned;
	bool isHighObstacle;
};

enum class DeadpoolState { IDLE, REACH, LIFT, THROW };

struct Projectile {
	float x, y;
	float vx, vy;
	float rotation;
	bool active;
	bool isShuriken;
};

void drawPrimitive(
	GLuint programID,
	GLuint vao,
	const glm::mat4& parentTransform,
	float xOffset,
	float yOffset,
	float width,
	float height,
	float rotation,
	const glm::vec4& color,
	GLuint texture = 0)
{
	glm::mat4 transform = parentTransform;
	transform = glm::translate(transform, glm::vec3(xOffset, yOffset, 0.0f));
	transform = glm::rotate(transform, rotation, glm::vec3(0.0f, 0.0f, 1.0f));
	transform = glm::scale(transform, glm::vec3(width / 0.1f, height / 0.1f, 1.0f));

	GLuint transformLocation = glGetUniformLocation(programID, "transform");
	glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(transform));

	GLuint colorLocation = glGetUniformLocation(programID, "color");
	glUniform4fv(colorLocation, 1, glm::value_ptr(color));

	GLuint useTextureLocation = glGetUniformLocation(programID, "useTexture");
	glUniform1i(useTextureLocation, texture != 0 ? GL_TRUE : GL_FALSE);

	if (texture != 0)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		glUniform1i(glGetUniformLocation(programID, "textureSampler"), 0);
	}

	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

// adjusting the viewport when resizing the window
void window_callback(GLFWwindow* window, int new_width, int new_height)
{
	glViewport(0, 0, new_width, new_height);
}

void drawSquare(
	GLuint programID,
	GLuint vao,
	float x,
	float y,
	float width,
	float height,
	const glm::vec4& color,
	GLuint texture = 0,
	float rotation = 0.0f)
{
	glm::mat4 transform(1.0f);
	transform = glm::translate(transform, glm::vec3(x, y, 0.0f));
	transform = glm::rotate(transform, rotation, glm::vec3(0.0f, 0.0f, 1.0f));
	transform = glm::scale(transform, glm::vec3(width / 0.1f, height / 0.1f, 1.0f));

	GLuint transformLocation = glGetUniformLocation(programID, "transform");
	glUniformMatrix4fv(
		transformLocation,
		1,
		GL_FALSE,
		glm::value_ptr(transform));

	GLuint colorLocation = glGetUniformLocation(programID, "color");
	glUniform4fv(colorLocation, 1, glm::value_ptr(color));

	GLuint useTextureLocation = glGetUniformLocation(programID, "useTexture");
	glUniform1i(useTextureLocation, texture != 0 ? GL_TRUE : GL_FALSE);

	if (texture != 0)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		glUniform1i(glGetUniformLocation(programID, "textureSampler"), 0);
	}

	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

GLuint loadTexture(const char* filePath)
{
	std::vector<std::wstring> paths;
	std::string path(filePath);
	std::wstring widePath(path.begin(), path.end());
	paths.push_back(widePath);

	char executablePath[MAX_PATH];
	DWORD pathLength = GetModuleFileNameA(NULL, executablePath, MAX_PATH);
	if (pathLength > 0 && pathLength < MAX_PATH)
	{
		std::string executableDirectory(executablePath, pathLength);
		size_t separator = executableDirectory.find_last_of("\\/");
		if (separator != std::string::npos)
		{
			executableDirectory.erase(separator);
			const std::string prefixes[] = { "/", "/../", "/../../" };
			for (size_t i = 0; i < 3; ++i)
			{
				std::string candidate = executableDirectory + prefixes[i] + path;
				paths.push_back(std::wstring(candidate.begin(), candidate.end()));
			}
		}
	}

	Gdiplus::Bitmap* bitmap = NULL;
	for (size_t i = 0; i < paths.size() && bitmap == NULL; ++i)
	{
		bitmap = Gdiplus::Bitmap::FromFile(paths[i].c_str(), FALSE);
		if (bitmap != NULL && bitmap->GetLastStatus() != Gdiplus::Ok)
		{
			delete bitmap;
			bitmap = NULL;
		}
	}

	if (bitmap == NULL)
	{
		throw std::runtime_error("Cannot open image file: " + path);
	}

	const UINT width = bitmap->GetWidth();
	const UINT height = bitmap->GetHeight();
	std::vector<unsigned char> pixels(width * height * 4);
	Gdiplus::Rect rectangle(0, 0, width, height);
	Gdiplus::BitmapData bitmapData;

	if (bitmap->LockBits(&rectangle, Gdiplus::ImageLockModeRead,
		PixelFormat32bppARGB, &bitmapData) != Gdiplus::Ok)
	{
		delete bitmap;
		throw std::runtime_error("Cannot read image file: " + path);
	}

	for (UINT y = 0; y < height; ++y)
	{
		const unsigned char* source = static_cast<const unsigned char*>(bitmapData.Scan0) + y * bitmapData.Stride;
		unsigned char* destination = &pixels[(height - 1 - y) * width * 4];
		for (UINT x = 0; x < width; ++x)
		{
			destination[x * 4 + 0] = source[x * 4 + 2];
			destination[x * 4 + 1] = source[x * 4 + 1];
			destination[x * 4 + 2] = source[x * 4 + 0];
			destination[x * 4 + 3] = source[x * 4 + 3];
		}
	}

	bitmap->UnlockBits(&bitmapData);
	delete bitmap;

	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &pixels[0]);

	return texture;
}

bool checkCollision(
	float playerX,
	float playerY,
	float playerWidth,
	float playerHeight,
	const Obstacle& obstacle)
{
	const float playerLeft = playerX - playerWidth / 2.0f;
	const float playerRight = playerX + playerWidth / 2.0f;
	const float playerBottom = playerY - playerHeight / 2.0f;
	const float playerTop = playerY + playerHeight / 2.0f;

	const float obstacleLeft = obstacle.x - obstacle.size / 2.0f;
	const float obstacleRight = obstacle.x + obstacle.size / 2.0f;
	const float obstacleBottom = obstacle.y - obstacle.size / 2.0f;
	const float obstacleTop = obstacle.y + obstacle.size / 2.0f;

	return playerLeft < obstacleRight&&
		playerRight > obstacleLeft &&
		playerBottom < obstacleTop&&
		playerTop > obstacleBottom;
}

Obstacle createObstacle(float x)
{
	Obstacle obstacle;
	obstacle.x = x;
	obstacle.size = 0.14f;
	obstacle.nextObstacleSpawned = false;

	const bool isHighObstacle = rand() % 2 == 0;
	obstacle.isHighObstacle = isHighObstacle;

	if (isHighObstacle)
	{
		// Obstacol sus
		obstacle.y = -0.40f;
	}
	else
	{
		// Obstacol jos
		obstacle.y = -0.66f;
	}

	return obstacle;
}

void drawDeadpool(
	GLuint programID, GLuint vao,
	float x, float y, float rightArmRotation, float leftArmRotation, bool headFacesLeft,
	GLuint bodyTex, GLuint headTex, GLuint armTex, GLuint boxTex, GLuint shuTex,
	DeadpoolState state, bool isShuriken, bool itemThrown)
{
	glm::mat4 bodyTransform(1.0f);
	bodyTransform = glm::translate(bodyTransform, glm::vec3(x, y, 0.0f));

	drawPrimitive(programID, vao, bodyTransform, 0.0f, 0.0f, 0.18f, 0.32f, 0.0f, glm::vec4(1.0f), bodyTex);

	float headWidth = headFacesLeft ? 0.05f : -0.05f;
	drawPrimitive(programID, vao, bodyTransform, 0.0f, 0.14f, headWidth, 0.05f, 0.0f, glm::vec4(1.0f), headTex);

	bool flipHands = (!isShuriken && state != DeadpoolState::IDLE);
	float leftArmWidth = flipHands ? 0.04f : -0.04f;
	float rightArmWidth = flipHands ? -0.04f : 0.04f;	

	glm::mat4 leftArmPivot = bodyTransform;
	leftArmPivot = glm::translate(leftArmPivot, glm::vec3(-0.055f, 0.12f, 0.0f));
	leftArmPivot = glm::rotate(leftArmPivot, leftArmRotation, glm::vec3(0.0f, 0.0f, 1.0f));
	drawPrimitive(programID, vao, leftArmPivot, 0.0f, -0.09f, leftArmWidth, 0.18f, 0.0f, glm::vec4(1.0f), armTex);

	glm::mat4 rightArmPivot = bodyTransform;
	rightArmPivot = glm::translate(rightArmPivot, glm::vec3(0.07f, 0.11f, 0.0f));
	rightArmPivot = glm::rotate(rightArmPivot, rightArmRotation, glm::vec3(0.0f, 0.0f, 1.0f));
	drawPrimitive(programID, vao, rightArmPivot, 0.0f, -0.09f, rightArmWidth, 0.18f, 0.0f, glm::vec4(1.0f), armTex);

	if (state != DeadpoolState::IDLE && !itemThrown)
	{
		if (!isShuriken) {

			glm::mat4 boxPivot = bodyTransform;
			boxPivot = glm::translate(boxPivot, glm::vec3(0.0f, 0.11f, 0.0f));
			boxPivot = glm::rotate(boxPivot, rightArmRotation, glm::vec3(0.0f, 0.0f, 1.0f));

			drawPrimitive(programID, vao, boxPivot, 0.0f, -0.19f, 0.14f, 0.14f, 0.0f, glm::vec4(1.0f), boxTex);
		}
		else {
			drawPrimitive(programID, vao, rightArmPivot, 0.0f, -0.19f, 0.10f, -0.15f, 0.0f, glm::vec4(1.0f), shuTex);
		}
	}
}

int main(void)
{
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(windowWidth, windowHeight, "Joc 2D", NULL, NULL);
	if (window == NULL)
	{
		fprintf(stderr, "Failed to open GLFW window.\n");
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glewExperimental = true;
	if (glewInit() != GLEW_OK)
	{
		fprintf(stderr, "Failed to initialize GLEW\n");
		glfwTerminate();
		return -1;
	}

	glViewport(0, 0, windowWidth, windowHeight);
	glfwSetFramebufferSizeCallback(window, window_callback);

	GLuint programID = LoadShaders(
		"Source/Shaders/SimpleVertexShader.vert",
		"Source/Shaders/SimpleFragmentShader.frag");

	GLfloat vertices[] = {
		0.05f,  0.05f, 0.0f,
		0.05f, -0.05f, 0.0f,
		-0.05f, -0.05f, 0.0f,
		-0.05f,  0.05f, 0.0f
	};
	GLuint indices[] = { 0, 3, 1, 1, 3, 2 };

	GLuint vao, vbo, ibo;
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ibo);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
	glEnableVertexAttribArray(0);

	GLfloat textureCoordinates[] = {
		1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f
	};
	GLuint textureCoordinateBuffer;
	glGenBuffers(1, &textureCoordinateBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, textureCoordinateBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(textureCoordinates), textureCoordinates, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), 0);
	glEnableVertexAttribArray(1);

	GLuint heartTexture = loadTexture("Resources/Images/heart_full.png");
	GLuint emptyHeartTexture = loadTexture("Resources/Images/heart_blank.png");
	GLuint woodTexture = loadTexture("Resources/Images/wood.bmp");
	GLuint upperObstacleTexture = loadTexture("Resources/Images/dpSword.png");
	GLuint backgroundTexture = loadTexture("Resources/Images/fundal.png");
	GLuint wolverineRunningTexture = loadTexture("Resources/Images/wolverine_running.png");
	GLuint wolverineCrouchingTexture = loadTexture("Resources/Images/wolverine_crouching.png");
	GLuint dpBodyTexture = loadTexture("Resources/Images/dpBody.png");
	GLuint dpHeadTexture = loadTexture("Resources/Images/dpHead.png");
	GLuint dpArmTexture = loadTexture("Resources/Images/dpArm.png");
	GLuint truckTexture = loadTexture("Resources/Images/truck.png");

	const float playerX = -0.60f;
	const float groundY = -0.75f;
	const float standingWidth = 0.24f;
	const float standingHeight = 0.30f;
	const float crouchingWidth = 0.26f;
	const float crouchingHeight = 0.18f;
	float playerY = groundY + standingHeight / 2.0f;
	float verticalVelocity = 0.0f;
	const float gravity = -3.5f;
	const float jumpVelocity = 1.55f;
	const float obstacleSpeed = 0.65f;
	bool isJumping = false;
	bool gameOver = false;
	int life = 3;
	bool isInvincible = false;
	float invincibilityTimer = 0.0f;
	const float invincibilityDuration = 1.0f;
	const float backgroundSpeed = 0.65f;
	float backgroundX = 0.0f;
	float upperObstacleRotation = 0.0f;

	srand(static_cast<unsigned int>(time(NULL)));
	std::vector<Obstacle> obstacles;
	double previousTime = glfwGetTime();
	glClearColor(0.05f, 0.08f, 0.20f, 1.0f);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Deadpool Animation & Projectile Variables
	DeadpoolState dpState = DeadpoolState::IDLE;
	float dpAnimTimer = 0.0f;
	float dpArmRotation = 0.0f;
	bool nextIsShuriken = false;
	float dpRightArmRot = 0.0f;
	float dpLeftArmRot = 0.0f;
	bool dpHeadFacesLeft = false;

	Projectile activeProjectile = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, false, false };

	float spawnTimer = 0.0f;
	const float baseSpawnCooldown = 1.5f;

	while (!glfwWindowShouldClose(window))
	{
		const double currentTime = glfwGetTime();
		float deltaTime = static_cast<float>(currentTime - previousTime);
		previousTime = currentTime;
		if (deltaTime > 0.05f) deltaTime = 0.05f;
		glfwPollEvents();

		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);

		// deltaTime la toate pentru a face orice obiect sa se miste la fel de repede
		if (!gameOver)
		{
			backgroundX -= backgroundSpeed * deltaTime;
			upperObstacleRotation -= 720.0f * deltaTime;
			if (backgroundX <= -2.0f)
				backgroundX += 2.0f;
		}

		if (!gameOver)
		{
			if (isInvincible)
			{
				invincibilityTimer -= deltaTime;
				if (invincibilityTimer <= 0.0f)
				{
					invincibilityTimer = 0.0f;
					isInvincible = false;
				}
			}

			const bool isHoldingDown = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;
			const bool useCrouchPose = isHoldingDown || isJumping;
			const float playerWidth = useCrouchPose ? crouchingWidth : standingWidth;
			const float playerHeight = useCrouchPose ? crouchingHeight : standingHeight;

			if (!isJumping && glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS && !isHoldingDown)
			{
				isJumping = true;
				verticalVelocity = jumpVelocity;
			}
			else if (!isJumping)
				playerY = groundY + playerHeight / 2.0f;

			if (isJumping)
			{
				verticalVelocity += gravity * deltaTime;
				playerY += verticalVelocity * deltaTime;
				const float standingGroundY = groundY + standingHeight / 2.0f;
				if (playerY <= standingGroundY)
				{
					playerY = standingGroundY;
					verticalVelocity = 0.0f;
					isJumping = false;
				}
			}

			std::vector<Obstacle> newObstacles;

			// Deadpool Animation Logic
			spawnTimer -= deltaTime;
			if (spawnTimer <= 0.0f && dpState == DeadpoolState::IDLE)
			{
				nextIsShuriken = (rand() % 2 == 0);
				dpAnimTimer = 0.0f;
				dpState = DeadpoolState::REACH;
			}

			if (dpState != DeadpoolState::IDLE)
			{
				dpAnimTimer += deltaTime;

				if (!nextIsShuriken)
				{
					if (dpState == DeadpoolState::REACH) {
						dpHeadFacesLeft = true;
						dpRightArmRot = 130.0f * (dpAnimTimer / 0.4f);
						dpLeftArmRot = 130.0f * (dpAnimTimer / 0.4f);
						if (dpAnimTimer >= 0.4f) { dpState = DeadpoolState::LIFT; dpAnimTimer = 0.0f; }
					}
					else if (dpState == DeadpoolState::LIFT) {
						dpHeadFacesLeft = false;
						dpRightArmRot = 130.0f + (50.0f * (dpAnimTimer / 0.5f));
						dpLeftArmRot = 130.0f + (50.0f * (dpAnimTimer / 0.5f));
						if (dpAnimTimer >= 0.5f) { dpState = DeadpoolState::THROW; dpAnimTimer = 0.0f; }
					}
					else if (dpState == DeadpoolState::THROW) {
						dpHeadFacesLeft = false;
						dpRightArmRot = 180.0f - (240.0f * (dpAnimTimer / 0.15f));
						dpLeftArmRot = 180.0f - (240.0f * (dpAnimTimer / 0.15f));
						if (dpAnimTimer >= 0.15f) {
							dpState = DeadpoolState::IDLE;
							dpRightArmRot = 0.0f;
							dpLeftArmRot = 0.0f;
							spawnTimer = baseSpawnCooldown;

							activeProjectile.active = true;
							activeProjectile.isShuriken = false;
							activeProjectile.x = 0.40f;
							activeProjectile.y = -0.24f;
							activeProjectile.vx = -0.65f;
							activeProjectile.vy = 1.5f;
						}
					}
				}
				else
				{
					dpHeadFacesLeft = false;
					dpLeftArmRot = 0.0f;

					if (dpState == DeadpoolState::REACH) {
						dpRightArmRot = 140.0f * (dpAnimTimer / 0.3f);
						if (dpAnimTimer >= 0.3f) { dpState = DeadpoolState::THROW; dpAnimTimer = 0.0f; }
					}
					else if (dpState == DeadpoolState::THROW) {
						dpRightArmRot = 140.0f - (230.0f * (dpAnimTimer / 0.1f));
						if (dpAnimTimer >= 0.1f) {
							dpState = DeadpoolState::LIFT;
							dpAnimTimer = 0.0f;

							activeProjectile.active = true;
							activeProjectile.isShuriken = true;
							activeProjectile.x = 0.40f;
							activeProjectile.y = -0.40f;
							activeProjectile.vx = -1.0f;
							activeProjectile.vy = 0.0f;
						}
					}
					else if (dpState == DeadpoolState::LIFT) {
						dpRightArmRot = -90.0f;
						if (dpAnimTimer >= 0.3f) {
							dpState = DeadpoolState::IDLE;
							dpRightArmRot = 0.0f;
							spawnTimer = baseSpawnCooldown;
						}
					}
				}
			}

			// Projectile Physics
			if (activeProjectile.active)
			{
				activeProjectile.x += activeProjectile.vx * deltaTime;
				activeProjectile.y += activeProjectile.vy * deltaTime;
				activeProjectile.rotation -= 10.0f * deltaTime;

				if (!activeProjectile.isShuriken) {
					activeProjectile.vy += gravity * deltaTime;
				}

				if ((!activeProjectile.isShuriken && activeProjectile.y <= -0.66f) ||
					(activeProjectile.isShuriken && activeProjectile.x <= 0.40f))
				{
					Obstacle newObs;
					newObs.x = activeProjectile.x;
					newObs.y = activeProjectile.isShuriken ? -0.40f : -0.66f;
					newObs.size = 0.14f;
					newObs.nextObstacleSpawned = false;
					newObs.isHighObstacle = activeProjectile.isShuriken;

					newObstacles.push_back(newObs);
					activeProjectile.active = false;
				}
			}
			for (size_t i = 0; i < obstacles.size(); ++i)
			{
				Obstacle& obstacle = obstacles[i];
				obstacle.x -= obstacleSpeed * deltaTime;

				if (!isInvincible && checkCollision(playerX, playerY, playerWidth, playerHeight, obstacle))
				{
					--life;
					isInvincible = true;
					invincibilityTimer = invincibilityDuration;
				}
			}

			if (life <= 0)
			{
				gameOver = true;
				glfwSetWindowTitle(window, "GAME OVER - apasa ESC pentru iesire");
			}
			for (size_t i = 0; i < newObstacles.size(); ++i)
				obstacles.push_back(newObstacles[i]);
			for (size_t i = 0; i < obstacles.size();)
			{
				if (obstacles[i].x < -1.20f) obstacles.erase(obstacles.begin() + i);
				else ++i;
			}
		}

		glClear(GL_COLOR_BUFFER_BIT);
		glUseProgram(programID);
		drawSquare(programID, vao, backgroundX, 0.0f, 2.0f, 2.0f, glm::vec4(1.0f), backgroundTexture);
		drawSquare(programID, vao, backgroundX + 2.0f, 0.0f, 2.0f, 2.0f, glm::vec4(1.0f), backgroundTexture);

		drawDeadpool(
			programID, vao,
			0.50f, -0.57f, dpRightArmRot, dpLeftArmRot, dpHeadFacesLeft,
			dpBodyTexture, dpHeadTexture, dpArmTexture, woodTexture, upperObstacleTexture,
			dpState, nextIsShuriken, activeProjectile.active
		);

		drawSquare(programID, vao, 0.75f, -0.65f, 0.8f, 0.5f, glm::vec4(1.0f), truckTexture);

		if (activeProjectile.active)
		{
			GLuint projTexture = activeProjectile.isShuriken ? upperObstacleTexture : woodTexture;
			drawSquare(
				programID, vao,
				activeProjectile.x, activeProjectile.y,
				0.14f, 0.14f,
				glm::vec4(1.0f),
				projTexture,
				activeProjectile.rotation
			);
		}

		const bool useCrouchPose = (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || isJumping) && !gameOver;
		const float playerWidth = useCrouchPose ? crouchingWidth : standingWidth;
		const float playerHeight = useCrouchPose ? crouchingHeight : standingHeight;
		const glm::vec4 playerColor = gameOver ? glm::vec4(0.8f, 0.2f, 0.2f, 1.0f) : glm::vec4(1.0f);
		const GLuint currentPlayerTexture = useCrouchPose ? wolverineCrouchingTexture : wolverineRunningTexture;

		const bool playerVisible = !isInvincible || std::fmod(invincibilityTimer, 0.16f) >= 0.08f;
		if (playerVisible || gameOver)
			drawSquare(programID, vao, playerX, playerY, playerWidth, playerHeight, playerColor, currentPlayerTexture);

		for (size_t i = 0; i < obstacles.size(); ++i)
		{
			const GLuint obstacleTexture = obstacles[i].isHighObstacle ? upperObstacleTexture : woodTexture;
			const float obstacleRotation = obstacles[i].isHighObstacle ? upperObstacleRotation : 0.0f;
			drawSquare(
				programID,
				vao,
				obstacles[i].x,
				obstacles[i].y,
				obstacles[i].size,
				obstacles[i].size,
				glm::vec4(1.0f),
				obstacleTexture,
				obstacleRotation);
		}

		for (int i = 0; i < 3; ++i)
			drawSquare(programID, vao, -0.90f + i * 0.10f, 0.88f, 0.07f, 0.07f, glm::vec4(1.0f), i < life ? heartTexture : emptyHeartTexture);

		glfwSwapBuffers(window);
	}

	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ibo);
	glDeleteBuffers(1, &textureCoordinateBuffer);
	glDeleteVertexArrays(1, &vao);
	glDeleteProgram(programID);
	glDeleteTextures(1, &heartTexture);
	glDeleteTextures(1, &emptyHeartTexture);
	glDeleteTextures(1, &woodTexture);
	glDeleteTextures(1, &upperObstacleTexture);
	glDeleteTextures(1, &backgroundTexture);
	glDeleteTextures(1, &wolverineRunningTexture);
	glDeleteTextures(1, &wolverineCrouchingTexture);
	glfwTerminate();
	Gdiplus::GdiplusShutdown(gdiplusToken);
	return 0;
}