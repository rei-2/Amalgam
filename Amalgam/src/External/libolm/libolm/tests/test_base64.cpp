#include "olm/base64.hh"
#include "olm/base64.h"
#include <cstring>
#include <vector>
#include <array>

#include "testing.hh"

 /* Base64 encode test */
TEST_CASE("Base64 C++ binding encode test") {

std::uint8_t input[] = "Hello World";
std::uint8_t expected_output[] = "SGVsbG8gV29ybGQ";
std::size_t input_length = sizeof(input) - 1;

std::size_t output_length = olm::encode_base64_length(input_length);
REQUIRE_EQ(std::size_t(15), output_length);

std::uint8_t output[15] = {};
olm::encode_base64(input, input_length, output);
CHECK_EQ_SIZE(expected_output, output, output_length);
}

TEST_CASE("Base64 C binding encode test") {

std::uint8_t input[] = "Hello World";
std::uint8_t expected_output[] = "SGVsbG8gV29ybGQ";
std::size_t input_length = sizeof(input) - 1;

std::size_t output_length = ::_olm_encode_base64_length(input_length);
REQUIRE_EQ(std::size_t(15), output_length);

std::uint8_t output[15];
output_length = ::_olm_encode_base64(input, input_length, output);
CHECK_EQ(std::size_t(15), output_length);
CHECK_EQ_SIZE(expected_output, output, output_length);
}

 /* Base64 decode test */
TEST_CASE("Base64 C++ binding decode test") {

std::uint8_t input[] = "SGVsbG8gV29ybGQ";
std::uint8_t expected_output[] = "Hello World";
std::size_t input_length = sizeof(input) - 1;

std::size_t output_length = olm::decode_base64_length(input_length);
REQUIRE_EQ(std::size_t(11), output_length);

std::uint8_t output[11];
olm::decode_base64(input, input_length, output);
CHECK_EQ_SIZE(expected_output, output, output_length);
}


TEST_CASE("Base64 C binding decode test") {

std::uint8_t input[] = "SGVsbG8gV29ybGQ";
std::uint8_t expected_output[] = "Hello World";
std::size_t input_length = sizeof(input) - 1;

std::size_t output_length = ::_olm_decode_base64_length(input_length);
REQUIRE_EQ(std::size_t(11), output_length);

std::uint8_t output[11];
output_length = ::_olm_decode_base64(input, input_length, output);
REQUIRE_EQ(std::size_t(11), output_length);
CHECK_EQ_SIZE(expected_output, output, output_length);
}

TEST_CASE("Decoding base64 of invalid length fails with -1") {
std::uint8_t input[] = "SGVsbG8gV29ybGQab";
std::size_t input_length = sizeof(input) - 1;

/* We use a longer but valid input length here so that we don't get back -1.
 * Nothing will be written to the output buffer anyway because the input is
 * invalid. */
std::size_t buf_length = olm::decode_base64_length(input_length + 1);
std::vector<std::uint8_t> output(buf_length, 0);
std::vector<std::uint8_t> expected_output(buf_length, 0);

std::size_t output_length = ::_olm_decode_base64(input, input_length, output.data());
REQUIRE_EQ(std::size_t(-1), output_length);
CHECK_EQ(output, expected_output);
}

