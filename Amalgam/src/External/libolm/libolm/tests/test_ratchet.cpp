/* Copyright 2015 OpenMarket Ltd
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "olm/ratchet.hh"
#include "olm/cipher.h"
#include "testing.hh"

#include <vector>

std::uint8_t root_info[] = "Olm";
std::uint8_t ratchet_info[] = "OlmRatchet";
std::uint8_t message_info[] = "OlmMessageKeys";

olm::KdfInfo kdf_info = {
    root_info, sizeof(root_info) - 1,
    ratchet_info, sizeof(ratchet_info) - 1
};

_olm_cipher_aes_sha_256 cipher0 = OLM_CIPHER_INIT_AES_SHA_256(message_info);
_olm_cipher *cipher = OLM_CIPHER_BASE(&cipher0);

std::uint8_t random_bytes[] = "0123456789ABDEF0123456789ABCDEF";
_olm_curve25519_key_pair alice_key = [] {
_olm_curve25519_key_pair tmp_key;
_olm_crypto_curve25519_generate_key(random_bytes, &tmp_key);
return tmp_key;
}();

std::uint8_t shared_secret[] = "A secret";

/* Send/Receive test case */
TEST_CASE("Olm Send/Receive") {

olm::Ratchet alice(kdf_info, cipher);
olm::Ratchet bob(kdf_info, cipher);

alice.initialise_as_alice(shared_secret, sizeof(shared_secret) - 1, alice_key);
bob.initialise_as_bob(shared_secret, sizeof(shared_secret) - 1, alice_key.public_key);

std::uint8_t plaintext[] = "Message";
std::size_t plaintext_length = sizeof(plaintext) - 1;

std::size_t message_length, random_length, output_length;
std::size_t encrypt_length, decrypt_length;
{
    /* Alice sends Bob a message */
    message_length = alice.encrypt_output_length(plaintext_length);
    random_length = alice.encrypt_random_length();
    CHECK_EQ(std::size_t(0), random_length);

    std::vector<std::uint8_t> message(message_length);

    encrypt_length = alice.encrypt(
        plaintext, plaintext_length,
        NULL, 0,
        message.data(), message_length
    );
    CHECK_EQ(message_length, encrypt_length);

    output_length = bob.decrypt_max_plaintext_length(message.data(), message_length);
    std::vector<std::uint8_t> output(output_length);
    decrypt_length = bob.decrypt(
        message.data(), message_length,
        output.data(), output_length
    );
    CHECK_EQ(plaintext_length, decrypt_length);
    CHECK_EQ_SIZE(plaintext, output.data(), decrypt_length);
}


{
    /* Bob sends Alice a message */
    message_length = bob.encrypt_output_length(plaintext_length);
    random_length = bob.encrypt_random_length();
    CHECK_EQ(std::size_t(32), random_length);

    std::vector<std::uint8_t> message(message_length);
    std::uint8_t random[] = "This is a random 32 byte string.";

    encrypt_length = bob.encrypt(
        plaintext, plaintext_length,
        random, 32,
        message.data(), message_length
    );
    CHECK_EQ(message_length, encrypt_length);

    output_length = alice.decrypt_max_plaintext_length(message.data(), message_length);
    std::vector<std::uint8_t> output(output_length);
    decrypt_length = alice.decrypt(
        message.data(), message_length,
        output.data(), output_length
    );
    CHECK_EQ(plaintext_length, decrypt_length);
    CHECK_EQ_SIZE(plaintext, output.data(), decrypt_length);
}

} /* Send/receive message test case */

/* Out of order test case */

TEST_CASE("Olm Out of Order") {

olm::Ratchet alice(kdf_info, cipher);
olm::Ratchet bob(kdf_info, cipher);

alice.initialise_as_alice(shared_secret, sizeof(shared_secret) - 1, alice_key);
bob.initialise_as_bob(shared_secret, sizeof(shared_secret) - 1, alice_key.public_key);

std::uint8_t plaintext_1[] = "First Message";
std::size_t plaintext_1_length = sizeof(plaintext_1) - 1;

std::uint8_t plaintext_2[] = "Second Messsage. A bit longer than the first.";
std::size_t plaintext_2_length = sizeof(plaintext_2) - 1;

std::size_t message_1_length, message_2_length, random_length, output_length;
std::size_t encrypt_length, decrypt_length;

{
    /* Alice sends Bob two messages and they arrive out of order */
    message_1_length = alice.encrypt_output_length(plaintext_1_length);
    random_length = alice.encrypt_random_length();
    CHECK_EQ(std::size_t(0), random_length);

    std::vector<std::uint8_t> message_1(message_1_length);
    std::uint8_t random[] = "This is a random 32 byte string.";
    encrypt_length = alice.encrypt(
        plaintext_1, plaintext_1_length,
        random, 32,
        message_1.data(), message_1_length
    );
    CHECK_EQ(message_1_length, encrypt_length);

    message_2_length = alice.encrypt_output_length(plaintext_2_length);
    random_length = alice.encrypt_random_length();
    CHECK_EQ(std::size_t(0), random_length);

    std::vector<std::uint8_t> message_2(message_2_length);
    encrypt_length = alice.encrypt(
        plaintext_2, plaintext_2_length,
        NULL, 0,
        message_2.data(), message_2_length
    );
    CHECK_EQ(message_2_length, encrypt_length);

    output_length = bob.decrypt_max_plaintext_length(
        message_2.data(), message_2_length
    );
    std::vector<std::uint8_t> output_1(output_length);
    decrypt_length = bob.decrypt(
        message_2.data(), message_2_length,
        output_1.data(), output_length
    );
    CHECK_EQ(plaintext_2_length, decrypt_length);
    CHECK_EQ_SIZE(plaintext_2, output_1.data(), decrypt_length);

    output_length = bob.decrypt_max_plaintext_length(
        message_1.data(), message_1_length
    );
    std::vector<std::uint8_t> output_2(output_length);
    decrypt_length = bob.decrypt(
        message_1.data(), message_1_length,
        output_2.data(), output_length
    );

    CHECK_EQ(plaintext_1_length, decrypt_length);
    CHECK_EQ_SIZE(plaintext_1, output_2.data(), decrypt_length);
}

} /* Out of order test case */

/* More messages */

TEST_CASE("Olm More Messages") {

olm::Ratchet alice(kdf_info, cipher);
olm::Ratchet bob(kdf_info, cipher);

alice.initialise_as_alice(shared_secret, sizeof(shared_secret) - 1, alice_key);
bob.initialise_as_bob(shared_secret, sizeof(shared_secret) - 1, alice_key.public_key);

std::uint8_t plaintext[] = "These 15 bytes";
CHECK_EQ(std::size_t(15), sizeof(plaintext));
std::uint8_t random[] = "This is a random 32 byte string";

for (unsigned i = 0; i < 8; ++i) {
{
    std::vector<std::uint8_t> msg(alice.encrypt_output_length(sizeof(plaintext)));
    alice.encrypt(
        plaintext, 15, random, 32, msg.data(), msg.size()
    );
    std::vector<std::uint8_t> output(bob.decrypt_max_plaintext_length(msg.data(), msg.size()));
    CHECK_EQ(
        std::size_t(15), bob.decrypt(msg.data(), msg.size(), output.data(), output.size())
    );
}
random[31]++;
{
    std::vector<std::uint8_t> msg(bob.encrypt_output_length(sizeof(plaintext)));
    bob.encrypt(
        plaintext, 15, random, 32, msg.data(), msg.size()
    );
    std::vector<std::uint8_t> output(alice.decrypt_max_plaintext_length(msg.data(), msg.size()));
    CHECK_EQ(
        std::size_t(15), alice.decrypt(msg.data(), msg.size(), output.data(), output.size())
    );
}
random[31]++;
}

}

