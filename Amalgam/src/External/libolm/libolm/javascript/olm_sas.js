/** @constructor */
function SAS() {
    var size = Module['_olm_sas_size']();
    var random_length = Module['_olm_create_sas_random_length']();
    var random = random_stack(random_length);
    this.buf = malloc(size);
    this.ptr = Module['_olm_sas'](this.buf);
    Module['_olm_create_sas'](this.ptr, random, random_length);
    bzero(random, random_length);
}

function sas_method(wrapped) {
    return function() {
        var result = wrapped.apply(this, arguments);
        if (result === OLM_ERROR) {
            var message = UTF8ToString(
                Module['_olm_sas_last_error'](arguments[0])
            );
            throw new Error("OLM." + message);
        }
        return result;
    }
}

SAS.prototype['free'] = function() {
    Module['_olm_clear_sas'](this.ptr);
    free(this.ptr);
};

SAS.prototype['get_pubkey'] = restore_stack(function() {
    var pubkey_length = sas_method(Module['_olm_sas_pubkey_length'])(this.ptr);
    var pubkey_buffer = stack(pubkey_length + NULL_BYTE_PADDING_LENGTH);
    sas_method(Module['_olm_sas_get_pubkey'])(this.ptr, pubkey_buffer, pubkey_length);
    return UTF8ToString(pubkey_buffer, pubkey_length);
});

SAS.prototype['set_their_key'] = restore_stack(function(their_key) {
    var their_key_array = array_from_string(their_key);
    var their_key_buffer = stack(their_key_array);
    sas_method(Module['_olm_sas_set_their_key'])(
        this.ptr,
        their_key_buffer, their_key_array.length
    );
});

SAS.prototype['is_their_key_set'] = restore_stack(function() {
    return sas_method(Module['_olm_sas_is_their_key_set'])(
        this.ptr
    ) ? true : false;
});

SAS.prototype['generate_bytes'] = restore_stack(function(info, length) {
    var info_array = array_from_string(info);
    var info_buffer = stack(info_array);
    var output_buffer = stack(length);
    sas_method(Module['_olm_sas_generate_bytes'])(
        this.ptr,
        info_buffer, info_array.length,
        output_buffer, length
    );
    // The inner Uint8Array creates a view of the buffer.  The outer Uint8Array
    // copies it to a new array to return, since the original buffer will get
    // deallocated from the stack and could get overwritten.
    var output_arr = new Uint8Array(
        new Uint8Array(Module['HEAPU8'].buffer, output_buffer, length)
    );
    return output_arr;
});

SAS.prototype['calculate_mac'] = restore_stack(function(input, info) {
    var input_array = array_from_string(input);
    var input_buffer = stack(input_array);
    var info_array = array_from_string(info);
    var info_buffer = stack(info_array);
    var mac_length = sas_method(Module['_olm_sas_mac_length'])(this.ptr);
    var mac_buffer = stack(mac_length + NULL_BYTE_PADDING_LENGTH);
    sas_method(Module['_olm_sas_calculate_mac'])(
        this.ptr,
        input_buffer, input_array.length,
        info_buffer, info_array.length,
        mac_buffer, mac_length
    );
    return UTF8ToString(mac_buffer, mac_length);
});

SAS.prototype['calculate_mac_fixed_base64'] = restore_stack(function(input, info) {
    var input_array = array_from_string(input);
    var input_buffer = stack(input_array);
    var info_array = array_from_string(info);
    var info_buffer = stack(info_array);
    var mac_length = sas_method(Module['_olm_sas_mac_length'])(this.ptr);
    var mac_buffer = stack(mac_length + NULL_BYTE_PADDING_LENGTH);
    sas_method(Module['_olm_sas_calculate_mac_fixed_base64'])(
        this.ptr,
        input_buffer, input_array.length,
        info_buffer, info_array.length,
        mac_buffer, mac_length
    );
    return UTF8ToString(mac_buffer, mac_length);
});

SAS.prototype['calculate_mac_long_kdf'] = restore_stack(function(input, info) {
    var input_array = array_from_string(input);
    var input_buffer = stack(input_array);
    var info_array = array_from_string(info);
    var info_buffer = stack(info_array);
    var mac_length = sas_method(Module['_olm_sas_mac_length'])(this.ptr);
    var mac_buffer = stack(mac_length + NULL_BYTE_PADDING_LENGTH);
    sas_method(Module['_olm_sas_calculate_mac_long_kdf'])(
        this.ptr,
        input_buffer, input_array.length,
        info_buffer, info_array.length,
        mac_buffer, mac_length
    );
    return UTF8ToString(mac_buffer, mac_length);
});
