#include "olm/olm.h"
#include "testing.hh"

#include <cstddef>
#include <cstdint>
#include <cstring>

struct MockRandom {
    MockRandom(std::uint8_t tag, std::uint8_t offset = 0)
        : tag(tag), current(offset) {}
    void operator()(
        void * buf, std::size_t length
    ) {
        std::uint8_t * bytes = (std::uint8_t *) buf;
        while (length > 32) {
            bytes[0] = tag;
            std::memset(bytes + 1, current, 31);
            length -= 32;
            bytes += 32;
            current += 1;
        }
        if (length) {
            bytes[0] = tag;
            std::memset(bytes + 1, current, length - 1);
            current += 1;
        }
    }
    std::uint8_t tag;
    std::uint8_t current;
};

std::uint8_t * check_malloc(std::size_t size) {
    if (size == std::size_t(-1)) {
        CHECK_NE(std::size_t(-1), size);
    }
    return (std::uint8_t *)::malloc(size);
}



 /** Signing Test */
TEST_CASE("Signing test") {

MockRandom mock_random_a('A', 0x00);

void * account_buffer = check_malloc(::olm_account_size());
::OlmAccount * account = ::olm_account(account_buffer);

std::size_t random_size = ::olm_create_account_random_length(account);
void * random = check_malloc(random_size);
mock_random_a(random, random_size);
::olm_create_account(account, random, random_size);
::free(random);

std::size_t message_size = 12;
void * message = check_malloc(message_size);
::memcpy(message, "Hello, World", message_size);

std::size_t signature_size = ::olm_account_signature_length(account);
void * signature = check_malloc(signature_size);
CHECK_NE(std::size_t(-1), ::olm_account_sign(
    account, message, message_size, signature, signature_size
));

std::size_t id_keys_size = ::olm_account_identity_keys_length(account);
std::uint8_t * id_keys = (std::uint8_t *) check_malloc(id_keys_size);
CHECK_NE(std::size_t(-1), ::olm_account_identity_keys(
    account, id_keys, id_keys_size
));

olm_clear_account(account);
free(account_buffer);

void * utility_buffer = check_malloc(::olm_utility_size());
::OlmUtility * utility = ::olm_utility(utility_buffer);

CHECK_NE(std::size_t(-1), ::olm_ed25519_verify(
    utility, id_keys + 71, 43, message, message_size, signature, signature_size
));

olm_clear_utility(utility);
free(utility_buffer);

free(id_keys);
free(signature);
free(message);

}

