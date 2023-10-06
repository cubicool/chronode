// vimrun! ./test-...

#include "chronode.hpp"

#include <iostream>
#include <random>

template <typename T, T N>
[[maybe_unused]] auto rand_1n() {
	std::mt19937_64 gen(std::random_device{}());

	return std::uniform_int_distribution<T>(1, N)(gen);
}

[[maybe_unused]] constexpr auto r_10 = []() { return rand_1n<size_t, 10>(); };
[[maybe_unused]] constexpr auto r_500ms = []() { return std::chrono::milliseconds(rand_1n<size_t, 500>()); };

int main(int argc, char** argv) {
	CHRONODE {
		// Maybe put your test here? :)
	}

	return 0;
}
