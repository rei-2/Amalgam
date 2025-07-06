/*
 * Copyright 2019 New Vector Ltd
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

import android.util.Log;

import java.io.UnsupportedEncodingException;

public class OlmSAS {

    private static final String LOG_TAG = OlmSAS.class.getName();
    /**
     * Session Id returned by JNI.
     * This value uniquely identifies the native SAS instance.
     **/
    private transient long mNativeId;

    private String theirPublicKey = null;

    public OlmSAS() throws OlmException {
        try {
            mNativeId = createNewSASJni();
        } catch (Exception e) {
            throw new OlmException(OlmException.EXCEPTION_CODE_SAS_CREATION, e.getMessage());
        }
    }

    /**
     * Gets the Public Key encoded in Base64 with no padding
     * @return The public key
     * @throws OlmException the failure reason
     */
    public String getPublicKey() throws OlmException {
        try {
            byte[] buffer = getPubKeyJni();

            if (null != buffer) {
                return new String(buffer, "UTF-8");
            }
        } catch (Exception e) {
            Log.e(LOG_TAG, "## sessionIdentifier(): " + e.getMessage());
            throw new OlmException(OlmException.EXCEPTION_CODE_SAS_ERROR, e.getMessage());
        }

        return null;
    }

    /**
     * Sets the public key of other user.
     *
     * @param otherPkey other user public key (base64 encoded with no padding)
     * @throws OlmException the failure reason
     */
    public void setTheirPublicKey(String otherPkey) throws OlmException {
        try {
            setTheirPubKey(otherPkey.getBytes("UTF-8"));
        } catch (UnsupportedEncodingException e) {
            throw new OlmException(OlmException.EXCEPTION_CODE_SAS_ERROR, e.getMessage());
        }
        this.theirPublicKey = otherPkey;
    }


    /**
     * Generate bytes to use for the short authentication string.
     *
     * @param info       info extra information to mix in when generating the bytes, as
     *                   per the Matrix spec.
     * @param byteNumber The size of the short code to generate
     * @return The generated shortcode
     * @throws OlmException the failure reason
     */
    public byte[] generateShortCode(String info, int byteNumber) throws OlmException {
        if (theirPublicKey == null || theirPublicKey.isEmpty()) {
            throw new OlmException(OlmException.EXCEPTION_CODE_SAS_MISSING_THEIR_PKEY, "call setTheirPublicKey first");
        }
        try {
            return generateShortCodeJni(info.getBytes("UTF-8"), byteNumber);
        } catch (Exception e) {
            Log.e(LOG_TAG, "## sessionIdentifier(): " + e.getMessage());
            throw new OlmException(OlmException.EXCEPTION_CODE_SAS_GENERATE_SHORT_CODE, e.getMessage());
        }
    }


    public String calculateMac(String message, String info) throws OlmException {
        try {
            byte[] bytes = calculateMacJni(message.getBytes("UTF-8"), info.getBytes("UTF-8"));
            if (bytes != null) return new String(bytes, "UTF-8");
        } catch (UnsupportedEncodingException e) {
            throw new OlmException(OlmException.EXCEPTION_CODE_SAS_ERROR, e.getMessage());
        }
        return null;
    }

    public String calculateMacFixedBase64(String message, String info) throws OlmException {
        try {
            byte[] bytes = calculateMacFixedBase64Jni(message.getBytes("UTF-8"), info.getBytes("UTF-8"));
            if (bytes != null) return new String(bytes, "UTF-8");
        } catch (UnsupportedEncodingException e) {
            throw new OlmException(OlmException.EXCEPTION_CODE_SAS_ERROR, e.getMessage());
        }
        return null;
    }

    public String calculateMacLongKdf(String message, String info) throws OlmException {
        try {
            byte[] bytes = calculateMacLongKdfJni(message.getBytes("UTF-8"), info.getBytes("UTF-8"));
            if (bytes != null) return new String(bytes, "UTF-8");
        } catch (UnsupportedEncodingException e) {
            throw new OlmException(OlmException.EXCEPTION_CODE_SAS_ERROR, e.getMessage());
        }
        return null;
    }

    /**
     * Create an OLM session in native side.<br>
     * Do not forget to call {@link #releaseSASJni()} when JAVA side is done.
     *
     * @return native account instance identifier or throw an exception.
     */
    private native long createNewSASJni();

    /**
     * Destroy the corresponding OLM session native object.<br>
     * This method must ALWAYS be called when this JAVA instance
     * is destroyed (ie. garbage collected) to prevent memory leak in native side.
     * See {@link #createNewSASJni()}.
     */
    private native void releaseSASJni();

    private native byte[] getPubKeyJni();

    private native void setTheirPubKey(byte[] pubKey);

    private native byte[] generateShortCodeJni(byte[] info, int byteNumber);

    private native byte[] calculateMacJni(byte[] message, byte[] info);

    private native byte[] calculateMacFixedBase64Jni(byte[] message, byte[] info);

    private native byte[] calculateMacLongKdfJni(byte[] message, byte[] info);

    /**
     * Release native session and invalid its JAVA reference counter part.<br>
     * Public API for {@link #releaseSASJni()}.
     */
    public void releaseSas() {
        if (0 != mNativeId) {
            releaseSASJni();
        }
        mNativeId = 0;
    }
}
