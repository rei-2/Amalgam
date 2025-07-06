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


/** More messages test */

TEST_CASE("More messages test") {
MockRandom mock_random_a('A', 0x00);
MockRandom mock_random_b('B', 0x80);

void * a_account_buffer = check_malloc(::olm_account_size());
::OlmAccount *a_account = ::olm_account(a_account_buffer);

std::size_t a_random_size = ::olm_create_account_random_length(a_account);
void * a_random = check_malloc(a_random_size);
mock_random_a(a_random, a_random_size);
::olm_create_account(a_account, a_random, a_random_size);
free(a_random);

void * b_account_buffer = check_malloc(::olm_account_size());
::OlmAccount *b_account = ::olm_account(b_account_buffer);

std::size_t b_random_size = ::olm_create_account_random_length(b_account);
void * b_random = check_malloc(b_random_size);
mock_random_b(b_random, b_random_size);
::olm_create_account(b_account, b_random, b_random_size);
free(b_random);

std::size_t o_random_size = ::olm_account_generate_one_time_keys_random_length(
    b_account, 42
);
void * o_random = check_malloc(o_random_size);
mock_random_b(o_random, o_random_size);
::olm_account_generate_one_time_keys(b_account, 42, o_random, o_random_size);
free(o_random);


std::size_t b_id_keys_size = ::olm_account_identity_keys_length(b_account);
std::size_t b_ot_keys_size = ::olm_account_one_time_keys_length(b_account);
std::uint8_t * b_id_keys = (std::uint8_t *) check_malloc(b_id_keys_size);
std::uint8_t * b_ot_keys = (std::uint8_t *) check_malloc(b_ot_keys_size);
::olm_account_identity_keys(b_account, b_id_keys, b_id_keys_size);
::olm_account_one_time_keys(b_account, b_ot_keys, b_ot_keys_size);

void * a_session_buffer = check_malloc(::olm_session_size());
::OlmSession *a_session = ::olm_session(a_session_buffer);

std::size_t a_rand_size = ::olm_create_outbound_session_random_length(a_session);
void * a_rand = check_malloc(a_rand_size);
mock_random_a(a_rand, a_rand_size);
CHECK_NE(std::size_t(-1), ::olm_create_outbound_session(
    a_session, a_account,
    b_id_keys + 15, 43,
    b_ot_keys + 25, 43,
    a_rand, a_rand_size
));
free(b_id_keys);
free(b_ot_keys);
free(a_rand);

void * plaintext = malloc(12);
::memcpy(plaintext, "Hello, World", 12);

std::size_t message_1_size = ::olm_encrypt_message_length(a_session, 12);
void * message_1 = check_malloc(message_1_size);
std::size_t a_message_random_size = ::olm_encrypt_random_length(a_session);
void * a_message_random = check_malloc(a_message_random_size);
mock_random_a(a_message_random, a_message_random_size);
CHECK_EQ(std::size_t(0), ::olm_encrypt_message_type(a_session));
CHECK_NE(std::size_t(-1), ::olm_encrypt(
    a_session,
    plaintext, 12,
    a_message_random, a_message_random_size,
    message_1, message_1_size
));
free(a_message_random);

void * tmp_message_1 = check_malloc(message_1_size);
std::memcpy(tmp_message_1, message_1, message_1_size);

void * b_session_buffer = check_malloc(olm_account_size());
::OlmSession *b_session = ::olm_session(b_session_buffer);
::olm_create_inbound_session(
    b_session, b_account, tmp_message_1, message_1_size
);

std::memcpy(tmp_message_1, message_1, message_1_size);

std::size_t plaintext_1_size = ::olm_decrypt_max_plaintext_length(
    b_session, 0, tmp_message_1, message_1_size
);
void * plaintext_1 = check_malloc(plaintext_1_size);
std::memcpy(tmp_message_1, message_1, message_1_size);
CHECK_EQ(std::size_t(12), ::olm_decrypt(
    b_session, 0,
    tmp_message_1, message_1_size,
    plaintext_1, plaintext_1_size
));
free(tmp_message_1);
free(plaintext_1);
free(message_1);

CHECK_NE(
    std::size_t(-1), ::olm_remove_one_time_keys(b_account, b_session)
);

for (unsigned i = 0; i < 8; ++i) {
    {
    std::size_t msg_a_size = ::olm_encrypt_message_length(a_session, 12);
    std::size_t rnd_a_size = ::olm_encrypt_random_length(a_session);
    void * msg_a = check_malloc(msg_a_size);
    void * rnd_a = check_malloc(rnd_a_size);
    mock_random_a(rnd_a, rnd_a_size);
    std::size_t type_a = ::olm_encrypt_message_type(a_session);
    CHECK_NE(std::size_t(-1), ::olm_encrypt(
        a_session, plaintext, 12, rnd_a, rnd_a_size, msg_a, msg_a_size
    ));
    free(rnd_a);

    void * tmp_a = check_malloc(msg_a_size);
    std::memcpy(tmp_a, msg_a, msg_a_size);
    std::size_t out_a_size = ::olm_decrypt_max_plaintext_length(
        b_session, type_a, tmp_a, msg_a_size
    );
    void * out_a = check_malloc(out_a_size);
    std::memcpy(tmp_a, msg_a, msg_a_size);
    CHECK_EQ(std::size_t(12), ::olm_decrypt(
        b_session, type_a, tmp_a, msg_a_size, out_a, out_a_size
    ));
    free(tmp_a);
    free(msg_a);
    free(out_a);
    }
    {
    std::size_t msg_b_size = ::olm_encrypt_message_length(b_session, 12);
    std::size_t rnd_b_size = ::olm_encrypt_random_length(b_session);
    void * msg_b = check_malloc(msg_b_size);
    void * rnd_b = check_malloc(rnd_b_size);
    mock_random_b(rnd_b, rnd_b_size);
    std::size_t type_b = ::olm_encrypt_message_type(b_session);
    CHECK_NE(std::size_t(-1), ::olm_encrypt(
            b_session, plaintext, 12, rnd_b, rnd_b_size, msg_b, msg_b_size
    ));
    free(rnd_b);

    void * tmp_b = check_malloc(msg_b_size);
    std::memcpy(tmp_b, msg_b, msg_b_size);
    std::size_t out_b_size = ::olm_decrypt_max_plaintext_length(
            a_session, type_b, tmp_b, msg_b_size
    );
    void * out_b = check_malloc(out_b_size);
    std::memcpy(tmp_b, msg_b, msg_b_size);
    CHECK_EQ(std::size_t(12), ::olm_decrypt(
            a_session, type_b, msg_b, msg_b_size, out_b, out_b_size
    ));
    free(tmp_b);
    free(msg_b);
    free(out_b);
    }
}
::olm_clear_account(a_account);
::olm_clear_account(b_account);
::olm_clear_session(a_session);
::olm_clear_session(b_session);

free(a_account_buffer);
free(b_account_buffer);
free(a_session_buffer);
free(b_session_buffer);
free(plaintext);

}

