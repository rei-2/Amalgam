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
"""libolm Account module.

This module contains the account part of the Olm library. It contains a single
Account class which handles the creation of new accounts as well as the storing
and restoring of them.

Examples:
    >>> acc = Account()
    >>> account.identity_keys()
    >>> account.generate_one_time_keys(1)

"""

import json
# pylint: disable=redefined-builtin,unused-import
from builtins import bytes, super
from typing import AnyStr, Dict, Optional, Type

# pylint: disable=no-name-in-module
from _libolm import ffi, lib  # type: ignore

from ._compat import URANDOM, to_bytearray
from ._finalize import track_for_finalization

# This is imported only for type checking purposes
if False:
    from .session import Session  # pragma: no cover


def _clear_account(account):
    # type: (ffi.cdata) -> None
    lib.olm_clear_account(account)


class OlmAccountError(Exception):
    """libolm Account error exception."""


class Account(object):
    """libolm Account class."""

    def __new__(cls):
        # type: (Type[Account]) -> Account
        obj = super().__new__(cls)
        obj._buf = ffi.new("char[]", lib.olm_account_size())
        obj._account = lib.olm_account(obj._buf)
        track_for_finalization(obj, obj._account, _clear_account)
        return obj

    def __init__(self):
        # type: () -> None
        """Create a new Olm account.

        Creates a new account and its matching identity key pair.

        Raises OlmAccountError on failure. If there weren't enough random bytes
        for the account creation the error message for the exception will be
        NOT_ENOUGH_RANDOM.
        """
        # This is needed to silence mypy not knowing the type of _account.
        # There has to be a better way for this.
        if False:  # pragma: no cover
            self._account = self._account  # type: ffi.cdata

        random_length = lib.olm_create_account_random_length(self._account)
        random = URANDOM(random_length)

        self._check_error(
            lib.olm_create_account(self._account, ffi.from_buffer(random),
                                   random_length))


    def _check_error(self, ret):
        # type: (int) -> None
        if ret != lib.olm_error():
            return

        last_error = ffi.string((lib.olm_account_last_error(self._account))).decode()

        raise OlmAccountError(last_error)

    def pickle(self, passphrase=""):
        # type: (Optional[str]) -> bytes
        """Store an Olm account.

        Stores an account as a base64 string. Encrypts the account using the
        supplied passphrase. Returns a byte object containing the base64
        encoded string of the pickled account. Raises OlmAccountError on
        failure.

        Args:
            passphrase(str, optional): The passphrase to be used to encrypt
                the account.
        """
        byte_key = bytearray(passphrase, "utf-8") if passphrase else b""

        pickle_length = lib.olm_pickle_account_length(self._account)
        pickle_buffer = ffi.new("char[]", pickle_length)

        try:
            self._check_error(
                lib.olm_pickle_account(self._account,
                                       ffi.from_buffer(byte_key),
                                       len(byte_key),
                                       pickle_buffer,
                                       pickle_length))
        finally:
            # zero out copies of the passphrase
            for i in range(0, len(byte_key)):
                byte_key[i] = 0

        return ffi.unpack(pickle_buffer, pickle_length)

    @classmethod
    def from_pickle(cls, pickle, passphrase=""):
        # type: (bytes, Optional[str]) -> Account
        """Load a previously stored olm account.

        Loads an account from a pickled base64-encoded string and returns an
        Account object. Decrypts the account using the supplied passphrase.
        Raises OlmAccountError on failure. If the passphrase doesn't match the
        one used to encrypt the account then the error message for the
        exception will be "BAD_ACCOUNT_KEY". If the base64 couldn't be decoded
        then the error message will be "INVALID_BASE64".

        Args:
            pickle(bytes): Base64 encoded byte string containing the pickled
                account
            passphrase(str, optional): The passphrase used to encrypt the
                account.
        """
        if not pickle:
            raise ValueError("Pickle can't be empty")

        byte_key = bytearray(passphrase, "utf-8") if passphrase else b""
        # copy because unpickle will destroy the buffer
        pickle_buffer = ffi.new("char[]", pickle)

        obj = cls.__new__(cls)

        try:
            ret = lib.olm_unpickle_account(obj._account,
                                           ffi.from_buffer(byte_key),
                                           len(byte_key),
                                           pickle_buffer,
                                           len(pickle))
            obj._check_error(ret)
        finally:
            for i in range(0, len(byte_key)):
                byte_key[i] = 0

        return obj

    @property
    def identity_keys(self):
        # type: () -> Dict[str, str]
        """dict: Public part of the identity keys of the account."""
        out_length = lib.olm_account_identity_keys_length(self._account)
        out_buffer = ffi.new("char[]", out_length)

        self._check_error(
            lib.olm_account_identity_keys(self._account, out_buffer,
                                          out_length))
        return json.loads(ffi.unpack(out_buffer, out_length).decode("utf-8"))

    def sign(self, message):
        # type: (AnyStr) -> str
        """Signs a message with this account.

        Signs a message with the private ed25519 identity key of this account.
        Returns the signature.
        Raises OlmAccountError on failure.

        Args:
            message(str): The message to sign.
        """
        bytes_message = to_bytearray(message)
        out_length = lib.olm_account_signature_length(self._account)
        out_buffer = ffi.new("char[]", out_length)

        try:
            self._check_error(
                lib.olm_account_sign(self._account,
                                     ffi.from_buffer(bytes_message),
                                     len(bytes_message), out_buffer,
                                     out_length))
        finally:
            # clear out copies of the message, which may be plaintext
            if bytes_message is not message:
                for i in range(0, len(bytes_message)):
                    bytes_message[i] = 0

        return ffi.unpack(out_buffer, out_length).decode()

    @property
    def max_one_time_keys(self):
        # type: () -> int
        """int: The maximum number of one-time keys the account can store."""
        return lib.olm_account_max_number_of_one_time_keys(self._account)

    def mark_keys_as_published(self):
        # type: () -> None
        """Mark the current set of one-time keys as being published."""
        lib.olm_account_mark_keys_as_published(self._account)

    def generate_one_time_keys(self, count):
        # type: (int) -> None
        """Generate a number of new one-time keys.

        If the total number of keys stored by this account exceeds
        max_one_time_keys() then the old keys are discarded.
        Raises OlmAccountError on error.

        Args:
            count(int): The number of keys to generate.
        """
        random_length = lib.olm_account_generate_one_time_keys_random_length(
            self._account, count)
        random = URANDOM(random_length)

        self._check_error(
            lib.olm_account_generate_one_time_keys(
                self._account, count, ffi.from_buffer(random), random_length))

    @property
    def one_time_keys(self):
        # type: () -> Dict[str, Dict[str, str]]
        """dict: The public part of the one-time keys for this account."""
        out_length = lib.olm_account_one_time_keys_length(self._account)
        out_buffer = ffi.new("char[]", out_length)

        self._check_error(
            lib.olm_account_one_time_keys(self._account, out_buffer,
                                          out_length))

        return json.loads(ffi.unpack(out_buffer, out_length).decode("utf-8"))

    def remove_one_time_keys(self, session):
        # type: (Session) -> None
        """Remove used one-time keys.

        Removes the one-time keys that the session used from the account.
        Raises OlmAccountError on failure. If the account doesn't have any
        matching one-time keys then the error message of the exception will be
        "BAD_MESSAGE_KEY_ID".

        Args:
            session(Session): An Olm Session object that was created with this
            account.
        """
        self._check_error(lib.olm_remove_one_time_keys(self._account,
                                                       session._session))

    def generate_fallback_key(self):
        # type: () -> None
        """Generate a new fallback key

        This will overwrite the existing fallback key, make sure that you upload
        the fallback key before rotating again. Internally there are two slots
        for the private part of the fallback key. Rotating without uploading
        means that we'll remove a fallback key that may be still used on the
        server side but has been removed on our side.

        When we receive pre-key messages that use such a removed fallback key we
        won't be able to create a new Olm session.
        """
        random_length = lib.olm_account_generate_fallback_key_random_length(
            self._account
        )
        random = URANDOM(random_length)

        self._check_error(
            lib.olm_account_generate_fallback_key(
                self._account, ffi.from_buffer(random), random_length
            )
        )

    def forget_old_fallback_key(self):
        """Forget about the old fallback key.

        This should be called once you are reasonably certain that you will not
        receive any more messages that use the old fallback key (e.g. 5 minutes
        after the new fallback key has been published).
        """
        lib.olm_account_forget_old_fallback_key(self._account)

    @property
    def fallback_key(self):
        """The public part of the current fallback for this account.

        The fallback key can be uploaded alongside of the one-time keys. It can
        be used instead of a one-time key to establish a new Olm Session. The
        fallback key comes into play if all one-time keys have been used up due
        to the client being offline and not replenishing the pool of one-time
        keys.
        """
        out_length = lib.olm_account_unpublished_fallback_key_length(self._account)
        out_buffer = ffi.new("char[]", out_length)

        ret = lib.olm_account_unpublished_fallback_key(self._account, out_buffer, out_length)
        self._check_error(ret)

        fallback_key = ffi.unpack(out_buffer, out_length)

        return json.loads(fallback_key)
