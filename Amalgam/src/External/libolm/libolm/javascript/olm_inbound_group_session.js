/** @constructor */
function InboundGroupSession() {
    var size = Module['_olm_inbound_group_session_size']();
    this.buf = malloc(size);
    this.ptr = Module['_olm_inbound_group_session'](this.buf);
}

function inbound_group_session_method(wrapped) {
    return function() {
        var result = wrapped.apply(this, arguments);
        if (result === OLM_ERROR) {
            var message = UTF8ToString(
                Module['_olm_inbound_group_session_last_error'](arguments[0])
            );
            throw new Error("OLM." + message);
        }
        return result;
    }
}

InboundGroupSession.prototype['free'] = function() {
    Module['_olm_clear_inbound_group_session'](this.ptr);
    free(this.ptr);
}

InboundGroupSession.prototype['pickle'] = restore_stack(function(key) {
    var key_array = array_from_string(key);
    var pickle_length = inbound_group_session_method(
        Module['_olm_pickle_inbound_group_session_length']
    )(this.ptr);
    var key_buffer = stack(key_array);
    var pickle_buffer = stack(pickle_length + NULL_BYTE_PADDING_LENGTH);
    try {
        inbound_group_session_method(Module['_olm_pickle_inbound_group_session'])(
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

InboundGroupSession.prototype['unpickle'] = restore_stack(function(key, pickle) {
    var key_array = array_from_string(key);
    var key_buffer = stack(key_array);
    var pickle_array = array_from_string(pickle);
    var pickle_buffer = stack(pickle_array);
    try {
        inbound_group_session_method(Module['_olm_unpickle_inbound_group_session'])(
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

InboundGroupSession.prototype['create'] = restore_stack(function(session_key) {
    var key_array = array_from_string(session_key);
    var key_buffer = stack(key_array);

    try {
        inbound_group_session_method(Module['_olm_init_inbound_group_session'])(
            this.ptr, key_buffer, key_array.length
        );
    } finally {
        // clear out copies of the key
        bzero(key_buffer, key_array.length)
        for (var i = 0; i < key_array.length; i++) {
            key_array[i] = 0;
        }
    }
});

InboundGroupSession.prototype['import_session'] = restore_stack(function(session_key) {
    var key_array = array_from_string(session_key);
    var key_buffer = stack(key_array);

    try {
        inbound_group_session_method(Module['_olm_import_inbound_group_session'])(
            this.ptr, key_buffer, key_array.length
        );
    } finally {
        // clear out copies of the key
        bzero(key_buffer, key_array.length)
        for (var i = 0; i < key_array.length; i++) {
            key_array[i] = 0;
        }
    }
});

InboundGroupSession.prototype['decrypt'] = restore_stack(function(
    message
) {
    var message_buffer, plaintext_buffer, plaintext_length;

    try {
        message_buffer = malloc(message.length);
        writeAsciiToMemory(message, message_buffer, true);

        var max_plaintext_length = inbound_group_session_method(
            Module['_olm_group_decrypt_max_plaintext_length']
        )(this.ptr, message_buffer, message.length);

        // caculating the length destroys the input buffer, so we need to re-copy it.
        writeAsciiToMemory(message, message_buffer, true);

        plaintext_buffer = malloc(max_plaintext_length + NULL_BYTE_PADDING_LENGTH);
        var message_index = stack(4);

        plaintext_length = inbound_group_session_method(
            Module["_olm_group_decrypt"]
        )(
            this.ptr,
            message_buffer, message.length,
            plaintext_buffer, max_plaintext_length,
            message_index
        );

        // UTF8ToString requires a null-terminated argument, so add the
        // null terminator.
        setValue(
            plaintext_buffer+plaintext_length,
            0, "i8"
        );

        return {
            "plaintext": UTF8ToString(plaintext_buffer, plaintext_length),
            "message_index": getValue(message_index, "i32")
        }
    } finally {
        if (message_buffer !== undefined) {
            free(message_buffer);
        }
        if (plaintext_buffer !== undefined) {
            // don't leave a copy of the plaintext in the heap.
            bzero(plaintext_buffer, plaintext_length);
            free(plaintext_buffer);
        }
    }
});

InboundGroupSession.prototype['session_id'] = restore_stack(function() {
    var length = inbound_group_session_method(
        Module['_olm_inbound_group_session_id_length']
    )(this.ptr);
    var session_id = stack(length + NULL_BYTE_PADDING_LENGTH);
    inbound_group_session_method(Module['_olm_inbound_group_session_id'])(
        this.ptr, session_id, length
    );
    return UTF8ToString(session_id, length);
});

InboundGroupSession.prototype['first_known_index'] = restore_stack(function() {
    return inbound_group_session_method(
        Module['_olm_inbound_group_session_first_known_index']
    )(this.ptr);
});

InboundGroupSession.prototype['export_session'] = restore_stack(function(message_index) {
    var key_length = inbound_group_session_method(
        Module['_olm_export_inbound_group_session_length']
    )(this.ptr);
    var key = stack(key_length + NULL_BYTE_PADDING_LENGTH);
    outbound_group_session_method(Module['_olm_export_inbound_group_session'])(
        this.ptr, key, key_length, message_index
    );
    var key_str = UTF8ToString(key, key_length);
    bzero(key, key_length); // clear out a copy of the key
    return key_str;
});

olm_exports['InboundGroupSession'] = InboundGroupSession;
