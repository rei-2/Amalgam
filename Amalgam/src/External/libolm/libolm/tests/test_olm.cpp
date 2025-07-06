#include "olm/olm.h"

#include "testing.hh"
#include "utils.hh"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>

struct MockRandom {
    MockRandom(std::uint8_t tag, std::uint8_t offset = 0)
        : tag(tag), current(offset) {}
    void operator()(
        std::uint8_t * bytes, std::size_t length
    ) {
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


 /** Pickle account test */

TEST_CASE("Pickle account test") {
MockRandom mock_random('P');


std::vector<std::uint8_t> account_buffer(::olm_account_size());
::OlmAccount *account = ::olm_account(account_buffer.data());
std::vector<std::uint8_t> random(::olm_create_account_random_length(account));
mock_random(random.data(), random.size());
::olm_create_account(account, random.data(), random.size());
std::vector<std::uint8_t> ot_random(::olm_account_generate_one_time_keys_random_length(
    account, 42
                                        ));
mock_random(ot_random.data(), ot_random.size());
::olm_account_generate_one_time_keys(account, 42, ot_random.data(), ot_random.size());

std::size_t pickle_length = ::olm_pickle_account_length(account);
std::vector<std::uint8_t> pickle1(pickle_length);
std::size_t res = ::olm_pickle_account(account, "secret_key", 10, pickle1.data(), pickle_length);
CHECK_EQ(pickle_length, res);

std::vector<std::uint8_t> pickle2(pickle1);

std::vector<std::uint8_t> account_buffer2(::olm_account_size());
::OlmAccount *account2 = ::olm_account(account_buffer2.data());
CHECK_NE(std::size_t(-1), ::olm_unpickle_account(
    account2, "secret_key", 10, pickle2.data(), pickle_length
));
CHECK_EQ(pickle_length, ::olm_pickle_account_length(account2));
res = ::olm_pickle_account(account2, "secret_key", 10, pickle2.data(), pickle_length);
CHECK_EQ(pickle_length, res);

CHECK_EQ_SIZE(pickle1.data(), pickle2.data(), pickle_length);

/* Deliberately corrupt the pickled account by supplying a junk suffix and
 * ensure this is caught as an error. */
const size_t junk_length = 1;
std::vector<std::uint8_t> junk_pickle(pickle_length + _olm_enc_output_length(junk_length));

res = ::olm_pickle_account(
    account, "secret_key", 10, junk_pickle.data(), pickle_length);
CHECK_EQ(pickle_length, res);

const size_t junk_pickle_length = add_junk_suffix_to_pickle(
    "secret_key", 10, junk_pickle.data(), pickle_length, junk_length);

CHECK_EQ(std::size_t(-1),
    ::olm_unpickle_account(account, "secret_key", 10,
        junk_pickle.data(), junk_pickle_length));
CHECK_EQ(OLM_PICKLE_EXTRA_DATA, olm_account_last_error_code(account));
}


    TEST_CASE("Old account unpickle test") {

    // this uses the old pickle format, which did not use enough space
    // for the Ed25519 key. We should reject it.
    std::uint8_t pickle[] =
        "x3h9er86ygvq56pM1yesdAxZou4ResPQC9Rszk/fhEL9JY/umtZ2N/foL/SUgVXS"
        "v0IxHHZTafYjDdzJU9xr8dQeBoOTGfV9E/lCqDGBnIlu7SZndqjEKXtzGyQr4sP4"
        "K/A/8TOu9iK2hDFszy6xETiousHnHgh2ZGbRUh4pQx+YMm8ZdNZeRnwFGLnrWyf9"
        "O5TmXua1FcU";

    std::vector<std::uint8_t> account_buffer(::olm_account_size());
    ::OlmAccount *account = ::olm_account(account_buffer.data());
    CHECK_EQ(
        std::size_t(-1),
        ::olm_unpickle_account(
            account, "", 0, pickle, sizeof(pickle)-1
        )
    );
    CHECK_EQ(
        std::string("BAD_LEGACY_ACCOUNT_PICKLE"),
        std::string(::olm_account_last_error(account))
    );
}


/** Pickle session test */

TEST_CASE("Pickle session test") {
MockRandom mock_random('P');

std::vector<std::uint8_t> account_buffer(::olm_account_size());
::OlmAccount *account = ::olm_account(account_buffer.data());
std::vector<std::uint8_t> random(::olm_create_account_random_length(account));
mock_random(random.data(), random.size());
::olm_create_account(account, random.data(), random.size());

std::vector<std::uint8_t> session_buffer(::olm_session_size());
::OlmSession *session = ::olm_session(session_buffer.data());
std::uint8_t identity_key[32];
std::uint8_t one_time_key[32];
mock_random(identity_key, sizeof(identity_key));
mock_random(one_time_key, sizeof(one_time_key));
std::vector<std::uint8_t> random2(::olm_create_outbound_session_random_length(session));
mock_random(random2.data(), random2.size());

::olm_create_outbound_session(
    session, account,
    identity_key, sizeof(identity_key),
    one_time_key, sizeof(one_time_key),
    random2.data(), random2.size()
);


std::size_t pickle_length = ::olm_pickle_session_length(session);
std::vector<std::uint8_t> pickle1(pickle_length);
std::size_t res = ::olm_pickle_session(session, "secret_key", 10, pickle1.data(), pickle_length);
CHECK_EQ(pickle_length, res);

std::vector<std::uint8_t> pickle2(pickle1);

std::vector<std::uint8_t> session_buffer2(::olm_session_size());
::OlmSession *session2 = ::olm_session(session_buffer2.data());
CHECK_NE(std::size_t(-1), ::olm_unpickle_session(
    session2, "secret_key", 10, pickle2.data(), pickle_length
));
CHECK_EQ(pickle_length, ::olm_pickle_session_length(session2));
res = ::olm_pickle_session(session2, "secret_key", 10, pickle2.data(), pickle_length);
CHECK_EQ(pickle_length, res);

CHECK_EQ_SIZE(pickle1.data(), pickle2.data(), pickle_length);

/* Deliberately corrupt the pickled session by supplying a junk suffix and
 * ensure this is caught as an error. */
const size_t junk_length = 1;
std::vector<std::uint8_t> junk_pickle(pickle_length + _olm_enc_output_length(junk_length));

res = ::olm_pickle_session(
    session, "secret_key", 10, junk_pickle.data(), pickle_length);
CHECK_EQ(pickle_length, res);

const size_t junk_pickle_length = add_junk_suffix_to_pickle(
    "secret_key", 10, junk_pickle.data(), pickle_length, junk_length);

CHECK_EQ(std::size_t(-1),
    ::olm_unpickle_session(session, "secret_key", 10,
        junk_pickle.data(), junk_pickle_length));
CHECK_EQ(OLM_PICKLE_EXTRA_DATA, olm_session_last_error_code(session));
}

/** Loopback test */

TEST_CASE("Loopback test") {
MockRandom mock_random_a('A', 0x00);
MockRandom mock_random_b('B', 0x80);

std::vector<std::uint8_t> a_account_buffer(::olm_account_size());
::OlmAccount *a_account = ::olm_account(a_account_buffer.data());
std::vector<std::uint8_t> a_random(::olm_create_account_random_length(a_account));
mock_random_a(a_random.data(), a_random.size());
::olm_create_account(a_account, a_random.data(), a_random.size());

std::vector<std::uint8_t> b_account_buffer(::olm_account_size());
::OlmAccount *b_account = ::olm_account(b_account_buffer.data());
std::vector<std::uint8_t> b_random(::olm_create_account_random_length(b_account));
mock_random_b(b_random.data(), b_random.size());
::olm_create_account(b_account, b_random.data(), b_random.size());
std::vector<std::uint8_t> o_random(::olm_account_generate_one_time_keys_random_length(
        b_account, 42
));
mock_random_b(o_random.data(), o_random.size());
::olm_account_generate_one_time_keys(b_account, 42, o_random.data(), o_random.size());

std::vector<std::uint8_t> a_id_keys(::olm_account_identity_keys_length(a_account));
::olm_account_identity_keys(a_account, a_id_keys.data(), a_id_keys.size());

std::vector<std::uint8_t> b_id_keys(::olm_account_identity_keys_length(b_account));
std::vector<std::uint8_t> b_ot_keys(::olm_account_one_time_keys_length(b_account));
::olm_account_identity_keys(b_account, b_id_keys.data(), b_id_keys.size());
::olm_account_one_time_keys(b_account, b_ot_keys.data(), b_ot_keys.size());

std::vector<std::uint8_t> a_session_buffer(::olm_session_size());
::OlmSession *a_session = ::olm_session(a_session_buffer.data());
std::vector<std::uint8_t> a_rand(::olm_create_outbound_session_random_length(a_session));
mock_random_a(a_rand.data(), a_rand.size());
CHECK_NE(std::size_t(-1), ::olm_create_outbound_session(
    a_session, a_account,
    b_id_keys.data() + 15, 43, // B's curve25519 identity key
    b_ot_keys.data() + 25, 43, // B's curve25519 one time key
    a_rand.data(), a_rand.size()
));

std::uint8_t plaintext[] = "Hello, World";
std::vector<std::uint8_t> message_1(::olm_encrypt_message_length(a_session, 12));
std::vector<std::uint8_t> a_message_random(::olm_encrypt_random_length(a_session));
mock_random_a(a_message_random.data(), a_message_random.size());
CHECK_EQ(std::size_t(0), ::olm_encrypt_message_type(a_session));
CHECK_NE(std::size_t(-1), ::olm_encrypt(
    a_session,
    plaintext, 12,
    a_message_random.data(), a_message_random.size(),
    message_1.data(), message_1.size()
));


std::vector<std::uint8_t> tmp_message_1(message_1);
std::vector<std::uint8_t> b_session_buffer(::olm_session_size());
::OlmSession *b_session = ::olm_session(b_session_buffer.data());
::olm_create_inbound_session(
    b_session, b_account, tmp_message_1.data(), message_1.size()
);

// Check that the inbound session matches the message it was created from.
std::memcpy(tmp_message_1.data(), message_1.data(), message_1.size());
CHECK_EQ(std::size_t(1), ::olm_matches_inbound_session(
    b_session,
    tmp_message_1.data(), message_1.size()
));

// Check that the inbound session matches the key this message is supposed
// to be from.
std::memcpy(tmp_message_1.data(), message_1.data(), message_1.size());
CHECK_EQ(std::size_t(1), ::olm_matches_inbound_session_from(
    b_session,
    a_id_keys.data() + 15, 43, // A's curve125519 identity key.
    tmp_message_1.data(), message_1.size()
));

// Check that the inbound session isn't from a different user.
std::memcpy(tmp_message_1.data(), message_1.data(), message_1.size());
CHECK_EQ(std::size_t(0), ::olm_matches_inbound_session_from(
    b_session,
    b_id_keys.data() + 15, 43, // B's curve25519 identity key.
    tmp_message_1.data(), message_1.size()
));

// Check that we can decrypt the message.
std::memcpy(tmp_message_1.data(), message_1.data(), message_1.size());
std::vector<std::uint8_t> plaintext_1(::olm_decrypt_max_plaintext_length(
    b_session, 0, tmp_message_1.data(), message_1.size()
));
std::memcpy(tmp_message_1.data(), message_1.data(), message_1.size());
CHECK_EQ(std::size_t(12), ::olm_decrypt(
    b_session, 0,
    tmp_message_1.data(), message_1.size(),
    plaintext_1.data(), plaintext_1.size()
));

CHECK_EQ_SIZE(plaintext, plaintext_1.data(), 12);

std::vector<std::uint8_t> message_2(::olm_encrypt_message_length(b_session, 12));
std::vector<std::uint8_t> b_message_random(::olm_encrypt_random_length(b_session));
mock_random_b(b_message_random.data(), b_message_random.size());
CHECK_EQ(std::size_t(1), ::olm_encrypt_message_type(b_session));
CHECK_NE(std::size_t(-1), ::olm_encrypt(
    b_session,
    plaintext, 12,
    b_message_random.data(), b_message_random.size(),
    message_2.data(), message_2.size()
));

std::vector<std::uint8_t> tmp_message_2(message_2);
std::vector<std::uint8_t> plaintext_2(::olm_decrypt_max_plaintext_length(
    a_session, 1, tmp_message_2.data(), message_2.size()
));
std::memcpy(tmp_message_2.data(), message_2.data(), message_2.size());
CHECK_EQ(std::size_t(12), ::olm_decrypt(
    a_session, 1,
    tmp_message_2.data(), message_2.size(),
    plaintext_2.data(), plaintext_2.size()
));

CHECK_EQ_SIZE(plaintext, plaintext_2.data(), 12);

std::memcpy(tmp_message_2.data(), message_2.data(), message_2.size());
CHECK_EQ(std::size_t(-1), ::olm_decrypt(
    a_session, 1,
    tmp_message_2.data(), message_2.size(),
    plaintext_2.data(), plaintext_2.size()
));

std::vector<std::uint8_t> a_session_id(::olm_session_id_length(a_session));
CHECK_NE(std::size_t(-1), ::olm_session_id(
    a_session, a_session_id.data(), a_session_id.size()
));

std::vector<std::uint8_t> b_session_id(::olm_session_id_length(b_session));
CHECK_NE(std::size_t(-1), ::olm_session_id(
    b_session, b_session_id.data(), b_session_id.size()
));

CHECK_EQ(a_session_id.size(), b_session_id.size());
CHECK_EQ_SIZE(a_session_id.data(), b_session_id.data(), b_session_id.size());

}

/** More messages test */

TEST_CASE("More messages test") {
MockRandom mock_random_a('A', 0x00);
MockRandom mock_random_b('B', 0x80);

std::vector<std::uint8_t> a_account_buffer(::olm_account_size());
::OlmAccount *a_account = ::olm_account(a_account_buffer.data());
std::vector<std::uint8_t> a_random(::olm_create_account_random_length(a_account));
mock_random_a(a_random.data(), a_random.size());
::olm_create_account(a_account, a_random.data(), a_random.size());

std::vector<std::uint8_t> b_account_buffer(::olm_account_size());
::OlmAccount *b_account = ::olm_account(b_account_buffer.data());
std::vector<std::uint8_t> b_random(::olm_create_account_random_length(b_account));
mock_random_b(b_random.data(), b_random.size());
::olm_create_account(b_account, b_random.data(), b_random.size());
std::vector<std::uint8_t> o_random(::olm_account_generate_one_time_keys_random_length(
        b_account, 42
));
mock_random_b(o_random.data(), o_random.size());
::olm_account_generate_one_time_keys(b_account, 42, o_random.data(), o_random.size());

std::vector<std::uint8_t> b_id_keys(::olm_account_identity_keys_length(b_account));
std::vector<std::uint8_t> b_ot_keys(::olm_account_one_time_keys_length(b_account));
::olm_account_identity_keys(b_account, b_id_keys.data(), b_id_keys.size());
::olm_account_one_time_keys(b_account, b_ot_keys.data(), b_ot_keys.size());

std::vector<std::uint8_t> a_session_buffer(::olm_session_size());
::OlmSession *a_session = ::olm_session(a_session_buffer.data());
std::vector<std::uint8_t> a_rand(::olm_create_outbound_session_random_length(a_session));
mock_random_a(a_rand.data(), a_rand.size());
CHECK_NE(std::size_t(-1), ::olm_create_outbound_session(
    a_session, a_account,
    b_id_keys.data() + 15, 43,
    b_ot_keys.data() + 25, 43,
    a_rand.data(), a_rand.size()
));

std::uint8_t plaintext[] = "Hello, World";
std::vector<std::uint8_t> message_1(::olm_encrypt_message_length(a_session, 12));
std::vector<std::uint8_t> a_message_random(::olm_encrypt_random_length(a_session));
mock_random_a(a_message_random.data(), a_message_random.size());
CHECK_EQ(std::size_t(0), ::olm_encrypt_message_type(a_session));
CHECK_NE(std::size_t(-1), ::olm_encrypt(
    a_session,
    plaintext, 12,
    a_message_random.data(), a_message_random.size(),
    message_1.data(), message_1.size()
));

std::vector<std::uint8_t> tmp_message_1(message_1);
std::vector<std::uint8_t> b_session_buffer(::olm_session_size());
::OlmSession *b_session = ::olm_session(b_session_buffer.data());
::olm_create_inbound_session(
    b_session, b_account, tmp_message_1.data(), message_1.size()
);

std::memcpy(tmp_message_1.data(), message_1.data(), message_1.size());
std::vector<std::uint8_t> plaintext_1(::olm_decrypt_max_plaintext_length(
    b_session, 0, tmp_message_1.data(), message_1.size()
));
std::memcpy(tmp_message_1.data(), message_1.data(), message_1.size());
CHECK_EQ(std::size_t(12), ::olm_decrypt(
    b_session, 0,
    tmp_message_1.data(), message_1.size(),
    plaintext_1.data(), plaintext_1.size()
));

for (unsigned i = 0; i < 8; ++i) {
    {
    std::vector<std::uint8_t> msg_a(::olm_encrypt_message_length(a_session, 12));
    std::vector<std::uint8_t> rnd_a(::olm_encrypt_random_length(a_session));
    mock_random_a(rnd_a.data(), rnd_a.size());
    std::size_t type_a = ::olm_encrypt_message_type(a_session);
    CHECK_NE(std::size_t(-1), ::olm_encrypt(
        a_session, plaintext, 12, rnd_a.data(), rnd_a.size(), msg_a.data(), msg_a.size()
    ));

    std::vector<std::uint8_t> tmp_a(msg_a);
    std::vector<std::uint8_t> out_a(::olm_decrypt_max_plaintext_length(
        b_session, type_a, tmp_a.data(), tmp_a.size()
    ));
    std::memcpy(tmp_a.data(), msg_a.data(), sizeof(msg_a));
    CHECK_EQ(std::size_t(12), ::olm_decrypt(
        b_session, type_a, msg_a.data(), msg_a.size(), out_a.data(), out_a.size()
    ));
    }
    {
    std::vector<std::uint8_t> msg_b(::olm_encrypt_message_length(b_session, 12));
    std::vector<std::uint8_t> rnd_b(::olm_encrypt_random_length(b_session));
    mock_random_b(rnd_b.data(), rnd_b.size());
    std::size_t type_b = ::olm_encrypt_message_type(b_session);
    CHECK_NE(std::size_t(-1), ::olm_encrypt(
        b_session, plaintext, 12, rnd_b.data(), rnd_b.size(), msg_b.data(), msg_b.size()
    ));

    std::vector<std::uint8_t> tmp_b(msg_b);
    std::vector<std::uint8_t> out_b(::olm_decrypt_max_plaintext_length(
        a_session, type_b, tmp_b.data(), tmp_b.size()
    ));
    std::memcpy(tmp_b.data(), msg_b.data(), msg_b.size());
    CHECK_EQ(std::size_t(12), ::olm_decrypt(
        a_session, type_b, msg_b.data(), msg_b.size(), out_b.data(), out_b.size()
    ));
    }
}
}

TEST_CASE("Fallback key test") {
MockRandom mock_random_a('A', 0x00);
MockRandom mock_random_b('B', 0x80);

// create a and b accounts
std::vector<std::uint8_t> a_account_buffer(::olm_account_size());
::OlmAccount *a_account = ::olm_account(a_account_buffer.data());
std::vector<std::uint8_t> a_random(::olm_create_account_random_length(a_account));
mock_random_a(a_random.data(), a_random.size());
::olm_create_account(a_account, a_random.data(), a_random.size());

std::vector<std::uint8_t> b_account_buffer(::olm_account_size());
::OlmAccount *b_account = ::olm_account(b_account_buffer.data());
std::vector<std::uint8_t> b_random(::olm_create_account_random_length(b_account));
mock_random_b(b_random.data(), b_random.size());
::olm_create_account(b_account, b_random.data(), b_random.size());

std::vector<std::uint8_t> a_id_keys(::olm_account_identity_keys_length(a_account));
::olm_account_identity_keys(a_account, a_id_keys.data(), a_id_keys.size());

std::vector<std::uint8_t> b_id_keys(::olm_account_identity_keys_length(b_account));

// create and fetch a fallback key for b
std::vector<std::uint8_t> f_random(::olm_account_generate_fallback_key_random_length(
        b_account
));
mock_random_b(f_random.data(), f_random.size());
::olm_account_generate_fallback_key(b_account, f_random.data(), f_random.size());
std::vector<std::uint8_t> b_fb_key(::olm_account_unpublished_fallback_key_length(b_account));
::olm_account_identity_keys(b_account, b_id_keys.data(), b_id_keys.size());
::olm_account_unpublished_fallback_key(b_account, b_fb_key.data(), b_fb_key.size());

// start a new olm session and encrypt a message
std::vector<std::uint8_t> a_session1_buffer(::olm_session_size());
::OlmSession *a_session1 = ::olm_session(a_session1_buffer.data());
std::vector<std::uint8_t> a_rand(::olm_create_outbound_session_random_length(a_session1));
mock_random_a(a_rand.data(), a_rand.size());
CHECK_NE(std::size_t(-1), ::olm_create_outbound_session(
    a_session1, a_account,
    b_id_keys.data() + 15, 43, // B's curve25519 identity key
    b_fb_key.data() + 25, 43, // B's curve25519 one time key
    a_rand.data(), a_rand.size()
));

std::uint8_t plaintext[] = "Hello, World";
std::vector<std::uint8_t> message_1(::olm_encrypt_message_length(a_session1, 12));
std::vector<std::uint8_t> a_message_random(::olm_encrypt_random_length(a_session1));
mock_random_a(a_message_random.data(), a_message_random.size());
CHECK_EQ(std::size_t(0), ::olm_encrypt_message_type(a_session1));
CHECK_NE(std::size_t(-1), ::olm_encrypt(
    a_session1,
    plaintext, 12,
    a_message_random.data(), a_message_random.size(),
    message_1.data(), message_1.size()
));


std::vector<std::uint8_t> tmp_message_1(message_1);
std::vector<std::uint8_t> b_session1_buffer(::olm_session_size());
::OlmSession *b_session1 = ::olm_session(b_session1_buffer.data());
::olm_create_inbound_session(
    b_session1, b_account, tmp_message_1.data(), message_1.size()
);

// Check that the inbound session matches the message it was created from.
std::memcpy(tmp_message_1.data(), message_1.data(), message_1.size());
CHECK_EQ(std::size_t(1), ::olm_matches_inbound_session(
    b_session1,
    tmp_message_1.data(), message_1.size()
));

// Check that the inbound session matches the key this message is supposed
// to be from.
std::memcpy(tmp_message_1.data(), message_1.data(), message_1.size());
CHECK_EQ(std::size_t(1), ::olm_matches_inbound_session_from(
    b_session1,
    a_id_keys.data() + 15, 43, // A's curve125519 identity key.
    tmp_message_1.data(), message_1.size()
));

// Check that the inbound session isn't from a different user.
std::memcpy(tmp_message_1.data(), message_1.data(), message_1.size());
CHECK_EQ(std::size_t(0), ::olm_matches_inbound_session_from(
    b_session1,
    b_id_keys.data() + 15, 43, // B's curve25519 identity key.
    tmp_message_1.data(), message_1.size()
));

// Check that we can decrypt the message.
std::memcpy(tmp_message_1.data(), message_1.data(), message_1.size());
std::vector<std::uint8_t> plaintext_1(::olm_decrypt_max_plaintext_length(
    b_session1, 0, tmp_message_1.data(), message_1.size()
));
std::memcpy(tmp_message_1.data(), message_1.data(), message_1.size());
CHECK_EQ(std::size_t(12), ::olm_decrypt(
    b_session1, 0,
    tmp_message_1.data(), message_1.size(),
    plaintext_1.data(), plaintext_1.size()
));

CHECK_EQ_SIZE(plaintext, plaintext_1.data(), 12);

// create a new fallback key for B (the old fallback should still be usable)
mock_random_b(f_random.data(), f_random.size());
::olm_account_generate_fallback_key(b_account, f_random.data(), f_random.size());


// start another session and encrypt a message
std::vector<std::uint8_t> a_session2_buffer(::olm_session_size());
::OlmSession *a_session2 = ::olm_session(a_session2_buffer.data());
mock_random_a(a_rand.data(), a_rand.size());
CHECK_NE(std::size_t(-1), ::olm_create_outbound_session(
    a_session2, a_account,
    b_id_keys.data() + 15, 43, // B's curve25519 identity key
    b_fb_key.data() + 25, 43, // B's curve25519 one time key
    a_rand.data(), a_rand.size()
));
std::vector<std::uint8_t> message_2(::olm_encrypt_message_length(a_session2, 12));
mock_random_a(a_message_random.data(), a_message_random.size());
CHECK_EQ(std::size_t(0), ::olm_encrypt_message_type(a_session2));
CHECK_NE(std::size_t(-1), ::olm_encrypt(
    a_session2,
    plaintext, 12,
    a_message_random.data(), a_message_random.size(),
    message_2.data(), message_2.size()
));


std::vector<std::uint8_t> tmp_message_2(message_2);
std::vector<std::uint8_t> b_session2_buffer(::olm_session_size());
::OlmSession *b_session2 = ::olm_session(b_session2_buffer.data());
CHECK_NE(std::size_t(-1), ::olm_create_inbound_session(
    b_session2, b_account, tmp_message_2.data(), message_2.size()
));

// Check that the inbound session matches the message it was created from.
std::memcpy(tmp_message_2.data(), message_2.data(), message_2.size());
CHECK_EQ(std::size_t(1), ::olm_matches_inbound_session(
    b_session2,
    tmp_message_2.data(), message_2.size()
));

// Check that the inbound session matches the key this message is supposed
// to be from.
std::memcpy(tmp_message_2.data(), message_2.data(), message_2.size());
CHECK_EQ(std::size_t(1), ::olm_matches_inbound_session_from(
    b_session2,
    a_id_keys.data() + 15, 43, // A's curve125519 identity key.
    tmp_message_2.data(), message_2.size()
));

// Check that the inbound session isn't from a different user.
std::memcpy(tmp_message_2.data(), message_2.data(), message_2.size());
CHECK_EQ(std::size_t(0), ::olm_matches_inbound_session_from(
    b_session2,
    b_id_keys.data() + 15, 43, // B's curve25519 identity key.
    tmp_message_2.data(), message_2.size()
));

// Check that we can decrypt the message.
std::memcpy(tmp_message_2.data(), message_2.data(), message_2.size());
std::vector<std::uint8_t> plaintext_2(::olm_decrypt_max_plaintext_length(
    b_session2, 0, tmp_message_2.data(), message_2.size()
));
std::memcpy(tmp_message_2.data(), message_2.data(), message_2.size());
CHECK_EQ(std::size_t(12), ::olm_decrypt(
    b_session2, 0,
    tmp_message_2.data(), message_2.size(),
    plaintext_2.data(), plaintext_2.size()
));

CHECK_EQ_SIZE(plaintext, plaintext_2.data(), 12);

// forget the old fallback key -- creating a new session should fail
::olm_account_forget_old_fallback_key(b_account);

std::vector<std::uint8_t> a_session3_buffer(::olm_session_size());
::OlmSession *a_session3 = ::olm_session(a_session3_buffer.data());
mock_random_a(a_rand.data(), a_rand.size());
CHECK_NE(std::size_t(-1), ::olm_create_outbound_session(
    a_session3, a_account,
    b_id_keys.data() + 15, 43, // B's curve25519 identity key
    b_fb_key.data() + 25, 43, // B's curve25519 one time key
    a_rand.data(), a_rand.size()
));

std::vector<std::uint8_t> message_3(::olm_encrypt_message_length(a_session3, 12));
mock_random_a(a_message_random.data(), a_message_random.size());
CHECK_EQ(std::size_t(0), ::olm_encrypt_message_type(a_session3));
CHECK_NE(std::size_t(-1), ::olm_encrypt(
    a_session3,
    plaintext, 12,
    a_message_random.data(), a_message_random.size(),
    message_3.data(), message_3.size()
));


std::vector<std::uint8_t> tmp_message_3(message_3);
std::vector<std::uint8_t> b_session3_buffer(::olm_session_size());
::OlmSession *b_session3 = ::olm_session(b_session3_buffer.data());
CHECK_EQ(std::size_t(-1), ::olm_create_inbound_session(
    b_session3, b_account, tmp_message_3.data(), message_3.size()
));
CHECK_EQ(
    std::string("BAD_MESSAGE_KEY_ID"),
    std::string(::olm_session_last_error(b_session3))
);
}

TEST_CASE("Old account (v3) unpickle test") {
    std::uint8_t pickle[] =
        "0mSqVn3duHffbhaTbFgW+4JPlcRoqT7z0x4mQ72N+g+eSAk5sgcWSoDzKpMazgcB"
        "46ItEpChthVHTGRA6PD3dly0dUs4ji7VtWTa+1tUv1UbxP92uYf1Ae3fomX0yAoH"
        "OjSrz1+RmuXr+At8jsmsf260sKvhB6LnI3qYsrw6AAtpgk5d5xZd66sLxvvYUuai"
        "+SmmcmT0bHosLTuDiiB9amBvPKkUKtKZmaEAl5ULrgnJygp1/FnwzVfSrw6PBSX6"
        "ZaUEZHZGX1iI6/WjbHqlTQeOQjtaSsPaL5XXpteS9dFsuaANAj+8ks7Ut2Hwg/JP"
        "Ih/ERYBwiMh9Mt3zSAG0NkvgUkcdipKxoSNZ6t+TkqZrN6jG6VCbx+4YpJO24iJb"
        "ShZy8n79aePIgIsxX94ycsTq1ic38sCRSkWGVbCSRkPloHW7ZssLHA";

    std::uint8_t expected_fallback[] =
        "{\"curve25519\":{\"AAAAAQ\":\"dr98y6VOWt6lJaQgFVZeWY2ky76mga9MEMbdItJTdng\"}}";
    std::uint8_t expected_unpublished_fallback[] =
        "{\"curve25519\":{}}";

    std::vector<std::uint8_t> account_buffer(::olm_account_size());
    ::OlmAccount *account = ::olm_account(account_buffer.data());
    CHECK_NE(
        std::size_t(-1),
        ::olm_unpickle_account(
            account, "", 0, pickle, sizeof(pickle)-1
        )
    );

    std::vector<std::uint8_t> fallback(::olm_account_fallback_key_length(account));
    size_t len = ::olm_account_fallback_key(account, fallback.data(), fallback.size());
    CHECK_EQ_SIZE(expected_fallback, fallback.data(), len);
    len = ::olm_account_unpublished_fallback_key(account, fallback.data(), fallback.size());
    CHECK_EQ_SIZE(expected_unpublished_fallback, fallback.data(), len);
}
