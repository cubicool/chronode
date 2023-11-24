// vimrun! ./test-timer-macro

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

CHRONODE_MILLITIMER_INIT(T);

int main(int argc, char** argv) {
	CHRONODE_START(T, "foo")

	chronode::sleep_for(10ms); {
		CHRONODE_START(T, "bar")

		chronode::sleep_for(20ms); {
			CHRONODE_START(T, "baz")

			chronode::sleep_for(30ms);

			CHRONODE_STOP(T);
		}

		CHRONODE_STOP(T);
	}

	CHRONODE_STOP(T);

	CHRONODE_START(T, "foofoo")

	chronode::sleep_for(50ms);

	CHRONODE_STOP(T);

	// TODO: This really needs to be improved.
	CHRONODE {
		auto t = T.reset();

		std::cout << *t << std::endl;
	}

	return 0;
}
