#include "test-timer-macro-multifile.hpp"

void test1() {
	CHRONODE_START(T, "file/test1")

	chronode::sleep_for(10ms);

	CHRONODE_STOP(T)
}
