/*
  Copyright (C) Lukas Riebel 2020.
  Distributed under the MIT License
  (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include <functional>
#include <utility>
#include <iostream>

#include "sorting_network_cpp/sorting_network_impl.hpp"

namespace tcc
{
  template <size_t N, NetworkType NWT = NetworkType::BoseNelsonSort>
  class SortingNetwork
  {
  public:
    static_assert(N > 0);

    template <typename ContainerT>
    void operator()(ContainerT& container) const
    {
      (*this)(container, std::less<typename ContainerT::value_type>());
    }

    template <typename ContainerT, typename ComparatorT>
    void operator()(ContainerT& container, ComparatorT comp) const
    {
      (*this)(std::data(container), comp);
    }

    template <typename T>
    void operator()(T* ptr) const
    {
      (*this)(ptr, std::less<T>());
    }

    template <typename T, typename ComparatorT>
    TCC_CUDA void operator()(T* ptr, ComparatorT comp) const
    {
      if constexpr (N > 1)
        details::SortingNetworkImpl<N, NWT>()(ptr, comp);
    }
  };
}
