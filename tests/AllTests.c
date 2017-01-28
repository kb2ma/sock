#include <embUnit/embUnit.h>

TestRef NanocoapTest_tests(void);

int main (int argc, const char* argv[])
{
	TestRunner_start();
		TestRunner_runTest(NanocoapTest_tests());
	TestRunner_end();
	return 0;
}
