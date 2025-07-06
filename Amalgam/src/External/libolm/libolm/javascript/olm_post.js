var malloc = Module['_malloc'];
var free = Module['_free'];
var OLM_ERROR;

function filled_stack(size, filler) {
    var ptr = stackAlloc(size);
    filler(new Uint8Array(Module['HEAPU8'].buffer, ptr, size));
    return ptr;
}

/* allocate a number of bytes of storage on the stack.
 *
 * If size_or_array is a Number, allocates that number of zero-initialised bytes.
 */
function stack(size_or_array) {
    return (typeof size_or_array == 'number')
        ? filled_stack(size_or_array, function(x) { x.fill(0) })
        : filled_stack(size_or_array.length, function(x) { x.set(size_or_array) });
}

function array_from_string(string) {
    return string instanceof Uint8Array ? string : intArrayFromString(string, true);
}

function random_stack(size) {
    return filled_stack(size, get_random_values);
}

function restore_stack(wrapped) {
    return function() {
        var sp = stackSave();
        try {
            return wrapped.apply(this, arguments);
        } finally {
            stackRestore(sp);
        }
    }
}

/* set a memory area to zero */
function bzero(ptr, n) {
    while(n-- > 0) {
        Module['HEAP8'][ptr++] = 0;
    }
}

/** @constructor */
function Account() {
    var size = Module['_olm_account_size']();
    this.buf = malloc(size);
    this.ptr = Module['_olm_account'](this.buf);
}

function account_method(wrapped) {
    return function() {
        var result = wrapped.apply(this, arguments);
        if (result === OLM_ERROR) {
            var message = UTF8ToString(
                Module['_olm_account_last_error'](arguments[0])
            );
            throw new Error("OLM." + message);
        }
        return result;
    }
}

Account.prototype['free'] = function() {
    Module['_olm_clear_account'](this.ptr);
    free(this.ptr);
}

Account.prototype['create'] = restore_stack(function() {
    var random_length = account_method(
        Module['_olm_create_account_random_length']
    )(this.ptr);
    var random = random_stack(random_length);
    try {
        account_method(Module['_olm_create_account'])(
            this.ptr, random, random_length
        );
    } finally {
        // clear the random buffer
        bzero(random, random_length);
    }
});

Account.prototype['identity_keys'] = restore_stack(function() {
    var keys_length = account_method(
        Module['_olm_account_identity_keys_length']
    )(this.ptr);
    var keys = stack(keys_length + NULL_BYTE_PADDING_LENGTH);
    account_method(Module['_olm_account_identity_keys'])(
        this.ptr, keys, keys_length
    );
    return UTF8ToString(keys, keys_length);
});

Account.prototype['sign'] = restore_stack(function(message) {
    var signature_length = account_method(
        Module['_olm_account_signature_length']
    )(this.ptr);
    var message_array = array_from_string(message);
    var message_buffer = stack(message_array);
    var signature_buffer = stack(signature_length + NULL_BYTE_PADDING_LENGTH);
    try {
        account_method(Module['_olm_account_sign'])(
            this.ptr,
            message_buffer, message_array.length,
            signature_buffer, signature_length
        );
    } finally {
        // clear out copies of the message, which may be plaintext
        bzero(message_buffer, message_array.length);
        for (var i = 0; i < message_array.length; i++) {
            message_array[i] = 0;
        }
    }
    return UTF8ToString(signature_buffer, signature_length);
});

Account.prototype['one_time_keys'] = restore_stack(function() {
    var keys_length = account_method(
        Module['_olm_account_one_time_keys_length']
    )(this.ptr);
    var keys = stack(keys_length + NULL_BYTE_PADDING_LENGTH);
    account_method(Module['_olm_account_one_time_keys'])(
        this.ptr, keys, keys_length
    );
    return UTF8ToString(keys, keys_length);
});

Account.prototype['mark_keys_as_published'] = restore_stack(function() {
    account_method(Module['_olm_account_mark_keys_as_published'])(this.ptr);
});

Account.prototype['max_number_of_one_time_keys'] = restore_stack(function() {
    return account_method(Module['_olm_account_max_number_of_one_time_keys'])(
        this.ptr
    );
});

Account.prototype['generate_one_time_keys'] = restore_stack(function(
    number_of_keys
) {
    var random_length = account_method(
        Module['_olm_account_generate_one_time_keys_random_length']
    )(this.ptr, number_of_keys);
    var random = random_stack(random_length);
    try {
        account_method(Module['_olm_account_generate_one_time_keys'])(
            this.ptr, number_of_keys, random, random_length
        );
    } finally {
        // clear the random buffer
        bzero(random, random_length);
    }
});

Account.prototype['remove_one_time_keys'] = restore_stack(function(session) {
    account_method(Module['_olm_remove_one_time_keys'])(
        this.ptr, session.ptr
    );
});

Account.prototype['generate_fallback_key'] = restore_stack(function() {
    var random_length = account_method(
        Module['_olm_account_generate_fallback_key_random_length']
    )(this.ptr);
    var random = random_stack(random_length);
    try {
        account_method(Module['_olm_account_generate_fallback_key'])(
            this.ptr, random, random_length
        );
    } finally {
        // clear the random buffer
        bzero(random, random_length);
    }
});

Account.prototype['fallback_key'] = restore_stack(function() {
    var keys_length = account_method(
        Module['_olm_account_fallback_key_length']
    )(this.ptr);
    var keys = stack(keys_length + NULL_BYTE_PADDING_LENGTH);
    account_method(Module['_olm_account_fallback_key'])(
        this.ptr, keys, keys_length
    );
    return UTF8ToString(keys, keys_length);
});

Account.prototype['unpublished_fallback_key'] = restore_stack(function() {
    var keys_length = account_method(
        Module['_olm_account_unpublished_fallback_key_length']
    )(this.ptr);
    var keys = stack(keys_length + NULL_BYTE_PADDING_LENGTH);
    account_method(Module['_olm_account_unpublished_fallback_key'])(
        this.ptr, keys, keys_length
    );
    return UTF8ToString(keys, keys_length);
});

Account.prototype['forget_old_fallback_key'] = restore_stack(function() {
    account_method(Module['_olm_account_forget_old_fallback_key'])(
        this.ptr
    );
});

Account.prototype['pickle'] = restore_stack(function(key) {
    var key_array = array_from_string(key);
    var pickle_length = account_method(
        Module['_olm_pickle_account_length']
    )(this.ptr);
    var key_buffer = stack(key_array);
    var pickle_buffer = stack(pickle_length + NULL_BYTE_PADDING_LENGTH);
    try {
        account_method(Module['_olm_pickle_account'])(
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

Account.prototype['unpickle'] = restore_stack(function(key, pickle) {
    var key_array = array_from_string(key);
    var key_buffer = stack(key_array);
    var pickle_array = array_from_string(pickle);
    var pickle_buffer = stack(pickle_array);
    try {
        account_method(Module['_olm_unpickle_account'])(
            this.ptr, key_buffer, key_array.length, pickle_buffer,
            pickle_array.length
        );
    } finally {
        // clear out copies of the pickle key
        bzero(key_buffer, key_array.length)
        for (var i = 0; i < key_array.length; i++) {
            key_array[i] = 0;
        }
    }
});

/** @constructor */
function Session() {
    var size = Module['_olm_session_size']();
    this.buf = malloc(size);
    this.ptr = Module['_olm_session'](this.buf);
}

function session_method(wrapped) {
    return function() {
        var result = wrapped.apply(this, arguments);
        if (result === OLM_ERROR) {
            var message = UTF8ToString(
                Module['_olm_session_last_error'](arguments[0])
            );
            throw new Error("OLM." + message);
        }
        return result;
    }
}

Session.prototype['free'] = function() {
    Module['_olm_clear_session'](this.ptr);
    free(this.ptr);
}

Session.prototype['pickle'] = restore_stack(function(key) {
    var key_array = array_from_string(key);
    var pickle_length = session_method(
        Module['_olm_pickle_session_length']
    )(this.ptr);
    var key_buffer = stack(key_array);
    var pickle_buffer = stack(pickle_length + NULL_BYTE_PADDING_LENGTH);
    try {
        session_method(Module['_olm_pickle_session'])(
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

Session.prototype['unpickle'] = restore_stack(function(key, pickle) {
    var key_array = array_from_string(key);
    var key_buffer = stack(key_array);
    var pickle_array = array_from_string(pickle);
    var pickle_buffer = stack(pickle_array);
    try {
        session_method(Module['_olm_unpickle_session'])(
            this.ptr, key_buffer, key_array.length, pickle_buffer,
            pickle_array.length
        );
    } finally {
        // clear out copies of the pickle key
        bzero(key_buffer, key_array.length)
        for (var i = 0; i < key_array.length; i++) {
            key_array[i] = 0;
        }
    }
});

Session.prototype['create_outbound'] = restore_stack(function(
    account, their_identity_key, their_one_time_key
) {
    var random_length = session_method(
        Module['_olm_create_outbound_session_random_length']
    )(this.ptr);
    var random = random_stack(random_length);
    var identity_key_array = array_from_string(their_identity_key);
    var one_time_key_array = array_from_string(their_one_time_key);
    var identity_key_buffer = stack(identity_key_array);
    var one_time_key_buffer = stack(one_time_key_array);
    try {
        session_method(Module['_olm_create_outbound_session'])(
            this.ptr, account.ptr,
            identity_key_buffer, identity_key_array.length,
            one_time_key_buffer, one_time_key_array.length,
            random, random_length
        );
    } finally {
        // clear the random buffer, which is key data
        bzero(random, random_length);
    }
});

Session.prototype['create_inbound'] = restore_stack(function(
    account, one_time_key_message
) {
    var message_array = array_from_string(one_time_key_message);
    var message_buffer = stack(message_array);
    try {
        session_method(Module['_olm_create_inbound_session'])(
            this.ptr, account.ptr, message_buffer, message_array.length
        );
    } finally {
        // clear out copies of the key
        bzero(message_buffer, message_array.length);
        for (var i = 0; i < message_array.length; i++) {
            message_array[i] = 0;
        }
    }
});

Session.prototype['create_inbound_from'] = restore_stack(function(
    account, identity_key, one_time_key_message
) {
    var identity_key_array = array_from_string(identity_key);
    var identity_key_buffer = stack(identity_key_array);
    var message_array = array_from_string(one_time_key_message);
    var message_buffer = stack(message_array);
    try {
        session_method(Module['_olm_create_inbound_session_from'])(
            this.ptr, account.ptr,
            identity_key_buffer, identity_key_array.length,
            message_buffer, message_array.length
        );
    } finally {
        // clear out copies of the key
        bzero(message_buffer, message_array.length);
        for (var i = 0; i < message_array.length; i++) {
            message_array[i] = 0;
        }
    }
});

Session.prototype['session_id'] = restore_stack(function() {
    var id_length = session_method(Module['_olm_session_id_length'])(this.ptr);
    var id_buffer = stack(id_length + NULL_BYTE_PADDING_LENGTH);
    session_method(Module['_olm_session_id'])(
        this.ptr, id_buffer, id_length
    );
    return UTF8ToString(id_buffer, id_length);
});

Session.prototype['has_received_message'] = function() {
    return session_method(Module['_olm_session_has_received_message'])(
        this.ptr
    ) ? true : false;
};


Session.prototype['matches_inbound'] = restore_stack(function(
    one_time_key_message
) {
    var message_array = array_from_string(one_time_key_message);
    var message_buffer = stack(message_array);
    return session_method(Module['_olm_matches_inbound_session'])(
        this.ptr, message_buffer, message_array.length
    ) ? true : false;
});

Session.prototype['matches_inbound_from'] = restore_stack(function(
    identity_key, one_time_key_message
) {
    var identity_key_array = array_from_string(identity_key);
    var identity_key_buffer = stack(identity_key_array);
    var message_array = array_from_string(one_time_key_message);
    var message_buffer = stack(message_array);
    return session_method(Module['_olm_matches_inbound_session_from'])(
        this.ptr,
        identity_key_buffer, identity_key_array.length,
        message_buffer, message_array.length
    ) ? true : false;
});

Session.prototype['encrypt'] = restore_stack(function(
    plaintext
) {
    var plaintext_buffer, message_buffer, plaintext_length, random, random_length;
    try {
        random_length = session_method(
            Module['_olm_encrypt_random_length']
        )(this.ptr);
        var message_type = session_method(
            Module['_olm_encrypt_message_type']
        )(this.ptr);

        plaintext_length = lengthBytesUTF8(plaintext);
        var message_length = session_method(
            Module['_olm_encrypt_message_length']
        )(this.ptr, plaintext_length);

        random = random_stack(random_length);

        // need to allow space for the terminator (which stringToUTF8 always
        // writes), hence + 1.
        plaintext_buffer = malloc(plaintext_length + 1);
        stringToUTF8(plaintext, plaintext_buffer, plaintext_length + 1);

        message_buffer = malloc(message_length + NULL_BYTE_PADDING_LENGTH);

        session_method(Module['_olm_encrypt'])(
            this.ptr,
            plaintext_buffer, plaintext_length,
            random, random_length,
            message_buffer, message_length
        );

        // UTF8ToString requires a null-terminated argument, so add the
        // null terminator.
        setValue(
            message_buffer+message_length,
            0, "i8"
        );

        return {
            "type": message_type,
            "body": UTF8ToString(message_buffer, message_length),
        };
    } finally {
        if (random !== undefined) {
            // clear out the random buffer, since it is the private key
            bzero(random, random_length);
        }
        if (plaintext_buffer !== undefined) {
            // don't leave a copy of the plaintext in the heap.
            bzero(plaintext_buffer, plaintext_length + 1);
            free(plaintext_buffer);
        }
        if (message_buffer !== undefined) {
            free(message_buffer);
        }
    }
});

Session.prototype['decrypt'] = restore_stack(function(
    message_type, message
) {
    var message_buffer, plaintext_buffer, max_plaintext_length;

    try {
        message_buffer = malloc(message.length);
        writeAsciiToMemory(message, message_buffer, true);

        max_plaintext_length = session_method(
            Module['_olm_decrypt_max_plaintext_length']
        )(this.ptr, message_type, message_buffer, message.length);

        // caculating the length destroys the input buffer, so we need to re-copy it.
        writeAsciiToMemory(message, message_buffer, true);

        plaintext_buffer = malloc(max_plaintext_length + NULL_BYTE_PADDING_LENGTH);

        var plaintext_length = session_method(Module["_olm_decrypt"])(
            this.ptr, message_type,
            message_buffer, message.length,
            plaintext_buffer, max_plaintext_length
        );

        // UTF8ToString requires a null-terminated argument, so add the
        // null terminator.
        setValue(
            plaintext_buffer+plaintext_length,
            0, "i8"
        );

        return UTF8ToString(plaintext_buffer, plaintext_length);
    } finally {
        if (message_buffer !== undefined) {
            free(message_buffer);
        }
        if (plaintext_buffer !== undefined) {
            // don't leave a copy of the plaintext in the heap.
            bzero(plaintext_buffer, max_plaintext_length);
            free(plaintext_buffer);
        }
    }

});

Session.prototype['describe'] = restore_stack(function() {
    var description_buf;
    try {
        description_buf = malloc(256);
        session_method(Module['_olm_session_describe'])(
            this.ptr, description_buf, 256
        );
        return UTF8ToString(description_buf);
    } finally {
        if (description_buf !== undefined) free(description_buf);
    }
});

/** @constructor */
function Utility() {
    var size = Module['_olm_utility_size']();
    this.buf = malloc(size);
    this.ptr = Module['_olm_utility'](this.buf);
}

function utility_method(wrapped) {
    return function() {
        var result = wrapped.apply(this, arguments);
        if (result === OLM_ERROR) {
            var message = UTF8ToString(
                Module['_olm_utility_last_error'](arguments[0])
            );
            throw new Error("OLM." + message);
        }
        return result;
    }
}

Utility.prototype['free'] = function() {
    Module['_olm_clear_utility'](this.ptr);
    free(this.ptr);
}

Utility.prototype['sha256'] = restore_stack(function(input) {
    var output_length = utility_method(Module['_olm_sha256_length'])(this.ptr);
    var input_array = array_from_string(input);
    var input_buffer = stack(input_array);
    var output_buffer = stack(output_length + NULL_BYTE_PADDING_LENGTH);
    try {
        utility_method(Module['_olm_sha256'])(
            this.ptr,
            input_buffer, input_array.length,
            output_buffer, output_length
        );
    } finally {
        // clear out copies of the input buffer, which may be plaintext
        bzero(input_buffer, input_array.length);
        for (var i = 0; i < input_array.length; i++) {
            input_array[i] = 0;
        }
    }
    return UTF8ToString(output_buffer, output_length);
});

Utility.prototype['ed25519_verify'] = restore_stack(function(
    key, message, signature
) {
    var key_array = array_from_string(key);
    var key_buffer = stack(key_array);
    var message_array = array_from_string(message);
    var message_buffer = stack(message_array);
    var signature_array = array_from_string(signature);
    var signature_buffer = stack(signature_array);
    try {
        utility_method(Module['_olm_ed25519_verify'])(
            this.ptr,
            key_buffer, key_array.length,
            message_buffer, message_array.length,
            signature_buffer, signature_array.length
        );
    } finally {
        // clear out copies of the input buffer, which may be plaintext
        bzero(message_buffer, message_array.length);
        for (var i = 0; i < message_array.length; i++) {
            message_array[i] = 0;
        }
    }
});

olm_exports["Account"] = Account;
olm_exports["Session"] = Session;
olm_exports["Utility"] = Utility;
olm_exports["PkEncryption"] = PkEncryption;
olm_exports["PkDecryption"] = PkDecryption;
olm_exports["PkSigning"] = PkSigning;
olm_exports["SAS"] = SAS;

olm_exports["get_library_version"] = restore_stack(function() {
    var buf = stack(3);
    Module['_olm_get_library_version'](buf, buf+1, buf+2);
    return [
        getValue(buf, 'i8'),
        getValue(buf+1, 'i8'),
        getValue(buf+2, 'i8'),
    ];
});
