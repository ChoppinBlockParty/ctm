#pragma once

#ifndef COMPILE_TIME_ARMOR_BYTES_HASH_ALGORITHM_DEFAULT
#define COMPILE_TIME_ARMOR_BYTES_HASH_ALGORITHM_DEFAULT FnvBytesHash
#endif

#ifndef COMPILE_TIME_ARMOR_BYTES_HASH_ALGORITHM_INTEGER_SIZE_DEFAULT
#define COMPILE_TIME_ARMOR_BYTES_HASH_ALGORITHM_INTEGER_SIZE_DEFAULT 4
#endif

#include <string>
#include <type_traits>

namespace CompileTimeArmor {
namespace Internal {
template <typename TResult, typename TArg>
struct HashBase {
  using ResultType = TResult;
  using ArgumentType = TArg;
};
}

template <typename T>
struct Hash;

template <typename T>
struct Hash<T*> : Internal::HashBase<std::size_t, T*> {
  constexpr std::size_t operator()(T* ptr) const noexcept {
    return reinterpret_cast<std::size_t>(ptr);
  }
};

#define COMPILE_TIME_ARMOR_TRIVIAL_HASH_DEFINITION(T)                                    \
  template <>                                                                            \
  struct Hash<T> : Internal::HashBase<std::size_t, T> {                                  \
    constexpr std::size_t operator()(T value) const noexcept {                           \
      return static_cast<std::size_t>(value);                                            \
    }                                                                                    \
  };

COMPILE_TIME_ARMOR_TRIVIAL_HASH_DEFINITION(bool)
COMPILE_TIME_ARMOR_TRIVIAL_HASH_DEFINITION(char)
COMPILE_TIME_ARMOR_TRIVIAL_HASH_DEFINITION(signed char)
COMPILE_TIME_ARMOR_TRIVIAL_HASH_DEFINITION(unsigned char)
COMPILE_TIME_ARMOR_TRIVIAL_HASH_DEFINITION(wchar_t)
COMPILE_TIME_ARMOR_TRIVIAL_HASH_DEFINITION(char16_t)
COMPILE_TIME_ARMOR_TRIVIAL_HASH_DEFINITION(char32_t)
COMPILE_TIME_ARMOR_TRIVIAL_HASH_DEFINITION(short)
COMPILE_TIME_ARMOR_TRIVIAL_HASH_DEFINITION(int)
COMPILE_TIME_ARMOR_TRIVIAL_HASH_DEFINITION(long)
COMPILE_TIME_ARMOR_TRIVIAL_HASH_DEFINITION(long long)
COMPILE_TIME_ARMOR_TRIVIAL_HASH_DEFINITION(unsigned short)
COMPILE_TIME_ARMOR_TRIVIAL_HASH_DEFINITION(unsigned int)
COMPILE_TIME_ARMOR_TRIVIAL_HASH_DEFINITION(unsigned long)
COMPILE_TIME_ARMOR_TRIVIAL_HASH_DEFINITION(unsigned long long)

#undef COMPILE_TIME_ARMOR_TRIVIAL_HASH_DEFINITION

namespace Internal {

constexpr std::size_t convertCharsToSizet(char const* chars) {
  std::size_t result = 0;
  constexpr auto size = sizeof(std::size_t);
  auto chars_end = chars + size;
  for (; chars != chars_end; ++chars) {
    auto ch = static_cast<std::size_t>(*chars);
    result >>= 8;
    ch <<= 8 * (size - 1);
    result |= ch;
  }
  return result;
}

// Loads n bytes, where 1 <= n < 8.
constexpr std::size_t loadBytes(char const* p, int n) {
  std::size_t result = 0;
  --n;
  do
    result = (result << 8) + static_cast<unsigned char>(p[n]);
  while (--n >= 0);
  return result;
}

constexpr std::size_t shiftMix(std::size_t v) { return v ^ (v >> 47); }

template <std::size_t N>
constexpr std::size_t
hashBytesWithFnv(char const* ptr, std::size_t size, std::size_t hash) {
  switch (N) {
  case 4: {
    // Implementation of FNV hash for 32-bit std::size_t.

    for (; size; --size) {
      hash ^= static_cast<std::size_t>(*ptr++);
      hash *= static_cast<std::size_t>(16777619UL);
    }
    return hash;
  }
  case 8: {
    // Implementation of FNV hash for 64-bit std::size_t.

    for (; size; --size) {
      hash ^= static_cast<std::size_t>(*ptr++);
      hash *= static_cast<std::size_t>(1099511628211ULL);
    }
    return hash;
  }
  default: {
    // Dummy hash implementation for unusual sizeof(std::size_t).

    for (; size; --size)
      hash = (hash * 131) + *ptr++;
    return hash;
  }
  }
}

template <std::size_t N>
constexpr std::size_t
hashBytesWithMurmur(char const* ptr, std::size_t size, std::size_t seed) {
  switch (N) {
  case 4: {
    // Implementation of Murmur hash for 32-bit std::size_t.

    constexpr std::size_t m = 0x5bd1e995;
    std::size_t hash = seed ^ size;
    auto buf = ptr;

    // Mix 4 bytes at a time into the hash.
    while (size >= 4) {
      std::size_t k = convertCharsToSizet(buf);
      k *= m;
      k ^= k >> 24;
      k *= m;
      hash *= m;
      hash ^= k;
      buf += 4;
      size -= 4;
    }

    // Handle the last few bytes of the input array.
    switch (size) {
    case 3:
      hash ^= static_cast<unsigned char>(buf[2]) << 16;
    case 2:
      hash ^= static_cast<unsigned char>(buf[1]) << 8;
    case 1:
      hash ^= static_cast<unsigned char>(buf[0]);
      hash *= m;
    };

    // Do a few final mixes of the hash.
    hash ^= hash >> 13;
    hash *= m;
    hash ^= hash >> 15;
    return hash;
  }
  case 8: {
    // Implementation of Murmur hash for 64-bit std::size_t.

    constexpr std::size_t const mul
      = (((std::size_t)0xc6a4a793UL) << 32UL) + (std::size_t)0x5bd1e995UL;

    // Remove the bytes not divisible by the sizeof(std::size_t).  This
    // allows the main loop to process the data as 64-bit integers.
    int const len_aligned = size & ~0x7;
    char const* const end = ptr + len_aligned;
    std::size_t hash = seed ^ (size * mul);
    for (char const* p = ptr; p != end; p += 8) {
      std::size_t const data = shiftMix(convertCharsToSizet(p) * mul) * mul;
      hash ^= data;
      hash *= mul;
    }
    if ((size & 0x7) != 0) {
      std::size_t const data = loadBytes(end, size & 0x7);
      hash ^= data;
      hash *= mul;
    }
    hash = shiftMix(hash) * mul;
    hash = shiftMix(hash);
    return hash;
  }
  default: {
    // Dummy hash implementation for unusual sizeof(std::size_t).

    std::size_t hash = seed;
    for (; size; --size)
      hash = (hash * 131) + *ptr++;
    return hash;
  }
  }
}
}

template <std::size_t N = sizeof(std::size_t)>
struct FnvBytesHash {
  constexpr static std::size_t hash(char const* ptr,
                                    std::size_t size,
                                    std::size_t seed
                                    = static_cast<std::size_t>(2166136261UL)) {
    return Internal::hashBytesWithFnv<N>(ptr, size, seed);
  }
};

template <std::size_t N = sizeof(std::size_t)>
struct MurmurBytesHash {
  constexpr static std::size_t hash(char const* ptr,
                                    std::size_t size,
                                    std::size_t seed
                                    = static_cast<std::size_t>(0xc70f6907UL)) {
    return Internal::hashBytesWithMurmur<N>(ptr, size, seed);
  }
};

struct BytesHash
  : COMPILE_TIME_ARMOR_BYTES_HASH_ALGORITHM_DEFAULT<COMPILE_TIME_ARMOR_BYTES_HASH_ALGORITHM_INTEGER_SIZE_DEFAULT> {
};

template <>
struct Hash<char const*> : Internal::HashBase<std::size_t, char const*> {
  constexpr std::size_t operator()(char const* chars) const noexcept {
    auto chars_begin = chars;
    while (*chars++ != '\0') {
    }
    return BytesHash::hash(chars_begin,
                           static_cast<std::size_t>(chars - chars_begin - 1));
  }

  constexpr std::size_t operator()(char const* chars, std::size_t size) const noexcept {
    return BytesHash::hash(chars, size);
  }
};

template <std::size_t N>
struct Hash<char[N]> : Internal::HashBase<std::size_t, const char*> {
  constexpr std::size_t operator()(char const (&chars)[N]) const noexcept {
    return BytesHash::hash(chars, N - 1);
  }
};

template <>
struct Hash<std::string> : Internal::HashBase<std::size_t, std::string> {
  std::size_t operator()(std::string const& string) const noexcept {
    return BytesHash::hash(string.data(), string.length());
  }
};
}
