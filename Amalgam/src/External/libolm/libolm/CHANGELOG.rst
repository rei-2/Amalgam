Changes in `3.2.16 <https://gitlab.matrix.org/matrix-org/olm/tags/3.2.16>`_
===========================================================================

This release includes the following changes since 3.2.15:

* Fix and modernize the Python packaging (thanks to Alfred Wingate)

Changes in `3.2.15 <https://gitlab.matrix.org/matrix-org/olm/tags/3.2.15>`_
===========================================================================

This release includes the following changes since 3.2.14:

* Improvements to Python packaging
  * No longer depend on ``future`` since Python 2 is no longer supported.
  * Improve compatibility with tox 4.
  * Add support for making standalone sdist.
* Improvements to Nix flake (Thanks to Jon Ringer)
  * Improve structure.
  * Enable Darwin builds.
* Typescript type fix.

Changes in `3.2.14 <https://gitlab.matrix.org/matrix-org/olm/tags/3.2.14>`_
===========================================================================

This release includes the following changes since 3.2.13:

* TypeScript type improvements.
* Improvements to Python packaging
* Documentation improvements.

Changes in `3.2.13 <https://gitlab.matrix.org/matrix-org/olm/tags/3.2.13>`_
===========================================================================

This release includes the following changes since 3.2.12:

* Fix compilation with newer versions of emscripten.
  * The npm package is compiled with emscripten 3.1.17 to fix compatibility with
    node 18.
* Add py.typed to Python wheels.
* Some documentation fixes and updates.
* Improve the pkgconfig file.

Changes in `3.2.12 <https://gitlab.matrix.org/matrix-org/olm/tags/3.2.12>`_
===========================================================================

This release includes the following changes since 3.2.11:

* Expose olm_sas_calculate_mac_fixed_base64 in the bindings.
* Allow memory to grow in wasm.  Thanks to benkuly for the suggestion.
* Fix Python type hints.
* Some Python build fixes.
* Initial work on a Nix flake for building and testing.

Changes in `3.2.11 <https://gitlab.matrix.org/matrix-org/olm/tags/3.2.11>`_
===========================================================================

This release includes the following changes since 3.2.10:

* Fix building documentation.  Thanks to Jonas Smedegaard.  The documents
  written in Markdown are now converted to HTML using Pandoc.
* Add methods for getting unpublished fallback key in Objective-C binding.
* Add public pickle/unpickle methods to Java binding.
* Add wrapper for olm_session_describe to Java binding.  Thanks to Alex Baker.

Changes in `3.2.10 <https://gitlab.matrix.org/matrix-org/olm/tags/3.2.10>`_
===========================================================================

This release includes no change since 3.2.9, but is created to be able to
publish again the Android library on MavenCentral.

Changes in `3.2.9 <https://gitlab.matrix.org/matrix-org/olm/tags/3.2.9>`_
=========================================================================

This release includes the following changes since 3.2.8:

* Switch C++ tests to use doctest.  Thanks to Nicolas Werner.
* Switch JavaScript tests to use jasmine instead of deprecated jasmine-node.
* Add session describe function to Python binding.  Thanks to Tulir Asokan.

Changes in `3.2.8 <https://gitlab.matrix.org/matrix-org/olm/tags/3.2.8>`_
=========================================================================

This release includes the following changes since 3.2.7:

* Improve handling of olm_session_describe when the buffer is too small.
* Ensure random arrays are blanked in JavaScript bindings.

Changes in `3.2.7 <https://gitlab.matrix.org/matrix-org/olm/tags/3.2.7>`_
=========================================================================

This release includes the following changes since 3.2.6:

* Fix installation with the Makefile.
* Fix exporting again, so we only export olm symbols.
* Fix WASM build.  Thanks to Benjamin Kampmann.
* Add more functions for fallback keys.

Changes in `3.2.6 <https://gitlab.matrix.org/matrix-org/olm/tags/3.2.6>`_
=========================================================================

This release includes the following changes since 3.2.5:

* Fix building on various platforms when using CMake.  Building from the
  Makefile still assumes that it is using gcc.

Changes in `3.2.5 <https://gitlab.matrix.org/matrix-org/olm/tags/3.2.5>`_
=========================================================================

This release includes the following changes since 3.2.4:

* Add functions for getting error codes rather than error strings.  Thanks to
  Nicolas Werner for the suggestion.
* Only export olm symbols.  Thanks to Mohammed Sadiq for the suggestion.
* Improve error handling in unpickle functions.
* Add support for fallback keys to the Objective-C and Android bindings.

Changes in `3.2.4 <https://gitlab.matrix.org/matrix-org/olm/tags/3.2.4>`_
=========================================================================

This release includes the following changes since 3.2.3:

* Android build fixes.

Changes in `3.2.3 <https://gitlab.matrix.org/matrix-org/olm/tags/3.2.3>`_
=========================================================================

This release includes the following changes since 3.2.2:

* Add some checks for invalid input and ensure all fields are initialized.
* Include LibreJS license tags.  Thanks to Johannes Marbach for the suggestion.
* Support for Swift Package Manager.  Thanks to Johannes Marbach.

Changes in `3.2.2 <https://gitlab.matrix.org/matrix-org/olm/tags/3.2.2>`_
=========================================================================

This release includes the following changes since 3.2.1:

* Fixes in the TypeScript definition file.
* CMake build fixes.  Thanks to Gorgurov Alexey.
* Change the JavaScript package name to ``@matrix-org/olm``.  Note that
  this means that packages will need to change their ``require`` or
  ``import`` statements to use this new name.
* Include file checksums in the JavaScript package.
* Fix length calculation in fallback key json.  Thanks to Tobias Furuholm.
* Add a new function to calculate the correct base64 encoding for SAS.
  (Currently only available in the C API.)
* Add the ability to specify a pickle key in the Objective-C binding.
* Add pkg-config file on Unix-like systems.

Changes in `3.2.1 <https://gitlab.matrix.org/matrix-org/olm/tags/3.2.1>`_
=========================================================================

This release includes the following changes since 3.2.0:

* Fixes in the TypeScript definition file.

Changes in `3.2.0 <https://gitlab.matrix.org/matrix-org/olm/tags/3.2.0>`_
=========================================================================

This release includes the following changes since 3.1.5:

* Add support for fallback keys (MSC2732).
* Allow some arguments in the JavaScript bindings to be either Uint8Array or
  strings.
* Fixes to the TypeScript definition file.
* Improvements to the JavaScript group demo. Thanks to Saúl Ibarra Corretgé.
* Ensure that the other party's public key has been set in SAS module. Thanks
  to Saúl Ibarra Corretgé.
* Fix building with newer versions of emscripten, and simplify makefile. Thanks
  to Lukas Lihotzki.
* Reduce pollution of the global namespace in the Javascript binding. Thanks to
  Lukas Lihotzki.

Changes in `3.1.5 <https://gitlab.matrix.org/matrix-org/olm/tags/3.1.5>`_
=========================================================================

This release includes the following changes since 3.1.4:

* Build improvements:

  * Fix CMake handling when installing in a non-standard location. Thanks to
    Alexey Rusakov.
  * Add support in the Makefile for creating a WASM-ready archive. Thanks to
    stoically.
  * Improve support for LLVM is Makefile. Thanks to caywin25 for reporting.

* Add a TypeScript definition file.
* Some documentation and example fixes.
* Add list of bindings to the README.

Changes in `3.1.4 <https://gitlab.matrix.org/matrix-org/olm/tags/3.1.4>`_
=========================================================================

This release includes the following changes since 3.1.3:

* Build improvements:
  * Install headers in the system-configured include directory with CMake.
  * Overwrite symbolic links when installing with make.
  * Improve compatibility with more emscripten versions.
* Don't use hypothesis in Python unit tests.
* Some documentation improvements.

Changes in `3.1.3 <https://gitlab.matrix.org/matrix-org/olm/tags/3.1.3>`_
=========================================================================

This release fixes unicode issues in the Python bindings, and adds some
clarifications to the documentation.

Changes in `3.1.2 <https://gitlab.matrix.org/matrix-org/olm/tags/3.1.2>`_
=========================================================================

This release updates the Android bindings to use a newer Android SDK version.

Changes in `3.1.1 <https://gitlab.matrix.org/matrix-org/olm/tags/3.1.1>`_
=========================================================================

This release fixes various build issues:

* Include the SAS files and tests in the CMake files.
* Address some build issues on Windows.

Changes in `3.1.0 <https://gitlab.matrix.org/matrix-org/olm/tags/3.1.0>`_
=========================================================================

This release includes the following changes since 3.0.0:

* Add functions to support Short Authentication String key verification.  The
  new functions are in the ``sas.h`` header file.  The Android, iOS, JavaScript
  and Python bindings also include corresponding functions.
* Add functions to perform public key signing.  These are meant for use with
  cross-signing.  The new functions are ``olm_pk_signing_size``,
  ``olm_pk_signing``, ``olm_pk_signing_last_error``, ``olm_clear_pk_signing``,
  ``olm_pk_signing_key_from_seed``, ``olm_pk_signing_seed_length``,
  ``olm_pk_signing_public_key_length``, ``olm_pk_signature_length``, and
  ``olm_pk_sign``.  Signatures generated by ``olm_pk_sign`` can be verified
  using ``olm_ed25519_verify``.  The Android, iOS, JavaScript and Python
  bindings also include corresponding functions.
* Fix compilation under some compilers.

JavaScript wrapper:

* Improved compatibility with newer versions of Emscripten, and dropped support
  for some older versions of Emscripten.

Python wrapper:

* Build fixes.
* Add bindings for the public key encryption/decryption functions from olm 2.3.0.

Changes in `3.0.0 <https://gitlab.matrix.org/matrix-org/olm/tags/3.0.0>`_
=========================================================================

This release includes the following changes to 2.3.0:

* Support for building using cmake. Thanks to Konstantinos Sideris.
* Add more functions for managing private keys in the public key decryption
  functionality. These are meant for use with server-side encrypted key
  backups.  The new functions are ``olm_pk_private_key_length``,
  ``olm_pk_key_from_private``, and ``olm_pk_get_private_key``.
* ``olm_pk_generate_key`` and ``olm_pk_generate_key_random_length`` are
  deprecated: to generate a random key, use ``olm_pk_key_from_private``
  with random bytes as the private key.

Python wrapper:

* BREAKING CHANGE: This release introduces a new API for the Python wrapper,
  thanks to Damir Jelić.  The new API should be much easier to use for Python
  developers.  However, this means that existing code will need to be rewritten
  to use the new API.

JavaScript wrapper:

* BREAKING CHANGE: Olm now uses WebAssembly which means it needs
  to load the wasm file asynchronously, and therefore needs to be
  started up asynchronously. The imported module now has an init()
  method which returns a promise. The library cannot be used until
  this promise resolves. It will reject if the library fails to start.
* Using ``olm/olm.js`` will use the WebAssembly version of the library.  For
  environments that do not support WebAssembly, use ``olm/olm_legacy.js``.

Objective-C wrapper:

* Add support for the public key encryption/decryption functionality.

Changes in `2.3.0 <https://gitlab.matrix.org/matrix-org/olm/tags/2.3.0>`_
=========================================================================

This release includes the following changes since 2.2.2:

* Support building on Windows. Thanks to Marcel Radzio.
* Avoid C99 inside C++ code. Thanks to Alexey Rusakov.
* Support building as a static library. Thanks to Andreas Zwinkau.

New functionality:

* Add a number of methods for public key encryption and decryption. This
  functionality is meant for use with allowing virus scanning of encrypted
  attachments, server-side encrypted key backups, and possibly other uses. The
  methods are listed in the ``olm/pk.h`` header file. Corresponding wrappers
  are available in the JavaScript and Android wrappers. Objective-C and Python
  wrappers will be available in a future release.

Android wrapper:

* Update build tool dependencies
* Apply some hardening flags and fix some compilation and run-time issues.
  Thanks in part to Arnaud Fontaine.

Objective-C wrapper:

* Update project file
* Fix compiler warnings

Python wrapper:

* Add binding for ``olm_remove_one_time_keys``. Thanks to Wilfried Klaebe.
* Add utility module for ``ed25519_verify``. Thanks to Alexander Maznev.
* Improve portability. Thanks to Jan Jancar.

Changes in `2.2.2 <https://gitlab.matrix.org/matrix-org/olm/tags/2.2.2>`_
=========================================================================

Objective-C wrapper:

* Fixed type of ``messageIndex`` argument in
  ``exportSessionAtMessageIndex``. Thanks to Greg Hughes.

Changes in `2.2.1 <https://gitlab.matrix.org/matrix-org/olm/tags/2.2.1>`_
=========================================================================

The only change in this release is a fix to the build scripts for the
Objective-C wrapper which made it impossible to release the 2.2.0 CocoaPod.

Changes in `2.2.0 <https://gitlab.matrix.org/matrix-org/olm/tags/2.2.0>`_
=========================================================================

This release includes the following changes since 2.1.0:

* Add Java wrappers to allow use under Android.

New functionality:

* Add a number of methods allowing InboundGroupSessions to be exported and
  imported. These are: ``olm_inbound_group_session_first_known_index``,
  ``olm_export_inbound_group_session_length``,
  ``olm_export_inbound_group_session``, ``olm_import_inbound_group_session``
  and ``olm_inbound_group_session_is_verified``. Corresponding wrappers are
  available in the Javascript, Python, Objective-C and Android wrappers.

Objective-C wrapper:

* Fix a number of issues with the build scripts which prevented it being used
  for macOS/Swift projects. Thanks to Avery Pierce.

Changes in `2.1.0 <https://gitlab.matrix.org/matrix-org/olm/tags/2.1.0>`_
=========================================================================

This release includes the following changes since 2.0.0:

* Add OLMKit, the Objective-C wrapper. Thanks to Chris Ballinger for the
  initial work on this.

Javascript wrapper:

* Handle exceptions during loading better (don't leave a half-initialised
  state).
* Allow applications to tune emscripten options (such as the amount of heap).
* Allocate memory for encrypted/decrypted messages on the empscripten heap,
  rather than the stack, allowing more efficient memory use.


Changes in `2.0.0 <https://gitlab.matrix.org/matrix-org/olm/tags/2.0.0>`_
=========================================================================

This release includes the following changes since 1.3.0:

* Fix a buffer bounds check when decoding group messages.
* Update ``olm_group_decrypt`` to return the ratchet index for decrypted
  messages.
* Fix ``olm_pickle_account``, ``olm_pickle_session``,
  ``olm_pickle_inbound_group_session`` and
  ``olm_pickle_outbound_group_session`` to correctly return the length of the
  pickled object.
* Add a `specification <./docs/megolm.rst>`_ of the Megolm ratchet, and add
  some information on mitigating unknown key-share attacks to the `Olm
  specification <./docs/olm.rst>`_.
* Add an ``install-headers`` target to the Makefile (and run it when installing
  the library). (Credit to Emmanuel Gil Peyrot).


Changes in `1.3.0 <https://gitlab.matrix.org/matrix-org/olm/tags/1.3.0>`_
=========================================================================

This release updates the group session identifier to avoid collisions.
Group sessions are now identified by their ed25519 public key.

These changes alter the pickle format of outbound group sessions, attempting
to unpickle an outbound group session created with a previous version of olm
will give ``OLM_CORRUPTED_PICKLE``. Inbound sessions are unaffected.

This release alters the format of group session_key messages to include the
ratchet counter. The session_key messages are now self signed with their
ed25519 key. No attempt was made to preserve backwards-compatibility.
Attempting to send session_keys between old and new versions will give
``OLM_BAD_SESSION_KEY``.

Changes in `1.2.0 <https://gitlab.matrix.org/matrix-org/olm/tags/1.2.0>`_
=========================================================================

This release updates the implementation of group session communications, to
include Ed25519 signatures on group messages, to ensure that participants in
group sessions cannot masquerade as each other.

These changes necessitate changes to the pickle format of inbound and outbound
group sessions, as well as the session_keys exchanged between them. No attempt
has been made to preserve backwards-compatibility:

* Attempting to restore old pickles will give ``OLM_CORRUPTED_PICKLE``.
* Attempting to send session_keys between old and new versions will give
  ``OLM_BAD_SESSION_KEY``.
* Attempting to send messages between old and new versions will give one of a
  number of errors.

There were also a number of implementation changes made as part of this
release, aimed at making the codebase more consistent, and to help with the
implementation of the group message signatures.


Changes in `1.1.0 <https://gitlab.matrix.org/matrix-org/olm/tags/1.1.0>`_
=========================================================================

This release includes a fix to a bug which caused Ed25519 keypairs to be
generated and used insecurely. Any Ed25519 keys generated by libolm 1.0.0
or earlier should be considered compromised.

The fix necessitates a change to the format of the OlmAccount pickle; since
existing OlmAccounts should in any case be considered compromised (as above),
the library refuses to load them, returning OLM_BAD_LEGACY_ACCOUNT_PICKLE.


Changes in `1.0.0 <https://gitlab.matrix.org/matrix-org/olm/tags/1.0.0>`_
=========================================================================

This release includes a fix to a bug which had the potential to leak sensitive
data to the application: see
https://github.com/vector-im/vector-web/issues/1719. Users of pre-1.x.x
versions of the Olm library should upgrade. Our thanks to `Dmitry Luyciv
<https://github.com/dluciv>`_ for bringing our attention to the bug.

Other changes since 0.1.0:

 * *Experimental* implementation of the primitives for group sessions. This
   implementation has not yet been used in an application and developers are
   advised not to rely on its stability.

 * Replace custom build scripts with a Makefile.

 * Include the major version number in the soname of libolm.so (credit to
   Emmanuel Gil Peyrot).
