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

#ifndef _OMLSAS_H
#define _OMLSAS_H

#include "olm_jni.h"
#include "olm/sas.h"

#define OLM_SAS_FUNC_DEF(func_name) FUNC_DEF(OlmSAS,func_name)

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT jlong OLM_SAS_FUNC_DEF(createNewSASJni)(JNIEnv *env, jobject thiz);
JNIEXPORT void OLM_SAS_FUNC_DEF(releaseSASJni)(JNIEnv *env, jobject thiz);
JNIEXPORT jbyteArray OLM_SAS_FUNC_DEF(getPubKeyJni)(JNIEnv *env, jobject thiz);
JNIEXPORT void OLM_SAS_FUNC_DEF(setTheirPubKey)(JNIEnv *env, jobject thiz,jbyteArray pubKey);
JNIEXPORT jbyteArray OLM_SAS_FUNC_DEF(generateShortCodeJni)(JNIEnv *env, jobject thiz, jbyteArray infoStringBytes, jint byteNb);
JNIEXPORT jbyteArray OLM_SAS_FUNC_DEF(calculateMacJni)(JNIEnv *env, jobject thiz, jbyteArray messageBuffer, jbyteArray infoBuffer);
JNIEXPORT jbyteArray OLM_SAS_FUNC_DEF(calculateMacFixedBase64Jni)(JNIEnv *env, jobject thiz, jbyteArray messageBuffer, jbyteArray infoBuffer);
JNIEXPORT jbyteArray OLM_SAS_FUNC_DEF(calculateMacLongKdfJni)(JNIEnv *env, jobject thiz, jbyteArray messageBuffer, jbyteArray infoBuffer);

#ifdef __cplusplus
}
#endif

#endif
