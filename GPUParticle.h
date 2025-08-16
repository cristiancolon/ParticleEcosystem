#pragma once

#include <cstddef>
#include "Color.h"

// GPU-side particle data layout (matches std430 SSBO layout)
#pragma pack(push, 1)
struct GPUParticle {
    float px, py;         //  0
    float vx, vy;         //  8
    float radius;         // 16
    float mass;          // 20
    float _gap_to_32[2];  // 24..31  (make color start at 32)
    float r, g, b, a;     // 32..47
    int   colorSpecies;        // 48..51
    float _pad1;          // 52..55
    float _pad2[2];       // 56..63
};
static_assert(sizeof(GPUParticle) == 64, "std430-compatible stride");

// Helper to get offsetof for OpenGL attribute setup
#define GPU_PARTICLE_OFFSET(member) offsetof(GPUParticle, member)

#pragma pack(pop)
