/* SPDX-License-Identifier: MIT */
#include <array>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <span>
#include <vector>
#include "../../src/utils/shader_format.hpp"

int main() {
  namespace f = renodx::utils::shader::format;
  const std::array<uint8_t, 0> empty{};
  assert(f::TextBytes(empty).empty());
  assert(f::SourceText(empty).empty());
  assert(!f::IsSpirv(empty));
  assert(!f::IsArbAssembly(empty));
  const std::array<uint8_t, 2> without_null{'x', '}'};
  const std::array<uint8_t, 3> with_null{'x', '}', 0};
  assert(f::SourceText(without_null) == "x}");
  assert(f::SourceText(with_null) == "x}");
  assert(f::TextBytes(without_null).size() == 2);
  assert(f::TextBytes(with_null).size() == 2);
  const std::array<uint8_t, 6> arb{'!', '!', 'A', 'R', 'B', 'f'};
  assert(f::IsArbAssembly(arb));
  assert(!f::IsArbAssembly(std::span(arb).first(4)));
  std::vector<uint8_t> binary{3, 2, 35, 7, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0};
  assert(f::IsSpirv(binary));
  assert(f::HasSpirvHeader(binary));
  assert(!f::HasSpirvHeader(std::span(binary).first(19)));
  binary.push_back(0);
  assert(!f::HasSpirvHeader(binary));
  // Detection must work for unaligned data, without uint32_t pointer casts.
  binary.insert(binary.begin(), 0xFF);
  assert(f::IsSpirv(std::span(binary).subspan(1)));
  const std::array<uint8_t, 4> wrong_endian{7, 35, 2, 3};
  assert(!f::IsSpirv(wrong_endian));
  std::cout << "OpenGL shader byte-format tests passed\n";
}
