#include "olm/olm.h"
#include "testing.hh"

#include <vector>

TEST_CASE("Olm sha256 test") {


std::vector<std::uint8_t> utility_buffer(::olm_utility_size());
::OlmUtility * utility = ::olm_utility(utility_buffer.data());

CHECK_EQ(std::size_t(43), ::olm_sha256_length(utility));
std::uint8_t output[43];
::olm_sha256(utility, "Hello, World", 12, output, 43);

std::uint8_t expected_output[] = "A2daxT/5zRU1zMffzfosRYxSGDcfQY3BNvLRmsH76KU";
CHECK_EQ_SIZE(output, expected_output, 43);

}
