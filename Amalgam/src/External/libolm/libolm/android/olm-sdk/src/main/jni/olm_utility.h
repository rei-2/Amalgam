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

#ifndef _OLMUTILITY_H
#define _OLMUTILITY_H

#include "olm_jni.h"
#include "olm/olm.h"

#define OLM_UTILITY_FUNC_DEF(func_name) FUNC_DEF(OlmUtility,func_name)


#ifdef __cplusplus
extern "C" {
#endif
JNIEXPORT jlong   OLM_UTILITY_FUNC_DEF(createUtilityJni)(JNIEnv *env, jobject thiz);
JNIEXPORT void    OLM_UTILITY_FUNC_DEF(releaseUtilityJni)(JNIEnv *env, jobject thiz);
JNIEXPORT jstring OLM_UTILITY_FUNC_DEF(verifyEd25519SignatureJni)(JNIEnv *env, jobject thiz, jbyteArray aSignature, jbyteArray aKey, jbyteArray aMessage);
JNIEXPORT jbyteArray OLM_UTILITY_FUNC_DEF(sha256Jni)(JNIEnv *env, jobject thiz, jbyteArray aMessageToHash);
#ifdef __cplusplus
}
#endif



#endif
