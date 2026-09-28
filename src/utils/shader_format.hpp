/*
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include <cstdint>
#include <cstring>
#include <span>
#include <string>

namespace renodx::utils::shader::format {

// Inspect bytes, never the graphics API or filename: OpenGL accepts both.
inline bool IsSpirv(std::span<const uint8_t> code) {
  return code.size() >= 4 && code[0] == 0x03 && code[1] == 0x02
         && code[2] == 0x23 && code[3] == 0x07;
}

inline bool IsArbAssembly(std::span<const uint8_t> code) {
  return code.size() >= 5 && std::memcmp(code.data(), "!!ARB", 5) == 0;
}

// The source callback may exclude the terminator; linked-program capture may
// include it. Keep the captured bytes/hash intact and trim only when exporting.
inline std::span<const uint8_t> TextBytes(std::span<const uint8_t> code) {
  if (!code.empty() && code.back() == 0) return code.first(code.size() - 1);
  return code;
}

inline std::string SourceText(std::span<const uint8_t> code) {
  code = TextBytes(code);
  if (code.empty()) return {};
  return {reinterpret_cast<const char*>(code.data()), code.size()};
}

inline const char* OpenGLName(std::span<const uint8_t> code) {
  if (code.empty()) return "Unavailable";
  if (IsSpirv(code)) return "SPIR-V";
  if (IsArbAssembly(code)) return "ARB assembly";
  return "GLSL source";
}

// Structural check only. Driver validation still decides whether the module is
// legal for OpenGL (a Vulkan module also has the same magic).
inline bool HasSpirvHeader(std::span<const uint8_t> code) {
  return IsSpirv(code) && code.size() >= 20 && code.size() % 4 == 0;
}

}  // namespace renodx::utils::shader::format
