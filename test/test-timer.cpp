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
[[maybe_unused]] constexpr auto r_500ms = []() { return std::chrono::milliseconds(rand_1n<size_t, 500>()); };

int main(int argc, char** argv) {
	// auto t = chronode::MicroTimer("timer");
	auto t = chronode::MilliTimer("timer");
	auto n = r_10();

	for(decltype(n) i = 0; i < n; i++) {
		t.start("A"); { chronode::sleep_for(10ms); t.start("a"); { chronode::sleep_for(10ms); }; t.stop(); }; t.stop();
		t.start("B"); { chronode::sleep_for(20ms); }; t.stop();
		t.start("C"); { chronode::sleep_for(30ms); }; t.stop();
		t.start("D"); { chronode::sleep_for(40ms); }; t.stop();
		t.start("E"); { chronode::sleep_for(30ms); t.start("e"); { chronode::sleep_for(30ms); }; t.stop(); }; t.stop();
		t.start("F"); { chronode::sleep_for(40ms); t.start("f"); { chronode::sleep_for(40ms); }; t.stop(); }; t.stop();

		auto s = r_500ms();

		// std::cout << "Sleep for: " << s.count() << std::endl;

		chronode::sleep_for(s);

		t.stop();

		chronode::report::ostream(t.node(), std::cout);

		// This stupid hack skips the last reset (so the Tablulate works below).
		if(i < n - 1) t.node().reset();
	}

#if 1
	tabulate::Table table;

	// The code below will fetch the string_view representation of the "unit_t" used.
	// For example: ms (milliseconds), us (microseconds), ns (nanoseconds), etc.
	constexpr const auto ds = chronode::duration_str<decltype(t)::node_t::unit_t>();

	table.add_row({"Id", "Parent", "Path", "#", ds, "Start", "Stop"});

	chronode::report::tabulate(t.node(), table);

	// Formatting needs to be done AFTER rows are added? Feels ... unnatural.
	table.row(0).format().font_style({tabulate::FontStyle::bold});

	table.column(6).format()
		// .font_style({tabulate::FontStyle::bold})
		.font_color(tabulate::Color::green)
		.font_align(tabulate::FontAlign::right)
	;

	std::cout << table << std::endl;
#endif

	return 0;
}
