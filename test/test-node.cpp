// vimrun! ./test-node

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

#if 0
template<typename Node, typename... Nodes>
void add_nodes(Node& node, const Nodes&... nodes) {
	node.start()

	for(const auto& n : {nodes...}) {
		auto c = node.child(n.first).start();

		chronode::sleep(n.second);

		c.stop();
	}

	node.stop();
}
#endif

int main(int argc, char** argv) {
	CHRONODE {
		auto n = chronode::MilliNode("n");

		n.start();
		n.child("a").start().sleep(10).stop();
		n.stop();

		chronode::report::ostream(n, std::cout);

		n.reset();

		n.start();
		n.child("b").start().sleep(20ms).stop();
		n.child("c").start().sleep(30ms).stop();

		auto& d = n.child("d");

		d.start();
		d.child("0").start().sleep(15ms).stop();
		d.child("1").start().sleep(25ms).stop();
		d.stop();

		n.child("e").start().sleep(50ms).stop();
		n.stop();

		chronode::report::ostream(n, std::cout);

		auto nc = n.copy();

		chronode::report::ostream(nc, std::cout);
	}

	return 0;
}
