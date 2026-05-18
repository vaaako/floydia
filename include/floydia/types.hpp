#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <cstdint>

using f64 = double;

using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using i8  = int8_t;
using i32 = int32_t;
using i16 = int16_t;
using i64 = int64_t;

template <typename T>
using vec2 = glm::vec<2, T>;
template <typename T>
using vec3 = glm::vec<3, T>;
template <typename T>
using vec4 = glm::vec<4, T>;

