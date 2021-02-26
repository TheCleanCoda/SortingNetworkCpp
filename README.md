# SortingNetworkCpp

SortingNetworkCpp offers templated implementations of various types of [sorting networks](https://en.wikipedia.org/wiki/Sorting_network) which can drastically improve performance for small problem sizes of simple data compared to regular sorting algorithms (e.g. `std::sort`).

Following listing shows the execution time for different algorithms to sort one million arrays of each 64 random `double` values on an AMD Ryzen 7 2700x @3.70 GHz (compiled with clang 10.0.0 `-O2`):

| algorithm                   | time (ms) | speedup vs `std::sort` |
|-----------------------------| :-------: | :--------------------: |
| Bubble Sort                 |  807.38   |          1.78          |
| Insertion Sort              |  784.23   |          1.83          |
| Batcher Odd Even Merge Sort |  223.23   |          6.43          |
| Bitonic Merge Sort          |  262.82   |          5.46          |
| Bose Nelson Sort            |  228.46   |          6.28          |
| std::sort                   |  1435.02  |          1.00          |

An interactive overview of performance measurements for the sorting networks on different data types and problem sizes can be found [here](https://raw.githack.com/TheGrumpyCoda/SortingNetworkCpp/master/doc/data_explorer.htm).

## Usage

A single class `SortingNetwork` is available which encapsulates the implementation of the different sorting networks.

Following listing gives a few examples:

```cpp
#include "sorting_network_cpp/sorting_network.hpp"
#include <array>

void Example()
{
  using namespace tcc;

  static constexpr int N = 16;

  {
    std::array<int, N> my_array;
    // fill array...

    SortingNetwork<N>()(my_array);
    // my_array is now sorted
  }

  {
    int data[N];
    // fill array...

    // raw pointers work as well...
    SortingNetwork<N>()(data);
    // data is now sorted
  }

  {
    int data[N];

    // by default std::less<T> is used as comparator, custom comparators
    // may be passed as follows:

    // custom comparators
    SortingNetwork<N>()(data, std::greater<int>());

    // lambdas
    SortingNetwork<N>()(data, [](int a, int b) { return a < b; });
  }
}
```

When no `NetworkType` is explicitly specified as template argument for `SortingNetwork`, the `NetworkType::BoseNelsonSort` is used.

Available network types ATM are:
* `NetworkType::InsertionSort`
* `NetworkType::BubbleSort`
* `NetworkType::BoseNelsonSort`
* `NetworkType::BatcherOddEvenMergeSort`
* `NetworkType::BitonicMergeSort`
* `NetworkType::SizeOptimizedSort`

Following example shows how to specify a different `NetworkType` than the default one:

```cpp
SortingNetwork<N, NetworkType::BitonicMergeSort>()(data_to_be_sorted);
```

## Using custom compare and swap implementations

The compare and swap operation is the fundamental element a sorting network is comprised of. The default implementation works well on scalar types, but if you want to test a custom implementation you may do this by providing a partial template specialization as in following (suboptimal) example:

```cpp
#include "sorting_network_cpp/sorting_network.hpp"
#include <array>

namespace tcc
{
  template <typename C>
  struct CompareAndSwap<float, C>
  {
    void operator()(float& a, float& b, C comp) const
    {
      if (a > b) std::swap(a,b);
    }
  };
}

void Example(std::array<float,3>& arr)
{
  using namespace tcc;
  tcc::SortingNetwork<arr.size()>()(arr); // will use the specialized CompareAndSwap implementation
}
```

## Requirements
A compiler with C++17 support

## Dependencies
For the bare sorting functionality no dependencies are required. If you want to run the tests, the Google Tests framework is required, which is contained in this repository as GIT submodule.

## Limitations
Depending on the implementation of the comparator the performance advantage of a sorting net compared to a regular sorting algorithm (e.g. `std::sort`) may diminish or even result in worse performance. This can be seen in the [interactive benchmark results overview](https://raw.githack.com/TheGrumpyCoda/SortingNetworkCpp/master/doc/data_explorer.htm) for the data type `Vec2i Z-order` which causes in most cases all variants of sorting networks being outperformed by `std::sort` (see [src/benchmark.cpp](src/benchmark.cpp) for the implementation of the aforementioned data type).

## Known issues
* Compiling with `-ffast-math` using clang 10.0, performance for floating point types worsens a lot

## References / Acknowledgements
* ["A Sorting Problem"](https://dl.acm.org/doi/pdf/10.1145/321119.321126) by Bose et al.
* ["Sorting networks and their applications"](https://core.ac.uk/download/pdf/192393620.pdf) by Batcher
* [Vectorized/Static-Sort](https://github.com/Vectorized/Static-Sort) adaption for Bose-Nelson sort
* [HS Flensburg](https://www.inf.hs-flensburg.de/lang/algorithmen/sortieren/networks/oemen.htm) explanation for Batcher's odd-even mergesort
* [HS Flensburg](https://www.inf.hs-flensburg.de/lang/algorithmen/sortieren/bitonic/oddn.htm) explanation for bitonic sort
* [SortHunter](https://github.com/bertdobbelaere/SorterHunter) for size optimized sorting networks
* [Google Test](https://github.com/google/googletest) for testing
* [Chart.js](https://www.chartjs.org/) and [Papa Parse](https://www.papaparse.com/) for the visualization of benchmark results

## License
[MIT](LICENSE)