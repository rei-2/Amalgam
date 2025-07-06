var get_random_values;

if (typeof(window) !== 'undefined') {
    // We're in a browser (directly, via browserify, or via webpack).
    get_random_values = function(buf) {
        window.crypto.getRandomValues(buf);
    };
} else if (module["exports"]) {
    // We're running in node.
    var nodeCrypto = require("crypto");
    get_random_values = function(buf) {
        // [''] syntax needed here rather than '.' to prevent
        // closure compiler from mangling the import(!)
        var bytes = nodeCrypto['randomBytes'](buf.length);
        buf.set(bytes);
    };
} else {
    throw new Error("Cannot find global to attach library to");
}

/* applications should define OLM_OPTIONS in the environment to override
 * emscripten module settings
 */
if (typeof(OLM_OPTIONS) !== 'undefined') {
    for (var olm_option_key in OLM_OPTIONS) {
        if (OLM_OPTIONS.hasOwnProperty(olm_option_key)) {
            Module[olm_option_key] = OLM_OPTIONS[olm_option_key];
        }
    }
}

/* The 'length' argument to Pointer_stringify doesn't work if the input
 * includes characters >= 128, which makes Pointer_stringify unreliable. We
 * could use it on strings which are known to be ascii, but that seems
 * dangerous. Instead we add a NULL character to all of our strings and just
 * use UTF8ToString.
 */
var NULL_BYTE_PADDING_LENGTH = 1;

Module['onRuntimeInitialized'] = function() {
    OLM_ERROR = Module['_olm_error']();
    olm_exports["PRIVATE_KEY_LENGTH"] = Module['_olm_pk_private_key_length']();
    if (onInitSuccess) onInitSuccess();
};

Module['onAbort'] = function(err) {
    if (onInitFail) onInitFail(err);
};
