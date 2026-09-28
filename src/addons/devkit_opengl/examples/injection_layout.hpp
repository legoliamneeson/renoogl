/* SPDX-License-Identifier: MIT */
#pragma once
#include <cstddef>

// Demonstration layout ONLY, not a replacement for a game's ShaderInjectData.
// Scalar-only, padded to std140's 16-byte block alignment.
struct alignas(16) OpenGLInjectionExample {
  float peak_nits = 1000.f;
  float game_nits = 203.f;
  float ui_nits = 203.f;
  float exposure = 1.f;
};
static_assert(sizeof(OpenGLInjectionExample) == 16);
static_assert(offsetof(OpenGLInjectionExample, exposure) == 12);

// In the game addon, before mods::shader::Use:
// renodx::mods::shader::expected_constant_buffer_index = 13;
// renodx::mods::shader::Use(fdw_reason, custom_shaders, &shader_injection);
// Register the relevant shader hashes even when editing through DevKit:
// DevKit alone does not know the game's injection structure or slider values.
