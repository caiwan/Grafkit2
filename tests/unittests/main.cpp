#include <gtest/gtest.h>

using ::testing::EmptyTestEventListener;
using ::testing::InitGoogleTest;
using ::testing::Test;
using ::testing::TestCase;
using ::testing::TestEventListeners;
using ::testing::TestInfo;
using ::testing::TestPartResult;
using ::testing::UnitTest;

constexpr size_t loggerPoolBuggerSize = 16 * 8192;

int main(int argc, char ** argv)
{
	InitGoogleTest(&argc, argv);
	const int res = RUN_ALL_TESTS();
	return res;
}
