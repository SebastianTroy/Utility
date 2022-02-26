#ifndef ENERGY_H
#define ENERGY_H

#include <cstdint>

using Energy = double;
constexpr Energy operator ""_Mj(unsigned long long num) { return static_cast<double>(num) * 1'000'000; }
constexpr Energy operator ""_kj(unsigned long long num) { return static_cast<double>(num) * 1'000; }
constexpr Energy operator ""_j (unsigned long long num) { return static_cast<double>(num); }
constexpr Energy operator ""_mj(unsigned long long num) { return static_cast<double>(num) / 1'000; }
constexpr Energy operator ""_uj(unsigned long long num) { return static_cast<double>(num) / 1'000'000; }

constexpr Energy operator ""_Mj(long double num) { return num * 1'000'000; }
constexpr Energy operator ""_kj(long double num) { return num * 1'000; }
constexpr Energy operator ""_j (long double num) { return num; }
constexpr Energy operator ""_mj(long double num) { return num / 1'000; }
constexpr Energy operator ""_uj(long double num) { return num / 1'000'000; }

#endif // ENERGY_H
