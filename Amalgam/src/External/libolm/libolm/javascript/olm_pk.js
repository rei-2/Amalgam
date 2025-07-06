/** @constructor */
function PkEncryption() {
    var size = Module['_olm_pk_encryption_size']();
    this.buf = malloc(size);
    this.ptr = Module['_olm_pk_encryption'](this.buf);
}

function pk_encryption_method(wrapped) {
    return function() {
        var result = wrapped.apply(this, arguments);
        if (result === OLM_ERROR) {
            var message = UTF8ToString(
                Module['_olm_pk_encryption_last_error'](arguments[0])
            );
            throw new Error("OLM." + message);
        }
        return result;
    }
}

PkEncryption.prototype['free'] = function() {
    Module['_olm_clear_pk_encryption'](this.ptr);
    free(this.ptr);
}

PkEncryption.prototype['set_recipient_key'] = restore_stack(function(key) {
    var key_array = array_from_string(key);
    var key_buffer = stack(key_array);
    pk_encryption_method(Module['_olm_pk_encryption_set_recipient_key'])(
        this.ptr, key_buffer, key_array.length
    );
});

PkEncryption.prototype['encrypt'] = restore_stack(function(
    plaintext
) {
    var plaintext_buffer, ciphertext_buffer, plaintext_length, random, random_length;
    try {
        plaintext_length = lengthBytesUTF8(plaintext)
        plaintext_buffer = malloc(plaintext_length + 1);
        stringToUTF8(plaintext, plaintext_buffer, plaintext_length + 1);
        random_length = pk_encryption_method(
            Module['_olm_pk_encrypt_random_length']
        )();
        random = random_stack(random_length);
        var ciphertext_length = pk_encryption_method(
            Module['_olm_pk_ciphertext_length']
        )(this.ptr, plaintext_length);
        ciphertext_buffer = malloc(ciphertext_length + NULL_BYTE_PADDING_LENGTH);
        var mac_length = pk_encryption_method(
            Module['_olm_pk_mac_length']
        )(this.ptr);
        var mac_buffer = stack(mac_length + NULL_BYTE_PADDING_LENGTH);
        setValue(
            mac_buffer + mac_length,
            0, "i8"
        );
        var ephemeral_length = pk_encryption_method(
            Module['_olm_pk_key_length']
        )();
        var ephemeral_buffer = stack(ephemeral_length + NULL_BYTE_PADDING_LENGTH);
        setValue(
            ephemeral_buffer + ephemeral_length,
            0, "i8"
        );
        pk_encryption_method(Module['_olm_pk_encrypt'])(
            this.ptr,
            plaintext_buffer, plaintext_length,
            ciphertext_buffer, ciphertext_length,
            mac_buffer, mac_length,
            ephemeral_buffer, ephemeral_length,
            random, random_length
        );
        // UTF8ToString requires a null-terminated argument, so add the
        // null terminator.
        setValue(
            ciphertext_buffer + ciphertext_length,
            0, "i8"
        );
        return {
            "ciphertext": UTF8ToString(ciphertext_buffer, ciphertext_length),
            "mac": UTF8ToString(mac_buffer, mac_length),
            "ephemeral": UTF8ToString(ephemeral_buffer, ephemeral_length)
        };
    } finally {
        if (random !== undefined) {
            // clear out the random buffer, since it is key data
            bzero(random, random_length);
        }
        if (plaintext_buffer !== undefined) {
            // don't leave a copy of the plaintext in the heap.
            bzero(plaintext_buffer, plaintext_length + 1);
            free(plaintext_buffer);
        }
        if (ciphertext_buffer !== undefined) {
            free(ciphertext_buffer);
        }
    }
});


/** @constructor */
function PkDecryption() {
    var size = Module['_olm_pk_decryption_size']();
    this.buf = malloc(size);
    this.ptr = Module['_olm_pk_decryption'](this.buf);
}

function pk_decryption_method(wrapped) {
    return function() {
        var result = wrapped.apply(this, arguments);
        if (result === OLM_ERROR) {
            var message = UTF8ToString(
                Module['_olm_pk_decryption_last_error'](arguments[0])
            );
            throw new Error("OLM." + message);
        }
        return result;
    }
}

PkDecryption.prototype['free'] = function() {
    Module['_olm_clear_pk_decryption'](this.ptr);
    free(this.ptr);
}

PkDecryption.prototype['init_with_private_key'] = restore_stack(function (private_key) {
    var private_key_buffer = stack(private_key.length);
    Module['HEAPU8'].set(private_key, private_key_buffer);

    var pubkey_length = pk_decryption_method(
        Module['_olm_pk_key_length']
    )();
    var pubkey_buffer = stack(pubkey_length + NULL_BYTE_PADDING_LENGTH);
    try {
        pk_decryption_method(Module['_olm_pk_key_from_private'])(
            this.ptr,
            pubkey_buffer, pubkey_length,
            private_key_buffer, private_key.length
        );
    } finally {
        // clear out our copy of the private key
        bzero(private_key_buffer, private_key.length);
    }
    return UTF8ToString(pubkey_buffer, pubkey_length);
});

PkDecryption.prototype['generate_key'] = restore_stack(function () {
    var random_length = pk_decryption_method(
        Module['_olm_pk_private_key_length']
    )();
    var random_buffer = random_stack(random_length);
    var pubkey_length = pk_decryption_method(
        Module['_olm_pk_key_length']
    )();
    var pubkey_buffer = stack(pubkey_length + NULL_BYTE_PADDING_LENGTH);
    try {
        pk_decryption_method(Module['_olm_pk_key_from_private'])(
            this.ptr,
            pubkey_buffer, pubkey_length,
            random_buffer, random_length
        );
    } finally {
        // clear out the random buffer (= private key)
        bzero(random_buffer, random_length);
    }
    return UTF8ToString(pubkey_buffer, pubkey_length);
});

PkDecryption.prototype['get_private_key'] = restore_stack(function () {
    var privkey_length = pk_encryption_method(
        Module['_olm_pk_private_key_length']
    )();
    var privkey_buffer  = stack(privkey_length);
    pk_decryption_method(Module['_olm_pk_get_private_key'])(
        this.ptr,
        privkey_buffer, privkey_length
    );
    // The inner Uint8Array creates a view of the buffer.  The outer Uint8Array
    // copies it to a new array to return, since the original buffer will get
    // deallocated from the stack and could get overwritten.
    var key_arr = new Uint8Array(
        new Uint8Array(Module['HEAPU8'].buffer, privkey_buffer, privkey_length)
    );
    bzero(privkey_buffer, privkey_length); // clear out our copy of the key
    return key_arr;
});

PkDecryption.prototype['pickle'] = restore_stack(function (key) {
    var key_array = array_from_string(key);
    var pickle_length = pk_decryption_method(
        Module['_olm_pickle_pk_decryption_length']
    )(this.ptr);
    var key_buffer = stack(key_array);
    var pickle_buffer = stack(pickle_length + NULL_BYTE_PADDING_LENGTH);
    try {
        pk_decryption_method(Module['_olm_pickle_pk_decryption'])(
            this.ptr, key_buffer, key_array.length, pickle_buffer, pickle_length
        );
    } finally {
        // clear out copies of the pickle key
        bzero(key_buffer, key_array.length)
        for (var i = 0; i < key_array.length; i++) {
            key_array[i] = 0;
        }
    }
    return UTF8ToString(pickle_buffer, pickle_length);
});

PkDecryption.prototype['unpickle'] = restore_stack(function (key, pickle) {
    var key_array = array_from_string(key);
    var key_buffer = stack(key_array);
    var pickle_array = array_from_string(pickle);
    var pickle_buffer = stack(pickle_array);
    var ephemeral_length = pk_decryption_method(
        Module["_olm_pk_key_length"]
    )();
    var ephemeral_buffer = stack(ephemeral_length + NULL_BYTE_PADDING_LENGTH);
    try {
        pk_decryption_method(Module['_olm_unpickle_pk_decryption'])(
            this.ptr, key_buffer, key_array.length, pickle_buffer,
            pickle_array.length, ephemeral_buffer, ephemeral_length
        );
    } finally {
        // clear out copies of the pickle key
        bzero(key_buffer, key_array.length)
        for (var i = 0; i < key_array.length; i++) {
            key_array[i] = 0;
        }
    }
    return UTF8ToString(ephemeral_buffer, ephemeral_length);
});

PkDecryption.prototype['decrypt'] = restore_stack(function (
    ephemeral_key, mac, ciphertext
) {
    var plaintext_buffer, ciphertext_buffer, plaintext_max_length;
    try {
        var ciphertext_length = lengthBytesUTF8(ciphertext)
        ciphertext_buffer = malloc(ciphertext_length + 1);
        stringToUTF8(ciphertext, ciphertext_buffer, ciphertext_length + 1);
        var ephemeralkey_array = array_from_string(ephemeral_key);
        var ephemeralkey_buffer = stack(ephemeralkey_array);
        var mac_array = array_from_string(mac);
        var mac_buffer = stack(mac_array);
        plaintext_max_length = pk_decryption_method(Module['_olm_pk_max_plaintext_length'])(
            this.ptr,
            ciphertext_length
        );
        plaintext_buffer = malloc(plaintext_max_length + NULL_BYTE_PADDING_LENGTH);
        var plaintext_length = pk_decryption_method(Module['_olm_pk_decrypt'])(
            this.ptr,
            ephemeralkey_buffer, ephemeralkey_array.length,
            mac_buffer, mac_array.length,
            ciphertext_buffer, ciphertext_length,
            plaintext_buffer, plaintext_max_length
        );
        // UTF8ToString requires a null-terminated argument, so add the
        // null terminator.
        setValue(
            plaintext_buffer + plaintext_length,
            0, "i8"
        );
        return UTF8ToString(plaintext_buffer, plaintext_length);
    } finally {
        if (plaintext_buffer !== undefined) {
            // don't leave a copy of the plaintext in the heap.
            bzero(plaintext_buffer, plaintext_length + 1);
            free(plaintext_buffer);
        }
        if (ciphertext_buffer !== undefined) {
            free(ciphertext_buffer);
        }
    }
})


/** @constructor */
function PkSigning() {
    var size = Module['_olm_pk_signing_size']();
    this.buf = malloc(size);
    this.ptr = Module['_olm_pk_signing'](this.buf);
}

function pk_signing_method(wrapped) {
    return function() {
        var result = wrapped.apply(this, arguments);
        if (result === OLM_ERROR) {
            var message = UTF8ToString(
                Module['_olm_pk_signing_last_error'](arguments[0])
            );
            throw new Error("OLM." + message);
        }
        return result;
    }
}

PkSigning.prototype['free'] = function() {
    Module['_olm_clear_pk_signing'](this.ptr);
    free(this.ptr);
}

PkSigning.prototype['init_with_seed'] = restore_stack(function (seed) {
    var seed_buffer = stack(seed.length);
    Module['HEAPU8'].set(seed, seed_buffer);

    var pubkey_length = pk_signing_method(
        Module['_olm_pk_signing_public_key_length']
    )();
    var pubkey_buffer = stack(pubkey_length + NULL_BYTE_PADDING_LENGTH);
    try {
        pk_signing_method(Module['_olm_pk_signing_key_from_seed'])(
            this.ptr,
            pubkey_buffer, pubkey_length,
            seed_buffer, seed.length
        );
    } finally {
        // clear out our copy of the seed
        bzero(seed_buffer, seed.length);
    }
    return UTF8ToString(pubkey_buffer, pubkey_length);
});

PkSigning.prototype['generate_seed'] = restore_stack(function () {
    var random_length = pk_signing_method(
        Module['_olm_pk_signing_seed_length']
    )();
    var random_buffer = random_stack(random_length);
    var key_arr = new Uint8Array(
        new Uint8Array(Module['HEAPU8'].buffer, random_buffer, random_length)
    );
    bzero(random_buffer, random_length);
    return key_arr;
});

PkSigning.prototype['sign'] = restore_stack(function (message) {
    // XXX: Should be able to sign any bytes rather than just strings,
    // but this is consistent with encrypt for now.
    //var message_buffer = stack(message.length);
    //Module['HEAPU8'].set(message, message_buffer);
    var message_buffer, message_length;

    try {
        message_length = lengthBytesUTF8(message)
        message_buffer = malloc(message_length + 1);
        stringToUTF8(message, message_buffer, message_length + 1);

        var sig_length = pk_signing_method(
            Module['_olm_pk_signature_length']
        )();
        var sig_buffer = stack(sig_length + NULL_BYTE_PADDING_LENGTH);
        pk_signing_method(Module['_olm_pk_sign'])(
            this.ptr,
            message_buffer, message_length,
            sig_buffer, sig_length
        );
        return UTF8ToString(sig_buffer, sig_length);
    } finally {
        if (message_buffer !== undefined) {
            // don't leave a copy of the plaintext in the heap.
            bzero(message_buffer, message_length + 1);
            free(message_buffer);
        }
    }
});
