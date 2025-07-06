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

import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.Serializable;

import java.util.Arrays;

/**
 * Class used to create an outbound a <a href="http://matrix.org/docs/guides/e2e_implementation.html#starting-a-megolm-session">Megolm session</a>.<br>
 * To send a first message in an encrypted room, the client should start a new outbound Megolm session.
 * The session ID and the session key must be shared with each device in the room within.
 *
 * <br><br>Detailed implementation guide is available at <a href="http://matrix.org/docs/guides/e2e_implementation.html">Implementing End-to-End Encryption in Matrix clients</a>.
 */
public class OlmOutboundGroupSession extends CommonSerializeUtils implements Serializable {
    private static final long serialVersionUID = -3133097431283604416L;
    private static final String LOG_TAG = "OlmOutboundGroupSession";

    /** Session Id returned by JNI.<br>
     * This value uniquely identifies the native outbound group session instance.
     */
    private transient long mNativeId;

    /**
     * Constructor.<br>
     * Create and save a new session native instance ID and
     * initialise a new outbound group session.<br>
     * @throws OlmException constructor failure
     */
    public OlmOutboundGroupSession() throws OlmException {
        try {
            mNativeId = createNewSessionJni();
        } catch (Exception e) {
            throw new OlmException(OlmException.EXCEPTION_CODE_CREATE_OUTBOUND_GROUP_SESSION, e.getMessage());
        }
    }

    /**
     * Create the corresponding OLM outbound group session in native side.<br>
     * An exception is thrown if the operation fails.
     * Do not forget to call {@link #releaseSession()} when JAVA side is done.
     * @return native session instance identifier (see {@link #mNativeId})
     */
    private native long createNewSessionJni();

    /**
     * Release native session and invalid its JAVA reference counter part.<br>
     * Public API for {@link #releaseSessionJni()}.
     */
    public void releaseSession() {
        if (0 != mNativeId) {
            releaseSessionJni();
        }
        mNativeId = 0;
    }

    /**
     * Destroy the corresponding OLM outbound group session native object.<br>
     * This method must ALWAYS be called when this JAVA instance
     * is destroyed (ie. garbage collected) to prevent memory leak in native side.
     * See {@link #createNewSessionJni()}.
     */
    private native void releaseSessionJni();

    /**
     * Return true the object resources have been released.<br>
     * @return true the object resources have been released
     */
    public boolean isReleased() {
        return (0 == mNativeId);
    }

    /**
     * Get a base64-encoded identifier for this session.
     * @return session identifier
     * @throws OlmException the failure reason
     */
    public String sessionIdentifier() throws OlmException {
        try {
            return new String(sessionIdentifierJni(), "UTF-8");
        } catch (Exception e) {
            Log.e(LOG_TAG, "## sessionIdentifier() failed " + e.getMessage());
            throw new OlmException(OlmException.EXCEPTION_CODE_OUTBOUND_GROUP_SESSION_IDENTIFIER, e.getMessage());
        }
    }

    /**
     * Return the session identifier.
     * An exception is thrown if the operation fails.
     * @return the session identifier
     */
    private native byte[] sessionIdentifierJni();

    /**
     * Get the current message index for this session.<br>
     * Each message is sent with an increasing index, this
     * method returns the index for the next message.
     * @return current session index
     */
    public int messageIndex() {
        return messageIndexJni();
    }

    /**
     * Get the current message index for this session.<br>
     * Each message is sent with an increasing index, this
     * method returns the index for the next message.
     * An exception is thrown if the operation fails.
     * @return current session index
     */
    private native int messageIndexJni();

    /**
     * Get the base64-encoded current ratchet key for this session.<br>
     * Each message is sent with a different ratchet key. This method returns the
     * ratchet key that will be used for the next message.
     * @return outbound session key
     * @exception OlmException the failure reason
     */
    public String sessionKey() throws OlmException {
        try {
            byte[] sessionKeyBuffer = sessionKeyJni();
            String ret = new String(sessionKeyBuffer, "UTF-8");
            Arrays.fill(sessionKeyBuffer, (byte) 0);
            return ret;
        } catch (Exception e) {
            Log.e(LOG_TAG, "## sessionKey() failed " + e.getMessage());
            throw new OlmException(OlmException.EXCEPTION_CODE_OUTBOUND_GROUP_SESSION_KEY, e.getMessage());
        }
    }

    /**
     * Return the session key.
     * An exception is thrown if the operation fails.
     * @return the session key
     */
    private native byte[] sessionKeyJni();

    /**
     * Encrypt some plain-text message.<br>
     * The message given as parameter is encrypted and returned as the return value.
     * @param aClearMsg message to be encrypted
     * @return the encrypted message
     * @exception OlmException the encryption failure reason
     */
    public String encryptMessage(String aClearMsg) throws OlmException {
        String retValue = null;

        if (!TextUtils.isEmpty(aClearMsg)) {
            try {
                byte[] clearMsgBuffer = aClearMsg.getBytes("UTF-8");
                byte[] encryptedBuffer = encryptMessageJni(clearMsgBuffer);
                Arrays.fill(clearMsgBuffer, (byte) 0);

                if (null != encryptedBuffer) {
                    retValue = new String(encryptedBuffer , "UTF-8");
                }
            } catch (Exception e) {
                Log.e(LOG_TAG, "## encryptMessage() failed " + e.getMessage());
                throw new OlmException(OlmException.EXCEPTION_CODE_OUTBOUND_GROUP_ENCRYPT_MESSAGE, e.getMessage());
            }
        }

        return retValue;
    }

    /**
     * Encrypt a bytes buffer messages.
     * An exception is thrown if the operation fails.
     * @param aClearMsgBuffer  the message to encode
     * @return the encoded message
     */
    private native byte[] encryptMessageJni(byte[] aClearMsgBuffer);

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
     * Return the current outbound group session as a base64 byte buffers.<br>
     * The session is serialized and encrypted with aKey.
     * In case of failure, an error human readable
     * description is provide in aErrorMsg.
     * @param aKey encryption key
     * @param aErrorMsg error message description
     * @return pickled base64 bytes buffer if operation succeed, null otherwise
     */
    @Override
    protected byte[] serialize(byte[] aKey, StringBuffer aErrorMsg) {
        byte[] pickleRetValue = null;

        // sanity check
        if(null == aErrorMsg) {
            Log.e(LOG_TAG,"## serialize(): invalid parameter - aErrorMsg=null");
        } else if (null == aKey) {
            aErrorMsg.append("Invalid input parameters in serialize()");
        } else {
            try {
                pickleRetValue = serializeJni(aKey);
            } catch (Exception e) {
                Log.e(LOG_TAG,"## serialize(): failed " + e.getMessage());
                aErrorMsg.append(e.getMessage());
            }
        }

        return pickleRetValue;
    }

    /**
     * JNI counter part of {@link #serialize(byte[], StringBuffer)}.
     * An exception is thrown if the operation fails.
     * @param aKey encryption key
     * @return the serialized session
     */
    private native byte[] serializeJni(byte[] aKey);

    /**
     * Loads an account from a pickled base64 string.<br>
     * See {@link #serialize(byte[], StringBuffer)}
     * @param aSerializedData pickled account in a base64 bytes buffer
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
            releaseSession();
            throw new OlmException(OlmException.EXCEPTION_CODE_ACCOUNT_DESERIALIZATION, errorMsg);
        }
    }

    /**
     * Allocate a new session and initialize it with the serialisation data.<br>
     * An exception is thrown if the operation fails.
     * @param aSerializedData the session serialisation buffer
     * @param aKey the key used to encrypt the serialized account data
     * @return the deserialized session
     **/
    private native long deserializeJni(byte[] aSerializedData, byte[] aKey);

    /**
     * Return a pickled outbound group session as a bytes buffer.<br>
     * The session is serialized and encrypted with aKey.
     * In case of failure, an error human readable
     * description is provide in aErrorMsg.
     * @param aKey encryption key
     * @param aErrorMsg error message description
     * @return the pickled outbound group session as bytes buffer
     */
    public byte[] pickle(byte[] aKey, StringBuffer aErrorMsg) {
        return serialize(aKey, aErrorMsg);
    }

    /**
     * Loads an outbound group session from a pickled bytes buffer.<br>
     * See {@link #serialize(byte[], StringBuffer)}
     * @param aSerializedData bytes buffer
     * @param aKey key used to encrypted
     * @exception Exception the exception
     */
    public void unpickle(byte[] aSerializedData, byte[] aKey) throws Exception {
        deserialize(aSerializedData, aKey);
    }

}
