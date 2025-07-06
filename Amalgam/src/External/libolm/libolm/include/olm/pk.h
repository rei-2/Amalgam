/* Copyright 2018, 2019 New Vector Ltd
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

#ifndef OLM_PK_H_
#define OLM_PK_H_

#include <stddef.h>
#include <stdint.h>

#include "olm/error.h"

#include "olm/olm_export.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct OlmPkEncryption OlmPkEncryption;

/* The size of an encryption object in bytes */
OLM_EXPORT size_t olm_pk_encryption_size(void);

/** Initialise an encryption object using the supplied memory
 *  The supplied memory must be at least olm_pk_encryption_size() bytes */
OLM_EXPORT OlmPkEncryption *olm_pk_encryption(
    void * memory
);

/** A null terminated string describing the most recent error to happen to an
 * encryption object */
OLM_EXPORT const char * olm_pk_encryption_last_error(
    const OlmPkEncryption * encryption
);

/** An error code describing the most recent error to happen to an encryption
 * object */
OLM_EXPORT enum OlmErrorCode olm_pk_encryption_last_error_code(
    const OlmPkEncryption * encryption
);

/** Clears the memory used to back this encryption object */
OLM_EXPORT size_t olm_clear_pk_encryption(
    OlmPkEncryption *encryption
);

/** Set the recipient's public key for encrypting to */
OLM_EXPORT size_t olm_pk_encryption_set_recipient_key(
    OlmPkEncryption *encryption,
    void const *public_key, size_t public_key_length
);

/** Get the length of the ciphertext that will correspond to a plaintext of the
 * given length. */
OLM_EXPORT size_t olm_pk_ciphertext_length(
    const OlmPkEncryption *encryption,
    size_t plaintext_length
);

/** Get the length of the message authentication code. */
OLM_EXPORT size_t olm_pk_mac_length(
    const OlmPkEncryption *encryption
);

/** Get the length of a public or ephemeral key */
OLM_EXPORT size_t olm_pk_key_length(void);

/** The number of random bytes needed to encrypt a message. */
OLM_EXPORT size_t olm_pk_encrypt_random_length(
    const OlmPkEncryption *encryption
);

/** Encrypt a plaintext for the recipient set using
 * olm_pk_encryption_set_recipient_key. Writes to the ciphertext, mac, and
 * ephemeral_key buffers, whose values should be sent to the recipient. mac is
 * a Message Authentication Code to ensure that the data is received and
 * decrypted properly. ephemeral_key is the public part of the ephemeral key
 * used (together with the recipient's key) to generate a symmetric encryption
 * key. Returns olm_error() on failure. If the ciphertext, mac, or
 * ephemeral_key buffers were too small then olm_pk_encryption_last_error()
 * will be "OUTPUT_BUFFER_TOO_SMALL". If there weren't enough random bytes then
 * olm_pk_encryption_last_error() will be "OLM_INPUT_BUFFER_TOO_SMALL". */
OLM_EXPORT size_t olm_pk_encrypt(
    OlmPkEncryption *encryption,
    void const * plaintext, size_t plaintext_length,
    void * ciphertext, size_t ciphertext_length,
    void * mac, size_t mac_length,
    void * ephemeral_key, size_t ephemeral_key_size,
    const void * random, size_t random_length
);

typedef struct OlmPkDecryption OlmPkDecryption;

/* The size of a decryption object in bytes */
OLM_EXPORT size_t olm_pk_decryption_size(void);

/** Initialise a decryption object using the supplied memory
 *  The supplied memory must be at least olm_pk_decryption_size() bytes */
OLM_EXPORT OlmPkDecryption *olm_pk_decryption(
    void * memory
);

/** A null terminated string describing the most recent error to happen to a
 * decription object */
OLM_EXPORT const char * olm_pk_decryption_last_error(
    const OlmPkDecryption * decryption
);

/** An error code describing the most recent error to happen to a decription
 * object */
OLM_EXPORT enum OlmErrorCode olm_pk_decryption_last_error_code(
    const OlmPkDecryption * decryption
);

/** Clears the memory used to back this decryption object */
OLM_EXPORT size_t olm_clear_pk_decryption(
    OlmPkDecryption *decryption
);

/** Get the number of bytes required to store an olm private key
 */
OLM_EXPORT size_t olm_pk_private_key_length(void);

/** DEPRECATED: Use olm_pk_private_key_length()
 */
OLM_EXPORT size_t olm_pk_generate_key_random_length(void);

/** Initialise the key from the private part of a key as returned by
 * olm_pk_get_private_key(). The associated public key will be written to the
 * pubkey buffer. Returns olm_error() on failure. If the pubkey buffer is too
 * small then olm_pk_decryption_last_error() will be "OUTPUT_BUFFER_TOO_SMALL".
 * If the private key was not long enough then olm_pk_decryption_last_error()
 * will be "OLM_INPUT_BUFFER_TOO_SMALL".
 *
 * Note that the pubkey is a base64 encoded string, but the private key is
 * an unencoded byte array
 */
OLM_EXPORT size_t olm_pk_key_from_private(
    OlmPkDecryption * decryption,
    void * pubkey, size_t pubkey_length,
    const void * privkey, size_t privkey_length
);

/** DEPRECATED: Use olm_pk_key_from_private
 */
OLM_EXPORT size_t olm_pk_generate_key(
    OlmPkDecryption * decryption,
    void * pubkey, size_t pubkey_length,
    const void * privkey, size_t privkey_length
);

/** Returns the number of bytes needed to store a decryption object. */
OLM_EXPORT size_t olm_pickle_pk_decryption_length(
    const OlmPkDecryption * decryption
);

/** Stores decryption object as a base64 string. Encrypts the object using the
 * supplied key. Returns the length of the pickled object on success.
 * Returns olm_error() on failure. If the pickle output buffer
 * is smaller than olm_pickle_pk_decryption_length() then
 * olm_pk_decryption_last_error() will be "OUTPUT_BUFFER_TOO_SMALL" */
OLM_EXPORT size_t olm_pickle_pk_decryption(
    OlmPkDecryption * decryption,
    void const * key, size_t key_length,
    void *pickled, size_t pickled_length
);

/** Loads a decryption object from a pickled base64 string. The associated
 * public key will be written to the pubkey buffer. Decrypts the object using
 * the supplied key. Returns olm_error() on failure. If the key doesn't
 * match the one used to encrypt the account then olm_pk_decryption_last_error()
 * will be "BAD_ACCOUNT_KEY". If the base64 couldn't be decoded then
 * olm_pk_decryption_last_error() will be "INVALID_BASE64". The input pickled
 * buffer is destroyed */
OLM_EXPORT size_t olm_unpickle_pk_decryption(
    OlmPkDecryption * decryption,
    void const * key, size_t key_length,
    void *pickled, size_t pickled_length,
    void *pubkey, size_t pubkey_length
);

/** Get the length of the plaintext that will correspond to a ciphertext of the
 * given length. */
OLM_EXPORT size_t olm_pk_max_plaintext_length(
    const OlmPkDecryption * decryption,
    size_t ciphertext_length
);

/** Decrypt a ciphertext. The input ciphertext buffer is destroyed. See the
 * olm_pk_encrypt function for descriptions of the ephemeral_key and mac
 * arguments. Returns the length of the plaintext on success. Returns
 * olm_error() on failure. If the plaintext buffer is too small then
 * olm_pk_encryption_last_error() will be "OUTPUT_BUFFER_TOO_SMALL". */
OLM_EXPORT size_t olm_pk_decrypt(
    OlmPkDecryption * decryption,
    void const * ephemeral_key, size_t ephemeral_key_length,
    void const * mac, size_t mac_length,
    void * ciphertext, size_t ciphertext_length,
    void * plaintext, size_t max_plaintext_length
);

/**
 * Get the private key for an OlmDecryption object as an unencoded byte array
 * private_key must be a pointer to a buffer of at least
 * olm_pk_private_key_length() bytes and this length must be passed in
 * private_key_length. If the given buffer is too small, returns olm_error()
 * and olm_pk_encryption_last_error() will be "OUTPUT_BUFFER_TOO_SMALL".
 * Returns the number of bytes written.
 */
OLM_EXPORT size_t olm_pk_get_private_key(
    OlmPkDecryption * decryption,
    void *private_key, size_t private_key_length
);

typedef struct OlmPkSigning OlmPkSigning;

/* The size of a signing object in bytes */
OLM_EXPORT size_t olm_pk_signing_size(void);

/** Initialise a signing object using the supplied memory
 *  The supplied memory must be at least olm_pk_signing_size() bytes */
OLM_EXPORT OlmPkSigning *olm_pk_signing(
    void * memory
);

/** A null terminated string describing the most recent error to happen to a
 * signing object */
OLM_EXPORT const char * olm_pk_signing_last_error(
    const OlmPkSigning * sign
);

/** A null terminated string describing the most recent error to happen to a
 * signing object */
OLM_EXPORT enum OlmErrorCode olm_pk_signing_last_error_code(
    const OlmPkSigning * sign
);

/** Clears the memory used to back this signing object */
OLM_EXPORT size_t olm_clear_pk_signing(
    OlmPkSigning *sign
);

/**
 * Initialise the signing object with a public/private keypair from a seed. The
 * associated public key will be written to the pubkey buffer. Returns
 * olm_error() on failure. If the public key buffer is too small then
 * olm_pk_signing_last_error() will be "OUTPUT_BUFFER_TOO_SMALL".  If the seed
 * buffer is too small then olm_pk_signing_last_error() will be
 * "INPUT_BUFFER_TOO_SMALL".
 */
OLM_EXPORT size_t olm_pk_signing_key_from_seed(
    OlmPkSigning * sign,
    void * pubkey, size_t pubkey_length,
    const void * seed, size_t seed_length
);

/**
 * The size required for the seed for initialising a signing object.
 */
OLM_EXPORT size_t olm_pk_signing_seed_length(void);

/**
 * The size of the public key of a signing object.
 */
OLM_EXPORT size_t olm_pk_signing_public_key_length(void);

/**
 * The size of a signature created by a signing object.
 */
OLM_EXPORT size_t olm_pk_signature_length(void);

/**
 * Sign a message. The signature will be written to the signature
 * buffer. Returns olm_error() on failure. If the signature buffer is too
 * small, olm_pk_signing_last_error() will be "OUTPUT_BUFFER_TOO_SMALL".
 */
OLM_EXPORT size_t olm_pk_sign(
    OlmPkSigning *sign,
    uint8_t const * message, size_t message_length,
    uint8_t * signature, size_t signature_length
);

#ifdef __cplusplus
}
#endif

#endif /* OLM_PK_H_ */
