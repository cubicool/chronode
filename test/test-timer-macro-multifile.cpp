// vimrun! ./test-timer-macro-multifile

#include "test-timer-macro-multifile.hpp"

#include <iostream>
#include <random>

template <typename T, T N>
[[maybe_unused]] auto rand_1n() {
	std::mt19937_64 gen(std::random_device{}());

	return std::uniform_int_distribution<T>(1, N)(gen);
}

[[maybe_unused]] constexpr auto r_10 = []() { return rand_1n<size_t, 10>(); };
[[maybe_unused]] constexpr auto r_500ms = []() { return std::chrono::milliseconds(rand_1n<size_t, 500>()); };

using chronode::sleep_for;

CHRONODE_MILLITIMER_INIT(T, "MilliTimer");

int main(int argc, char** argv) {
	CHRONODE_START(T, "foo")

	sleep_for(10ms); {
		CHRONODE_START(T, "bar")

		sleep_for(20ms); {
			CHRONODE_START(T, "baz")

			sleep_for(30ms);

			CHRONODE_STOP(T)
		}

		sleep_for(15ms);

		CHRONODE_STOP(T)
	}

	sleep_for(25ms);

	CHRONODE_STOP(T)

	sleep_for(55ms);

	CHRONODE_START(T, "foofoo")

	sleep_for(50ms);

	CHRONODE_STOP(T)

	sleep_for(75ms);

	test0();

	sleep_for(10ms);

	test1();

	CHRONODE_STOP(T)

	CHRONODE {
		chronode::report::ostream_json(T.node(), std::cout);

		/* const auto& rs = chronode::report::rows_spans(T.node());

		for(size_t i = 0; i < rs.size(); i++) {
			const auto& r = rs[i];

			if(!i) std::cout << "RANGE: " << r[0].first << " - " << r[0].second << std::endl;

			else {
				std::cout << "TODO: row " << i << "(" << r.size() << ")" << std::endl;
			}
		} */
	}

	return 0;
}
