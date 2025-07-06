/*
 * Copyright 2018,2019 New Vector Ltd
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

#include "olm_pk.h"

#include "olm/olm.h"

using namespace AndroidOlmSdk;

OlmPkEncryption * initializePkEncryptionMemory()
{
    size_t encryptionSize = olm_pk_encryption_size();
    OlmPkEncryption *encryptionPtr = (OlmPkEncryption *)malloc(encryptionSize);

    if (encryptionPtr)
    {
        // init encryption object
        encryptionPtr = olm_pk_encryption(encryptionPtr);
        LOGD(
            "## initializePkEncryptionMemory(): success - OLM encryption size=%lu",
            static_cast<long unsigned int>(encryptionSize)
        );
    }
    else
    {
        LOGE("## initializePkEncryptionMemory(): failure - OOM");
    }

    return encryptionPtr;
}

JNIEXPORT jlong OLM_PK_ENCRYPTION_FUNC_DEF(createNewPkEncryptionJni)(JNIEnv *env, jobject thiz)
{
    const char* errorMessage = NULL;
    OlmPkEncryption *encryptionPtr = initializePkEncryptionMemory();

    // init encryption memory allocation
    if (!encryptionPtr)
    {
        LOGE("## createNewPkEncryptionJni(): failure - init encryption OOM");
        errorMessage = "init encryption OOM";
    }
    else
    {
        LOGD("## createNewPkEncryptionJni(): success - OLM encryption created");
        LOGD(
            "## createNewPkEncryptionJni(): encryptionPtr=%p (jlong)(intptr_t)encryptionPtr=%lld",
            encryptionPtr, (jlong)(intptr_t)encryptionPtr
        );
    }

    if (errorMessage)
    {
        // release the allocated data
        if (encryptionPtr)
        {
            olm_clear_pk_encryption(encryptionPtr);
            free(encryptionPtr);
        }
        env->ThrowNew(env->FindClass("java/lang/Exception"), errorMessage);
    }

    return (jlong)(intptr_t)encryptionPtr;
}

JNIEXPORT void OLM_PK_ENCRYPTION_FUNC_DEF(releasePkEncryptionJni)(JNIEnv *env, jobject thiz)
{
    LOGD("## releasePkEncryptionJni(): IN");

    OlmPkEncryption* encryptionPtr = getPkEncryptionInstanceId(env, thiz);

    if (!encryptionPtr)
    {
        LOGE(" ## releasePkEncryptionJni(): failure - invalid Encryption ptr=NULL");
    }
    else
    {
        LOGD(" ## releasePkEncryptionJni(): encryptionPtr=%p", encryptionPtr);
        olm_clear_pk_encryption(encryptionPtr);

        LOGD(" ## releasePkEncryptionJni(): IN");
        // even if free(NULL) does not crash, logs are performed for debug
        // purpose
        free(encryptionPtr);
        LOGD(" ## releasePkEncryptionJni(): OUT");
    }
}

JNIEXPORT void OLM_PK_ENCRYPTION_FUNC_DEF(setRecipientKeyJni)(
    JNIEnv *env, jobject thiz, jbyteArray aKeyBuffer
) {
    const char *errorMessage = NULL;
    jbyte *keyPtr = NULL;

    OlmPkEncryption *encryptionPtr = getPkEncryptionInstanceId(env, thiz);

    if (!encryptionPtr)
    {
        LOGE(" ## pkSetRecipientKeyJni(): failure - invalid Encryption ptr=NULL");
    }
    else if (!aKeyBuffer)
    {
        LOGE(" ## pkSetRecipientKeyJni(): failure - invalid key");
        errorMessage = "invalid key";
    }
    else if (!(keyPtr = env->GetByteArrayElements(aKeyBuffer, 0)))
    {
        LOGE(" ## pkSetRecipientKeyJni(): failure - key JNI allocation OOM");
        errorMessage = "key JNI allocation OOM";
    }
    else
    {
        if (olm_pk_encryption_set_recipient_key(encryptionPtr, keyPtr, (size_t)env->GetArrayLength(aKeyBuffer)) == olm_error())
        {
            errorMessage = olm_pk_encryption_last_error(encryptionPtr);
            LOGE(
                " ## pkSetRecipientKeyJni(): failure - olm_pk_encryption_set_recipient_key Msg=%s",
                errorMessage
            );
        }
    }

    if (keyPtr)
    {
        env->ReleaseByteArrayElements(aKeyBuffer, keyPtr, JNI_ABORT);
    }

    if (errorMessage)
    {
        env->ThrowNew(env->FindClass("java/lang/Exception"), errorMessage);
    }
}

JNIEXPORT jbyteArray OLM_PK_ENCRYPTION_FUNC_DEF(encryptJni)(
    JNIEnv *env, jobject thiz, jbyteArray aPlaintextBuffer, jobject aEncryptedMsg
) {
    jbyteArray encryptedMsgRet = 0;
    const char* errorMessage = NULL;
    jbyte *plaintextPtr = NULL;
    jboolean plaintextIsCopied = JNI_FALSE;

    OlmPkEncryption *encryptionPtr = getPkEncryptionInstanceId(env, thiz);
    jclass encryptedMsgJClass = 0;
    jfieldID macFieldId;
    jfieldID ephemeralFieldId;

    if (!encryptionPtr)
    {
        LOGE(" ## pkEncryptJni(): failure - invalid Encryption ptr=NULL");
    }
    else if (!aPlaintextBuffer)
    {
        LOGE(" ## pkEncryptJni(): failure - invalid clear message");
        errorMessage = "invalid clear message";
    }
    else if (!(plaintextPtr = env->GetByteArrayElements(aPlaintextBuffer, &plaintextIsCopied)))
    {
        LOGE(" ## pkEncryptJni(): failure - plaintext JNI allocation OOM");
        errorMessage = "plaintext JNI allocation OOM";
    }
    else if (!(encryptedMsgJClass = env->GetObjectClass(aEncryptedMsg)))
    {
        LOGE(" ## pkEncryptJni(): failure - unable to get encrypted message class");
        errorMessage = "unable to get encrypted message class";
    }
    else if (!(macFieldId = env->GetFieldID(encryptedMsgJClass, "mMac", "Ljava/lang/String;")))
    {
        LOGE("## pkEncryptJni(): failure - unable to get MAC field");
        errorMessage = "unable to get MAC field";
    }
    else if (!(ephemeralFieldId = env->GetFieldID(encryptedMsgJClass, "mEphemeralKey", "Ljava/lang/String;")))
    {
        LOGE("## pkEncryptJni(): failure - unable to get ephemeral key field");
        errorMessage = "unable to get ephemeral key field";
    }
    else
    {
        size_t plaintextLength = (size_t)env->GetArrayLength(aPlaintextBuffer);
        size_t ciphertextLength = olm_pk_ciphertext_length(encryptionPtr, plaintextLength);
        size_t macLength = olm_pk_mac_length(encryptionPtr);
        size_t ephemeralLength = olm_pk_key_length();
        uint8_t *ciphertextPtr = NULL, *macPtr = NULL, *ephemeralPtr = NULL;
        size_t randomLength = olm_pk_encrypt_random_length(encryptionPtr);
        uint8_t *randomBuffPtr = NULL;
        LOGD("## pkEncryptJni(): randomLength=%lu",static_cast<long unsigned int>(randomLength));
        if (!(ciphertextPtr = (uint8_t*)malloc(ciphertextLength)))
        {
            LOGE("## pkEncryptJni(): failure - ciphertext JNI allocation OOM");
            errorMessage = "ciphertext JNI allocation OOM";
        }
        else if (!(macPtr = (uint8_t*)malloc(macLength + 1)))
        {
            LOGE("## pkEncryptJni(): failure - MAC JNI allocation OOM");
            errorMessage = "MAC JNI allocation OOM";
        }
        else if (!(ephemeralPtr = (uint8_t*)malloc(ephemeralLength + 1)))
        {
            LOGE("## pkEncryptJni(): failure: ephemeral key JNI allocation OOM");
            errorMessage = "ephemeral JNI allocation OOM";
        }
        else if (!setRandomInBuffer(env, &randomBuffPtr, randomLength))
        {
            LOGE("## pkEncryptJni(): failure - random buffer init");
            errorMessage = "random buffer init";
        }
        else
        {
            macPtr[macLength] = '\0';
            ephemeralPtr[ephemeralLength] = '\0';

            size_t returnValue = olm_pk_encrypt(
                encryptionPtr,
                plaintextPtr, plaintextLength,
                ciphertextPtr, ciphertextLength,
                macPtr, macLength,
                ephemeralPtr, ephemeralLength,
                randomBuffPtr, randomLength
            );

            if (returnValue == olm_error())
            {
                errorMessage = olm_pk_encryption_last_error(encryptionPtr);
                LOGE("## pkEncryptJni(): failure - olm_pk_encrypt Msg=%s", errorMessage);
            }
            else
            {
                encryptedMsgRet = env->NewByteArray(ciphertextLength);
                env->SetByteArrayRegion(
                    encryptedMsgRet, 0, ciphertextLength, (jbyte*)ciphertextPtr
                );

                jstring macStr = env->NewStringUTF((char*)macPtr);
                env->SetObjectField(aEncryptedMsg, macFieldId, macStr);
                jstring ephemeralStr = env->NewStringUTF((char*)ephemeralPtr);
                env->SetObjectField(aEncryptedMsg, ephemeralFieldId, ephemeralStr);
            }
        }

        if (randomBuffPtr)
        {
            memset(randomBuffPtr, 0, randomLength);
            free(randomBuffPtr);
        }
        if (ephemeralPtr)
        {
            free(ephemeralPtr);
        }
        if (macPtr)
        {
            free(macPtr);
        }
        if (ciphertextPtr)
        {
            free(ciphertextPtr);
        }
    }

    if (plaintextPtr)
    {
        if (plaintextIsCopied)
        {
            memset(plaintextPtr, 0, (size_t)env->GetArrayLength(aPlaintextBuffer));
        }
        env->ReleaseByteArrayElements(aPlaintextBuffer, plaintextPtr, JNI_ABORT);
    }

    if (errorMessage)
    {
        env->ThrowNew(env->FindClass("java/lang/Exception"), errorMessage);
    }

    return encryptedMsgRet;
}

OlmPkDecryption * initializePkDecryptionMemory()
{
    size_t decryptionSize = olm_pk_decryption_size();
    OlmPkDecryption *decryptionPtr = (OlmPkDecryption *)malloc(decryptionSize);

    if (decryptionPtr)
    {
        // init decryption object
        decryptionPtr = olm_pk_decryption(decryptionPtr);
        LOGD(
            "## initializePkDecryptionMemory(): success - OLM decryption size=%lu",
            static_cast<long unsigned int>(decryptionSize)
        );
    }
    else
    {
        LOGE("## initializePkDecryptionMemory(): failure - OOM");
    }

    return decryptionPtr;
}

JNIEXPORT jlong OLM_PK_DECRYPTION_FUNC_DEF(createNewPkDecryptionJni)(JNIEnv *env, jobject thiz)
{
    const char* errorMessage = NULL;
    OlmPkDecryption *decryptionPtr = initializePkDecryptionMemory();

    // init encryption memory allocation
    if (!decryptionPtr)
    {
        LOGE("## createNewPkDecryptionJni(): failure - init decryption OOM");
        errorMessage = "init decryption OOM";
    }
    else
    {
        LOGD("## createNewPkDecryptionJni(): success - OLM decryption created");
        LOGD(
            "## createNewPkDecryptionJni(): decryptionPtr=%p (jlong)(intptr_t)decryptionPtr=%lld",
            decryptionPtr, (jlong)(intptr_t)decryptionPtr
        );
    }

    if (errorMessage)
    {
        // release the allocated data
        if (decryptionPtr)
        {
            olm_clear_pk_decryption(decryptionPtr);
            free(decryptionPtr);
        }
        env->ThrowNew(env->FindClass("java/lang/Exception"), errorMessage);
    }

    return (jlong)(intptr_t)decryptionPtr;
}

JNIEXPORT void OLM_PK_DECRYPTION_FUNC_DEF(releasePkDecryptionJni)(JNIEnv *env, jobject thiz)
{
    LOGD("## releasePkDecryptionJni(): IN");

    OlmPkDecryption* decryptionPtr = getPkDecryptionInstanceId(env, thiz);

    if (!decryptionPtr)
    {
        LOGE(" ## releasePkDecryptionJni(): failure - invalid Decryption ptr=NULL");
    }
    else
    {
        LOGD(" ## releasePkDecryptionJni(): decryptionPtr=%p", encryptionPtr);
        olm_clear_pk_decryption(decryptionPtr);

        LOGD(" ## releasePkDecryptionJni(): IN");
        // even if free(NULL) does not crash, logs are performed for debug
        // purpose
        free(decryptionPtr);
        LOGD(" ## releasePkDecryptionJni(): OUT");
    }
}

JNIEXPORT jint OLM_PK_DECRYPTION_FUNC_DEF(privateKeyLength)(JNIEnv *env, jobject thiz)
{
    return (jint) olm_pk_private_key_length();
}

JNIEXPORT jbyteArray OLM_PK_DECRYPTION_FUNC_DEF(setPrivateKeyJni)(JNIEnv *env, jobject thiz, jbyteArray key)
{
    jbyteArray publicKeyRet = 0;
    jbyte *keyPtr = NULL;
    jboolean keyWasCopied = JNI_FALSE;

    const char* errorMessage = NULL;

    OlmPkDecryption* decryptionPtr = getPkDecryptionInstanceId(env, thiz);

    if (!decryptionPtr)
    {
        LOGE(" ## pkSetPrivateKeyJni(): failure - invalid Decryption ptr=NULL");
    }
    else if (!key)
    {
        LOGE(" ## pkSetPrivateKeyJni(): failure - invalid key");
        errorMessage = "invalid key";
    }
    else if (!(keyPtr = env->GetByteArrayElements(key, &keyWasCopied)))
    {
        LOGE(" ## pkSetPrivateKeyJni(): failure - key JNI allocation OOM");
        errorMessage = "key JNI allocation OOM";
    }
    else
    {
        size_t publicKeyLength = olm_pk_key_length();
        uint8_t *publicKeyPtr = NULL;
        size_t keyLength = (size_t)env->GetArrayLength(key);
        if (!(publicKeyPtr = (uint8_t*)malloc(publicKeyLength)))
        {
            LOGE("## pkSetPrivateKeyJni(): failure - public key JNI allocation OOM");
            errorMessage = "public key JNI allocation OOM";
        }
        else
        {
            size_t returnValue = olm_pk_key_from_private(
                decryptionPtr,
                publicKeyPtr, publicKeyLength,
                keyPtr, keyLength
            );
            if (returnValue == olm_error())
            {
                errorMessage = olm_pk_decryption_last_error(decryptionPtr);
                LOGE(" ## pkSetPrivateKeyJni(): failure - olm_pk_key_from_private Msg=%s", errorMessage);
            }
            else
            {
                publicKeyRet = env->NewByteArray(publicKeyLength);
                env->SetByteArrayRegion(
                    publicKeyRet, 0, publicKeyLength, (jbyte*)publicKeyPtr
                );
            }
        }
    }

    if (keyPtr)
    {
        if (keyWasCopied)
        {
            memset(keyPtr, 0, (size_t)env->GetArrayLength(key));
        }
        env->ReleaseByteArrayElements(key, keyPtr, JNI_ABORT);
    }

    if (errorMessage)
    {
        env->ThrowNew(env->FindClass("java/lang/Exception"), errorMessage);
    }

    return publicKeyRet;
}

JNIEXPORT jbyteArray OLM_PK_DECRYPTION_FUNC_DEF(generateKeyJni)(JNIEnv *env, jobject thiz)
{
    size_t randomLength = olm_pk_private_key_length();
    uint8_t *randomBuffPtr = NULL;

    jbyteArray publicKeyRet = 0;
    uint8_t *publicKeyPtr = NULL;
    size_t publicKeyLength = olm_pk_key_length();
    const char* errorMessage = NULL;

    OlmPkDecryption *decryptionPtr = getPkDecryptionInstanceId(env, thiz);

    if (!decryptionPtr)
    {
        LOGE(" ## pkGenerateKeyJni(): failure - invalid Decryption ptr=NULL");
        errorMessage = "invalid Decryption ptr=NULL";
    }
    else if (!setRandomInBuffer(env, &randomBuffPtr, randomLength))
    {
        LOGE("## pkGenerateKeyJni(): failure - random buffer init");
        errorMessage = "random buffer init";
    }
    else if (!(publicKeyPtr = static_cast<uint8_t*>(malloc(publicKeyLength))))
    {
        LOGE("## pkGenerateKeyJni(): failure - public key allocation OOM");
        errorMessage = "public key allocation OOM";
    }
    else
    {
        if (olm_pk_key_from_private(decryptionPtr, publicKeyPtr, publicKeyLength, randomBuffPtr, randomLength) == olm_error())
        {
            errorMessage = olm_pk_decryption_last_error(decryptionPtr);
            LOGE("## pkGenerateKeyJni(): failure - olm_pk_generate_key Msg=%s", errorMessage);
        }
        else
        {
            publicKeyRet = env->NewByteArray(publicKeyLength);
            env->SetByteArrayRegion(publicKeyRet, 0, publicKeyLength, (jbyte*)publicKeyPtr);
            LOGD("## pkGenerateKeyJni(): public key generated");
        }
    }

    if (randomBuffPtr)
    {
        memset(randomBuffPtr, 0, randomLength);
        free(randomBuffPtr);
    }

    if (errorMessage)
    {
        env->ThrowNew(env->FindClass("java/lang/Exception"), errorMessage);
    }

    return publicKeyRet;
}

JNIEXPORT jbyteArray OLM_PK_DECRYPTION_FUNC_DEF(privateKeyJni)(JNIEnv *env, jobject thiz)
{
    jbyteArray privateKeyRet = 0;

    const char* errorMessage = NULL;

    OlmPkDecryption* decryptionPtr = getPkDecryptionInstanceId(env, thiz);

    if (!decryptionPtr)
    {
        LOGE(" ## pkPrivateKeyJni(): failure - invalid Decryption ptr=NULL");
    }
    else
    {
        size_t privateKeyLength = olm_pk_private_key_length();
        uint8_t *privateKeyPtr = NULL;
        if (!(privateKeyPtr = (uint8_t*)malloc(privateKeyLength)))
        {
            LOGE("## pkPrivateKeyJni(): failure - private key JNI allocation OOM");
            errorMessage = "private key JNI allocation OOM";
        }
        else
        {
            size_t returnValue = olm_pk_get_private_key(
                decryptionPtr,
                privateKeyPtr, privateKeyLength
            );
            if (returnValue == olm_error())
            {
                errorMessage = olm_pk_decryption_last_error(decryptionPtr);
                LOGE(" ## pkPrivateKeyJni(): failure - olm_pk_get_private_key Msg=%s", errorMessage);
            }
            else
            {
                privateKeyRet = env->NewByteArray(privateKeyLength);
                env->SetByteArrayRegion(
                    privateKeyRet, 0, privateKeyLength, (jbyte*)privateKeyPtr
                );
                memset(privateKeyPtr, 0, privateKeyLength);
            }
        }
    }

    if (errorMessage)
    {
        env->ThrowNew(env->FindClass("java/lang/Exception"), errorMessage);
    }

    return privateKeyRet;
}

JNIEXPORT jbyteArray OLM_PK_DECRYPTION_FUNC_DEF(decryptJni)(
    JNIEnv *env, jobject thiz, jobject aEncryptedMsg
) {
    const char* errorMessage = NULL;
    OlmPkDecryption *decryptionPtr = getPkDecryptionInstanceId(env, thiz);

    jclass encryptedMsgJClass = 0;
    jstring ciphertextJstring = 0;
    jstring macJstring = 0;
    jstring ephemeralKeyJstring = 0;
    jfieldID ciphertextFieldId;
    jfieldID macFieldId;
    jfieldID ephemeralKeyFieldId;

    const char *ciphertextPtr = NULL;
    const char *macPtr = NULL;
    const char *ephemeralKeyPtr = NULL;

    jbyteArray decryptedMsgRet = 0;

    if (!decryptionPtr)
    {
        LOGE(" ## pkDecryptJni(): failure - invalid Decryption ptr=NULL");
        errorMessage = "invalid Decryption ptr=NULL";
    }
    else if (!aEncryptedMsg)
    {
        LOGE(" ## pkDecryptJni(): failure - invalid encrypted message");
        errorMessage = "invalid encrypted message";
    }
    else if (!(encryptedMsgJClass = env->GetObjectClass(aEncryptedMsg)))
    {
        LOGE("## pkDecryptJni(): failure - unable to get encrypted message class");
        errorMessage = "unable to get encrypted message class";
    }
    else if (!(ciphertextFieldId = env->GetFieldID(encryptedMsgJClass,"mCipherText","Ljava/lang/String;")))
    {
        LOGE("## pkDecryptJni(): failure - unable to get message field");
        errorMessage = "unable to get message field";
    }
    else if (!(ciphertextJstring = (jstring)env->GetObjectField(aEncryptedMsg, ciphertextFieldId)))
    {
        LOGE("## pkDecryptJni(): failure - no ciphertext");
        errorMessage = "no ciphertext";
    }
    else if (!(ciphertextPtr = env->GetStringUTFChars(ciphertextJstring, 0)))
    {
        LOGE("## pkDecryptJni(): failure - ciphertext JNI allocation OOM");
        errorMessage = "ciphertext JNI allocation OOM";
    }
    else if (!(ciphertextJstring = (jstring)env->GetObjectField(aEncryptedMsg, ciphertextFieldId)))
    {
        LOGE("## pkDecryptJni(): failure - no ciphertext");
        errorMessage = "no ciphertext";
    }
    else if (!(ciphertextPtr = env->GetStringUTFChars(ciphertextJstring, 0)))
    {
        LOGE("## decryptMessageJni(): failure - ciphertext JNI allocation OOM");
        errorMessage = "ciphertext JNI allocation OOM";
    }
    else if (!(macFieldId = env->GetFieldID(encryptedMsgJClass,"mMac","Ljava/lang/String;")))
    {
        LOGE("## pkDecryptJni(): failure - unable to get MAC field");
        errorMessage = "unable to get MAC field";
    }
    else if (!(macJstring = (jstring)env->GetObjectField(aEncryptedMsg, macFieldId)))
    {
        LOGE("## pkDecryptJni(): failure - no MAC");
        errorMessage = "no MAC";
    }
    else if (!(macPtr = env->GetStringUTFChars(macJstring, 0)))
    {
        LOGE("## pkDecryptJni(): failure - MAC JNI allocation OOM");
        errorMessage = "ciphertext JNI allocation OOM";
    }
    else if (!(ephemeralKeyFieldId = env->GetFieldID(encryptedMsgJClass,"mEphemeralKey","Ljava/lang/String;")))
    {
        LOGE("## pkDecryptJni(): failure - unable to get ephemeral key field");
        errorMessage = "unable to get ephemeral key field";
    }
    else if (!(ephemeralKeyJstring = (jstring)env->GetObjectField(aEncryptedMsg, ephemeralKeyFieldId)))
    {
        LOGE("## pkDecryptJni(): failure - no ephemeral key");
        errorMessage = "no ephemeral key";
    }
    else if (!(ephemeralKeyPtr = env->GetStringUTFChars(ephemeralKeyJstring, 0)))
    {
        LOGE("## pkDecryptJni(): failure - ephemeral key JNI allocation OOM");
        errorMessage = "ephemeral key JNI allocation OOM";
    }
    else
    {
        size_t maxPlaintextLength = olm_pk_max_plaintext_length(
            decryptionPtr,
            (size_t)env->GetStringUTFLength(ciphertextJstring)
        );
        uint8_t *plaintextPtr = NULL;
        uint8_t *tempCiphertextPtr = NULL;
        size_t ciphertextLength = (size_t)env->GetStringUTFLength(ciphertextJstring);
        if (!(plaintextPtr = (uint8_t*)malloc(maxPlaintextLength)))
        {
            LOGE("## pkDecryptJni(): failure - plaintext JNI allocation OOM");
            errorMessage = "plaintext JNI allocation OOM";
        }
        else if (!(tempCiphertextPtr = (uint8_t*)malloc(ciphertextLength)))
        {
            LOGE("## pkDecryptJni(): failure - temp ciphertext JNI allocation OOM");
        }
        else
        {
            memcpy(tempCiphertextPtr, ciphertextPtr, ciphertextLength);
            size_t plaintextLength = olm_pk_decrypt(
                decryptionPtr,
                ephemeralKeyPtr, (size_t)env->GetStringUTFLength(ephemeralKeyJstring),
                macPtr, (size_t)env->GetStringUTFLength(macJstring),
                tempCiphertextPtr, ciphertextLength,
                plaintextPtr, maxPlaintextLength
            );
            if (plaintextLength == olm_error())
            {
                errorMessage = olm_pk_decryption_last_error(decryptionPtr);
                LOGE("## pkDecryptJni(): failure - olm_pk_decrypt Msg=%s", errorMessage);
            }
            else
            {
                decryptedMsgRet = env->NewByteArray(plaintextLength);
                env->SetByteArrayRegion(decryptedMsgRet, 0, plaintextLength, (jbyte*)plaintextPtr);
                LOGD(
                    "## pkDecryptJni(): success returnedLg=%lu OK",
                    static_cast<long unsigned int>(plaintextLength)
                );
            }
        }

        if (tempCiphertextPtr)
        {
          free(tempCiphertextPtr);
        }
        if (plaintextPtr)
        {
          memset(plaintextPtr, 0, maxPlaintextLength);
          free(plaintextPtr);
        }
    }

    if (ciphertextPtr)
    {
        env->ReleaseStringUTFChars(ciphertextJstring, ciphertextPtr);
    }
    if (macPtr)
    {
        env->ReleaseStringUTFChars(macJstring, macPtr);
    }
    if (ephemeralKeyPtr)
    {
        env->ReleaseStringUTFChars(ephemeralKeyJstring, ephemeralKeyPtr);
    }

    if (errorMessage)
    {
        env->ThrowNew(env->FindClass("java/lang/Exception"), errorMessage);
    }

    return decryptedMsgRet;
}

OlmPkSigning * initializePkSigningMemory()
{
    size_t signingSize = olm_pk_signing_size();
    OlmPkSigning *signingPtr = (OlmPkSigning *)malloc(signingSize);

    if (signingPtr)
    {
        // init encryption object
        signingPtr = olm_pk_signing(signingPtr);
        LOGD(
            "## initializePkSigningMemory(): success - OLM signing size=%lu",
            static_cast<long unsigned int>(signingSize)
        );
    }
    else
    {
        LOGE("## initializePkSigningMemory(): failure - OOM");
    }

    return signingPtr;
}

JNIEXPORT jlong OLM_PK_SIGNING_FUNC_DEF(createNewPkSigningJni)(JNIEnv *env, jobject thiz)
{
    const char* errorMessage = NULL;
    OlmPkSigning *signingPtr = initializePkSigningMemory();

    // init signing memory allocation
    if (!signingPtr)
    {
        LOGE("## createNewPkSigningJni(): failure - init signing OOM");
        errorMessage = "init signing OOM";
    }
    else
    {
        LOGD("## createNewPkSigningJni(): success - OLM signing created");
        LOGD(
            "## createNewPkSigningJni(): signingPtr=%p (jlong)(intptr_t)signingPtr=%lld",
            signingPtr, (jlong)(intptr_t)signingPtr
        );
    }

    if (errorMessage)
    {
        // release the allocated data
        if (signingPtr)
        {
            olm_clear_pk_signing(signingPtr);
            free(signingPtr);
        }
        env->ThrowNew(env->FindClass("java/lang/Exception"), errorMessage);
    }

    return (jlong)(intptr_t)signingPtr;
}

JNIEXPORT void OLM_PK_SIGNING_FUNC_DEF(releasePkSigningJni)(JNIEnv *env, jobject thiz)
{
    LOGD("## releasePkSigningJni(): IN");

    OlmPkSigning* signingPtr = getPkSigningInstanceId(env, thiz);

    if (!signingPtr)
    {
        LOGE(" ## releasePkSigningJni(): failure - invalid Signing ptr=NULL");
    }
    else
    {
        LOGD(" ## releasePkSigningJni(): signingPtr=%p", signingPtr);
        olm_clear_pk_signing(signingPtr);

        LOGD(" ## releasePkSigningJni(): IN");
        // even if free(NULL) does not crash, logs are performed for debug
        // purpose
        free(signingPtr);
        LOGD(" ## releasePkSigningJni(): OUT");
    }
}

JNIEXPORT jbyteArray OLM_PK_SIGNING_FUNC_DEF(generateSeedJni)(JNIEnv *env, jobject thiz)
{
    size_t randomLength = olm_pk_signing_seed_length();
    uint8_t *randomBuffPtr = NULL;
    jbyteArray randomRet = 0;
    const char* errorMessage = NULL;

    if (!setRandomInBuffer(env, &randomBuffPtr, randomLength))
    {
        errorMessage = "random buffer init";
        LOGE("## pkSigningGenerateSeedJni(): failure - %s", errorMessage);
    }
    else if (!(randomRet = env->NewByteArray(randomLength)))
    {
        errorMessage = "randomRet JNI allocation OOM";
        LOGE(" ## pkSigningGenerateSeedJni(): falure - %s", errorMessage);
    }
    else
    {
        env->SetByteArrayRegion(
            randomRet, 0, randomLength, (jbyte*)randomBuffPtr
        );
    }

    if (randomBuffPtr)
    {
        memset(randomBuffPtr, 0, randomLength);
        free(randomBuffPtr);
    }

    if (errorMessage)
    {
        env->ThrowNew(env->FindClass("java/lang/Exception"), errorMessage);
    }

    return randomRet;
}

JNIEXPORT jint OLM_PK_SIGNING_FUNC_DEF(seedLength)(JNIEnv *env, jobject thiz)
{
    return (jint) olm_pk_signing_seed_length();
}

JNIEXPORT jbyteArray OLM_PK_SIGNING_FUNC_DEF(setKeyFromSeedJni)(JNIEnv *env, jobject thiz, jbyteArray seed)
{
    const char* errorMessage = NULL;
    OlmPkSigning *signingPtr = getPkSigningInstanceId(env, thiz);

    jbyteArray publicKeyRet = 0;
    jbyte *seedPtr = NULL;
    jboolean seedWasCopied = JNI_FALSE;

    if (!signingPtr)
    {
        errorMessage = "invalid Siging ptr=NULL";
        LOGE(" ## setPkSigningKeyFromSeedJni(): failure - %s", errorMessage);
    }
    else if (!seed)
    {
        errorMessage = "invalid seed";
        LOGE(" ## setPkSigningKeyFromSeedJni: failure - %s", errorMessage);
    }
    else if (!(seedPtr = env->GetByteArrayElements(seed, &seedWasCopied)))
    {
        errorMessage = "seed JNI allocation OOM";
        LOGE(" ## setPkSigningKeyFromSeedJni(): failure - %s", errorMessage);
    }
    else
    {
        size_t publicKeyLength = olm_pk_signing_public_key_length();
        uint8_t *publicKeyPtr = NULL;
        size_t seedLength = (size_t)env->GetArrayLength(seed);
        if (!(publicKeyPtr = (uint8_t*)malloc(publicKeyLength)))
        {
            errorMessage = "public key JNI allocation OOM";
            LOGE(" ## setPkSigningKeyFromSeedJni(): falure - %s", errorMessage);
        }
        else
        {
            size_t returnValue = olm_pk_signing_key_from_seed(
                signingPtr,
                publicKeyPtr, publicKeyLength,
                seedPtr, seedLength
            );
            if (returnValue == olm_error())
            {
                errorMessage = olm_pk_signing_last_error(signingPtr);
                LOGE(" ## setPkSigningKeyFromSeedJni: failure - olm_pk_signing_key_from_seed Msg=%s", errorMessage);
            }
            else
            {
                if (!(publicKeyRet = env->NewByteArray(publicKeyLength))) {
                    errorMessage = "publicKeyRet JNI allocation OOM";
                    LOGE(" ## setPkSigningKeyFromSeedJni(): falure - %s", errorMessage);
                } else {
                    env->SetByteArrayRegion(
                        publicKeyRet, 0, publicKeyLength, (jbyte*)publicKeyPtr
                    );
                }
            }
        }
    }

    if (seedPtr)
    {
        if (seedWasCopied)
        {
            memset(seedPtr, 0, (size_t)env->GetArrayLength(seed));
        }
        env->ReleaseByteArrayElements(seed, seedPtr, JNI_ABORT);
    }

    if (errorMessage)
    {
        env->ThrowNew(env->FindClass("java/lang/Exception"), errorMessage);
    }

    return publicKeyRet;
}

JNIEXPORT jbyteArray OLM_PK_SIGNING_FUNC_DEF(pkSignJni)(JNIEnv *env, jobject thiz, jbyteArray aMessage)
{
    const char* errorMessage = NULL;
    OlmPkSigning *signingPtr = getPkSigningInstanceId(env, thiz);

    jbyteArray signatureRet = 0;
    jbyte *messagePtr = NULL;
    jboolean messageWasCopied = JNI_FALSE;

    if (!signingPtr)
    {
        errorMessage = "invalid Siging ptr=NULL";
        LOGE(" ## setPkSignJni(): failure - %s", errorMessage);
    }
    else if (!aMessage)
    {
        errorMessage = "message seed";
        LOGE(" ## setPkSignJni: failure - %s", errorMessage);
    }
    else if (!(messagePtr = env->GetByteArrayElements(aMessage, &messageWasCopied)))
    {
        errorMessage = "message JNI allocation OOM";
        LOGE(" ## setPkSignJni(): failure - %s", errorMessage);
    }
    else
    {
        size_t signatureLength = olm_pk_signature_length();
        uint8_t *signaturePtr = NULL;
        size_t messageLength = (size_t)env->GetArrayLength(aMessage);
        if (!(signaturePtr = (uint8_t*)malloc(signatureLength)))
        {
            errorMessage = "signature JNI allocation OOM";
            LOGE(" ## setPkSignJni(): falure - %s", errorMessage);
        }
        else
        {
            size_t returnValue = olm_pk_sign(
                signingPtr,
                (uint8_t *)messagePtr, messageLength,
                signaturePtr, signatureLength
            );
            if (returnValue == olm_error())
            {
                errorMessage = olm_pk_signing_last_error(signingPtr);
                LOGE(" ## setPkSignJni: failure - olm_pk_sign Msg=%s", errorMessage);
            }
            else
            {
                if (!(signatureRet = env->NewByteArray(signatureLength))) {
                    errorMessage = "signatureRet JNI allocation OOM";
                    LOGE(" ## setPkSignJni(): falure - %s", errorMessage);
                } else {
                    env->SetByteArrayRegion(
                        signatureRet, 0, signatureLength, (jbyte*)signaturePtr
                    );
                }
            }
        }
    }

    if (messagePtr)
    {
        if (messageWasCopied)
        {
            memset(messagePtr, 0, (size_t)env->GetArrayLength(aMessage));
        }
        env->ReleaseByteArrayElements(aMessage, messagePtr, JNI_ABORT);
    }

    if (errorMessage)
    {
        env->ThrowNew(env->FindClass("java/lang/Exception"), errorMessage);
    }

    return signatureRet;
}
