#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <random>
#include <vector>

#include "Renderer.h"
#include "Geometry.h"
#include "Color.h"
#include "GPUParticle.h"

using namespace Geometry;
using namespace Color;

// Set this to false to disable the P, R, and Esc keybindings
constexpr bool ENABLE_KEYBINDINGS = true;
constexpr float DEATH_PROBABILITY = 0.0001f;

// A simple struct to hold the simulation's state
struct SimulationState {
    bool isPaused = false;
    bool shouldRestart = false;
};

float generateRandomRadius(std::mt19937& rng, float maxRadius) {
    // Probability is inversely proportional to radius
    
    // Create a piecewise linear distribution
    // Probability density: f(r) = k/r where k is normalization constant
    // Over [1,3]: integral of k/r dr = k * ln(3) = 1, so k = 1/ln(3)
    
    std::uniform_real_distribution<float> uniform(0.0f, 1.0f);
    float u = uniform(rng);
    
    // Inverse CDF: F^(-1)(u) = exp(u * ln(3))
    // This gives us the desired inverse probability distribution
    return std::exp(u * std::log(maxRadius));
}

inline float calculateMass(float radius) {
    // For simplicity, we'll use just r³ as the mass (ignoring constants)
    return radius * radius * radius;
}

GPUParticle createRandomParticle(std::mt19937& rng, const GLFWvidmode* videoMode) {
    std::uniform_real_distribution<float> xdist(0.0f, (float)videoMode->width);
    std::uniform_real_distribution<float> ydist(0.0f, (float)videoMode->height);
    std::uniform_int_distribution<size_t> colorVal(0, NUM_SPECIES - 1);
    
    const float radius = 1.0f;
    const float mass = calculateMass(radius);
    const auto color = colorMap.at(static_cast<ColorSpecies>(colorVal(rng)));
    
    return GPUParticle{
        xdist(rng), ydist(rng),                   // px, py
        0.0f, 0.0f,                               // vx, vy
        radius, mass,                             // radius, mass
        {0.0f, 0.0f},                             // _gap_to_32
        color.r, color.g, color.b, 1.0f,          // r, g, b, a
        colorToSpecies(color),                    // species
        0.0f,                                     // _pad1
        {0.0f, 0.0f}                              // _pad2
    };
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    auto* state = static_cast<SimulationState*>(glfwGetWindowUserPointer(window));
    if (state == nullptr) return;

    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose(window, true);
                break;
            case GLFW_KEY_P:
                state->isPaused = !state->isPaused;
                break;
            case GLFW_KEY_R:
                state->shouldRestart = true;
                break;
        }
    }
}

void resetSimulation(
    std::vector<GPUParticle>& particles,
    int numPoints,
    const GLFWvidmode* videoMode
) {
    particles.clear();

    std::mt19937 rng{std::random_device{}()};
    
    for (int i = 0; i < numPoints; ++i) {
        particles.push_back(createRandomParticle(rng, videoMode));
    }
}


int main() {
	if (!glfwInit()) {
		std::cout << "Failed to initialize GLFW" << std::endl;
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Get monitor for full-screen mode
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    if (monitor == NULL) {
        std::cout << "Failed to get primary monitor" << std::endl;
        glfwTerminate();
        return -1;
    }

    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    if (mode == NULL) {
        std::cout << "Failed to get video mode" << std::endl;
        glfwTerminate();
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "Particle Sim", monitor, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

    glEnable(GL_BLEND);


	glClearColor(0.04f, 0.05f, 0.1f, 1.0f);

	constexpr int numPoints = 40000;
	
    // Create state and set up callbacks
    SimulationState simState;
    glfwSetWindowUserPointer(window, &simState);

    if (ENABLE_KEYBINDINGS) {
        glfwSetKeyCallback(window, key_callback);
    }
	
	// Create particle vectors
	std::vector<GPUParticle> particles;
	particles.reserve(numPoints);

    // Perform the initial simulation setup
    resetSimulation(particles, numPoints, mode);

	Particles::Renderer renderer(window);

	// Initialize GPU buffer
	GLuint particleBuffer = 0;
	renderer.initializeGPUBuffer(particles, particleBuffer);
	
	// Set up GPU geometry
	renderer.createGeometryGPU(particleBuffer);

	// Time tracking for delta time
	double lastTime = glfwGetTime();

	std::mt19937 rng{std::random_device{}()};
	while (!glfwWindowShouldClose(window)) {
        // Handle Restarting
        if (simState.shouldRestart) {
            resetSimulation(particles, numPoints, mode);
            // Re-upload all particle data to the GPU buffer
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleBuffer);
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, particles.size() * sizeof(GPUParticle), particles.data());
            
            simState.shouldRestart = false;
            simState.isPaused = false;
        }
        
        // Only update the simulation logic if not paused
        if (!simState.isPaused) {
            // ---- GPU simulation step ----
            double currentTime = glfwGetTime();
            float deltaTime = static_cast<float>(currentTime - lastTime);
            lastTime = currentTime;
            
            deltaTime = std::min(deltaTime, 0.016f);

            renderer.dispatchComputeShader(particleBuffer, numPoints, deltaTime);

            std::vector<size_t> deadIndices;
            std::uniform_real_distribution<float> deathDist(0.0f, 1.0f);

            for (size_t i = 0; i < numPoints; ++i) {
                // Check if the particle dies this frame
                if (deathDist(rng) < DEATH_PROBABILITY) {
                    deadIndices.push_back(i);
                }
            }

            if (!deadIndices.empty()) {
                std::vector<GPUParticle> newBirths;
                newBirths.reserve(deadIndices.size());
                
                for (size_t i = 0; i < deadIndices.size(); ++i) {
                    newBirths.push_back(createRandomParticle(rng, mode));
                }

                glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleBuffer);
                for (size_t i = 0; i < deadIndices.size(); ++i) {
                    size_t gpuIndex = deadIndices[i];
                    const GLsizeiptr offset = static_cast<GLsizeiptr>(gpuIndex) * sizeof(GPUParticle);
                    glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset,
                                    sizeof(GPUParticle), &newBirths[i]);
                }
            }
        }

		// ---- Draw (always, even when paused) ----
		glClear(GL_COLOR_BUFFER_BIT);
		renderer.drawPointsGPU(particleBuffer, numPoints);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// Cleanup
	if (particleBuffer) {
		glDeleteBuffers(1, &particleBuffer);
	}

	glfwTerminate();
	return 0;
}
