python-olm
==========

Python bindings for Olm.

The specification of the Olm cryptographic ratchet which is used for peer to
peer sessions of this library can be found [here][4].

The specification of the Megolm cryptographic ratchet which is used for group
sessions of this library can be found [here][5].

An example of the implementation of the Olm and Megolm cryptographic protocol
can be found in the Matrix protocol for which the implementation guide can be
found [here][6].

The full API reference can be found [here][7].

# Installation instructions

To install from the source package, you will need:

- cmake (recommended) or GNU make
- a C/C++ compiler

You can then run `pip install python-olm`.

This should work in UNIX-like environments, including macOS, and may work in
other environments too, but is known to not work yet in Windows.

# Accounts

Accounts create and hold the central identity of the Olm protocol, they consist of a fingerprint and identity
key pair. They also produce one time keys that are used to start peer to peer
encrypted communication channels.

## Account Creation

A new account is created with the Account class, it creates a new Olm key pair.
The public parts of the key pair are available using the identity_keys property
of the class.

```python
>>> alice = Account()
>>> alice.identity_keys
{'curve25519': '2PytGagXercwHjzQETLcMa3JOsaU2qkPIESaqoi59zE',
 'ed25519': 'HHpOuFYdHwoa54GxSttz9YmaTmbuVU3js92UTUjYJgM'}
```


## One Time keys

One time keys need to be generated before people can start an encrypted peer to
peer channel to an account.

```python
>>> alice.generate_one_time_keys(1)
>>> alice.one_time_keys
{'curve25519': {'AAAAAQ': 'KiHoW6CIy905UC4V1Frmwr3VW8bTWkBL4uWtWFFllxM'}}
```

After the one time keys are published they should be marked as such so they
aren't reused.

```python
>>> alice.mark_keys_as_published()
>>> alice.one_time_keys
{'curve25519': {}}
```

## Pickling

Accounts should be stored for later reuse, storing an account is done with the
pickle method while the restoring step is done with the from_pickle class
method.

```python
>>> pickle = alice.pickle()
>>> restored = Account.from_pickle(pickle)
```

# Sessions

Sessions are used to create an encrypted peer to peer communication channel
between two accounts.

## Session Creation
```python
>>> alice = Account()
>>> bob = Account()
>>> bob.generate_one_time_keys(1)
>>> id_key = bob.identity_keys["curve25519"]
>>> one_time = list(bob.one_time_keys["curve25519"].values())[0]
>>> alice_session = OutboundSession(alice, id_key, one_time)
```

## Encryption

After an outbound session is created an encrypted message can be exchanged:

```python
>>> message = alice_session.encrypt("It's a secret to everybody")
>>> message.ciphertext
'AwogkL7RoakT9gnjcZMra+y39WXKRmnxBPEaEp6OSueIA0cSIJxGpBoP8YZ+CGweXQ10LujbXMgK88
xG/JZMQJ5ulK9ZGiC8TYrezNYr3qyIBLlecXr/9wnegvJaSFDmWDVOcf4XfyI/AwogqIZfAklRXGC5b
ZJcZxVxQGgJ8Dz4OQII8k0Dp8msUXwQACIQvagY1dO55Qvnk5PZ2GF+wdKnvj6Zxl2g'
>>> message.message_type
0
```

After the message is transfered, bob can create an InboundSession to decrypt the
message.

```python
>>> bob_session = InboundSession(bob, message)
>>> bob_session.decrypt(message)
"It's a secret to everybody"
```

## Pickling

Sessions like accounts can be stored for later use the API is the same as for
accounts.

```python
>>> pickle = session.pickle()
>>> restored = Session.from_pickle(pickle)
```

# Group Sessions

Group Sessions are used to create a one-to-many encrypted communication channel.
The group session key needs to be shared with all participants that should be able
to decrypt the group messages. Another thing to notice is that, since the group
session key is ratcheted every time a message is encrypted, the session key should
be shared before any messages are encrypted.

## Group Session Creation

Group sessions aren't bound to an account like peer-to-peer sessions so their
creation is straightforward.

```python
>>> alice_group = OutboundGroupSession()
>>> bob_inbound_group = InboundGroupSession(alice_group.session_key)
```

## Group Encryption

Group encryption is pretty simple. The important part is to share the session
key with all participants over a secure channel (e.g. peer-to-peer Olm
sessions).

```python
>>> message = alice_group.encrypt("It's a secret to everybody")
>>> bob_inbound_group.decrypt(message)
("It's a secret to everybody", 0)
```

## Pickling

Pickling works the same way as for peer-to-peer Olm sessions.

```python
>>> pickle = session.pickle()
>>> restored = InboundGroupSession.from_pickle(pickle)
```
[1]: https://git.matrix.org/git/olm/about/
[2]: https://git.matrix.org/git/olm/tree/python?id=f8c61b8f8432d0b0b38d57f513c5048fb42f22ab
[3]: https://cffi.readthedocs.io/en/latest/
[4]: https://git.matrix.org/git/olm/about/docs/olm.rst
[5]: https://git.matrix.org/git/olm/about/docs/megolm.rst
[6]: https://matrix.org/docs/guides/end-to-end-encryption-implementation-guide
[7]: https://poljar.github.io/python-olm/html/index.html
