#ifdef NDEBUG
#undef NDEBUG
#endif

#include <HashMap.hpp>

#include <cassert>
#include <iostream>
#include <string>

using namespace ctm;

int f1() { return 1; }

int f2() { return 2; }

int f3() { return 3; }

int f4() { return 4; }

int f5() { return 5; }

int f6() { return 5; }

int fDuplicate() { return 999; }

constexpr auto key4 = "ac";

constexpr auto makeTestMap0010() {
  constexpr auto data = makeHashMapSpec(1.0,
                                        0.5,
                                        std::make_tuple("bsd", f1),
                                        std::make_tuple("holy", f2),
                                        std::make_tuple("", f3),
                                        std::make_tuple("duplicate", f4),
                                        std::make_tuple(key4, f5),
                                        std::make_tuple("duplicate", fDuplicate),
                                        std::make_tuple("ab", f6));
  return HashMap<decltype(data),
                 data.maxBucketSize,
                 data.bucketCount,
                 data.elementCount>::make(data);
}

void test0010() {
  constexpr auto map = makeTestMap0010();

  static_assert(sizeof(map) == 10 * sizeof(std::tuple<String, void*>), "Invalid sizeof");

  static_assert(std::is_same<decltype(map)::KeyType, String>::value, "Invalid key type");
  static_assert(std::is_same<decltype(map)::ValueType, int (*)()>::value,
                "Invalid value type");
  static_assert(
    std::is_same<decltype(map)::PairType, std::pair<String, int (*)()>>::value,
    "Invalid pair type");
  static_assert(map.bucketSize() == 1, "Invalid bucket size");
  static_assert(map.bucketCount() == 10, "Invalid bucket count");
  static_assert(map.size() == 6, "Invalid size");
  assert(map.bucketSize() == 1);
  assert(map.bucketCount() == 10);
  assert(map.size() == 6);

  static_assert(map["bsd"] == f1, "Invalid value");
  static_assert(map["holy"] == f2, "Invalid value");
  static_assert(map[""] == f3, "Invalid value");
  static_assert(map["duplicate"] == f4, "Invalid value");
  static_assert(map["ac"] == f5, "Invalid value");
  static_assert(map["ab"] == f6, "Invalid value");
  static_assert(map["unknown"] == nullptr, "Invalid value");
  assert(map["bsd"] == f1);
  assert(map["holy"] == f2);
  assert(map[""] == f3);
  assert(map["duplicate"] == f4);
  assert(map["ac"] == f5);
  assert(map["ab"] == f6);
  assert(map["unknown"] == nullptr);
  auto key = std::string("holy");
  assert(map[key] == f2);
  assert(map[key]() == 2);
  key = "moly";
  assert(map[key] == nullptr);

  bool has_occured = false;
  for (auto const& bucket : map) {
    for (auto const& pair : bucket) {
      if (pair.first && pair.first == "ac") {
        has_occured = true;
        assert(pair.first.chars() == key4);
      }
    }
  }
  assert(has_occured);
}

int fTest00201(int value) { return 1 * value; }

int fTest00202(int value) { return 2 * value; }

int fTest00203(int value) { return 3 * value; }

constexpr auto makeTestMap0020() {
  constexpr auto data
    = makeHashMapSpec(1.0,
                      0.5,
                      std::make_tuple("Eeny", fTest00201, fTest00202, fTest00203),
                      std::make_tuple("meeny", fTest00202, fTest00201, fTest00203),
                      std::make_tuple("miny", fTest00203, fTest00202, fTest00201),
                      std::make_tuple("moe", fTest00201, fTest00203, fTest00202));
  return HashMap<decltype(data),
                 data.maxBucketSize,
                 data.bucketCount,
                 data.elementCount>::make(data);
}

void test0020() {
  constexpr auto map = makeTestMap0020();

  static_assert(sizeof(map) == 5 * sizeof(std::tuple<String, void*, void*, void*>),
                "Invalid sizeof");

  static_assert(std::is_same<decltype(map)::KeyType, String>::value, "Invalid key type");
  static_assert(std::is_same<decltype(map)::ValueType,
                             std::tuple<int (*)(int), int (*)(int), int (*)(int)>>::value,
                "Invalid value type");
  static_assert(
    std::is_same<
      decltype(map)::PairType,
      std::pair<String, std::tuple<int (*)(int), int (*)(int), int (*)(int)>>>::value,
    "Invalid pair type");
  static_assert(map.bucketSize() == 1, "Invalid bucket size");
  static_assert(map.bucketCount() == 5, "Invalid bucket count");
  static_assert(map.size() == 4, "Invalid size");
  assert(map.bucketSize() == 1);
  assert(map.bucketCount() == 5);
  assert(map.size() == 4);

  static_assert(std::get<0>(map["Eeny"]) == fTest00201, "Invalid value");
  static_assert(std::get<1>(map["Eeny"]) == fTest00202, "Invalid value");
  static_assert(std::get<2>(map["Eeny"]) == fTest00203, "Invalid value");
  static_assert(std::get<0>(map["meeny"]) == fTest00202, "Invalid value");
  static_assert(std::get<1>(map["meeny"]) == fTest00201, "Invalid value");
  static_assert(std::get<2>(map["meeny"]) == fTest00203, "Invalid value");
  static_assert(std::get<0>(map["miny"]) == fTest00203, "Invalid value");
  static_assert(std::get<1>(map["miny"]) == fTest00202, "Invalid value");
  static_assert(std::get<2>(map["miny"]) == fTest00201, "Invalid value");
  static_assert(std::get<0>(map["moe"]) == fTest00201, "Invalid value");
  static_assert(std::get<1>(map["moe"]) == fTest00203, "Invalid value");
  static_assert(std::get<2>(map["moe"]) == fTest00202, "Invalid value");
  static_assert(std::get<0>(map["unknown"]) == nullptr, "Invalid value");
  static_assert(std::get<1>(map["unknown"]) == nullptr, "Invalid value");
  static_assert(std::get<2>(map["unknown"]) == nullptr, "Invalid value");

  assert(std::get<0>(map["Eeny"]) == fTest00201);
  assert(std::get<1>(map["Eeny"]) == fTest00202);
  assert(std::get<2>(map["Eeny"]) == fTest00203);
  assert(std::get<0>(map["meeny"]) == fTest00202);
  assert(std::get<1>(map["meeny"]) == fTest00201);
  assert(std::get<2>(map["meeny"]) == fTest00203);
  assert(std::get<0>(map["miny"]) == fTest00203);
  assert(std::get<1>(map["miny"]) == fTest00202);
  assert(std::get<2>(map["miny"]) == fTest00201);
  assert(std::get<0>(map["moe"]) == fTest00201);
  assert(std::get<1>(map["moe"]) == fTest00203);
  assert(std::get<2>(map["moe"]) == fTest00202);

  assert(std::get<0>(map["Eeny"])(10) == 10);
  assert(std::get<1>(map["Eeny"])(10) == 20);
  assert(std::get<2>(map["Eeny"])(10) == 30);
  assert(std::get<0>(map["meeny"])(10) == 20);
  assert(std::get<1>(map["meeny"])(10) == 10);
  assert(std::get<2>(map["meeny"])(10) == 30);
  assert(std::get<0>(map["miny"])(10) == 30);
  assert(std::get<1>(map["miny"])(10) == 20);
  assert(std::get<2>(map["miny"])(10) == 10);
  assert(std::get<0>(map["moe"])(10) == 10);
  assert(std::get<1>(map["moe"])(10) == 30);
  assert(std::get<2>(map["moe"])(10) == 20);
  assert(std::get<0>(map["unknown"]) == nullptr);
  assert(std::get<1>(map["unknown"]) == nullptr);
  assert(std::get<2>(map["unknown"]) == nullptr);
}

constexpr auto makeTestMap0030() {
  constexpr auto spec
    = makeHashMapSpec(4.0,
                      2.0,
                      std::make_tuple("Catch", 'c'),
                      std::make_tuple("a tiger", 'a'),
                      std::make_tuple("by the toe", 'b'),
                      std::make_tuple("If he hollers, let him go", 'i'));

  return HashMap<decltype(spec),
                 spec.maxBucketSize,
                 spec.bucketCount,
                 spec.elementCount>::make(spec);
}

void test0030() {
  constexpr auto map = makeTestMap0030();

  static_assert(sizeof(map) == 6 * sizeof(std::tuple<String, char>), "Invalid sizeof");
  static_assert(map.bucketSize() == 3, "Invalid bucket size");
  static_assert(map.bucketCount() == 2, "Invalid bucket count");
  static_assert(map.size() == 4, "Invalid size");
  assert(map.bucketSize() == 3);
  assert(map.bucketCount() == 2);
  assert(map.size() == 4);

  static_assert(map["Catch"] == 'c', "Invalid value");
  static_assert(map["a tiger"] == 'a', "Invalid value");
  static_assert(map["by the toe"] == 'b', "Invalid value");
  static_assert(map["If he hollers, let him go"] == 'i', "Invalid value");
  static_assert(map["unknown"] == '\0', "Invalid value");
  assert(map["Catch"] == 'c');
  assert(map["a tiger"] == 'a');
  assert(map["by the toe"] == 'b');
  assert(map["If he hollers, let him go"] == 'i');
  assert(map["unknown"] == '\0');
}

constexpr auto makeTestMap0040() {
  constexpr auto data = makeHashMapSpec(1.0,
                                        0.5,
                                        std::make_tuple(4096, 1, 'q'),
                                        std::make_tuple(2048, 2, 'w'),
                                        std::make_tuple(8192, 3, 'e'),
                                        std::make_tuple(1024, 4, 'r'));

  return HashMap<decltype(data),
                 data.maxBucketSize,
                 data.bucketCount,
                 data.elementCount>::make(data);
}

void test0040() {
  constexpr auto map = makeTestMap0040();
  static_assert(sizeof(map) == 5 * sizeof(std::tuple<int, int, char>), "Invalid sizeof");
  static_assert(map.bucketSize() == 1, "Invalid bucket size");
  static_assert(map.bucketCount() == 5, "Invalid bucket count");
  static_assert(map.size() == 4, "Invalid size");
  assert(map.bucketSize() == 1);
  assert(map.bucketCount() == 5);
  assert(map.size() == 4);

  static_assert(std::get<0>(map[4096]) == 1, "Invalid value");
  static_assert(std::get<1>(map[4096]) == 'q', "Invalid value");
  static_assert(std::get<0>(map[2048]) == 2, "Invalid value");
  static_assert(std::get<1>(map[2048]) == 'w', "Invalid value");
  static_assert(std::get<0>(map[8192]) == 3, "Invalid value");
  static_assert(std::get<1>(map[8192]) == 'e', "Invalid value");
  static_assert(std::get<0>(map[1024]) == 4, "Invalid value");
  static_assert(std::get<1>(map[1024]) == 'r', "Invalid value");
  static_assert(std::get<0>(map[0]) == 0, "Invalid value");
  static_assert(std::get<1>(map[0]) == '\0', "Invalid value");

  assert(std::get<0>(map[4096]) == 1);
  assert(std::get<1>(map[4096]) == 'q');
  assert(std::get<0>(map[2048]) == 2);
  assert(std::get<1>(map[2048]) == 'w');
  assert(std::get<0>(map[8192]) == 3);
  assert(std::get<1>(map[8192]) == 'e');
  assert(std::get<0>(map[1024]) == 4);
  assert(std::get<1>(map[1024]) == 'r');
  assert(std::get<0>(map[0]) == 0);
  assert(std::get<1>(map[0]) == '\0');
}

int main() {
  test0010();
  test0020();
  test0030();
  test0040();
  return 0;
}
