#include "test-timer-macro-multifile.hpp"

void test0() {
	CHRONODE_START(T, "file/test0")

	chronode::sleep_for(25ms);

	CHRONODE_STOP(T)
}
