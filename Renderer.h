#pragma once

#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "GPUParticle.h"
#include "Color.h"
 
namespace Particles {
 
 	class Renderer {
	public:
		Renderer(GLFWwindow* window);
		~Renderer();

		void drawPointsGPU(GLuint particleBuffer, size_t particleCount);
		void initializeGPUBuffer(const std::vector<GPUParticle>& initialParticles, GLuint& particleBuffer);
		void dispatchComputeShader(GLuint particleBuffer, size_t particleCount, float deltaTime);
		void createGeometryGPU(GLuint particleBuffer);

	private:
		GLuint shaderProgram { 0 };
		GLuint computeProgram { 0 };
		GLuint vao { 0 };
		GLuint vbo { 0 };
		GLuint instanceVbo { 0 }; // Kept for consistency, though its setup is unused
		int framebufferWidth { 1 };
		int framebufferHeight { 1 };
		GLuint attractionTexture { 0 };

		void createShaders();
		void createComputeShader();
		void updateFramebufferSize(GLFWwindow* window);
		void createAttractionTexture();
	};
}