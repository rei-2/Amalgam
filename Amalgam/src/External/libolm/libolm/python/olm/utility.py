# -*- coding: utf-8 -*-
# libolm python bindings
# Copyright © 2015-2017 OpenMarket Ltd
# Copyright © 2018 Damir Jelić <poljar@termina.org.uk>
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
"""libolm Utility module.

This module contains utilities for olm.
It only contains the ed25519_verify function for signature verification.

Examples:
    >>> alice = Account()

    >>> message = "Test"
    >>> signature = alice.sign(message)
    >>> signing_key = alice.identity_keys["ed25519"]

    >>> ed25519_verify(signing_key, message, signature)

"""

# pylint: disable=redefined-builtin,unused-import
from typing import AnyStr, Type

# pylint: disable=no-name-in-module
from _libolm import ffi, lib  # type: ignore

from ._compat import to_bytearray, to_bytes
from ._finalize import track_for_finalization


def _clear_utility(utility):  # pragma: no cover
    # type: (ffi.cdata) -> None
    lib.olm_clear_utility(utility)


class OlmVerifyError(Exception):
    """libolm signature verification exception."""


class OlmHashError(Exception):
    """libolm hash calculation exception."""


class _Utility(object):
    # pylint: disable=too-few-public-methods
    """libolm Utility class."""

    _buf = None
    _utility = None

    @classmethod
    def _allocate(cls):
        # type: (Type[_Utility]) -> None
        cls._buf = ffi.new("char[]", lib.olm_utility_size())
        cls._utility = lib.olm_utility(cls._buf)
        track_for_finalization(cls, cls._utility, _clear_utility)

    @classmethod
    def _check_error(cls, ret, error_class):
        # type: (int, Type) -> None
        if ret != lib.olm_error():
            return

        raise error_class("{}".format(
            ffi.string(lib.olm_utility_last_error(
                cls._utility)).decode("utf-8")))

    @classmethod
    def _ed25519_verify(cls, key, message, signature):
        # type: (Type[_Utility], AnyStr, AnyStr, AnyStr) -> None
        if not cls._utility:
            cls._allocate()

        byte_key = to_bytes(key)
        byte_message = to_bytearray(message)
        byte_signature = to_bytearray(signature)

        try:
            ret = lib.olm_ed25519_verify(
                cls._utility,
                byte_key,
                len(byte_key),
                ffi.from_buffer(byte_message),
                len(byte_message),
                ffi.from_buffer(byte_signature),
                len(byte_signature)
            )

            cls._check_error(ret, OlmVerifyError)

        finally:
            # clear out copies of the message, which may be a plaintext
            if byte_message is not message:
                for i in range(0, len(byte_message)):
                    byte_message[i] = 0

    @classmethod
    def _sha256(cls, input):
        # type: (Type[_Utility], AnyStr) -> str
        if not cls._utility:
            cls._allocate()

        byte_input = to_bytes(input)
        hash_length = lib.olm_sha256_length(cls._utility)
        hash = ffi.new("char[]", hash_length)

        ret = lib.olm_sha256(cls._utility, byte_input, len(byte_input),
                             hash, hash_length)

        cls._check_error(ret, OlmHashError)

        return ffi.unpack(hash, hash_length).decode()


def ed25519_verify(key, message, signature):
    # type: (AnyStr, AnyStr, AnyStr) -> None
    """Verify an ed25519 signature.

    Raises an OlmVerifyError if verification fails.

    Args:
        key(str): The ed25519 public key used for signing.
        message(str): The signed message.
        signature(bytes): The message signature.
    """
    return _Utility._ed25519_verify(key, message, signature)


def sha256(input_string):
    # type: (AnyStr) -> str
    """Calculate the SHA-256 hash of the input and encodes it as base64.

    Args:
        input_string(str): The input for which the hash will be calculated.

    """
    return _Utility._sha256(input_string)
