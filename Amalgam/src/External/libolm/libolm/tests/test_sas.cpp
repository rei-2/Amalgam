#include "olm/sas.h"
#include "olm/crypto.h"
#include "olm/olm.h"

#include "testing.hh"

#include <iostream>
#include <vector>



/* Generate bytes */

TEST_CASE("SAS generate bytes") {

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

std::vector<std::uint8_t> alice_sas_buffer(olm_sas_size());
OlmSAS *alice_sas = olm_sas(alice_sas_buffer.data());
olm_create_sas(alice_sas, alice_private, sizeof(alice_private));
std::vector<std::uint8_t> bob_sas_buffer(olm_sas_size());
OlmSAS *bob_sas = olm_sas(bob_sas_buffer.data());
olm_create_sas(bob_sas, bob_private, sizeof(bob_private));

std::vector<std::uint8_t> pubkey(::olm_sas_pubkey_length(alice_sas));

olm_sas_get_pubkey(alice_sas, pubkey.data(), pubkey.size());

CHECK_EQ_SIZE(alice_public, (const uint8_t*)pubkey.data(), olm_sas_pubkey_length(alice_sas));

olm_sas_set_their_key(bob_sas, pubkey.data(), olm_sas_pubkey_length(bob_sas));

olm_sas_get_pubkey(bob_sas, pubkey.data(), pubkey.size());

CHECK_EQ_SIZE(bob_public, (const uint8_t*)pubkey.data(), olm_sas_pubkey_length(bob_sas));

olm_sas_set_their_key(alice_sas, pubkey.data(), olm_sas_pubkey_length(alice_sas));

std::uint8_t alice_bytes[6];
std::uint8_t bob_bytes[6];

olm_sas_generate_bytes(alice_sas, "SAS", 3, alice_bytes, 6);
olm_sas_generate_bytes(bob_sas, "SAS", 3, bob_bytes, 6);

CHECK_EQ_SIZE(alice_bytes, bob_bytes, 6);

}

/* Calculate MAC */

TEST_CASE("SAS calculate MAC") {

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

std::vector<std::uint8_t> alice_sas_buffer(olm_sas_size());
OlmSAS *alice_sas = olm_sas(alice_sas_buffer.data());
olm_create_sas(alice_sas, alice_private, sizeof(alice_private));
std::vector<std::uint8_t> bob_sas_buffer(olm_sas_size());
OlmSAS *bob_sas = olm_sas(bob_sas_buffer.data());
olm_create_sas(bob_sas, bob_private, sizeof(bob_private));

std::vector<std::uint8_t> pubkey(::olm_sas_pubkey_length(alice_sas));

olm_sas_get_pubkey(alice_sas, pubkey.data(), pubkey.size());

CHECK_EQ_SIZE(alice_public, (const uint8_t*)pubkey.data(), olm_sas_pubkey_length(alice_sas));

olm_sas_set_their_key(bob_sas, pubkey.data(), olm_sas_pubkey_length(bob_sas));

olm_sas_get_pubkey(bob_sas, pubkey.data(), pubkey.size());

CHECK_EQ_SIZE(bob_public, (const uint8_t*)pubkey.data(), olm_sas_pubkey_length(bob_sas));

olm_sas_set_their_key(alice_sas, pubkey.data(), olm_sas_pubkey_length(alice_sas));

std::vector<std::uint8_t> alice_mac(olm_sas_mac_length(alice_sas));
std::vector<std::uint8_t> bob_mac(olm_sas_mac_length(bob_sas));

olm_sas_calculate_mac(alice_sas, (void *) "Hello world!", 12, "MAC", 3, alice_mac.data(), olm_sas_mac_length(alice_sas));
olm_sas_calculate_mac(bob_sas, (void *) "Hello world!", 12, "MAC", 3, bob_mac.data(), olm_sas_mac_length(bob_sas));

CHECK_EQ_SIZE(alice_mac.data(), bob_mac.data(), olm_sas_mac_length(alice_sas));

}
