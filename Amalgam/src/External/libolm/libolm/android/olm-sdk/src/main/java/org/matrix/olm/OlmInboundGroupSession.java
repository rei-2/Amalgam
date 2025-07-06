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
 * Class used to create an inbound <a href="http://matrix.org/docs/guides/e2e_implementation.html#handling-an-m-room-key-event">Megolm session</a>.<br>
 * Counter part of the outbound group session {@link OlmOutboundGroupSession}, this class decrypts the messages sent by the outbound side.
 *
 * <br><br>Detailed implementation guide is available at <a href="http://matrix.org/docs/guides/e2e_implementation.html">Implementing End-to-End Encryption in Matrix clients</a>.
 */
public class OlmInboundGroupSession extends CommonSerializeUtils implements Serializable {
    private static final long serialVersionUID = -772028491251653253L;
    private static final String LOG_TAG = "OlmInboundGroupSession";

    /** Session Id returned by JNI.<br>
     * This value uniquely identifies the native inbound group session instance.
     */
    private transient long mNativeId;

    /**
     * Result in {@link #decryptMessage(String)}
     */
    public static class DecryptMessageResult {
        /** decrypt message **/
        public String mDecryptedMessage;

        /** decrypt index **/
        public long mIndex;
    }

    /**
     * Constructor.<br>
     * Create and save a new native session instance ID and start a new inbound group session.
     * The session key parameter is retrieved from an outbound group session.
     * @param aSessionKey session key
     * @throws OlmException constructor failure
     */
    public OlmInboundGroupSession(String aSessionKey) throws OlmException {
        this(aSessionKey, false);
    }

    /**
     * Constructor.<br>
     * Create and save a new native session instance ID and start a new inbound group session.
     * The session key parameter is retrieved from an outbound group session.
     * @param aSessionKey session key
     * @param isImported true when the session key has been retrieved from a backup
     * @throws OlmException constructor failure
     */
    private OlmInboundGroupSession(String aSessionKey, boolean isImported) throws OlmException {
        if (TextUtils.isEmpty(aSessionKey)) {
            Log.e(LOG_TAG, "## initInboundGroupSession(): invalid session key");
            throw new OlmException(OlmException.EXCEPTION_CODE_INIT_INBOUND_GROUP_SESSION, "invalid session key");
        } else {
            byte[] sessionBuffer = null;
            try {
                sessionBuffer = aSessionKey.getBytes("UTF-8");
                mNativeId = createNewSessionJni(aSessionKey.getBytes("UTF-8"), isImported);
            } catch (Exception e) {
                throw new OlmException(OlmException.EXCEPTION_CODE_INIT_INBOUND_GROUP_SESSION, e.getMessage());
            } finally {
                if (null != sessionBuffer) {
                    Arrays.fill(sessionBuffer, (byte) 0);
                }
            }
        }
    }

    /**
     * Initialize a new inbound group session and return it to JAVA side.<br>
     * Since a C prt is returned as a jlong, special care will be taken
     * to make the cast (OlmInboundGroupSession* to jlong) platform independent.
     * @param aSessionKeyBuffer session key from an outbound session
     * @param isImported true when the session key has been retrieved from a backup
     * @return the initialized OlmInboundGroupSession* instance or throw an exception it fails.
     **/
    private native long createNewSessionJni(byte[] aSessionKeyBuffer, boolean isImported);

    /**
     * Create an OlmInboundGroupSession from its exported session data.
     * @param exported the exported data
     * @return the created OlmException
     * @throws OlmException the failure reason
     */
    public static OlmInboundGroupSession importSession(String exported) throws OlmException {
        return new OlmInboundGroupSession(exported, true);
    }

    /**
     * Release native session and invalid its JAVA reference counter part.<br>
     * Public API for {@link #releaseSessionJni()}.
     */
    public void releaseSession(){
        if (0 != mNativeId) {
            releaseSessionJni();
        }
        mNativeId = 0;
    }

    /**
     * Destroy the corresponding OLM inbound group session native object.<br>
     * This method must ALWAYS be called when this JAVA instance
     * is destroyed (ie. garbage collected) to prevent memory leak in native side.
     * See {@link #createNewSessionJni(byte[], boolean)}.
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
     * Retrieve the base64-encoded identifier for this inbound group session.
     * @return the session ID
     * @throws OlmException the failure reason
     */
    public String sessionIdentifier() throws OlmException {
        try {
            return new String(sessionIdentifierJni(), "UTF-8");
        } catch (Exception e) {
            Log.e(LOG_TAG, "## sessionIdentifier() failed " + e.getMessage());
            throw new OlmException(OlmException.EXCEPTION_CODE_INBOUND_GROUP_SESSION_IDENTIFIER, e.getMessage());
        }
    }

    /**
     * Get a base64-encoded identifier for this inbound group session.
     * An exception is thrown if the operation fails.
     * @return the base64-encoded identifier
     */
    private native byte[] sessionIdentifierJni();

    /**
     * Provides the first known index.
     * @return the first known index.
     * @throws OlmException the failure reason
     */
    public long getFirstKnownIndex() throws OlmException {
        long index = 0;

        try {
            index = firstKnownIndexJni();
        } catch (Exception e) {
            Log.e(LOG_TAG, "## getFirstKnownIndex() failed " + e.getMessage());
            throw new OlmException(OlmException.EXCEPTION_CODE_INBOUND_GROUP_SESSION_FIRST_KNOWN_INDEX, e.getMessage());
        }

        return index;
    }

    /**
     * Provides the first known index.
     * An exception is thrown if the operation fails.
     * @return the first known index.
     */
    private native long firstKnownIndexJni();

    /**
     * Tells if the session is verified.
     * @return true if the session is verified
     * @throws OlmException the failure reason
     */
    public boolean isVerified() throws OlmException {
        boolean isVerified;

        try {
            isVerified = isVerifiedJni();
        } catch (Exception e) {
            Log.e(LOG_TAG, "## isVerified() failed " + e.getMessage());
            throw new OlmException(OlmException.EXCEPTION_CODE_INBOUND_GROUP_SESSION_IS_VERIFIED, e.getMessage());
        }

        return isVerified;
    }

    /**
     * Tells if the session is verified.
     * @return true if the session is verified
     */
    private native boolean isVerifiedJni();

    /**
     * Export the session from a message index as String.
     * @param messageIndex the message index
     * @return the session as String
     * @throws OlmException the failure reason
     */
    public String export(long messageIndex) throws OlmException {
        String result = null;

        try {
            byte[] bytesBuffer = exportJni(messageIndex);

            if (null != bytesBuffer) {
                result = new String(bytesBuffer, "UTF-8");
                Arrays.fill(bytesBuffer, (byte) 0);
            }
        } catch (Exception e) {
            Log.e(LOG_TAG, "## export() failed " + e.getMessage());
            throw new OlmException(OlmException.EXCEPTION_CODE_INBOUND_GROUP_SESSION_EXPORT, e.getMessage());
        }

        return result;
    }

    /**
     * Exports the session as byte array from a message index
     * An exception is thrown if the operation fails.
     * @param messageIndex key used to encrypt the serialized session data
     * @return the session saved as bytes array
     */
    private native byte[] exportJni(long messageIndex);

    /**
     * Decrypt the message passed in parameter.<br>
     * In case of error, null is returned and an error message description is provided in aErrorMsg.
     * @param aEncryptedMsg the message to be decrypted
     * @return the decrypted message information
     * @exception OlmException the failure reason
     */
    public DecryptMessageResult decryptMessage(String aEncryptedMsg) throws OlmException {
        DecryptMessageResult result = new DecryptMessageResult();

        try {
            byte[] decryptedMessageBuffer = decryptMessageJni(aEncryptedMsg.getBytes("UTF-8"), result);

            if (null != decryptedMessageBuffer) {
                result.mDecryptedMessage = new String(decryptedMessageBuffer, "UTF-8");
                Arrays.fill(decryptedMessageBuffer, (byte) 0);
            }
        } catch (Exception e) {
            Log.e(LOG_TAG, "## decryptMessage() failed " + e.getMessage());
            throw new OlmException(OlmException.EXCEPTION_CODE_INBOUND_GROUP_SESSION_DECRYPT_SESSION, e.getMessage());
        }

        return result;
    }

    /**
     * Decrypt a message.
     * An exception is thrown if the operation fails.
     * @param aEncryptedMsg the encrypted message
     * @param aDecryptMessageResult the decryptMessage information
     * @return the decrypted message
     */
    private native byte[] decryptMessageJni(byte[] aEncryptedMsg, DecryptMessageResult aDecryptMessageResult);

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
     * Return the current inbound group session as a bytes buffer.<br>
     * The session is serialized and encrypted with aKey.
     * In case of failure, an error human readable
     * description is provide in aErrorMsg.
     * @param aKey encryption key
     * @param aErrorMsg error message description
     * @return pickled bytes buffer if operation succeed, null otherwise
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
     * JNI counter part of {@link #serialize(byte[], StringBuffer)}.
     * @param aKey encryption key
     * @return the serialized session
     */
    private native byte[] serializeJni(byte[] aKey);

    /**
     * Loads an account from a pickled base64 string.<br>
     * See {@link #serialize(byte[], StringBuffer)}
     * @param aSerializedData pickled account in a bytes buffer
     * @param aKey key used to encrypted
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
     * Return a pickled inbound group session as a bytes buffer.<br>
     * The session is serialized and encrypted with aKey.
     * In case of failure, an error human readable
     * description is provide in aErrorMsg.
     * @param aKey encryption key
     * @param aErrorMsg error message description
     * @return the pickled inbound group session as bytes buffer
     */
    public byte[] pickle(byte[] aKey, StringBuffer aErrorMsg) {
        return serialize(aKey, aErrorMsg);
    }

    /**
     * Loads an inbound group session from a pickled bytes buffer.<br>
     * See {@link #serialize(byte[], StringBuffer)}
     * @param aSerializedData bytes buffer
     * @param aKey key used to encrypted
     * @exception Exception the exception
     */
    public void unpickle(byte[] aSerializedData, byte[] aKey) throws Exception {
        deserialize(aSerializedData, aKey);
    }

}
