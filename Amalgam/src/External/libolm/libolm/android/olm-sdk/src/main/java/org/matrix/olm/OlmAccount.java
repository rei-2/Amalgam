/*
 * Copyright 2017 OpenMarket Ltd
 * Copyright 2017 Vector Creations Ltd
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.matrix.olm;

import android.text.TextUtils;
import android.util.Log;

import org.json.JSONObject;

import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.Serializable;
import java.util.Arrays;
import java.util.Map;

/**
 * Account class used to create Olm sessions in conjunction with {@link OlmSession} class.<br>
 * OlmAccount provides APIs to retrieve the Olm keys.
 *<br><br>Detailed implementation guide is available at <a href="http://matrix.org/docs/guides/e2e_implementation.html">Implementing End-to-End Encryption in Matrix clients</a>.
 */
public class OlmAccount extends CommonSerializeUtils implements Serializable {
    private static final long serialVersionUID = 3497486121598434824L;
    private static final String LOG_TAG = "OlmAccount";

    // JSON keys used in the JSON objects returned by JNI
    /** As well as the identity key, each device creates a number of Curve25519 key pairs which are
     also used to establish Olm sessions, but can only be used once. Once again, the private part
     remains on the device. but the public part is published to the Matrix network **/
    public static final String JSON_KEY_ONE_TIME_KEY = "curve25519";

    /** Curve25519 identity key is a public-key cryptographic system which can be used to establish a shared
     secret.<br>In Matrix, each device has a long-lived Curve25519 identity key which is used to establish
     Olm sessions with that device. The private key should never leave the device, but the
     public part is signed with the Ed25519 fingerprint key ({@link #JSON_KEY_FINGER_PRINT_KEY}) and published to the network. **/
    public static final String JSON_KEY_IDENTITY_KEY = "curve25519";

    /** Ed25519 finger print is a public-key cryptographic system for signing messages.<br>In Matrix, each device has
     an Ed25519 key pair which serves to identify that device. The private the key should
     never leave the device, but the public part is published to the Matrix network. **/
    public static final String JSON_KEY_FINGER_PRINT_KEY = "ed25519";

    /** Account Id returned by JNI.
     * This value identifies uniquely the native account instance.
     */
    private transient long mNativeId;

    public OlmAccount() throws OlmException {
        try {
            mNativeId = createNewAccountJni();
        } catch (Exception e) {
            throw new OlmException(OlmException.EXCEPTION_CODE_INIT_ACCOUNT_CREATION, e.getMessage());
        }
    }

    /**
     * Create a new account and return it to JAVA side.<br>
     * Since a C prt is returned as a jlong, special care will be taken
     * to make the cast (OlmAccount* to jlong) platform independent.
     * @return the initialized OlmAccount* instance or throw an exception if fails
     **/
    private native long createNewAccountJni();

    /**
     * Getter on the account ID.
     * @return native account ID
     */
    long getOlmAccountId(){
        return mNativeId;
    }

    /**
     * Release native account and invalid its JAVA reference counter part.<br>
     * Public API for {@link #releaseAccountJni()}.
     */
    public void releaseAccount() {
        if (0 != mNativeId) {
            releaseAccountJni();
        }
        mNativeId = 0;
    }

    /**
     * Destroy the corresponding OLM account native object.<br>
     * This method must ALWAYS be called when this JAVA instance
     * is destroyed (ie. garbage collected) to prevent memory leak in native side.
     * See {@link #createNewAccountJni()}.
     */
    private native void releaseAccountJni();

    /**
     * Return true the object resources have been released.<br>
     * @return true the object resources have been released
     */
    public boolean isReleased() {
        return (0 == mNativeId);
    }

    /**
     * Return the identity keys (identity and fingerprint keys) in a dictionary.<br>
     * Public API for {@link #identityKeysJni()}.<br>
     * Ex:<code>
     * {
     *  "curve25519":"Vam++zZPMqDQM6ANKpO/uAl5ViJSHxV9hd+b0/fwRAg",
     *  "ed25519":"+v8SOlOASFTMrX3MCKBM4iVnYoZ+JIjpNt1fi8Z9O2I"
     * }</code>
     * @return identity keys dictionary if operation succeeds, null otherwise
     * @exception OlmException the failure reason
     */
    public Map<String, String> identityKeys() throws OlmException {
        JSONObject identityKeysJsonObj = null;

        byte[] identityKeysBuffer;

        try {
            identityKeysBuffer = identityKeysJni();
        } catch (Exception e) {
            Log.e(LOG_TAG, "## identityKeys(): Failure - " + e.getMessage());
            throw new OlmException(OlmException.EXCEPTION_CODE_ACCOUNT_IDENTITY_KEYS, e.getMessage());
        }

        if (null != identityKeysBuffer) {
            try {
                identityKeysJsonObj = new JSONObject(new String(identityKeysBuffer, "UTF-8"));
            } catch (Exception e) {
                Log.e(LOG_TAG, "## identityKeys(): Exception - Msg=" + e.getMessage());
            }
        } else {
            Log.e(LOG_TAG, "## identityKeys(): Failure - identityKeysJni()=null");
        }

        return OlmUtility.toStringMap(identityKeysJsonObj);
    }

    /**
     * Get the public identity keys (Ed25519 fingerprint key and Curve25519 identity key).<br>
     * Keys are Base64 encoded.
     * These keys must be published on the server.
     * @return the identity keys or throw an exception if it fails
     */
    private native byte[] identityKeysJni();

    /**
     * Return the largest number of "one time keys" this account can store.
     * @return the max number of "one time keys", -1 otherwise
     */
    public long maxOneTimeKeys() {
        return maxOneTimeKeysJni();
    }

    /**
     * Return the largest number of "one time keys" this account can store.
     * @return the max number of "one time keys", -1 otherwise
     */
    private native long maxOneTimeKeysJni();

    /**
     * Generate a number of new one time keys.<br> If total number of keys stored
     * by this account exceeds {@link #maxOneTimeKeys()}, the old keys are discarded.<br>
     * The corresponding keys are retrieved by {@link #oneTimeKeys()}.
     * @param aNumberOfKeys number of keys to generate
     * @exception OlmException the failure reason
     */
    public void generateOneTimeKeys(int aNumberOfKeys) throws OlmException {
        try {
            generateOneTimeKeysJni(aNumberOfKeys);
        } catch (Exception e) {
            throw new OlmException(OlmException.EXCEPTION_CODE_ACCOUNT_GENERATE_ONE_TIME_KEYS, e.getMessage());
        }
    }

    /**
     * Generate a number of new one time keys.<br> If total number of keys stored
     * by this account exceeds {@link #maxOneTimeKeys()}, the old keys are discarded.
     * An exception is thrown if the operation fails.<br>
     * @param aNumberOfKeys number of keys to generate
     */
    private native void generateOneTimeKeysJni(int aNumberOfKeys);

    /**
     * Return the "one time keys" in a dictionary.<br>
     * The number of "one time keys", is specified by {@link #generateOneTimeKeys(int)}<br>
     * Ex:<code>
     * { "curve25519":
     *  {
     *      "AAAABQ":"qefVZd8qvjOpsFzoKSAdfUnJVkIreyxWFlipCHjSQQg",
     *      "AAAABA":"/X8szMU+p+lsTnr56wKjaLgjTMQQkCk8EIWEAilZtQ8",
     *      "AAAAAw":"qxNxxFHzevFntaaPdT0fhhO7tc7pco4+xB/5VRG81hA",
     *  }
     * }</code><br>
     * Public API for {@link #oneTimeKeysJni()}.<br>
     * Note: these keys are to be published on the server.
     * @return one time keys in string dictionary.
     * @exception OlmException the failure reason
     */
    public Map<String, Map<String, String>> oneTimeKeys() throws OlmException {
        JSONObject oneTimeKeysJsonObj = null;
        byte[] oneTimeKeysBuffer;

        try {
            oneTimeKeysBuffer = oneTimeKeysJni();
        } catch (Exception e) {
            throw new OlmException(OlmException.EXCEPTION_CODE_ACCOUNT_ONE_TIME_KEYS, e.getMessage());
        }

        if( null != oneTimeKeysBuffer) {
            try {
                oneTimeKeysJsonObj = new JSONObject(new String(oneTimeKeysBuffer, "UTF-8"));
            } catch (Exception e) {
                Log.e(LOG_TAG, "## oneTimeKeys(): Exception - Msg=" + e.getMessage());
            }
        } else {
            Log.e(LOG_TAG, "## oneTimeKeys(): Failure - identityKeysJni()=null");
        }

        return OlmUtility.toStringMapMap(oneTimeKeysJsonObj);
    }

    /**
     * Get the public parts of the unpublished "one time keys" for the account.<br>
     * The returned data is a JSON-formatted object with the single property
     * <code>curve25519</code>, which is itself an object mapping key id to
     * base64-encoded Curve25519 key.<br>
     * @return byte array containing the one time keys or throw an exception if it fails
     */
    private native byte[] oneTimeKeysJni();

    /**
     * Remove the "one time keys" that the session used from the account.
     * @param aSession session instance
     * @throws OlmException the failure reason
     */
    public void removeOneTimeKeys(OlmSession aSession) throws OlmException {
        if (null != aSession) {
            try {
                removeOneTimeKeysJni(aSession.getOlmSessionId());
            } catch (Exception e) {
                throw new OlmException(OlmException.EXCEPTION_CODE_ACCOUNT_REMOVE_ONE_TIME_KEYS, e.getMessage());
            }
        }
    }

    /**
     * Remove the "one time keys" that the session used from the account.
     * An exception is thrown if the operation fails.
     * @param aNativeOlmSessionId native session instance identifier
     */
    private native void removeOneTimeKeysJni(long aNativeOlmSessionId);

    /**
     * Marks the current set of "one time keys" as being published.
     * @exception OlmException the failure reason
     */
    public void markOneTimeKeysAsPublished() throws OlmException {
        try {
            markOneTimeKeysAsPublishedJni();
        } catch (Exception e) {
            throw new OlmException(OlmException.EXCEPTION_CODE_ACCOUNT_MARK_ONE_KEYS_AS_PUBLISHED, e.getMessage());
        }
    }

    /**
     * Marks the current set of "one time keys" as being published.
     * An exception is thrown if the operation fails.
     */
    private native void markOneTimeKeysAsPublishedJni();

    /**
     * Sign a message with the ed25519 fingerprint key for this account.<br>
     * The signed message is returned by the method.
     * @param aMessage message to sign
     * @return the signed message
     * @exception OlmException the failure reason
     */
    public String signMessage(String aMessage) throws OlmException {
        String result = null;

        if (null != aMessage) {
            byte[] utf8String = null;
            try {
                utf8String = aMessage.getBytes("UTF-8");
                if (null != utf8String) {
                    byte[] signedMessage = signMessageJni(utf8String);

                    if (null != signedMessage) {
                        result = new String(signedMessage, "UTF-8");
                    }
                }
            } catch (Exception e) {
                throw new OlmException(OlmException.EXCEPTION_CODE_ACCOUNT_SIGN_MESSAGE, e.getMessage());
            } finally {
                if (null != utf8String) {
                    Arrays.fill(utf8String, (byte) 0);
                }
            }
        }

        return result;
    }

    /**
     * Sign a message with the ed25519 fingerprint key for this account.<br>
     * The signed message is returned by the method.
     * @param aMessage message to sign
     * @return the signed message
     */
    private native byte[] signMessageJni(byte[] aMessage);

    //==============================================================================================================
    // Serialization management
    //==============================================================================================================

    /**
     * Kick off the serialization mechanism.
     * @param aOutStream output stream for serializing
     * @throws IOException exception
     */
    private void writeObject(ObjectOutputStream aOutStream) throws IOException {
        serialize(aOutStream);
    }

    /**
     * Kick off the deserialization mechanism.
     * @param aInStream input stream
     * @throws Exception exception
     */
    private void readObject(ObjectInputStream aInStream) throws Exception {
        deserialize(aInStream);
    }

    /**
     * Return an account as a bytes buffer.<br>
     * The account is serialized and encrypted with aKey.
     * In case of failure, an error human readable
     * description is provide in aErrorMsg.
     * @param aKey encryption key
     * @param aErrorMsg error message description
     * @return the account as bytes buffer
     */
    @Override
    protected byte[] serialize(byte[] aKey, StringBuffer aErrorMsg) {
        byte[] pickleRetValue = null;

        // sanity check
        if(null == aErrorMsg) {
            Log.e(LOG_TAG,"## serialize(): invalid parameter - aErrorMsg=null");
        } else if (null ==  aKey) {
            aErrorMsg.append("Invalid input parameters in serializeDataWithKey()");
        } else {
            aErrorMsg.setLength(0);
            try {
                pickleRetValue = serializeJni(aKey);
            } catch (Exception e) {
                Log.e(LOG_TAG, "## serialize() failed " + e.getMessage());
                aErrorMsg.append(e.getMessage());
            }
        }

        return pickleRetValue;
    }

    /**
     * Serialize and encrypt account instance.<br>
     * @param aKeyBuffer key used to encrypt the serialized account data
     * @return the serialised account as bytes buffer.
     **/
    private native byte[] serializeJni(byte[] aKeyBuffer);

    /**
     * Loads an account from a pickled bytes buffer.<br>
     * See {@link #serialize(byte[], StringBuffer)}
     * @param aSerializedData bytes buffer
     * @param aKey key used to encrypted
     * @exception Exception the exception
     */
    @Override
    protected void deserialize(byte[] aSerializedData, byte[] aKey) throws Exception {
        String errorMsg = null;

        try {
            if ((null == aSerializedData) || (null == aKey)) {
                Log.e(LOG_TAG, "## deserialize(): invalid input parameters");
                errorMsg = "invalid input parameters";
            } else {
                mNativeId = deserializeJni(aSerializedData, aKey);
            }
        } catch (Exception e) {
            Log.e(LOG_TAG, "## deserialize() failed " + e.getMessage());
            errorMsg = e.getMessage();
        }

        if (!TextUtils.isEmpty(errorMsg)) {
            releaseAccount();
            throw new OlmException(OlmException.EXCEPTION_CODE_ACCOUNT_DESERIALIZATION, errorMsg);
        }
    }

    /**
     * Allocate a new account and initialize it with the serialisation data.<br>
     * @param aSerializedDataBuffer the account serialisation buffer
     * @param aKeyBuffer the key used to encrypt the serialized account data
     * @return the deserialized account
     **/
    private native long deserializeJni(byte[] aSerializedDataBuffer, byte[] aKeyBuffer);

    /**
     * Return a pickled account as a bytes buffer.<br>
     * The account is serialized and encrypted with aKey.
     * In case of failure, an error human readable
     * description is provide in aErrorMsg.
     * @param aKey encryption key
     * @param aErrorMsg error message description
     * @return the pickled account as bytes buffer
     */
    public byte[] pickle(byte[] aKey, StringBuffer aErrorMsg) {
        return serialize(aKey, aErrorMsg);
    }

    /**
     * Loads an account from a pickled bytes buffer.<br>
     * See {@link #serialize(byte[], StringBuffer)}
     * @param aSerializedData bytes buffer
     * @param aKey key used to encrypted
     * @exception Exception the exception
     */
    public void unpickle(byte[] aSerializedData, byte[] aKey) throws Exception {
        deserialize(aSerializedData, aKey);
    }

    /**
     * Generates a new fallback key.
     * @throws OlmException exception with a reason.
     */
    public void generateFallbackKey() throws OlmException {
        try {
            generateFallbackKeyJni();
        } catch (Exception e) {
            throw new OlmException(OlmException.EXCEPTION_CODE_ACCOUNT_GENERATE_FALLBACK_KEY, e.getMessage());
        }
    }

    private native void generateFallbackKeyJni();

    /**
     * Return the "fallback key" in a dictionary.<br>
     * Ex:<code>
     * { "curve25519":
     *  {
     *      "AAAABQ":"qefVZd8qvjOpsFzoKSAdfUnJVkIreyxWFlipCHjSQQg"
     *  }
     * }</code><br>
     * Public API for {@link #fallbackKeyJni()}.<br>
     * Note: the key is to be published on the server.
     * @return fallback key in string dictionary.
     * @exception OlmException the failure reason
     */
    public Map<String, Map<String, String>> fallbackKey() throws OlmException {
        JSONObject fallbackKeyJsonObj = null;
        byte[] fallbackKeyBuffer;

        try {
            fallbackKeyBuffer = fallbackKeyJni();
        } catch (Exception e) {
            throw new OlmException(OlmException.EXCEPTION_CODE_ACCOUNT_FALLBACK_KEY, e.getMessage());
        }

        if( null != fallbackKeyBuffer) {
            try {
                fallbackKeyJsonObj = new JSONObject(new String(fallbackKeyBuffer, "UTF-8"));
            } catch (Exception e) {
                Log.e(LOG_TAG, "## fallbackKey(): Exception - Msg=" + e.getMessage());
            }
        } else {
            Log.e(LOG_TAG, "## fallbackKey(): Failure - identityKeysJni()=null");
        }

        return OlmUtility.toStringMapMap(fallbackKeyJsonObj);
    }

    private native byte[] fallbackKeyJni();


    /**
     * Forget about the old fallback key.
     *
     * This should be called once you are reasonably certain that you will not
     * receive any more messages that use the old fallback key (e.g. 5 minutes
     * after the new fallback key has been published).
     * @throws OlmException the failure reason
     **/
    public void forgetFallbackKey() throws OlmException {
        try {
            forgetFallbackKeyJni();
        } catch (Exception e) {
            throw new OlmException(OlmException.EXCEPTION_CODE_ACCOUNT_FORGET_FALLBACK_KEY, e.getMessage());
        }
    }

    private native void forgetFallbackKeyJni();
}
