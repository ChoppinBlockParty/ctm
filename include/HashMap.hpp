#pragma once

#include <limits>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

#include "Hash.hpp"

namespace ctm {
template <typename T, std::size_t N>
struct Array {
  using value_type = T;

  constexpr std::size_t size() const { return N; }

  constexpr T const* begin() const { return data; }

  constexpr T const* end() const { return data + N; }

  constexpr T* begin() { return data; }

  constexpr T* end() { return data + N; }

  constexpr T const& operator[](std::size_t index) const { return data[index]; }

  constexpr T& operator[](std::size_t index) { return data[index]; }

  T data[N];
};

class String {
public:
  constexpr String() : _ptr(nullptr), _size(0) {}

  template <std::size_t N>
  constexpr String(char const (&ptr)[N]) : _ptr(ptr), _size(N - 1) {}

  String(std::string const& string) : _ptr(string.data()), _size(string.size()) {}

  constexpr String(char const* ptr) : _ptr(ptr), _size(0) {
    while ((*ptr++) != '\0') {
    }
    _size = static_cast<std::size_t>(ptr - _ptr - 1);
  }

  constexpr char const* chars() const { return _ptr; }

  constexpr std::size_t size() const { return _size; }

  constexpr bool empty() const { return _size == 0; }

  constexpr std::size_t hash() const { return Hash<char const*>()(_ptr, _size); }

  std::string toStdString() const { return std::string(_ptr, _size); }

  constexpr bool operator==(String const& other) const {
    if (_size != other._size)
      return false;
    for (auto lhs_ptr = _ptr, rhs_ptr = other._ptr; *lhs_ptr != '\0';
         ++lhs_ptr, ++rhs_ptr) {
      if (*lhs_ptr != *rhs_ptr)
        return false;
    }
    return true;
  }

  constexpr bool operator==(char const* chars) const {
    auto lhs_ptr = _ptr, rhs_ptr = chars;
    for (; *lhs_ptr != '\0'; ++lhs_ptr, ++rhs_ptr) {
      if (*rhs_ptr == '\0')
        return false;
      if (*lhs_ptr != *rhs_ptr)
        return false;
    }
    return *rhs_ptr == '\0';
  }

  constexpr operator bool() const { return _ptr; }

private:
  char const* _ptr;
  std::size_t _size;
};

template <>
struct Hash<String> {
  using ResultType = std::size_t;
  using ArgumentType = String;

  constexpr std::size_t operator()(String const& string) { return string.hash(); }
};

namespace Internal {
template <typename THead, typename... TTail>
struct TupleHeadTypeProvider {
  using type = THead;
};

template <std::size_t N, std::size_t M, typename T, typename... TTail>
struct TupleToPairConversionImpl
  : public TupleToPairConversionImpl<N + 1,
                                     M,
                                     T,
                                     typename std::tuple_element<N, T>::type,
                                     TTail...> {

  using Base = TupleToPairConversionImpl<N + 1,
                                         M,
                                         T,
                                         typename std::tuple_element<N, T>::type,
                                         TTail...>;

  constexpr static void setPairTupleValueFromTuple(typename Base::ValueType& pair_tuple,
                                                   T const& tuple) {
    std::get<N - 1>(pair_tuple) = std::get<N>(tuple);
    Base::setPairTupleValueFromTuple(pair_tuple, tuple);
  }
};

template <typename T>
struct TupleToPairConversionImpl<1, 2, T> : public TupleToPairConversionImpl<2, 2, T> {

  using Base = TupleToPairConversionImpl<2, 2, T>;

  constexpr static void setPairTupleValueFromTuple(typename Base::ValueType& pair_tuple,
                                                   T const& tuple) {
    pair_tuple = std::get<1>(tuple);
  }
};

template <std::size_t M, typename T>
struct TupleToPairConversionImpl<0, M, T> : public TupleToPairConversionImpl<1, M, T> {
  using Base = TupleToPairConversionImpl<1, M, T>;

  using KeyType = typename std::conditional<
    std::is_convertible<typename std::remove_cv<typename std::remove_reference<
                          typename std::tuple_element<0, T>::type>::type>::type,
                        char const*>::value,
    String,
    typename std::tuple_element<0, T>::type>::type;
  using ValueType = typename Base::ValueType;
  using PairType = std::pair<KeyType, ValueType>;

  constexpr static auto makePairFromTuple(T const& t) {
    PairType pair;
    pair.first = std::get<0>(t);
    Base::setPairTupleValueFromTuple(pair.second, t);
    return pair;
  }
};

template <std::size_t M, typename T, typename... TTail>
struct TupleToPairConversionImpl<M, M, T, TTail...> {
  using ValueType = std::tuple<TTail...>;

  constexpr static auto setPairTupleValueFromTuple(ValueType& pair_tuple,
                                                   T const& tuple) {}

  constexpr static bool isValueTuple = true;
};

template <typename T, typename... TTail>
struct TupleToPairConversionImpl<2, 2, T, TTail...> {
  using ValueType = typename std::tuple_element<1, T>::type;

  constexpr static auto setPairTupleValueFromTuple(ValueType& pair_tuple,
                                                   T const& tuple) {}

  constexpr static bool isValueTuple = false;
};

template <typename T>
struct TupleToPairConversion
  : public TupleToPairConversionImpl<0, std::tuple_size<T>::value, T> {};

template <std::size_t N, typename THead, typename... TTail>
struct TupleAssignmentImpl : TupleAssignmentImpl<N + 1, TTail...> {
  using Base = TupleAssignmentImpl<N + 1, TTail...>;

  template <typename... TArgs>
  constexpr static auto assign(std::tuple<TArgs...>& lhs,
                               std::tuple<TArgs...> const& rhs) {
    std::get<N>(lhs) = std::get<N>(rhs);
    Base::assign(lhs, rhs);
  }
};

template <std::size_t N, typename THead>
struct TupleAssignmentImpl<N, THead> {

  template <typename... TArgs>
  constexpr static auto assign(std::tuple<TArgs...>& lhs,
                               std::tuple<TArgs...> const& rhs) {
    std::get<N>(lhs) = std::get<N>(rhs);
  }
};

template <typename T>
constexpr auto assignTuples(T& lhs, T const& rhs) {
  lhs = rhs;
}

template <typename... TArgs>
constexpr auto assignTuples(std::tuple<TArgs...>& lhs, std::tuple<TArgs...> const& rhs) {
  TupleAssignmentImpl<0, TArgs...>::assign(lhs, rhs);
}
}

template <typename T, std::size_t N>
struct HashMapSpec {
  using KeyType = typename T::first_type;
  using ValueType = typename T::second_type;
  using PairType = T;

  std::size_t maxBucketSize;
  std::size_t bucketCount;
  std::size_t elementCount;
  Array<PairType, N> dataPairs;
  Array<std::size_t, N> bucketIndexes;
  Array<bool, N> nonuniquenesses;
};

namespace Internal {
template <typename... TArgs>
constexpr auto
makeHashMapSpecImpl(double load_factor, double min_load_factor, TArgs&&... args) {
  using tuple_type = typename Internal::TupleHeadTypeProvider<TArgs...>::type;
  using tuple_pair_converter_type = Internal::TupleToPairConversion<tuple_type>;
  using pair_type = typename tuple_pair_converter_type::PairType;

  Array<pair_type, sizeof...(args)> const data_pairs{
    {tuple_pair_converter_type::makePairFromTuple(args)...}};
  Array<std::size_t, sizeof...(args)> bucket_indexes{};
  Array<bool, sizeof...(args)> nonuniquenesses{};
  for (std::size_t i = 0; i < bucket_indexes.size(); ++i) {
    bucket_indexes[i]
      = Hash<typename tuple_pair_converter_type::KeyType>()(data_pairs[i].first);
  }
  for (std::size_t i = 0; i < bucket_indexes.size(); ++i) {
    if (nonuniquenesses[i])
      continue;
    for (std::size_t j = i + 1; j < bucket_indexes.size(); ++j) {
      if (bucket_indexes[j] == bucket_indexes[i]) {
        if (data_pairs[j].first == data_pairs[i].first) {
          nonuniquenesses[j] = true;
        }
      }
    }
  }
  std::size_t element_count = 0;
  for (std::size_t i = 0; i < nonuniquenesses.size(); ++i) {
    if (!nonuniquenesses[i])
      ++element_count;
  }
  std::size_t current_bucket_count
    = static_cast<std::size_t>(element_count / load_factor);
  std::size_t last_improving_bucket_count = current_bucket_count;
  std::size_t current_max_bucket_size = 0;
  std::size_t last_improving_max_bucket_size = std::numeric_limits<std::size_t>::max();
  while (true) {
    current_max_bucket_size = 0;
    for (std::size_t i = 0; i < bucket_indexes.size(); ++i) {
      if (nonuniquenesses[i])
        continue;
      std::size_t current_bucket_size = 1;
      std::size_t current_bucket_index = bucket_indexes[i] % current_bucket_count;
      for (std::size_t j = i + 1; j < bucket_indexes.size(); ++j) {
        if (!nonuniquenesses[j]
            && (bucket_indexes[j] % current_bucket_count) == current_bucket_index)
          ++current_bucket_size;
      }
      if (current_bucket_size > current_max_bucket_size)
        current_max_bucket_size = current_bucket_size;
    }
    if (current_max_bucket_size < last_improving_max_bucket_size) {
      last_improving_bucket_count = current_bucket_count;
      last_improving_max_bucket_size = current_max_bucket_size;
    }
    if (current_max_bucket_size <= 1) {
      break;
    }
    ++current_bucket_count;
    if ((float)element_count / current_bucket_count < min_load_factor) {
      break;
    }
  }
  for (std::size_t i = 0; i < bucket_indexes.size(); ++i) {
    if (!nonuniquenesses[i])
      bucket_indexes[i] %= last_improving_bucket_count;
  }
  return HashMapSpec<pair_type, sizeof...(TArgs)>{last_improving_max_bucket_size,
                                                  last_improving_bucket_count,
                                                  element_count,
                                                  data_pairs,
                                                  bucket_indexes,
                                                  nonuniquenesses};
}
}

template <typename... TArgs,
          typename = typename std::enable_if<std::is_floating_point<
            typename Internal::TupleHeadTypeProvider<TArgs...>::type>::value>::type>
constexpr static auto makeHashMapSpec(TArgs&&... args) {
  return Internal::makeHashMapSpecImpl(std::forward<TArgs>(args)...);
}

template <typename... TArgs,
          typename std::enable_if<
            !std::is_floating_point<
              typename Internal::TupleHeadTypeProvider<TArgs...>::type>::value,
            int>::type
          = 0>
constexpr static auto makeHashMapSpec(TArgs&&... args) {
  return Internal::makeHashMapSpecImpl(1.0, 0.5, std::forward<TArgs>(args)...);
}

template <typename TSpec, std::size_t N, std::size_t M, std::size_t C>
class HashMap {
public:
  using KeyType = typename TSpec::KeyType;
  using ValueType = typename TSpec::ValueType;
  using PairType = typename TSpec::PairType;

  constexpr auto begin() const { return _buckets.begin(); }

  constexpr auto end() const { return _buckets.end(); }

  constexpr std::size_t bucketSize() const { return N; };

  constexpr std::size_t bucketCount() const { return M; };

  constexpr std::size_t size() const { return C; };

  template <typename U>
  constexpr ValueType find(U const& key) const noexcept {
    for (auto ptr = _buckets[Hash<U>()(key) % M].begin(), end_ptr = ptr + N;
         ptr != end_ptr;
         ++ptr) {
      if (!ptr->first)
        return ValueType{};
      if (ptr->first == key)
        return ptr->second;
    }
    return ValueType{};
  }

  template <typename U>
  constexpr auto operator[](U const& key) const {
    return find(key);
  }

  static constexpr HashMap make(TSpec const& spec) {
    Array<Array<PairType, N>, M> array{};
    for (std::size_t i = 0; i < spec.dataPairs.size(); ++i) {
      if (spec.nonuniquenesses[i])
        continue;
      for (auto ptr = array[spec.bucketIndexes[i]].begin(), end_ptr = ptr + N;
           ptr != end_ptr;
           ++ptr) {
        if (ptr->first)
          continue;
        ptr->first = spec.dataPairs[i].first;
        Internal::assignTuples(ptr->second, spec.dataPairs[i].second);
        break;
      }
    }
    return HashMap{array};
  }

private:
  constexpr HashMap(Array<Array<PairType, N>, M> const& data) : _buckets(data){};

  Array<Array<PairType, N>, M> _buckets;
};
}
