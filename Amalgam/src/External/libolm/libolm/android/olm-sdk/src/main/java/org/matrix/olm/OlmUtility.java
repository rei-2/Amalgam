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

import java.security.SecureRandom;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

/**
 * Olm SDK helper class.
 */
public class OlmUtility {
    private static final String LOG_TAG = "OlmUtility";

    public static final int RANDOM_KEY_SIZE = 32;

    /** Instance Id returned by JNI.
     * This value uniquely identifies this utility instance.
     **/
    private long mNativeId;

    public OlmUtility() throws OlmException  {
        initUtility();
    }

    /**
     * Create a native utility instance.
     * To be called before any other API call.
     * @exception OlmException the exception
     */
    private void initUtility() throws OlmException {
        try {
            mNativeId = createUtilityJni();
        } catch (Exception e) {
            throw new OlmException(OlmException.EXCEPTION_CODE_UTILITY_CREATION, e.getMessage());
        }
    }

    private native long createUtilityJni();

    /**
     * Release native instance.<br>
     * Public API for {@link #releaseUtilityJni()}.
     */
    public void releaseUtility() {
        if (0 != mNativeId) {
            releaseUtilityJni();
        }
        mNativeId = 0;
    }
    private native void releaseUtilityJni();

    /**
     * Verify an ed25519 signature.<br>
     * An exception is thrown if the operation fails.
     * @param aSignature the base64-encoded message signature to be checked.
     * @param aFingerprintKey the ed25519 key (fingerprint key)
     * @param aMessage the signed message
     * @exception OlmException the failure reason
     */
    public void verifyEd25519Signature(String aSignature, String aFingerprintKey, String aMessage) throws OlmException {
        String errorMessage;
        byte[] messageBuffer = null;

        try {
            if (TextUtils.isEmpty(aSignature) || TextUtils.isEmpty(aFingerprintKey) || TextUtils.isEmpty(aMessage)) {
                Log.e(LOG_TAG, "## verifyEd25519Signature(): invalid input parameters");
                errorMessage = "JAVA sanity check failure - invalid input parameters";
            } else {
                messageBuffer = aMessage.getBytes("UTF-8");
                errorMessage =  verifyEd25519SignatureJni(aSignature.getBytes("UTF-8"), aFingerprintKey.getBytes("UTF-8"), messageBuffer);
            }
        } catch (Exception e) {
            Log.e(LOG_TAG, "## verifyEd25519Signature(): failed " + e.getMessage());
            errorMessage = e.getMessage();
        } finally {
            if (messageBuffer != null) {
                Arrays.fill(messageBuffer, (byte) 0);
            }
        }

        if (!TextUtils.isEmpty(errorMessage)) {
            throw new OlmException(OlmException.EXCEPTION_CODE_UTILITY_VERIFY_SIGNATURE, errorMessage);
        }
    }

    /**
     * Verify an ed25519 signature.
     * Return a human readable error message in case of verification failure.
     * @param aSignature the base64-encoded message signature to be checked.
     * @param aFingerprintKey the ed25519 key
     * @param aMessage the signed message
     * @return null if validation succeed, the error message string if operation failed
     */
    private native String verifyEd25519SignatureJni(byte[] aSignature, byte[] aFingerprintKey, byte[] aMessage);

    /**
     * Compute the hash(SHA-256) value of the string given in parameter(aMessageToHash).<br>
     * The hash value is the returned by the method.
     * @param aMessageToHash message to be hashed
     * @return hash value if operation succeed, null otherwise
     */
     public String sha256(String aMessageToHash) {
         String hashRetValue = null;

         if (null != aMessageToHash) {
             byte[] messageBuffer = null;
             try {
                 messageBuffer = aMessageToHash.getBytes("UTF-8");
                 hashRetValue = new String(sha256Jni(messageBuffer), "UTF-8");
             } catch (Exception e) {
                 Log.e(LOG_TAG, "## sha256(): failed " + e.getMessage());
             } finally {
                 if (null != messageBuffer) {
                     Arrays.fill(messageBuffer, (byte) 0);
                 }
             }
         }

        return hashRetValue;
    }

    /**
     * Compute the digest (SHA 256) for the message passed in parameter.<br>
     * The digest value is the function return value.
     * An exception is thrown if the operation fails.
     * @param aMessage the message
     * @return digest of the message.
     **/
    private native byte[] sha256Jni(byte[] aMessage);

    /**
     * Helper method to compute a string based on random integers.
     * @return bytes buffer containing randoms integer values
     */
    public static byte[] getRandomKey() {
        SecureRandom secureRandom = new SecureRandom();
        byte[] buffer = new byte[RANDOM_KEY_SIZE];
        secureRandom.nextBytes(buffer);

        // the key is saved as string
        // so avoid the UTF8 marker bytes
        for(int i = 0; i < RANDOM_KEY_SIZE; i++) {
            buffer[i] = (byte)(buffer[i] & 0x7F);
        }
        return buffer;
    }

    /**
     * Return true the object resources have been released.<br>
     * @return true the object resources have been released
     */
    public boolean isReleased() {
        return (0 == mNativeId);
    }

    /**
     * Build a string-string dictionary from a jsonObject.<br>
     * @param jsonObject the object to parse
     * @return the map
     */
    public static Map<String, String> toStringMap(JSONObject jsonObject) {
        if (null != jsonObject) {
            HashMap<String, String> map = new HashMap<>();
            Iterator<String> keysItr = jsonObject.keys();
            while(keysItr.hasNext()) {
                String key = keysItr.next();
                try {
                    Object value = jsonObject.get(key);

                    if (value instanceof String) {
                        map.put(key, (String) value);
                    } else {
                        Log.e(LOG_TAG, "## toStringMap(): unexpected type " + value.getClass());
                    }
                } catch (Exception e) {
                    Log.e(LOG_TAG, "## toStringMap(): failed " + e.getMessage());
                }
            }

            return map;
        }

        return null;
    }

    /**
     * Build a string-string dictionary of string dictionary from a jsonObject.<br>
     * @param jsonObject the object to parse
     * @return the map
     */
    public static Map<String, Map<String, String>> toStringMapMap(JSONObject jsonObject) {
        if (null != jsonObject) {
            HashMap<String, Map<String, String>> map = new HashMap<>();

            Iterator<String> keysItr = jsonObject.keys();
            while(keysItr.hasNext()) {
                String key = keysItr.next();
                try {
                    Object value = jsonObject.get(key);

                    if (value instanceof JSONObject) {
                        map.put(key, toStringMap((JSONObject) value));
                    } else {
                        Log.e(LOG_TAG, "## toStringMapMap(): unexpected type " + value.getClass());
                    }
                } catch (Exception e) {
                    Log.e(LOG_TAG, "## toStringMapMap(): failed " + e.getMessage());
                }
            }

            return map;
        }

        return null;
    }
}

