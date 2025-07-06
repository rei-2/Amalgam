#include "olm/crypto.h"
#include "olm/olm.h"
#include "olm/pk.h"

#include "testing.hh"
#include "utils.hh"

#include <iostream>
#include <vector>


/* Encryption Test Case 1 */

TEST_CASE("Public Key Encryption/Decryption Test Case 1") {

std::vector<std::uint8_t> decryption_buffer(olm_pk_decryption_size());
OlmPkDecryption *decryption = olm_pk_decryption(decryption_buffer.data());

std::uint8_t alice_private[32] = {
    0x77, 0x07, 0x6D, 0x0A, 0x73, 0x18, 0xA5, 0x7D,
    0x3C, 0x16, 0xC1, 0x72, 0x51, 0xB2, 0x66, 0x45,
    0xDF, 0x4C, 0x2F, 0x87, 0xEB, 0xC0, 0x99, 0x2A,
    0xB1, 0x77, 0xFB, 0xA5, 0x1D, 0xB9, 0x2C, 0x2A
};

const std::uint8_t *alice_public = (std::uint8_t *) "hSDwCYkwp1R0i33ctD73Wg2/Og0mOBr066SpjqqbTmo";

std::uint8_t bob_private[32] = {
    0x5D, 0xAB, 0x08, 0x7E, 0x62, 0x4A, 0x8A, 0x4B,
    0x79, 0xE1, 0x7F, 0x8B, 0x83, 0x80, 0x0E, 0xE6,
    0x6F, 0x3B, 0xB1, 0x29, 0x26, 0x18, 0xB6, 0xFD,
    0x1C, 0x2F, 0x8B, 0x27, 0xFF, 0x88, 0xE0, 0xEB
};

const std::uint8_t *bob_public = (std::uint8_t *) "3p7bfXt9wbTTW2HC7OQ1Nz+DQ8hbeGdNrfx+FG+IK08";

std::vector<std::uint8_t> pubkey(::olm_pk_key_length());

olm_pk_key_from_private(
    decryption,
    pubkey.data(), pubkey.size(),
    alice_private, sizeof(alice_private)
);

CHECK_EQ_SIZE(alice_public, (const std::uint8_t*)pubkey.data(), olm_pk_key_length());

uint8_t *alice_private_back_out = (uint8_t *)malloc(olm_pk_private_key_length());
olm_pk_get_private_key(decryption, alice_private_back_out, olm_pk_private_key_length());
CHECK_EQ_SIZE(alice_private, alice_private_back_out, olm_pk_private_key_length());
free(alice_private_back_out);

std::vector<std::uint8_t> encryption_buffer(olm_pk_encryption_size());
OlmPkEncryption *encryption = olm_pk_encryption(encryption_buffer.data());

olm_pk_encryption_set_recipient_key(encryption, pubkey.data(), pubkey.size());

const size_t plaintext_length = 14;
const std::uint8_t *plaintext = (std::uint8_t *) "This is a test";

size_t ciphertext_length = olm_pk_ciphertext_length(encryption, plaintext_length);
std::uint8_t *ciphertext_buffer = (std::uint8_t *) malloc(ciphertext_length);

std::vector<std::uint8_t> output_buffer(olm_pk_mac_length(encryption));
std::vector<std::uint8_t> ephemeral_key(olm_pk_key_length());

olm_pk_encrypt(
    encryption,
    plaintext, plaintext_length,
    ciphertext_buffer, ciphertext_length,
    output_buffer.data(), output_buffer.size(),
    ephemeral_key.data(), ephemeral_key.size(),
    bob_private, sizeof(bob_private)
);

CHECK_EQ_SIZE(bob_public, (const std::uint8_t*)ephemeral_key.data(), olm_pk_key_length());

size_t max_plaintext_length = olm_pk_max_plaintext_length(decryption, ciphertext_length);
std::uint8_t *plaintext_buffer = (std::uint8_t *) malloc(max_plaintext_length);

olm_pk_decrypt(
    decryption,
    ephemeral_key.data(), ephemeral_key.size(),
    output_buffer.data(), output_buffer.size(),
    ciphertext_buffer, ciphertext_length,
    plaintext_buffer, max_plaintext_length
);

CHECK_EQ_SIZE(plaintext, (const std::uint8_t*)plaintext_buffer, plaintext_length);

free(ciphertext_buffer);
free(plaintext_buffer);

}

/* Encryption Test Case 1 */

TEST_CASE("Public Key Decryption pickling") {

std::vector<std::uint8_t> decryption_buffer(olm_pk_decryption_size());
OlmPkDecryption *decryption = olm_pk_decryption(decryption_buffer.data());

std::uint8_t alice_private[32] = {
    0x77, 0x07, 0x6D, 0x0A, 0x73, 0x18, 0xA5, 0x7D,
    0x3C, 0x16, 0xC1, 0x72, 0x51, 0xB2, 0x66, 0x45,
    0xDF, 0x4C, 0x2F, 0x87, 0xEB, 0xC0, 0x99, 0x2A,
    0xB1, 0x77, 0xFB, 0xA5, 0x1D, 0xB9, 0x2C, 0x2A
};

const std::uint8_t *alice_public = (std::uint8_t *) "hSDwCYkwp1R0i33ctD73Wg2/Og0mOBr066SpjqqbTmoK";

std::vector<std::uint8_t> pubkey(olm_pk_key_length());

olm_pk_key_from_private(
    decryption,
    pubkey.data(), pubkey.size(),
    alice_private, sizeof(alice_private)
);

const uint8_t *PICKLE_KEY=(uint8_t *)"secret_key";
std::vector<std::uint8_t> pickle_buffer(olm_pickle_pk_decryption_length(decryption));
const uint8_t *expected_pickle = (uint8_t *) "qx37WTQrjZLz5tId/uBX9B3/okqAbV1ofl9UnHKno1eipByCpXleAAlAZoJgYnCDOQZDQWzo3luTSfkF9pU1mOILCbbouubs6TVeDyPfgGD9i86J8irHjA";

olm_pickle_pk_decryption(
    decryption,
    PICKLE_KEY, strlen((char *)PICKLE_KEY),
    pickle_buffer.data(), pickle_buffer.size()
);
CHECK_EQ_SIZE(expected_pickle, (const std::uint8_t*)pickle_buffer.data(), olm_pickle_pk_decryption_length(decryption));

olm_clear_pk_decryption(decryption);

memset(pubkey.data(), 0, olm_pk_key_length());

olm_unpickle_pk_decryption(
    decryption,
    PICKLE_KEY, strlen((char *)PICKLE_KEY),
    pickle_buffer.data(), pickle_buffer.size(),
    pubkey.data(), pubkey.size()
);

CHECK_EQ_SIZE(alice_public, (const std::uint8_t*)pubkey.data(), olm_pk_key_length());

/* Deliberately corrupt the pickled session by supplying a junk suffix and
 * ensure this is caught as an error. */
const size_t junk_length = 1;
std::size_t pickle_length = olm_pickle_pk_decryption_length(decryption);
std::vector<std::uint8_t> junk_pickle(pickle_length + _olm_enc_output_length(junk_length));

olm_pickle_pk_decryption(
    decryption,
    PICKLE_KEY, strlen((char *)PICKLE_KEY),
    junk_pickle.data(), pickle_length
);

const size_t junk_pickle_length = add_junk_suffix_to_pickle(
    PICKLE_KEY, strlen((char *)PICKLE_KEY),
    junk_pickle.data(),
    pickle_length,
    junk_length);

CHECK_EQ(std::size_t(-1),
    olm_unpickle_pk_decryption(
        decryption,
        PICKLE_KEY, strlen((char *)PICKLE_KEY),
        junk_pickle.data(), junk_pickle_length,
        pubkey.data(), pubkey.size()
    ));
CHECK_EQ(OLM_PICKLE_EXTRA_DATA, olm_pk_decryption_last_error_code(decryption));
/***/

char *ciphertext = strdup("ntk49j/KozVFtSqJXhCejg");
const char *mac = "zpzU6BkZcNI";
const char *ephemeral_key = "3p7bfXt9wbTTW2HC7OQ1Nz+DQ8hbeGdNrfx+FG+IK08";

size_t max_plaintext_length = olm_pk_max_plaintext_length(decryption, strlen(ciphertext));
std::uint8_t *plaintext_buffer = (std::uint8_t *) malloc(max_plaintext_length);

olm_pk_decrypt(
    decryption,
    ephemeral_key, strlen(ephemeral_key),
    mac, strlen(mac),
    ciphertext, strlen(ciphertext),
    plaintext_buffer, max_plaintext_length
);

const std::uint8_t *plaintext = (std::uint8_t *) "This is a test";

CHECK_EQ_SIZE(plaintext, (const std::uint8_t*)plaintext_buffer, strlen((const char *)plaintext));

free(ciphertext);
free(plaintext_buffer);

}

/* Signing Test Case 1 */

TEST_CASE("Public Key Signing") {

std::vector<std::uint8_t> signing_buffer(olm_pk_signing_size());
OlmPkSigning *signing = olm_pk_signing(signing_buffer.data());

std::uint8_t seed[32] = {
    0x77, 0x07, 0x6D, 0x0A, 0x73, 0x18, 0xA5, 0x7D,
    0x3C, 0x16, 0xC1, 0x72, 0x51, 0xB2, 0x66, 0x45,
    0xDF, 0x4C, 0x2F, 0x87, 0xEB, 0xC0, 0x99, 0x2A,
    0xB1, 0x77, 0xFB, 0xA5, 0x1D, 0xB9, 0x2C, 0x2A
};

//const std::uint8_t *pub_key = (std::uint8_t *) "hSDwCYkwp1R0i33ctD73Wg2/Og0mOBr066SpjqqbTmoK";

std::vector<char> pubkey(olm_pk_signing_public_key_length() + 1);

olm_pk_signing_key_from_seed(
    signing,
    pubkey.data(), pubkey.size() - 1,
    seed, sizeof(seed)
);

char *message = strdup("We hold these truths to be self-evident, that all men are created equal, that they are endowed by their Creator with certain unalienable Rights, that among these are Life, Liberty and the pursuit of Happiness.");

std::uint8_t *sig_buffer = (std::uint8_t *) malloc(olm_pk_signature_length() + 1);

olm_pk_sign(
    signing,
    (const uint8_t *)message, strlen(message),
    sig_buffer, olm_pk_signature_length()
);

void * utility_buffer = malloc(::olm_utility_size());
::OlmUtility * utility = ::olm_utility(utility_buffer);

size_t result;

result = ::olm_ed25519_verify(
    utility,
    pubkey.data(), olm_pk_signing_public_key_length(),
    message, strlen(message),
    sig_buffer, olm_pk_signature_length()
);

CHECK_EQ((size_t)0, result);

sig_buffer[5] = 'm';

result = ::olm_ed25519_verify(
    utility,
    pubkey.data(), olm_pk_signing_public_key_length(),
    message, strlen(message),
    sig_buffer, olm_pk_signature_length()
);

CHECK_EQ((size_t)-1, result);

olm_clear_utility(utility);
free(utility_buffer);

free(message);
free(sig_buffer);

olm_clear_pk_signing(signing);

}
