/*
  Copyright (C) Lukas Riebel 2020.
  Distributed under the MIT License
  (license terms are at http://opensource.org/licenses/MIT).
*/

#include <gtest/gtest.h>

#include <fstream>
#include <random>
#include <array>

#include "sorting_network_cpp/sorting_network.hpp"

namespace the_grumpy_coda
{

static constexpr size_t MaxN = 16;
static constexpr size_t NumRuns = 100;

struct PlusOneStep  { static constexpr inline int Value(int i) { return i + 1; } };
struct TimesTwoStep { static constexpr inline int Value(int i) { return i * 2; } };

template <int Current, int End, typename StepF = PlusOneStep, typename F>
static void static_for(F callable);

template <size_t UpTo, NetworkType NWT, typename RandT, typename StepF = PlusOneStep>
void TestRandomSorting(RandT& rd);

template <size_t UpTo, NetworkType NWT, typename RandT, typename StepF = PlusOneStep>
void TestOrderedSorting(RandT& rd);

template <size_t UpTo, NetworkType NWT, typename RandT, typename StepF = PlusOneStep>
void TestReverseOrderedSorting(RandT& rd);

class SortingNetworkTests : public ::testing::Test
{
protected:
  std::default_random_engine rd{42};
};

TEST_F(SortingNetworkTests, RandomBubbleSort)
{
  TestRandomSorting<MaxN, NetworkType::BubbleSort>(rd);
}

TEST_F(SortingNetworkTests, RandomInsertionSort)
{
  TestRandomSorting<MaxN, NetworkType::InsertionSort>(rd);
}

TEST_F(SortingNetworkTests, RandomBoseNelsonSort)
{
  TestRandomSorting<MaxN, NetworkType::BoseNelsonSort>(rd);
}

TEST_F(SortingNetworkTests, RandomBatcherOddEvenMergeSort)
{
  TestRandomSorting<MaxN, NetworkType::BatcherOddEvenMergeSort, decltype(rd), TimesTwoStep>(rd);
}

TEST_F(SortingNetworkTests, RandomBitonicMergeSort)
{
  TestRandomSorting<MaxN, NetworkType::BitonicMergeSort>(rd);
}

TEST_F(SortingNetworkTests, RandomSizeOptimizedSort)
{
  TestRandomSorting<32, NetworkType::SizeOptimizedSort>(rd);
}

TEST_F(SortingNetworkTests, SortedBubbleSort)
{
  TestOrderedSorting<MaxN, NetworkType::BubbleSort>(rd);
}

TEST_F(SortingNetworkTests, SortedInsertionSort)
{
  TestOrderedSorting<MaxN, NetworkType::InsertionSort>(rd);
}

TEST_F(SortingNetworkTests, SortedBoseNelsonSort)
{
  TestOrderedSorting<MaxN, NetworkType::BoseNelsonSort>(rd);
}

TEST_F(SortingNetworkTests, ReverseSortedBubbleSort)
{
  TestReverseOrderedSorting<MaxN, NetworkType::BubbleSort>(rd);
}

TEST_F(SortingNetworkTests, ReverseSortedInsertionSort)
{
  TestReverseOrderedSorting<MaxN, NetworkType::InsertionSort>(rd);
}

TEST_F(SortingNetworkTests, ReverseSortedBoseNelsonSort)
{
  TestReverseOrderedSorting<MaxN, NetworkType::BoseNelsonSort>(rd);
}

template <typename ContainerT, typename Comp = std::less<typename ContainerT::value_type>>
bool IsOrdered(const ContainerT& container, Comp c = {})
{
  bool failed = false;

  for (auto it = (std::begin(container) + 1), end = std::end(container); it != end && !failed; ++it)
    failed |= c(*it, *(it - 1));

  return !failed;
}

template <typename T, size_t S, typename RandomDevice>
void FillRandom(std::array<T, S>& arr, RandomDevice& rd)
{
  std::uniform_int_distribution<T> dis(
    std::numeric_limits<T>::lowest(), std::numeric_limits<T>::max()
  );

  std::generate(arr.begin(), arr.end(), [&rd, &dis] { return dis(rd); });
}

template <size_t UpTo, NetworkType NWT, typename RandT, typename StepF>
void TestRandomSorting(RandT& rd)
{
  static_for<1, UpTo + 1, StepF>([&](auto N)
  {
    for (size_t i = 0; i < NumRuns; ++i)
    {
      std::array<int32_t, N> arr{};

      do
      {
        FillRandom(arr, rd);
      } while (N > 1 && IsOrdered(arr));

      SortingNetwork<N, NWT>()(arr);
      ASSERT_TRUE(IsOrdered(arr)) << " for array of size " << N;
    }
  });
}

template <size_t UpTo, NetworkType NWT, typename RandT, typename StepF>
void TestOrderedSorting(RandT& rd)
{
  static_for<1, UpTo + 1, StepF>([&](auto N)
  {
    for (size_t i = 0; i < NumRuns; ++i)
    {
      std::array<int32_t, N> arr{};

      FillRandom(arr, rd);
      std::sort(arr.begin(), arr.end());

      SortingNetwork<N, NWT>()(arr);
      ASSERT_TRUE(IsOrdered(arr)) << " for array of size " << N;
    }
  });
}

template <size_t UpTo, NetworkType NWT, typename RandT, typename StepF>
void TestReverseOrderedSorting(RandT& rd)
{
  static_for<1, UpTo + 1, StepF>([&](auto N)
  {
    for (size_t i = 0; i < NumRuns; ++i)
    {
      std::array<int32_t, N> arr{};

      FillRandom(arr, rd);
      std::sort(arr.begin(), arr.end(), std::greater<int32_t>());

      SortingNetwork<N, NWT>()(arr);
      ASSERT_TRUE(IsOrdered(arr)) << " for array of size " << N;
    }
  });
}

template <int Current, int End, typename StepF, typename F>
static void static_for(F callable)
{
  if constexpr (Current < End)
  {
    callable(std::integral_constant<int, Current>());
    static_for<StepF::Value(Current), End, StepF, F>(callable);
  }
}

}

