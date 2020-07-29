/*
  Copyright (C) Lukas Riebel 2020.
  Distributed under the MIT License
  (license terms are at http://opensource.org/licenses/MIT).
*/

#include <string>
#include <chrono>
#include <sstream>

template <typename F> double MeasureExectionTimeMillis(F f);
template <typename T> std::string DataTypeName();

template <typename F>
double MeasureExectionTimeMillis(F f)
{
  using Clock = std::conditional_t<std::chrono::high_resolution_clock::is_steady,
    std::chrono::high_resolution_clock,
    std::chrono::steady_clock>;

  const auto start = Clock::now();
  f();
  const auto end = Clock::now();

  const auto nanos = std::chrono::duration_cast<std::chrono::duration<uint64_t, std::nano>>(end - start);
  return std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(nanos).count();
}

inline std::string CompilerName()
{
  std::ostringstream os;

#if defined(_MSC_VER)
  os << "msvc " << _MSC_VER;
#elif defined(__clang__)
  os << "clang " << __clang_major__ << "." << __clang_minor__ << "." << __clang_patchlevel__;
#elif defined(__GNUC__)
  os << "gcc " << __GNUC__ << "." << __GNUC_MINOR__;
#else
  os << "unknown compiler";
#endif

  return os.str();
}
