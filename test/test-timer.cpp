// vimrun! ./test-timer

#include "chronode.hpp"

#include <iostream>
#include <random>

template <typename T, T N>
[[maybe_unused]] auto rand_1n() {
	std::mt19937_64 gen(std::random_device{}());

	return std::uniform_int_distribution<T>(1, N)(gen);
}

[[maybe_unused]] constexpr auto r_10 = []() { return rand_1n<size_t, 10>(); };
[[maybe_unused]] constexpr auto r_50ms = []() { return std::chrono::milliseconds(rand_1n<size_t, 50>()); };
[[maybe_unused]] constexpr auto r_100ms = []() { return std::chrono::milliseconds(rand_1n<size_t, 100>()); };
[[maybe_unused]] constexpr auto s_50ms = []() { chronode::sleep_for(r_50ms()); };
[[maybe_unused]] constexpr auto s_100ms = []() { chronode::sleep_for(r_100ms()); };

#if 0
template<typename Timer, typename... Nodes>
void add_nodes(Timer& timer, const Nodes&... nodes) {
	for(const auto& pair : {nodes...}) {
		// std::cout << pair.first << ": " << pair.second << std::endl;
	}
}
#endif

template<typename T>
void func_50ms(T& timer) {
	timer.start("func_50ms");

	s_50ms();

	timer.stop();
}

template<typename T>
void func_100ms(T& timer) {
	timer.start("func_100ms");

	func_50ms(timer);
	func_50ms(timer);

	timer.stop();
}

constexpr auto sf = [](auto i) { chronode::sleep_for(std::chrono::milliseconds(10 * (i + 1))); };

int main(int argc, char** argv) {
	CHRONODE {
		auto p = chronode::MilliProfile(3);
		auto t = chronode::MilliTimer("timer");

		for(auto i = 0; i < 5; i++) {
			t.start("main"); sf(i); {
				t.start("A"); sf(i); {
					t.start("a0"); sf(i); t.stop();
					t.start("a1"); sf(i); t.stop();
				} t.stop();

				t.start("B"); sf(i); {
					t.start("b"); sf(i); t.stop();
				} t.stop();

				t.start("C"); sf(i); t.stop();
			} t.stop();

			t.start("submain"); sf(i); t.stop();

			p.add(t);
			// p.add(t.reset());

			// auto tmp = t.reset();
			// std::cout << *tmp << std::endl;
			// delete tmp;
		}

		std::cout << p << std::endl;
	}

	return 0;
}

#if 0
int main(int argc, char** argv) {
	CHRONODE {
		auto p = chronode::MilliProfile(5);
		auto t = chronode::MilliTimer("timer");
		auto n = r_10();

		// TODO: Make a variadic template function to handle these calls!
		for(decltype(n) i = 0; i < n; i++) {
			t.start("A"); { s_50ms(); t.start("a"); { s_50ms(); }; t.stop(); }; t.stop(); s_100ms();
			t.start("B"); { s_50ms(); }; t.stop(); s_100ms();
			t.start("C"); { s_50ms(); }; t.stop(); s_100ms();
			t.start("D"); { s_50ms(); }; t.stop(); s_100ms();
			t.start("E"); { s_50ms(); t.start("e"); { s_50ms(); }; t.stop(); }; t.stop(); s_100ms();
			t.start("F"); { s_50ms(); t.start("f"); { s_50ms(); }; t.stop(); }; t.stop(); s_100ms();

			func_100ms(t); s_50ms();

			p.add(t.reset());

			// If you wanted, you could also do the following:
			/* const auto& tmp = t.reset();

			std::cout << tmp << std::endl;

			p.add(tmp); */
		}

		std::cout << p << std::endl;
	}

	return 0;
}
#endif
