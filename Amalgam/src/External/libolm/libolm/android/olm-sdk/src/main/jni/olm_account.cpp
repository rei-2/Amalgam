/*
 * Copyright 2016 OpenMarket Ltd
 * Copyright 2016 Vector Creations Ltd
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

#include "olm_account.h"

using namespace AndroidOlmSdk;

/**
 * Init memory allocation for account creation.
 * @return valid memory allocation, NULL otherwise
 **/
OlmAccount* initializeAccountMemory()
{
    size_t accountSize = olm_account_size();
    OlmAccount* accountPtr = (OlmAccount*)malloc(accountSize);

    if (accountPtr)
    {
        // init account object
        accountPtr = olm_account(accountPtr);
        LOGD("## initializeAccountMemory(): success - OLM account size=%lu",static_cast<long unsigned int>(accountSize));
    }
    else
    {
        LOGE("## initializeAccountMemory(): failure - OOM");
    }

    return accountPtr;
}

/**
 * Create a new account and return it to JAVA side.<br>
 * Since a C prt is returned as a jlong, special care will be taken
 * to make the cast (OlmAccount* => jlong) platform independent.
 * @return the initialized OlmAccount* instance or throw an exception if fails
 **/
JNIEXPORT jlong OLM_ACCOUNT_FUNC_DEF(createNewAccountJni)(JNIEnv *env, jobject thiz)
{
    const char* errorMessage = NULL;
    OlmAccount *accountPtr = initializeAccountMemory();

    // init account memory allocation
    if (!accountPtr)
    {
        LOGE("## initNewAccount(): failure - init account OOM");
        errorMessage = "init account OOM";
    }
    else
    {
        // get random buffer size
        size_t randomSize = olm_create_account_random_length(accountPtr);

        LOGD("## initNewAccount(): randomSize=%lu", static_cast<long unsigned int>(randomSize));

        uint8_t *randomBuffPtr = NULL;
        size_t accountRetCode;

        // allocate random buffer
        if ((0 != randomSize) && !setRandomInBuffer(env, &randomBuffPtr, randomSize))
        {
            LOGE("## initNewAccount(): failure - random buffer init");
            errorMessage = "random buffer init";
        }
        else
        {
            // create account
            accountRetCode = olm_create_account(accountPtr, (void*)randomBuffPtr, randomSize);

            if (accountRetCode == olm_error())
            {
                LOGE("## initNewAccount(): failure - account creation failed Msg=%s", olm_account_last_error(accountPtr));
                errorMessage = olm_account_last_error(accountPtr);
            }

            LOGD("## initNewAccount(): success - OLM account created");
            LOGD("## initNewAccount(): success - accountPtr=%p (jlong)(intptr_t)accountPtr=%lld",accountPtr,(jlong)(intptr_t)accountPtr);
        }

        if (randomBuffPtr)
        {
            memset(randomBuffPtr, 0, randomSize);
            free(randomBuffPtr);
        }
    }

    if (errorMessage)
    {
        // release the allocated data
        if (accountPtr)
        {
            olm_clear_account(accountPtr);
            free(accountPtr);
        }
        env->ThrowNew(env->FindClass("java/lang/Exception"), errorMessage);
    }

    return (jlong)(intptr_t)accountPtr;
}
/**
 * Release the account allocation made by initializeAccountMemory().<br>
 * This method MUST be called when java counter part account instance is done.
 */
JNIEXPORT void OLM_ACCOUNT_FUNC_DEF(releaseAccountJni)(JNIEnv *env, jobject thiz)
{
    LOGD("## releaseAccountJni(): IN");

    OlmAccount* accountPtr = getAccountInstanceId(env, thiz);

    if (!accountPtr)
    {
        LOGE(" ## releaseAccountJni(): failure - invalid Account ptr=NULL");
    }
    else
    {
        LOGD(" ## releaseAccountJni(): accountPtr=%p",accountPtr);
        olm_clear_account(accountPtr);

        LOGD(" ## releaseAccountJni(): IN");
        // even if free(NULL) does not crash, logs are performed for debug purpose
        free(accountPtr);
        LOGD(" ## releaseAccountJni(): OUT");
    }
}

// *********************************************************************
// ************************* IDENTITY KEYS API *************************
// *********************************************************************

/**
 * Get identity keys: Ed25519 fingerprint key and Curve25519 identity key.<br>
 * The keys are returned in the byte array.
 * @return the identity keys or throw an exception if it fails
 **/
JNIEXPORT jbyteArray OLM_ACCOUNT_FUNC_DEF(identityKeysJni)(JNIEnv *env, jobject thiz)
{
    const char* errorMessage = NULL;
    jbyteArray byteArrayRetValue = NULL;
    OlmAccount* accountPtr = getAccountInstanceId(env, thiz);

    if (!accountPtr)
    {
        LOGE("## identityKeys(): failure - invalid Account ptr=NULL");
        errorMessage = "invalid Account ptr";
    }
    else
    {
        LOGD("## identityKeys(): accountPtr =%p", accountPtr);

        // identity keys allocation
        size_t identityKeysLength = olm_account_identity_keys_length(accountPtr);
        uint8_t *identityKeysBytesPtr = (uint8_t*)malloc(identityKeysLength);

        if (!identityKeysBytesPtr)
        {
            LOGE("## identityKeys(): failure - identity keys array OOM");
            errorMessage = "identity keys array OOM";
        }
        else
        {
            // retrieve key pairs in identityKeysBytesPtr
            size_t keysResult = olm_account_identity_keys(accountPtr, identityKeysBytesPtr, identityKeysLength);

            if (keysResult == olm_error())
            {
                errorMessage = (const char *)olm_account_last_error(accountPtr);
                LOGE("## identityKeys(): failure - error getting identity keys Msg=%s", errorMessage);
            }
            else
            {
                // allocate the byte array to be returned to java
                byteArrayRetValue = env->NewByteArray(identityKeysLength);

                if (!byteArrayRetValue)
                {
                    LOGE("## identityKeys(): failure - return byte array OOM");
                    errorMessage = "byte array OOM";
                }
                else
                {
                    env->SetByteArrayRegion(byteArrayRetValue, 0/*offset*/, identityKeysLength, (const jbyte*)identityKeysBytesPtr);
                    LOGD("## identityKeys(): success - result=%lu", static_cast<long unsigned int>(keysResult));
                }
            }

            free(identityKeysBytesPtr);
        }
    }

    if (errorMessage)
    {
        env->ThrowNew(env->FindClass("java/lang/Exception"), errorMessage);
    }

    return byteArrayRetValue;
}

// *********************************************************************
// ************************* ONE TIME KEYS API *************************
// *********************************************************************

/**
 * Get the public parts of the unpublished "one time keys" for the account.<br>
 * The returned data is a JSON-formatted object with the single property
 * <tt>curve25519</tt>, which is itself an object mapping key id to
 * base64-encoded Curve25519 key.<br>
 * @return byte array containing the one time keys or throw an exception if it fails
 */
JNIEXPORT jlong OLM_ACCOUNT_FUNC_DEF(maxOneTimeKeysJni)(JNIEnv *env, jobject thiz)
{
    OlmAccount* accountPtr = getAccountInstanceId(env, thiz);
    size_t maxKeys = -1;

    if (!accountPtr)
    {
        LOGE("## maxOneTimeKey(): failure - invalid Account ptr=NULL");
    }
    else
    {
        maxKeys = olm_account_max_number_of_one_time_keys(accountPtr);
    }

    LOGD("## maxOneTimeKey(): Max keys=%lu", static_cast<long unsigned int>(maxKeys));

    return (jlong)maxKeys;
}

/**
 * Generate "one time keys".
 * An exception is thrown if the operation fails.
 * @param aNumberOfKeys number of keys to generate
 **/
JNIEXPORT void OLM_ACCOUNT_FUNC_DEF(generateOneTimeKeysJni)(JNIEnv *env, jobject thiz, jint aNumberOfKeys)
{
    const char* errorMessage = NULL;
    OlmAccount *accountPtr = getAccountInstanceId(env, thiz);

    if (!accountPtr)
    {
        LOGE("## generateOneTimeKeysJni(): failure - invalid Account ptr");
        errorMessage = "invalid Account ptr";
    }
    else
    {
        // keys memory allocation
        size_t randomLength = olm_account_generate_one_time_keys_random_length(accountPtr, (size_t)aNumberOfKeys);
        LOGD("## generateOneTimeKeysJni(): randomLength=%lu", static_cast<long unsigned int>(randomLength));

        uint8_t *randomBufferPtr = NULL;

        if ((0 != randomLength) && !setRandomInBuffer(env, &randomBufferPtr, randomLength))
        {
            LOGE("## generateOneTimeKeysJni(): failure - random buffer init");
            errorMessage = "random buffer init";
        }
        else
        {
            LOGD("## generateOneTimeKeysJni(): accountPtr =%p aNumberOfKeys=%d",accountPtr, aNumberOfKeys);

            // retrieve key pairs in keysBytesPtr
            size_t result = olm_account_generate_one_time_keys(accountPtr, (size_t)aNumberOfKeys, (void*)randomBufferPtr, randomLength);

            if (result == olm_error())
            {
                errorMessage = olm_account_last_error(accountPtr);
                LOGE("## generateOneTimeKeysJni(): failure - error generating one time keys Msg=%s", errorMessage);
            }
            else
            {
                LOGD("## generateOneTimeKeysJni(): success - result=%lu", static_cast<long unsigned int>(result));
            }
        }


        if (randomBufferPtr)
        {
            memset(randomBufferPtr, 0, randomLength);
            free(randomBufferPtr);
        }
    }

    if (errorMessage)
    {
        env->ThrowNew(env->FindClass("java/lang/Exception"), errorMessage);
    }
}

/**
 * Get "one time keys".<br>
 * Return the public parts of the unpublished "one time keys" for the account
 * @return a valid byte array if operation succeed, null otherwise
 **/
JNIEXPORT jbyteArray OLM_ACCOUNT_FUNC_DEF(oneTimeKeysJni)(JNIEnv *env, jobject thiz)
{
    const char* errorMessage = NULL;
    jbyteArray byteArrayRetValue = NULL;
    OlmAccount* accountPtr = getAccountInstanceId(env, thiz);

    LOGD("## oneTimeKeysJni(): IN");

    if (!accountPtr)
    {
        LOGE("## oneTimeKeysJni(): failure - invalid Account ptr");
        errorMessage = "invalid Account ptr";
    }
    else
    {
        // keys memory allocation
        size_t keysLength = olm_account_one_time_keys_length(accountPtr);
        uint8_t *keysBytesPtr = (uint8_t *)malloc(keysLength*sizeof(uint8_t));

        if (!keysBytesPtr)
        {
            LOGE("## oneTimeKeysJni(): failure - one time keys array OOM");
            errorMessage = "one time keys array OOM";
        }
        else
        {
            // retrieve key pairs in keysBytesPtr
            size_t keysResult = olm_account_one_time_keys(accountPtr, keysBytesPtr, keysLength);

            if (keysResult == olm_error()) {
                LOGE("## oneTimeKeysJni(): failure - error getting one time keys Msg=%s",(const char *)olm_account_last_error(accountPtr));
                errorMessage = (const char *)olm_account_last_error(accountPtr);
            }
            else
            {
                // allocate the byte array to be returned to java
                byteArrayRetValue = env->NewByteArray(keysLength);

                if (!byteArrayRetValue)
                {
                    LOGE("## oneTimeKeysJni(): failure - return byte array OOM");
                    errorMessage = "return byte array OOM";
                }
                else
                {
                    env->SetByteArrayRegion(byteArrayRetValue, 0/*offset*/, keysLength, (const jbyte*)keysBytesPtr);
                    LOGD("## oneTimeKeysJni(): success");
                }
            }

            free(keysBytesPtr);
        }
    }

    if (errorMessage)
    {
        env->ThrowNew(env->FindClass("java/lang/Exception"), errorMessage);
    }

    return byteArrayRetValue;
}

/**
 * Remove the "one time keys"  that the session used from the account.
 * An exception is thrown if the operation fails.
 * @param aNativeOlmSessionId session instance
 **/
JNIEXPORT void OLM_ACCOUNT_FUNC_DEF(removeOneTimeKeysJni)(JNIEnv *env, jobject thiz, jlong aNativeOlmSessionId)
{
    const char* errorMessage = NULL;
    OlmAccount* accountPtr = NULL;
    OlmSession* sessionPtr = (OlmSession*)aNativeOlmSessionId;

    if (!sessionPtr)
    {
        LOGE("## removeOneTimeKeysJni(): failure - invalid session ptr");
        errorMessage = "invalid session ptr";
    }
    else if (!(accountPtr = getAccountInstanceId(env, thiz)))
    {
        LOGE("## removeOneTimeKeysJni(): failure - invalid account ptr");
        errorMessage = "invalid account ptr";
    }
    else
    {
        size_t result = olm_remove_one_time_keys(accountPtr, sessionPtr);

        if (result == olm_error())
        {   // the account doesn't have any matching "one time keys"..
            LOGW("## removeOneTimeKeysJni(): failure - removing one time keys Msg=%s", olm_account_last_error(accountPtr));
            errorMessage = (const char *)olm_account_last_error(accountPtr);
        }
        else
        {
            LOGD("## removeOneTimeKeysJni(): success");
        }
    }

    if (errorMessage)
    {
        env->ThrowNew(env->FindClass("java/lang/Exception"), errorMessage);
    }
}

/**
 * Mark the current set of "one time keys" as being published.
 * An exception is thrown if the operation fails.
 **/
JNIEXPORT void OLM_ACCOUNT_FUNC_DEF(markOneTimeKeysAsPublishedJni)(JNIEnv *env, jobject thiz)
{
    const char* errorMessage = NULL;
    OlmAccount* accountPtr = getAccountInstanceId(env, thiz);

    if (!accountPtr)
    {
        LOGE("## markOneTimeKeysAsPublishedJni(): failure - invalid account ptr");
        errorMessage = "invalid account ptr";
    }
    else
    {
        size_t result = olm_account_mark_keys_as_published(accountPtr);

        if (result == olm_error())
        {
            LOGW("## markOneTimeKeysAsPublishedJni(): failure - Msg=%s",(const char *)olm_account_last_error(accountPtr));
            errorMessage = (const char *)olm_account_last_error(accountPtr);
        }
        else
        {
            LOGD("## markOneTimeKeysAsPublishedJni(): success - retCode=%lu",static_cast<long unsigned int>(result));
        }
    }

    if (errorMessage)
    {
        env->ThrowNew(env->FindClass("java/lang/Exception"), errorMessage);
    }
}

/**
 * Generate "fallback key".
 * An exception is thrown if the operation fails.
 **/
JNIEXPORT void OLM_ACCOUNT_FUNC_DEF(generateFallbackKeyJni)(JNIEnv *env, jobject thiz)
{
    const char* errorMessage = NULL;
    OlmAccount *accountPtr = getAccountInstanceId(env, thiz);

    if (!accountPtr)
    {
        LOGE("## generateFallbackKeyJni(): failure - invalid Account ptr");
        errorMessage = "invalid Account ptr";
    }
    else
    {
        // keys memory allocation
        size_t randomLength = olm_account_generate_fallback_key_random_length(accountPtr);
        LOGD("## generateFallbackKeyJni(): randomLength=%lu", static_cast<long unsigned int>(randomLength));

        uint8_t *randomBufferPtr = NULL;

        if ((0 != randomLength) && !setRandomInBuffer(env, &randomBufferPtr, randomLength))
        {
            LOGE("## generateFallbackKeyJni(): failure - random buffer init");
            errorMessage = "random buffer init";
        }
        else
        {
            LOGD("## generateFallbackKeyJni(): accountPtr =%p", accountPtr);

            // retrieve key pairs in keysBytesPtr
            size_t result = olm_account_generate_fallback_key(accountPtr, (void*)randomBufferPtr, randomLength);

            if (result == olm_error())
            {
                errorMessage = olm_account_last_error(accountPtr);
                LOGE("## generateFallbackKeyJni(): failure - error generating fallback keys Msg=%s", errorMessage);
            }
            else
            {
                LOGD("## generateFallbackKeyJni(): success - result=%lu", static_cast<long unsigned int>(result));
            }
        }

        if (randomBufferPtr)
        {
            memset(randomBufferPtr, 0, randomLength);
            free(randomBufferPtr);
        }
    }

    if (errorMessage)
    {
        env->ThrowNew(env->FindClass("java/lang/Exception"), errorMessage);
    }
}

/**
 * Get "fallback key".<br>
 * Return the public parts of the unpublished "fallback key" for the account
 * @return a valid byte array if operation succeed, null otherwise
 **/
JNIEXPORT jbyteArray OLM_ACCOUNT_FUNC_DEF(fallbackKeyJni)(JNIEnv *env, jobject thiz)
{
    const char* errorMessage = NULL;
    jbyteArray byteArrayRetValue = NULL;
    OlmAccount* accountPtr = getAccountInstanceId(env, thiz);

    LOGD("## fallbackKeyJni(): IN");

    if (!accountPtr)
    {
        LOGE("## fallbackKeyJni(): failure - invalid Account ptr");
        errorMessage = "invalid Account ptr";
    }
    else
    {
        // keys memory allocation
        size_t keysLength = olm_account_unpublished_fallback_key_length(accountPtr);
        uint8_t *keysBytesPtr = (uint8_t *)malloc(keysLength*sizeof(uint8_t));

        if (!keysBytesPtr)
        {
            LOGE("## fallbackKeyJni(): failure - fallback key OOM");
            errorMessage = "fallback key OOM";
        }
        else
        {
            // retrieve key pairs in keysBytesPtr
            size_t keysResult = olm_account_unpublished_fallback_key(accountPtr, keysBytesPtr, keysLength);

            if (keysResult == olm_error()) {
                LOGE("## fallbackKeyJni(): failure - error getting fallback key Msg=%s",(const char *)olm_account_last_error(accountPtr));
                errorMessage = (const char *)olm_account_last_error(accountPtr);
            }
            else
            {
                // allocate the byte array to be returned to java
                byteArrayRetValue = env->NewByteArray(keysLength);

                if (!byteArrayRetValue)
                {
                    LOGE("## fallbackKeyJni(): failure - return byte array OOM");
                    errorMessage = "return byte array OOM";
                }
                else
                {
                    env->SetByteArrayRegion(byteArrayRetValue, 0/*offset*/, keysLength, (const jbyte*)keysBytesPtr);
                    LOGD("## fallbackKeyJni(): success");
                }
            }

            free(keysBytesPtr);
        }
    }

    if (errorMessage)
    {
        env->ThrowNew(env->FindClass("java/lang/Exception"), errorMessage);
    }

    return byteArrayRetValue;
}


/**
 * Forget about the old fallback key.
 *
 * This should be called once you are reasonably certain that you will not
 * receive any more messages that use the old fallback key (e.g. 5 minutes
 * after the new fallback key has been published).
 **/
JNIEXPORT void OLM_ACCOUNT_FUNC_DEF(forgetFallbackKeyJni)(JNIEnv *env, jobject thiz)
{
    const char* errorMessage = NULL;
    OlmAccount *accountPtr = getAccountInstanceId(env, thiz);

    if (!accountPtr)
    {
        LOGE("## forgetFallbackKeyJni(): failure - invalid Account ptr");
        errorMessage = "invalid Account ptr";
    }
    else
    {
       olm_account_forget_old_fallback_key(accountPtr);
    }

    if (errorMessage)
    {
        env->ThrowNew(env->FindClass("java/lang/Exception"), errorMessage);
    }

}

/**
 * Sign a message with the ed25519 key (fingerprint) for this account.<br>
 * The signed message is returned by the function.
 * @param aMessage message to sign
 * @return the signed message, null otherwise
 **/
JNIEXPORT jbyteArray OLM_ACCOUNT_FUNC_DEF(signMessageJni)(JNIEnv *env, jobject thiz, jbyteArray aMessage)
{
    const char* errorMessage = NULL;
    OlmAccount* accountPtr = NULL;
    jbyteArray signedMsgRetValueBuffer = NULL;

    if (!aMessage)
    {
        LOGE("## signMessageJni(): failure - invalid aMessage param");
        errorMessage = "invalid aMessage param";
    }
    else if (!(accountPtr = getAccountInstanceId(env, thiz)))
    {
        LOGE("## signMessageJni(): failure - invalid account ptr");
        errorMessage = "invalid account ptr";
    }
    else
    {
        int messageLength = env->GetArrayLength(aMessage);
        jbyte* messageToSign = env->GetByteArrayElements(aMessage, NULL);

        // signature memory allocation
        size_t signatureLength = olm_account_signature_length(accountPtr);
        void* signedMsgPtr = malloc(signatureLength * sizeof(uint8_t));

        if (!signedMsgPtr)
        {
            LOGE("## signMessageJni(): failure - signature allocation OOM");
            errorMessage = "signature allocation OOM";
        }
        else
        {
            // sign message
            size_t resultSign = olm_account_sign(accountPtr,
                                   (void*)messageToSign,
                                   (size_t)messageLength,
                                   signedMsgPtr,
                                   signatureLength);

            if (resultSign == olm_error())
            {
                LOGE("## signMessageJni(): failure - error signing message Msg=%s",(const char *)olm_account_last_error(accountPtr));
                errorMessage = (const char *)olm_account_last_error(accountPtr);
            }
            else
            {
                LOGD("## signMessageJni(): success - retCode=%lu signatureLength=%lu", static_cast<long unsigned int>(resultSign), static_cast<long unsigned int>(signatureLength));

                signedMsgRetValueBuffer = env->NewByteArray(signatureLength);
                env->SetByteArrayRegion(signedMsgRetValueBuffer, 0 , signatureLength, (jbyte*)signedMsgPtr);
            }

            free(signedMsgPtr);
        }

        // release messageToSign
        if (messageToSign)
        {
            env->ReleaseByteArrayElements(aMessage, messageToSign, JNI_ABORT);
        }
    }

    if (errorMessage)
    {
        env->ThrowNew(env->FindClass("java/lang/Exception"), errorMessage);
    }

    return signedMsgRetValueBuffer;
}

/**
 * Serialize and encrypt account instance.<br>
 * @param aKeyBuffer key used to encrypt the serialized account data
 * @return the serialised account as bytes buffer.
 **/
JNIEXPORT jbyteArray OLM_ACCOUNT_FUNC_DEF(serializeJni)(JNIEnv *env, jobject thiz, jbyteArray aKeyBuffer)
{
    const char* errorMessage = NULL;
    jbyteArray pickledDataRetValue = 0;
    jbyte* keyPtr = NULL;
    jboolean keyIsCopied = JNI_FALSE;
    OlmAccount* accountPtr = NULL;

    LOGD("## serializeJni(): IN");

    if (!aKeyBuffer)
    {
        LOGE(" ## serializeJni(): failure - invalid key");
        errorMessage = "invalid key";
    }
    else if (!(accountPtr = getAccountInstanceId(env, thiz)))
    {
       LOGE(" ## serializeJni(): failure - invalid account ptr");
       errorMessage = "invalid account ptr";
    }
    else if (!(keyPtr = env->GetByteArrayElements(aKeyBuffer, &keyIsCopied)))
    {
        LOGE(" ## serializeJni(): failure - keyPtr JNI allocation OOM");
        errorMessage = "keyPtr JNI allocation OOM";
    }
    else
    {
        size_t pickledLength = olm_pickle_account_length(accountPtr);
        size_t keyLength = (size_t)env->GetArrayLength(aKeyBuffer);
        LOGD(" ## serializeJni(): pickledLength=%lu keyLength=%lu",static_cast<long unsigned int>(pickledLength), static_cast<long unsigned int>(keyLength));

        void* pickledPtr = malloc(pickledLength * sizeof(uint8_t));

        if (!pickledPtr)
        {
            LOGE(" ## serializeJni(): failure - pickledPtr buffer OOM");
            errorMessage = "pickledPtr buffer OOM";
        }
        else
        {
            size_t result = olm_pickle_account(accountPtr,
                                               (void const *)keyPtr,
                                               keyLength,
                                               (void*)pickledPtr,
                                               pickledLength);
            if (result == olm_error())
            {
                errorMessage = olm_account_last_error(accountPtr);
                LOGE(" ## serializeJni(): failure - olm_pickle_account() Msg=%s", errorMessage);
            }
            else
            {
                LOGD(" ## serializeJni(): success - result=%lu pickled=%.*s", static_cast<long unsigned int>(result), static_cast<int>(pickledLength), static_cast<char*>(pickledPtr));
                pickledDataRetValue = env->NewByteArray(pickledLength);
                env->SetByteArrayRegion(pickledDataRetValue, 0 , pickledLength, (jbyte*)pickledPtr);
            }

            free(pickledPtr);
        }
    }

    // free alloc
    if (keyPtr)
    {
        if (keyIsCopied) {
            memset(keyPtr, 0, (size_t)env->GetArrayLength(aKeyBuffer));
        }
        env->ReleaseByteArrayElements(aKeyBuffer, keyPtr, JNI_ABORT);
    }

    if (errorMessage)
    {
        env->ThrowNew(env->FindClass("java/lang/Exception"), errorMessage);
    }

    return pickledDataRetValue;
}

/**
 * Allocate a new account and initialise it with the serialisation data.<br>
 * @param aSerializedDataBuffer the account serialisation buffer
 * @param aKeyBuffer the key used to encrypt the serialized account data
 * @return the deserialised account
 **/
JNIEXPORT jlong OLM_ACCOUNT_FUNC_DEF(deserializeJni)(JNIEnv *env, jobject thiz, jbyteArray aSerializedDataBuffer, jbyteArray aKeyBuffer)
{
    const char* errorMessage = NULL;

    OlmAccount* accountPtr = NULL;

    jbyte* keyPtr = NULL;
    jboolean keyIsCopied = JNI_FALSE;
    jbyte* pickledPtr = NULL;

    LOGD("## deserializeJni(): IN");

    if (!aKeyBuffer)
    {
        LOGE(" ## deserializeJni(): failure - invalid key");
        errorMessage = "invalid key";
    }
    else if (!aSerializedDataBuffer)
    {
        LOGE(" ## deserializeJni(): failure - invalid serialized data");
        errorMessage = "invalid serialized data";
    }
    else if (!(accountPtr = initializeAccountMemory()))
    {
        LOGE(" ## deserializeJni(): failure - account failure OOM");
        errorMessage = "account failure OOM";
    }
    else if (!(keyPtr = env->GetByteArrayElements(aKeyBuffer, &keyIsCopied)))
    {
        LOGE(" ## deserializeJni(): failure - keyPtr JNI allocation OOM");
        errorMessage = "keyPtr JNI allocation OOM";
    }
    else if (!(pickledPtr = env->GetByteArrayElements(aSerializedDataBuffer, 0)))
    {
        LOGE(" ## deserializeJni(): failure - pickledPtr JNI allocation OOM");
        errorMessage = "pickledPtr JNI allocation OOM";
    }
    else
    {
        size_t pickledLength = (size_t)env->GetArrayLength(aSerializedDataBuffer);
        size_t keyLength = (size_t)env->GetArrayLength(aKeyBuffer);
        LOGD(" ## deserializeJni(): pickledLength=%lu keyLength=%lu",static_cast<long unsigned int>(pickledLength), static_cast<long unsigned int>(keyLength));
        LOGD(" ## deserializeJni(): pickled=%.*s", static_cast<int> (pickledLength), (char const *)pickledPtr);

        size_t result = olm_unpickle_account(accountPtr,
                                             (void const *)keyPtr,
                                             keyLength,
                                             (void*)pickledPtr,
                                             pickledLength);
        if (result == olm_error())
        {
            errorMessage = olm_account_last_error(accountPtr);
            LOGE(" ## deserializeJni(): failure - olm_unpickle_account() Msg=%s", errorMessage);
        }
        else
        {
            LOGD(" ## deserializeJni(): success - result=%lu ", static_cast<long unsigned int>(result));
        }
    }

    // free alloc
    if (keyPtr)
    {
        if (keyIsCopied) {
            memset(keyPtr, 0, (size_t)env->GetArrayLength(aKeyBuffer));
        }
        env->ReleaseByteArrayElements(aKeyBuffer, keyPtr, JNI_ABORT);
    }

    if (pickledPtr)
    {
        env->ReleaseByteArrayElements(aSerializedDataBuffer, pickledPtr, JNI_ABORT);
    }

    if (errorMessage)
    {
        if (accountPtr)
        {
            olm_clear_account(accountPtr);
            free(accountPtr);
        }
        env->ThrowNew(env->FindClass("java/lang/Exception"), errorMessage);
    }

    return (jlong)(intptr_t)accountPtr;
}
