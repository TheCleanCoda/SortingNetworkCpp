/*
  Copyright (C) Lukas Riebel 2020.
  Distributed under the MIT License
  (license terms are at http://opensource.org/licenses/MIT).
*/

#include <array>
#include <random>
#include <optional>
#include <fstream>

#include "sorting_network_cpp/sorting_network.hpp"
#include "util.hpp"

using Vec2i = std::array<uint16_t, 2>;
struct Vec2iComp;

template <typename T, typename Comp, size_t N_>
void BenchmarkN();

template <typename T, typename Comp = std::less<T>, size_t ... Indices>
void Benchmark(std::index_sequence<Indices...>)
{
  ((BenchmarkN<T, Comp, Indices>()), ...);
}

namespace the_grumpy_coda
{
  template <typename C>
  struct CompareAndSwap<float, C>
  {
    void operator()(float& a, float& b, C comp) const
    {
      if (a > b) std::swap(a, b);
    }
  };
}

int main(int argc, const char** args)
{
  const std::vector<std::string> headers{
    "compiler", "data type", "N",
    "SN::BubbleSort", "SN::InsertionSort",
    "SN::BatcherOddEvenMergeSort", "SN::BitonicMergeSort",
    "SN::SizeOptimizedSort", "SN::BoseNelsonSort",
    "std::sort"
  };

  for (size_t i = 0, len = headers.size(); i < len; ++i)
    std::cout << headers[i] << (i + 1 < len ? "\t" : "");

  std::cout << std::endl;

  constexpr auto seq = std::make_index_sequence<8>();

  Benchmark<int16_t>(seq);
  Benchmark<int32_t>(seq);
  Benchmark<int64_t>(seq);
  Benchmark<float>(seq);
  Benchmark<double>(seq);
  Benchmark<Vec2i, Vec2iComp>(seq);

  return 0;
}

template <the_grumpy_coda::NetworkType NT, typename Comp, typename T, size_t N>
double BenchmarkSortingNet(std::vector<std::array<T, N>> vector);

template <typename T, size_t S, typename RandEngine>
void FillVectorOfArraysRandom(std::vector<std::array<T, S>>& vector, RandEngine& rd);

std::string CompilerName();

template <typename T, typename Comp, size_t N_>
void BenchmarkN()
{
  static constexpr size_t N = 1 << N_;
  static constexpr bool NIsPowerOfTwo = (N > 0) && !(N & (N - 1));

  std::vector<std::array<T, N>> unsorted_data(1'000'000);
  std::default_random_engine rd(42);

  FillVectorOfArraysRandom(unsorted_data, rd);

  using NT = the_grumpy_coda::NetworkType;

  double bubble_sort;
  double insertion_sort;
  double bitonic_sort;
  double bose_nelson_sort;
  std::optional<double> batcher_sort;
  std::optional<double> size_opt_sort;

  bubble_sort = BenchmarkSortingNet<NT::BubbleSort, Comp>(unsorted_data);
  insertion_sort = BenchmarkSortingNet<NT::InsertionSort, Comp>(unsorted_data);
  bitonic_sort = BenchmarkSortingNet<NT::BitonicMergeSort, Comp>(unsorted_data);
  bose_nelson_sort = BenchmarkSortingNet<NT::BoseNelsonSort, Comp>(unsorted_data);

  if constexpr (NIsPowerOfTwo)
    batcher_sort = BenchmarkSortingNet<NT::BatcherOddEvenMergeSort, Comp>(unsorted_data);

  if constexpr (N <= 32)
    size_opt_sort = BenchmarkSortingNet<NT::SizeOptimizedSort, Comp>(unsorted_data);

  double std_sort = 0;
  {
    std::vector<std::array<T, N>> cpy = unsorted_data;

    for (auto& arr : cpy)
      std_sort += MeasureExectionTimeMillis([&] { std::sort(arr.begin(), arr.end(), Comp()); });
  }

  std::vector<std::string> row_values{
    CompilerName(), DataTypeName<T>(), std::to_string(N),
    std::to_string(bubble_sort),
    std::to_string(insertion_sort),
    batcher_sort ? std::to_string(*batcher_sort) : "",
    std::to_string(bitonic_sort),
    size_opt_sort ? std::to_string(*size_opt_sort) : "",
    std::to_string(bose_nelson_sort),
    std::to_string(std_sort),
  };

  for (size_t i = 0, len = row_values.size(); i < len; ++i)
    std::cout << row_values[i] << (i + 1 < len ? "\t" : "");

  std::cout << std::endl;
}

template <the_grumpy_coda::NetworkType NT, typename Comp, typename T, size_t N>
double BenchmarkSortingNet(std::vector<std::array<T, N>> vector)
{
  double elapsed_time = 0;

  const the_grumpy_coda::SortingNetwork<N, NT> sorting_network;

  for (auto& arr : vector)
    elapsed_time += MeasureExectionTimeMillis([&] { sorting_network(arr, Comp()); });

  return elapsed_time;
}

// A slightly more complex comparison operation:
// compare the morton code of two points (by generating the
// morton codes on the fly)
struct Vec2iComp
{
  static uint16_t ExpandBits(uint16_t x)
  {
    x &= 0x000003ff;                  // x = ---- ---- ---- ---- ---- --98 7654 3210
    x = (x ^ (x << 16)) & 0x030000ff; // x = ---- --98 ---- ---- ---- ---- 7654 3210
    x = (x ^ (x << 8))  & 0x0300f00f; // x = ---- --98 ---- ---- 7654 ---- ---- 3210
    x = (x ^ (x << 4))  & 0x030c30c3; // x = ---- --98 ---- 76-- --54 ---- 32-- --10
    x = (x ^ (x << 2))  & 0x09249249; // x = ---- 9--8 --7- -6-- 5--4 --3- -2-- 1--0

    return x;
  }

  static uint32_t GenerateMortonCode2d(const Vec2i& p)
  {
    return (ExpandBits(p[0]) << 1 | ExpandBits(p[1]));
  }

  bool operator()(const Vec2i& lhs, const Vec2i& rhs) const
  {
    return GenerateMortonCode2d(lhs) < GenerateMortonCode2d(rhs);
  }
};

template <typename T, typename RandEngine>
struct RandomElementGenerator
{
  T operator()(RandEngine& rd) const
  {
    static_assert(std::is_scalar_v<T>, "Cannot generate random values for this data type, provide a custom random data generator");

    std::conditional_t<std::is_floating_point_v<T>,
      std::uniform_real_distribution<T>,
      std::uniform_int_distribution<size_t>
    > dis(0, 1'000'000);

    return static_cast<T>(dis(rd));
  }
};

template <typename RandEngine>
struct RandomElementGenerator<Vec2i, RandEngine>
{
  Vec2i operator()(RandEngine& rd) const
  {
    std::uniform_int_distribution<uint16_t> dis(0, 1 << 10);

    return { dis(rd), dis(rd) };
  }
};

template <typename It, typename RandEngine>
void GenerateRandom(It begin, It end, RandEngine& rd)
{
  std::generate(begin, end, [&rd] {return RandomElementGenerator<typename std::iterator_traits<It>::value_type, RandEngine>()(rd); });
}

template <typename T, size_t S, typename RandEngine>
void FillVectorOfArraysRandom(std::vector<std::array<T, S>>& vector, RandEngine& rd)
{
  for (auto& arr : vector)
    for (auto& t : arr)
      t = RandomElementGenerator<T, RandEngine>()(rd);
}

template <> std::string DataTypeName<int16_t>() { return "int16_t"; };
template <> std::string DataTypeName<int32_t>() { return "int32_t"; };
template <> std::string DataTypeName<int64_t>() { return "int64_t"; };
template <> std::string DataTypeName<float>()   { return "float"; };
template <> std::string DataTypeName<double>()  { return "double"; };
template <> std::string DataTypeName<Vec2i>()   { return "Vec2i Z-order"; };
