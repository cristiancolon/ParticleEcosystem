 #include "Renderer.h"
 
 #include <string>
 #include <cstdio>
 
 namespace Particles {
 
 	// Fullscreen pixel-space quad (NDC via vertex shader)
 	// We'll instance-render tiny quads per point.
 
 	static const char* kVertex = R"(
		#version 330 core
		layout (location = 0) in vec2 aCircleVertex;
		layout (location = 1) in vec2 aPosPx;
		layout (location = 2) in float aRadiusPx;
		layout (location = 3) in vec3 aColor;

		out vec3 vColor;
		out vec2 vCircleCoord;

		uniform vec2 uFramebufferSize; // width, height in pixels
		uniform float uRadiusScale;

		void main(){
			vec2 px = aPosPx + (aCircleVertex * (aRadiusPx * uRadiusScale));
			// normalize to [-1, 1] (normalized device coordinates)
			vec2 ndc = vec2(
				(px.x / uFramebufferSize.x) * 2.0 - 1.0,
				1.0 - (px.y / uFramebufferSize.y) * 2.0
			);
			gl_Position = vec4(ndc, 0.0, 1.0);
			vColor = aColor;
			vCircleCoord = aCircleVertex;
		}
	)";
 
 	static const char* kFragment = R"(
		#version 330 core
		in vec3 vColor;
		in vec2 vCircleCoord;
		out vec4 FragColor;

		uniform int  uDoGlow;          // 0 = core, 1 = glow pass
		uniform float uGlowIntensity;
		uniform float uGlowSharpness;

		void main(){
			float dist = length(vCircleCoord);
			if(dist > 1.0) discard;

			if (uDoGlow == 1) {
				float a = exp(-pow(dist * uGlowSharpness, 2.0)) * uGlowIntensity;
				FragColor = vec4(vColor * a, a); // non-premultiplied output
			} else {
				float alpha = 1.0 - smoothstep(0.8, 1.0, dist);
				FragColor = vec4(vColor, alpha);
			}
		}
	)";
 
 	static GLuint compile(GLenum type, const char* src) {
 		GLuint s = glCreateShader(type);
 		glShaderSource(s, 1, &src, nullptr);
 		glCompileShader(s);
 		GLint ok = 0; glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
 		if(!ok){
 			char log[1024]; GLsizei n=0; glGetShaderInfoLog(s, 1024, &n, log);
 			fprintf(stderr, "Shader compile error: %s\n", log);
 		}
 		return s;
 	}
 
 	static GLuint link(GLuint vs, GLuint fs){
 		GLuint p = glCreateProgram();
 		glAttachShader(p, vs);
 		glAttachShader(p, fs);
 		glLinkProgram(p);
 		GLint ok=0; glGetProgramiv(p, GL_LINK_STATUS, &ok);
 		if(!ok){
 			char log[1024]; GLsizei n=0; glGetProgramInfoLog(p, 1024, &n, log);
 			fprintf(stderr, "Program link error: %s\n", log);
 		}
 		glDetachShader(p, vs);
 		glDetachShader(p, fs);
 		glDeleteShader(vs);
 		glDeleteShader(fs);
 		return p;
 	}
 
 		Renderer::Renderer(GLFWwindow* window) {
		updateFramebufferSize(window);
		createShaders();
		createComputeShader();
		createAttractionTexture();
	}
 
 		Renderer::~Renderer(){
		if(instanceVbo) glDeleteBuffers(1, &instanceVbo);
		if(vbo) glDeleteBuffers(1, &vbo);
		if(vao) glDeleteVertexArrays(1, &vao);
		if(shaderProgram) glDeleteProgram(shaderProgram);
		if(computeProgram) glDeleteProgram(computeProgram);
	}
 
 	void Renderer::updateFramebufferSize(GLFWwindow* window){
 		glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
 	}
 
 		void Renderer::createShaders(){
		GLuint vs = compile(GL_VERTEX_SHADER, kVertex);
		GLuint fs = compile(GL_FRAGMENT_SHADER, kFragment);
		shaderProgram = link(vs, fs);
	}

	static const char* kCompute = R"(
		#version 430
		layout(local_size_x = 256) in;

		struct Particle {
			vec2 pos;      // offset  0
			vec2 vel;      // offset  8
			float radius;  // offset 16
			float mass;   // offset 20
			vec4  color;   // offset 32  (16-byte aligned)
			int   species; // offset 48
			float _pad1;   // offset 52
			vec2  _pad2;   // offset 56 -> total stride 64 bytes
		};


		layout(std430, binding = 0) buffer Particles {
			Particle p[];
		};

		uniform int   uCount;
		uniform float uMaxDist;
		uniform float uRepelDist;
		uniform float uDt;
		uniform float uDamping; // 0..1 per step
		uniform float uForceScale;

		// Attraction matrix as a texture
		uniform sampler2D uAttractionMatrix;

		void main() {
			uint i = gl_GlobalInvocationID.x;
			if (i >= uint(uCount)) return;

			vec2 xi = p[i].pos;
			float ri = p[i].radius;
			float mi = p[i].mass;
			int si   = p[i].species;

			vec2 dV = vec2(0.0);

			for (int j = 0; j < uCount; ++j) {
				if (j == int(i)) continue;

				vec2 xj = p[j].pos;
				vec2 d  = xj - xi;
				float d2 = dot(d, d);
				if (d2 == 0.0) continue;

				float dist = sqrt(d2);
				if (dist > uMaxDist) continue;

				float invd2 = 1.0 / d2;
				float k = texelFetch(uAttractionMatrix, ivec2(si, p[j].species), 0).r;

				float massProd = mi * p[j].mass;

				float contact = ri + p[j].radius;
				float f;
				if (dist > contact + uRepelDist) {
					f = k * massProd * invd2;
				} else {
					float repelMag = (k != 0.0) ? abs(k) * massProd : massProd;
					f = -repelMag * invd2;
				}
				dV += uForceScale * f * d;
			}
			
			vec2 acc = dV / mi;

			// simple velocity + damping
			vec2 v = p[i].vel + acc * uDt;
			v *= (1.0 - uDamping);

			p[i].vel = v;
			p[i].pos = xi + v;
		}
	)";

	void Renderer::createAttractionTexture() {
		const int numSpecies = Color::NUM_SPECIES;
		std::vector<float> attractionData(numSpecies * numSpecies, 0.0f);
		
		// Fill the matrix from Color::attractionMatrix
		for (const auto& [c1n, c2n, weight] : Color::attractionMatrix) {
			int i = static_cast<int>(c1n);
			int j = static_cast<int>(c2n);
			attractionData[i * numSpecies + j] = weight;
		}
		
		// Create and configure the texture
		glGenTextures(1, &attractionTexture);
		glBindTexture(GL_TEXTURE_2D, attractionTexture);
		
		// Upload the data as an numSpecies x numSpecies R32F texture
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, numSpecies, numSpecies, 0, GL_RED, GL_FLOAT, attractionData.data());
		
		// Set texture parameters for exact pixel sampling
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void Renderer::createComputeShader(){
		GLuint cs = compile(GL_COMPUTE_SHADER, kCompute);
		computeProgram = glCreateProgram();
		glAttachShader(computeProgram, cs);
		glLinkProgram(computeProgram);
		GLint ok=0; glGetProgramiv(computeProgram, GL_LINK_STATUS, &ok);
		if(!ok){
			char log[1024]; GLsizei n=0; glGetProgramInfoLog(computeProgram, 1024, &n, log);
			fprintf(stderr, "Compute program link error: %s\n", log);
		}
		glDeleteShader(cs);
	}
 
	void Renderer::initializeGPUBuffer(const std::vector<GPUParticle>& initialParticles, GLuint& particleBuffer) {
		glGenBuffers(1, &particleBuffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER,
					 initialParticles.size() * sizeof(GPUParticle),
					 initialParticles.data(), GL_DYNAMIC_DRAW);
	}

	void Renderer::drawPointsGPU(GLuint particleBuffer, size_t particleCount) {
		if (particleCount == 0) return;
	
		glBindVertexArray(vao);
		glUseProgram(shaderProgram);
	
		if (GLint loc = glGetUniformLocation(shaderProgram, "uFramebufferSize"); loc >= 0) {
			glUniform2f(loc, (float)framebufferWidth, (float)framebufferHeight);
		}
	
		// ---------- Pass 1: OUTER GLOW (very wide, soft, faint) ----------
		glUniform1i(glGetUniformLocation(shaderProgram, "uDoGlow"), 1);
		glUniform1f(glGetUniformLocation(shaderProgram, "uRadiusScale"), 3.0f);   // huge radius
		glUniform1f(glGetUniformLocation(shaderProgram, "uGlowSharpness"), 0.6f); // soft falloff
		glUniform1f(glGetUniformLocation(shaderProgram, "uGlowIntensity"), 0.18f);// faint but covers area
		glBlendFunc(GL_ONE, GL_ONE);
		glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, (GLsizei)particleCount);
	
		// ---------- Pass 2: INNER GLOW (medium radius, tighter, bright) ----------
		glUniform1f(glGetUniformLocation(shaderProgram, "uRadiusScale"), 1.5f);   // medium radius
		glUniform1f(glGetUniformLocation(shaderProgram, "uGlowSharpness"), 1.1f); // tighter curve
		glUniform1f(glGetUniformLocation(shaderProgram, "uGlowIntensity"), 0.50f);// make it HOT
		glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, (GLsizei)particleCount);
	
		// ---------- Pass 3: CORE (regular alpha) ----------
		glUniform1i(glGetUniformLocation(shaderProgram, "uDoGlow"), 0);
		glUniform1f(glGetUniformLocation(shaderProgram, "uRadiusScale"), 1.0f);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, (GLsizei)particleCount);
	
		glBindVertexArray(0);
	}

	void Renderer::createGeometryGPU(GLuint particleBuffer) {
		// A single quad's vertices. The vertex shader will scale and position it.
		// We're using a triangle strip to draw the quad with 4 vertices.
		static const float quadVertices[] = {
			-1.0f, -1.0f, // bottom-left
			 1.0f, -1.0f, // bottom-right
			-1.0f,  1.0f, // top-left
			 1.0f,  1.0f  // top-right
		};

		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

		// Wire the particle buffer as instanced attributes
		glBindBuffer(GL_ARRAY_BUFFER, particleBuffer);
		GLsizei stride = sizeof(GPUParticle);

		// aPosPx (loc=1): 2 floats at offset 0
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)GPU_PARTICLE_OFFSET(px));
		glVertexAttribDivisor(1, 1);

		// aRadiusPx (loc=2): 1 float at offset offsetof(radius)
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, stride, (void*)GPU_PARTICLE_OFFSET(radius));
		glVertexAttribDivisor(2, 1);

		// aColor (loc=3): 3 floats at offset offsetof(r)
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (void*)GPU_PARTICLE_OFFSET(r));
		glVertexAttribDivisor(3, 1);

				glBindVertexArray(0);
	}

	void Renderer::dispatchComputeShader(GLuint particleBuffer, size_t particleCount, float deltaTime) {
		glUseProgram(computeProgram);

		// bind SSBO
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, particleBuffer);

		// Bind attraction matrix texture to texture unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, attractionTexture);
		glUniform1i(glGetUniformLocation(computeProgram, "uAttractionMatrix"), 0);

		// uniforms
		glUniform1i(glGetUniformLocation(computeProgram, "uCount"), (GLint)particleCount);
		glUniform1f(glGetUniformLocation(computeProgram, "uMaxDist"), 200.0f);
		glUniform1f(glGetUniformLocation(computeProgram, "uRepelDist"), 30.0f);
		glUniform1f(glGetUniformLocation(computeProgram, "uDt"), deltaTime);
		glUniform1f(glGetUniformLocation(computeProgram, "uDamping"), 0.08f);
		glUniform1f(glGetUniformLocation(computeProgram, "uForceScale"), 0.3f);

		// dispatch
		GLuint wg = 256;
		GLuint numGroups = (GLuint)((particleCount + wg - 1) / wg);
		glDispatchCompute(numGroups, 1, 1);

		// ensure writes visible to vertex fetch
		glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
	}
}
 

