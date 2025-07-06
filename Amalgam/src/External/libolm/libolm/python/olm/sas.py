# -*- coding: utf-8 -*-
# libolm python bindings
# Copyright © 2019 Damir Jelić <poljar@termina.org.uk>
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

"""libolm SAS module.

This module contains functions to perform key verification using the Short
Authentication String (SAS) method.

Examples:
    >>> sas = Sas()
    >>> bob_key = "3p7bfXt9wbTTW2HC7OQ1Nz+DQ8hbeGdNrfx+FG+IK08"
    >>> message = "Hello world!"
    >>> extra_info = "MAC"
    >>> sas_alice.set_their_pubkey(bob_key)
    >>> sas_alice.calculate_mac(message, extra_info)
    >>> sas_alice.generate_bytes(extra_info, 5)

"""

from builtins import bytes
from functools import wraps
from typing import Optional

from _libolm import ffi, lib

from ._compat import URANDOM, to_bytearray, to_bytes
from ._finalize import track_for_finalization


def _clear_sas(sas):
    # type: (ffi.cdata) -> None
    lib.olm_clear_sas(sas)


class OlmSasError(Exception):
    """libolm Sas error exception."""


class Sas(object):
    """libolm Short Authenticaton String (SAS) class."""

    def __init__(self, other_users_pubkey=None):
        # type: (Optional[str]) -> None
        """Create a new SAS object.

        Args:
            other_users_pubkey(str, optional): The other users public key, this
                key is necesary to generate bytes for the authentication string
                as well as to calculate the MAC.

        Raises OlmSasError on failure.

        """
        self._buf = ffi.new("char[]", lib.olm_sas_size())
        self._sas = lib.olm_sas(self._buf)
        track_for_finalization(self, self._sas, _clear_sas)

        random_length = lib.olm_create_sas_random_length(self._sas)
        random = URANDOM(random_length)

        self._create_sas(random, random_length)

        if other_users_pubkey:
            self.set_their_pubkey(other_users_pubkey)

    def _create_sas(self, buffer, buffer_length):
        self._check_error(
            lib.olm_create_sas(
                self._sas,
                ffi.from_buffer(buffer),
                buffer_length
            )
        )

    def _check_error(self, ret):
        # type: (int) -> None
        if ret != lib.olm_error():
            return

        last_error = ffi.string((lib.olm_sas_last_error(self._sas))).decode()

        raise OlmSasError(last_error)

    @property
    def pubkey(self):
        # type: () -> str
        """Get the public key for the SAS object.

        This returns the public key of the SAS object that can then be shared
        with another user to perform the authentication process.

        Raises OlmSasError on failure.

        """
        pubkey_length = lib.olm_sas_pubkey_length(self._sas)
        pubkey_buffer = ffi.new("char[]", pubkey_length)

        self._check_error(
            lib.olm_sas_get_pubkey(self._sas, pubkey_buffer, pubkey_length)
        )

        return ffi.unpack(pubkey_buffer, pubkey_length).decode()

    @property
    def other_key_set(self):
        # type: () -> bool
        """Check if the other user's pubkey has been set.
        """
        return lib.olm_sas_is_their_key_set(self._sas) == 1

    def set_their_pubkey(self, key):
        # type: (str) -> None
        """Set the public key of the other user.

        This sets the public key of the other user, it needs to be set before
        bytes can be generated for the authentication string and a MAC can be
        calculated.

        Args:
            key (str): The other users public key.

        Raises OlmSasError on failure.

        """
        byte_key = to_bytearray(key)

        self._check_error(
            lib.olm_sas_set_their_key(
                self._sas,
                ffi.from_buffer(byte_key),
                len(byte_key)
            )
        )

    def generate_bytes(self, extra_info, length):
        # type: (str, int) -> bytes
        """Generate bytes to use for the short authentication string.

        Args:
            extra_info (str): Extra information to mix in when generating the
                bytes.
            length (int): The number of bytes to generate.

        Raises OlmSasError if the other users persons public key isn't set or
        an internal Olm error happens.

        """
        if length < 1:
            raise ValueError("The length needs to be a positive integer value")

        byte_info = to_bytearray(extra_info)
        out_buffer = ffi.new("char[]", length)

        self._check_error(
            lib.olm_sas_generate_bytes(
                self._sas,
                ffi.from_buffer(byte_info),
                len(byte_info),
                out_buffer,
                length
            )
        )

        return ffi.unpack(out_buffer, length)

    def calculate_mac(self, message, extra_info):
        # type: (str, str) -> str
        """Generate a message authentication code based on the shared secret.

        Args:
            message (str): The message to produce the authentication code for.
            extra_info (str): Extra information to mix in when generating the
                MAC

        Raises OlmSasError on failure.

        """
        byte_message = to_bytes(message)
        byte_info = to_bytes(extra_info)

        mac_length = lib.olm_sas_mac_length(self._sas)
        mac_buffer = ffi.new("char[]", mac_length)

        self._check_error(
            lib.olm_sas_calculate_mac(
                self._sas,
                ffi.from_buffer(byte_message),
                len(byte_message),
                ffi.from_buffer(byte_info),
                len(byte_info),
                mac_buffer,
                mac_length
            )
        )
        return ffi.unpack(mac_buffer, mac_length).decode()

    def calculate_mac_fixed_base64(self, message, extra_info):
        # type: (str, str) -> str
        """Generate a message authentication code based on the shared secret.

        This function uses a fixed base64 encoding that is compatible with
        other base64 implementations.

        Args:
            message (str): The message to produce the authentication code for.
            extra_info (str): Extra information to mix in when generating the
                MAC

        Raises OlmSasError on failure.

        """
        byte_message = to_bytes(message)
        byte_info = to_bytes(extra_info)

        mac_length = lib.olm_sas_mac_length(self._sas)
        mac_buffer = ffi.new("char[]", mac_length)

        self._check_error(
            lib.olm_sas_calculate_mac_fixed_base64(
                self._sas,
                ffi.from_buffer(byte_message),
                len(byte_message),
                ffi.from_buffer(byte_info),
                len(byte_info),
                mac_buffer,
                mac_length
            )
        )
        return ffi.unpack(mac_buffer, mac_length).decode()

    def calculate_mac_long_kdf(self, message, extra_info):
        # type: (str, str) -> str
        """Generate a message authentication code based on the shared secret.

        This function should not be used unless compatibility with an older
        non-tagged Olm version is required.

        Args:
            message (str): The message to produce the authentication code for.
            extra_info (str): Extra information to mix in when generating the
                MAC

        Raises OlmSasError on failure.

        """
        byte_message = to_bytes(message)
        byte_info = to_bytes(extra_info)

        mac_length = lib.olm_sas_mac_length(self._sas)
        mac_buffer = ffi.new("char[]", mac_length)

        self._check_error(
            lib.olm_sas_calculate_mac_long_kdf(
                self._sas,
                ffi.from_buffer(byte_message),
                len(byte_message),
                ffi.from_buffer(byte_info),
                len(byte_info),
                mac_buffer,
                mac_length
            )
        )
        return ffi.unpack(mac_buffer, mac_length).decode()
