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

#include "olm_utility.h"

using namespace AndroidOlmSdk;

OlmUtility* initializeUtilityMemory()
{
    size_t utilitySize = olm_utility_size();
    OlmUtility* utilityPtr = (OlmUtility*)malloc(utilitySize);

    if (utilityPtr)
    {
        utilityPtr = olm_utility(utilityPtr);
        LOGD("## initializeUtilityMemory(): success - OLM utility size=%lu",static_cast<long unsigned int>(utilitySize));
    }
    else
    {
        LOGE("## initializeUtilityMemory(): failure - OOM");
    }

    return utilityPtr;
}

JNIEXPORT jlong OLM_UTILITY_FUNC_DEF(createUtilityJni)(JNIEnv *env, jobject thiz)
{
    OlmUtility* utilityPtr = initializeUtilityMemory();

    LOGD("## createUtilityJni(): IN");

    // init account memory allocation
    if (!utilityPtr)
    {
        LOGE(" ## createUtilityJni(): failure - init OOM");
        env->ThrowNew(env->FindClass("java/lang/Exception"), "init OOM");
    }
    else
    {
       LOGD(" ## createUtilityJni(): success");
    }

    return (jlong)(intptr_t)utilityPtr;
}


JNIEXPORT void OLM_UTILITY_FUNC_DEF(releaseUtilityJni)(JNIEnv *env, jobject thiz)
{
    OlmUtility* utilityPtr = getUtilityInstanceId(env, thiz);

    LOGD("## releaseUtilityJni(): IN");

    if (!utilityPtr)
    {
        LOGE("## releaseUtilityJni(): failure - utility ptr=NULL");
    }
    else
    {
        olm_clear_utility(utilityPtr);
        free(utilityPtr);
    }
}


/**
 * Verify an ed25519 signature.
 * @param aSignature the base64-encoded message signature to be checked.
 * @param aKey the ed25519 key (fingerprint key)
 * @param aMessage the message which was signed
 * @return 0 if validation succeed, an error message string if operation failed
 */
JNIEXPORT jstring OLM_UTILITY_FUNC_DEF(verifyEd25519SignatureJni)(JNIEnv *env, jobject thiz, jbyteArray aSignatureBuffer, jbyteArray aKeyBuffer, jbyteArray aMessageBuffer)
{
    jstring errorMessageRetValue = 0;
    OlmUtility* utilityPtr = getUtilityInstanceId(env, thiz);
    jbyte* signaturePtr = NULL;
    jbyte* keyPtr = NULL;
    jbyte* messagePtr = NULL;
    jboolean messageWasCopied = JNI_FALSE;

    LOGD("## verifyEd25519SignatureJni(): IN");

    if (!utilityPtr)
    {
        LOGE(" ## verifyEd25519SignatureJni(): failure - invalid utility ptr=NULL");
    }
    else if (!aSignatureBuffer || !aKeyBuffer || !aMessageBuffer)
    {
        LOGE(" ## verifyEd25519SignatureJni(): failure - invalid input parameters ");
    }
    else if (!(signaturePtr = env->GetByteArrayElements(aSignatureBuffer, 0)))
    {
        LOGE(" ## verifyEd25519SignatureJni(): failure - signature JNI allocation OOM");
    }
    else if (!(keyPtr = env->GetByteArrayElements(aKeyBuffer, 0)))
    {
        LOGE(" ## verifyEd25519SignatureJni(): failure - key JNI allocation OOM");
    }
    else if (!(messagePtr = env->GetByteArrayElements(aMessageBuffer, &messageWasCopied)))
    {
        LOGE(" ## verifyEd25519SignatureJni(): failure - message JNI allocation OOM");
    }
    else
    {
        size_t signatureLength = (size_t)env->GetArrayLength(aSignatureBuffer);
        size_t keyLength = (size_t)env->GetArrayLength(aKeyBuffer);
        size_t messageLength = (size_t)env->GetArrayLength(aMessageBuffer);
        LOGD(" ## verifyEd25519SignatureJni(): signatureLength=%lu keyLength=%lu messageLength=%lu",static_cast<long unsigned int>(signatureLength),static_cast<long unsigned int>(keyLength),static_cast<long unsigned int>(messageLength));
        LOGD(" ## verifyEd25519SignatureJni(): key=%.*s", static_cast<int>(keyLength), keyPtr);

        size_t result = olm_ed25519_verify(utilityPtr,
                                           (void const *)keyPtr,
                                           keyLength,
                                           (void const *)messagePtr,
                                           messageLength,
                                           (void*)signaturePtr,
                                           signatureLength);
        if (result == olm_error()) {
            const char *errorMsgPtr = olm_utility_last_error(utilityPtr);
            errorMessageRetValue = env->NewStringUTF(errorMsgPtr);
            LOGE("## verifyEd25519SignatureJni(): failure - olm_ed25519_verify Msg=%s",errorMsgPtr);
        }
        else
        {
            LOGD("## verifyEd25519SignatureJni(): success - result=%lu", static_cast<long unsigned int>(result));
        }
    }

    // free alloc
    if (signaturePtr)
    {
        env->ReleaseByteArrayElements(aSignatureBuffer, signaturePtr, JNI_ABORT);
    }

    if (keyPtr)
    {
        env->ReleaseByteArrayElements(aKeyBuffer, keyPtr, JNI_ABORT);
    }

    if (messagePtr)
    {
        if (messageWasCopied) {
            memset(messagePtr, 0, (size_t)env->GetArrayLength(aMessageBuffer));
        }
        env->ReleaseByteArrayElements(aMessageBuffer, messagePtr, JNI_ABORT);
    }

    return errorMessageRetValue;
}

/**
 * Compute the digest (SHA 256) for the message passed in parameter.<br>
 * The digest value is the function return value.
 * An exception is thrown if the operation fails.
 * @param aMessage the message
 * @return digest of the message.
 **/
JNIEXPORT jbyteArray OLM_UTILITY_FUNC_DEF(sha256Jni)(JNIEnv *env, jobject thiz, jbyteArray aMessageToHashBuffer)
{
    jbyteArray sha256Ret = 0;

    OlmUtility* utilityPtr = getUtilityInstanceId(env, thiz);
    jbyte* messagePtr = NULL;
    jboolean messageWasCopied = JNI_FALSE;

    LOGD("## sha256Jni(): IN");

    if (!utilityPtr)
    {
        LOGE(" ## sha256Jni(): failure - invalid utility ptr=NULL");
    }
    else if(!aMessageToHashBuffer)
    {
        LOGE(" ## sha256Jni(): failure - invalid message parameters ");
    }
    else if(!(messagePtr = env->GetByteArrayElements(aMessageToHashBuffer, &messageWasCopied)))
    {
        LOGE(" ## sha256Jni(): failure - message JNI allocation OOM");
    }
    else
    {
        // get lengths
        size_t messageLength = (size_t)env->GetArrayLength(aMessageToHashBuffer);
        size_t hashLength = olm_sha256_length(utilityPtr);
        void* hashValuePtr = malloc((hashLength)*sizeof(uint8_t));

        if (!hashValuePtr)
        {
            LOGE("## sha256Jni(): failure - hash value allocation OOM");
        }
        else
        {
            size_t result = olm_sha256(utilityPtr,
                                       (void const *)messagePtr,
                                       messageLength,
                                       (void *)hashValuePtr,
                                       hashLength);
            if (result == olm_error())
            {
                LOGE("## sha256Jni(): failure - hash creation Msg=%s",(const char *)olm_utility_last_error(utilityPtr));
            }
            else
            {
                LOGD("## sha256Jni(): success - result=%lu hashValue=%.*s",static_cast<long unsigned int>(result), static_cast<int>(result), (char*)hashValuePtr);
                sha256Ret = env->NewByteArray(result);
                env->SetByteArrayRegion(sha256Ret, 0 , result, (jbyte*)hashValuePtr);
            }

            free(hashValuePtr);
        }
    }

    if (messagePtr)
    {
        if (messageWasCopied) {
            memset(messagePtr, 0, (size_t)env->GetArrayLength(aMessageToHashBuffer));
        }
        env->ReleaseByteArrayElements(aMessageToHashBuffer, messagePtr, JNI_ABORT);
    }

    return sha256Ret;
}
