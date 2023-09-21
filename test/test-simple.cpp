// vimrun! ./test-simple

#include "chronode.hpp"

#include <iostream>
#include <random>

template <typename T, T N>
auto rand_1n() {
	std::mt19937_64 gen(std::random_device{}());

	return std::uniform_int_distribution<T>(1, N)(gen);
}

constexpr auto r_10 = []() { return rand_1n<size_t, 10>(); };
constexpr auto r_500ms = []() { return std::chrono::milliseconds(rand_1n<size_t, 500>()); };

void test_node() {
	CHRONODE {
		auto n = chronode::MilliNode<>("n");

		n.start();
		n.child("n.0").start().sleep(10ms).stop();
		n.stop();

		chronode::report::ostream(n, std::cout);

		n.reset();

		n.start();
		n.child("n.1").start().sleep(20ms).stop();
		n.child("n.2").start().sleep(30ms).stop();
		n.child("n.3").start().sleep(100ms).stop();
		n.stop();

		chronode::report::ostream(n, std::cout);
	}
}

void test_timer(size_t n) {
	auto mt = chronode::MilliTimer<>("mt");

	for(decltype(n) i = 0; i < n; i++) {
		mt.start("A"); { chronode::sleep_for(10ms); mt.start("a"); { chronode::sleep_for(10ms); }; mt.stop(); }; mt.stop();
		mt.start("B"); { chronode::sleep_for(20ms); }; mt.stop();
		mt.start("C"); { chronode::sleep_for(30ms); }; mt.stop();
		mt.start("D"); { chronode::sleep_for(40ms); }; mt.stop();
		mt.start("E"); { chronode::sleep_for(30ms);
			mt.start("e"); { chronode::sleep_for(30ms); }; mt.stop();
		}; mt.stop();
		mt.start("F"); { chronode::sleep_for(40ms);
			mt.start("f"); { chronode::sleep_for(40ms); }; mt.stop();
		}; mt.stop();

		auto s = r_500ms();

		// std::cout << "Sleep for: " << s.count() << std::endl;

		chronode::sleep_for(s);

		mt.stop();

		chronode::report::ostream(mt.node(), std::cout);

		// TODO: Skip the last reset (so the tablulate works).
		if(i < n - 1) mt.node().reset();
	}

#if 1
	tabulate::Table table;

	table.add_row({"Id", "Parent", "Path", "#", "MS", "Start", "Stop"});

	chronode::report::tabulate(mt.node(), table);

	// Formatting needs to be done AFTER rows are added?
	table.row(0).format().font_style({tabulate::FontStyle::bold});

	table.column(6).format()
		// .font_style({tabulate::FontStyle::bold})
		.font_color(tabulate::Color::green)
		.font_align(tabulate::FontAlign::right)
	;

	std::cout << table << std::endl;
#endif

	mt.node().AVERAGES();

	std::cout << "Average was: " << mt.node().duration(true) << std::endl;
}

int main(int argc, char** argv) {
	// for(size_t i = 0; i < 10; i++) std::cout << rand_1n<size_t, 10>() << std::endl;

	test_node();
	test_timer(r_10());

	/* auto tp = std::chrono::system_clock::now();
	auto tpv = std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::time_point_cast<std::chrono::milliseconds>(
			// std::chrono::high_resolution_clock::now()
			tp
		).time_since_epoch()
	).count(); */

	return 0;
}
