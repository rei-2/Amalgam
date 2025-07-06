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
"""libolm Group session module.

This module contains the group session part of the Olm library. It contains two
classes for creating inbound and outbound group sessions.

Examples:
    >>> outbound = OutboundGroupSession()
    >>> InboundGroupSession(outbound.session_key)
"""

# pylint: disable=redefined-builtin,unused-import
from builtins import bytes, super
from typing import AnyStr, Optional, Tuple, Type

# pylint: disable=no-name-in-module
from _libolm import ffi, lib  # type: ignore

from ._compat import URANDOM, to_bytearray, to_bytes, to_unicode_str
from ._finalize import track_for_finalization


def _clear_inbound_group_session(session):
    # type: (ffi.cdata) -> None
    lib.olm_clear_inbound_group_session(session)


def _clear_outbound_group_session(session):
    # type: (ffi.cdata) -> None
    lib.olm_clear_outbound_group_session(session)


class OlmGroupSessionError(Exception):
    """libolm Group session error exception."""


class InboundGroupSession(object):
    """Inbound group session for encrypted multiuser communication."""

    def __new__(
        cls,              # type: Type[InboundGroupSession]
        session_key=None  # type: Optional[str]
    ):
        # type: (...) -> InboundGroupSession
        obj = super().__new__(cls)
        obj._buf = ffi.new("char[]", lib.olm_inbound_group_session_size())
        obj._session = lib.olm_inbound_group_session(obj._buf)
        track_for_finalization(obj, obj._session, _clear_inbound_group_session)
        return obj

    def __init__(self, session_key):
        # type: (AnyStr) -> None
        """Create a new inbound group session.
        Start a new inbound group session, from a key exported from
        an outbound group session.

        Raises OlmGroupSessionError on failure. The error message of the
        exception will be "OLM_INVALID_BASE64" if the session key is not valid
        base64 and "OLM_BAD_SESSION_KEY" if the session key is invalid.
        """
        if False:  # pragma: no cover
            self._session = self._session  # type: ffi.cdata

        byte_session_key = to_bytearray(session_key)

        try:
            ret = lib.olm_init_inbound_group_session(
                self._session,
                ffi.from_buffer(byte_session_key), len(byte_session_key)
            )
        finally:
            if byte_session_key is not session_key:
                for i in range(0, len(byte_session_key)):
                    byte_session_key[i] = 0
        self._check_error(ret)

    def pickle(self, passphrase=""):
        # type: (Optional[str]) -> bytes
        """Store an inbound group session.

        Stores a group session as a base64 string. Encrypts the session using
        the supplied passphrase. Returns a byte object containing the base64
        encoded string of the pickled session.

        Args:
            passphrase(str, optional): The passphrase to be used to encrypt
                the session.
        """
        byte_passphrase = bytearray(passphrase, "utf-8") if passphrase else b""

        pickle_length = lib.olm_pickle_inbound_group_session_length(
            self._session)
        pickle_buffer = ffi.new("char[]", pickle_length)

        try:
            ret = lib.olm_pickle_inbound_group_session(
                self._session,
                ffi.from_buffer(byte_passphrase), len(byte_passphrase),
                pickle_buffer, pickle_length
            )
            self._check_error(ret)
        finally:
            # clear out copies of the passphrase
            for i in range(0, len(byte_passphrase)):
                    byte_passphrase[i] = 0

        return ffi.unpack(pickle_buffer, pickle_length)

    @classmethod
    def from_pickle(cls, pickle, passphrase=""):
        # type: (bytes, Optional[str]) -> InboundGroupSession
        """Load a previously stored inbound group session.

        Loads an inbound group session from a pickled base64 string and returns
        an InboundGroupSession object. Decrypts the session using the supplied
        passphrase. Raises OlmSessionError on failure. If the passphrase
        doesn't match the one used to encrypt the session then the error
        message for the exception will be "BAD_ACCOUNT_KEY". If the base64
        couldn't be decoded then the error message will be "INVALID_BASE64".

        Args:
            pickle(bytes): Base64 encoded byte string containing the pickled
                session
            passphrase(str, optional): The passphrase used to encrypt the
                session
        """
        if not pickle:
            raise ValueError("Pickle can't be empty")

        byte_passphrase = bytearray(passphrase, "utf-8") if passphrase else b""
        # copy because unpickle will destroy the buffer
        pickle_buffer = ffi.new("char[]", pickle)

        obj = cls.__new__(cls)

        try:
            ret = lib.olm_unpickle_inbound_group_session(
                obj._session,
                ffi.from_buffer(byte_passphrase),
                len(byte_passphrase),
                pickle_buffer,
                len(pickle)
            )
            obj._check_error(ret)
        finally:
            # clear out copies of the passphrase
            for i in range(0, len(byte_passphrase)):
                    byte_passphrase[i] = 0

        return obj

    def _check_error(self, ret):
        # type: (int) -> None
        if ret != lib.olm_error():
            return

        last_error = ffi.string(
            lib.olm_inbound_group_session_last_error(self._session)
        ).decode()

        raise OlmGroupSessionError(last_error)

    def decrypt(self, ciphertext, unicode_errors="replace"):
        # type: (AnyStr, str) -> Tuple[str, int]
        """Decrypt a message

        Returns a tuple of the decrypted plain-text and the message index of
        the decrypted message or raises OlmGroupSessionError on failure.
        On failure the error message of the exception  will be:

        * OLM_INVALID_BASE64         if the message is not valid base64
        * OLM_BAD_MESSAGE_VERSION    if the message was encrypted with an
            unsupported version of the protocol
        * OLM_BAD_MESSAGE_FORMAT     if the message headers could not be
            decoded
        * OLM_BAD_MESSAGE_MAC        if the message could not be verified
        * OLM_UNKNOWN_MESSAGE_INDEX  if we do not have a session key
            corresponding to the message's index (i.e., it was sent before
            the session key was shared with us)

        Args:
            ciphertext(str): Base64 encoded ciphertext containing the encrypted
                message
            unicode_errors(str, optional): The error handling scheme to use for
                unicode decoding errors. The default is "replace" meaning that
                the character that was unable to decode will be replaced with
                the unicode replacement character (U+FFFD). Other possible
                values are "strict", "ignore" and "xmlcharrefreplace" as well
                as any other name registered with codecs.register_error that
                can handle UnicodeEncodeErrors.
        """
        if not ciphertext:
            raise ValueError("Ciphertext can't be empty.")

        byte_ciphertext = to_bytes(ciphertext)

        # copy because max_plaintext_length will destroy the buffer
        ciphertext_buffer = ffi.new("char[]", byte_ciphertext)

        max_plaintext_length = lib.olm_group_decrypt_max_plaintext_length(
            self._session, ciphertext_buffer, len(byte_ciphertext)
        )
        self._check_error(max_plaintext_length)
        plaintext_buffer = ffi.new("char[]", max_plaintext_length)
        # copy because max_plaintext_length will destroy the buffer
        ciphertext_buffer = ffi.new("char[]", byte_ciphertext)

        message_index = ffi.new("uint32_t*")
        plaintext_length = lib.olm_group_decrypt(
            self._session, ciphertext_buffer, len(byte_ciphertext),
            plaintext_buffer, max_plaintext_length,
            message_index
        )

        self._check_error(plaintext_length)

        plaintext = to_unicode_str(
            ffi.unpack(plaintext_buffer, plaintext_length),
            errors=unicode_errors
        )

        # clear out copies of the plaintext
        lib.memset(plaintext_buffer, 0, max_plaintext_length)

        return plaintext, message_index[0]

    @property
    def id(self):
        # type: () -> str
        """str: A base64 encoded identifier for this session."""
        id_length = lib.olm_inbound_group_session_id_length(self._session)
        id_buffer = ffi.new("char[]", id_length)
        ret = lib.olm_inbound_group_session_id(
            self._session,
            id_buffer,
            id_length
        )
        self._check_error(ret)
        return ffi.unpack(id_buffer, id_length).decode()

    @property
    def first_known_index(self):
        # type: () -> int
        """int: The first message index we know how to decrypt."""
        return lib.olm_inbound_group_session_first_known_index(self._session)

    def export_session(self, message_index):
        # type: (int) -> str
        """Export an inbound group session

        Export the base64-encoded ratchet key for this session, at the given
        index, in a format which can be used by import_session().

        Raises OlmGroupSessionError on failure. The error message for the
        exception will be:

        * OLM_UNKNOWN_MESSAGE_INDEX if we do not have a session key
            corresponding to the given index (ie, it was sent before the
            session key was shared with us)

        Args:
            message_index(int): The message index at which the session should
                be exported.
        """

        export_length = lib.olm_export_inbound_group_session_length(
            self._session)

        export_buffer = ffi.new("char[]", export_length)
        ret = lib.olm_export_inbound_group_session(
            self._session,
            export_buffer,
            export_length,
            message_index
        )
        self._check_error(ret)
        export_str = ffi.unpack(export_buffer, export_length).decode()

        # clear out copies of the key
        lib.memset(export_buffer, 0, export_length)

        return export_str

    @classmethod
    def import_session(cls, session_key):
        # type: (AnyStr) -> InboundGroupSession
        """Create an InboundGroupSession from an exported session key.

        Creates an InboundGroupSession with an previously exported session key,
        raises OlmGroupSessionError on failure. The error message for the
        exception will be:

        * OLM_INVALID_BASE64  if the session_key is not valid base64
        * OLM_BAD_SESSION_KEY if the session_key is invalid

        Args:
            session_key(str): The exported session key with which the inbound
                group session will be created
        """
        obj = cls.__new__(cls)

        byte_session_key = to_bytearray(session_key)

        try:
            ret = lib.olm_import_inbound_group_session(
                obj._session,
                ffi.from_buffer(byte_session_key),
                len(byte_session_key)
            )
            obj._check_error(ret)
        finally:
            # clear out copies of the key
            if byte_session_key is not session_key:
                for i in range(0, len(byte_session_key)):
                    byte_session_key[i] = 0

        return obj


class OutboundGroupSession(object):
    """Outbound group session for encrypted multiuser communication."""

    def __new__(cls):
        # type: (Type[OutboundGroupSession]) -> OutboundGroupSession
        obj = super().__new__(cls)
        obj._buf = ffi.new("char[]", lib.olm_outbound_group_session_size())
        obj._session = lib.olm_outbound_group_session(obj._buf)
        track_for_finalization(
            obj,
            obj._session,
            _clear_outbound_group_session
        )
        return obj

    def __init__(self):
        # type: () -> None
        """Create a new outbound group session.

        Start a new outbound group session. Raises OlmGroupSessionError on
        failure.
        """
        if False:  # pragma: no cover
            self._session = self._session  # type: ffi.cdata

        random_length = lib.olm_init_outbound_group_session_random_length(
            self._session
        )
        random = URANDOM(random_length)

        ret = lib.olm_init_outbound_group_session(
            self._session, ffi.from_buffer(random), random_length
        )
        self._check_error(ret)

    def _check_error(self, ret):
        # type: (int) -> None
        if ret != lib.olm_error():
            return

        last_error = ffi.string(
            lib.olm_outbound_group_session_last_error(self._session)
        ).decode()

        raise OlmGroupSessionError(last_error)

    def pickle(self, passphrase=""):
        # type: (Optional[str]) -> bytes
        """Store an outbound group session.

        Stores a group session as a base64 string. Encrypts the session using
        the supplied passphrase. Returns a byte object containing the base64
        encoded string of the pickled session.

        Args:
            passphrase(str, optional): The passphrase to be used to encrypt
                the session.
        """
        byte_passphrase = bytearray(passphrase, "utf-8") if passphrase else b""
        pickle_length = lib.olm_pickle_outbound_group_session_length(
            self._session)
        pickle_buffer = ffi.new("char[]", pickle_length)

        try:
            ret = lib.olm_pickle_outbound_group_session(
                self._session,
                ffi.from_buffer(byte_passphrase), len(byte_passphrase),
                pickle_buffer, pickle_length
            )
            self._check_error(ret)
        finally:
            # clear out copies of the passphrase
            for i in range(0, len(byte_passphrase)):
                    byte_passphrase[i] = 0

        return ffi.unpack(pickle_buffer, pickle_length)

    @classmethod
    def from_pickle(cls, pickle, passphrase=""):
        # type: (bytes, Optional[str]) -> OutboundGroupSession
        """Load a previously stored outbound group session.

        Loads an outbound group session from a pickled base64 string and
        returns an OutboundGroupSession object. Decrypts the session using the
        supplied passphrase. Raises OlmSessionError on failure. If the
        passphrase doesn't match the one used to encrypt the session then the
        error message for the exception will be "BAD_ACCOUNT_KEY". If the
        base64 couldn't be decoded then the error message will be
        "INVALID_BASE64".

        Args:
            pickle(bytes): Base64 encoded byte string containing the pickled
                session
            passphrase(str, optional): The passphrase used to encrypt the
        """
        if not pickle:
            raise ValueError("Pickle can't be empty")

        byte_passphrase = bytearray(passphrase, "utf-8") if passphrase else b""
        # copy because unpickle will destroy the buffer
        pickle_buffer = ffi.new("char[]", pickle)

        obj = cls.__new__(cls)

        try:
            ret = lib.olm_unpickle_outbound_group_session(
                obj._session,
                ffi.from_buffer(byte_passphrase),
                len(byte_passphrase),
                pickle_buffer,
                len(pickle)
            )
            obj._check_error(ret)
        finally:
            # clear out copies of the passphrase
            for i in range(0, len(byte_passphrase)):
                    byte_passphrase[i] = 0

        return obj

    def encrypt(self, plaintext):
        # type: (AnyStr) -> str
        """Encrypt a message.

        Returns the encrypted ciphertext.

        Args:
            plaintext(str): A string that will be encrypted using the group
                session.
        """
        byte_plaintext = to_bytearray(plaintext)
        message_length = lib.olm_group_encrypt_message_length(
            self._session, len(byte_plaintext)
        )

        message_buffer = ffi.new("char[]", message_length)

        try:
            ret = lib.olm_group_encrypt(
                self._session,
                ffi.from_buffer(byte_plaintext), len(byte_plaintext),
                message_buffer, message_length,
            )
            self._check_error(ret)
        finally:
            # clear out copies of plaintext
            if byte_plaintext is not plaintext:
                for i in range(0, len(byte_plaintext)):
                    byte_plaintext[i] = 0

        return ffi.unpack(message_buffer, message_length).decode()

    @property
    def id(self):
        # type: () -> str
        """str: A base64 encoded identifier for this session."""
        id_length = lib.olm_outbound_group_session_id_length(self._session)
        id_buffer = ffi.new("char[]", id_length)

        ret = lib.olm_outbound_group_session_id(
            self._session,
            id_buffer,
            id_length
        )
        self._check_error(ret)

        return ffi.unpack(id_buffer, id_length).decode()

    @property
    def message_index(self):
        # type: () -> int
        """int: The current message index of the session.

        Each message is encrypted with an increasing index. This is the index
        for the next message.
        """
        return lib.olm_outbound_group_session_message_index(self._session)

    @property
    def session_key(self):
        # type: () -> str
        """The base64-encoded current ratchet key for this session.

        Each message is encrypted with a different ratchet key. This function
        returns the ratchet key that will be used for the next message.
        """
        key_length = lib.olm_outbound_group_session_key_length(self._session)
        key_buffer = ffi.new("char[]", key_length)

        ret = lib.olm_outbound_group_session_key(
            self._session,
            key_buffer,
            key_length
        )
        self._check_error(ret)

        return ffi.unpack(key_buffer, key_length).decode()
