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

#include "olm_manager.h"

using namespace AndroidOlmSdk;

JNIEXPORT jstring OLM_MANAGER_FUNC_DEF(getOlmLibVersionJni)(JNIEnv* env, jobject thiz)
{
    uint8_t majorVer=0, minorVer=0, patchVer=0;
    jstring returnValueStr=0;
    char buff[150];

    olm_get_library_version(&majorVer, &minorVer, &patchVer);
    LOGD("## getOlmLibVersionJni(): Major=%d Minor=%d Patch=%d", majorVer, minorVer, patchVer);

    snprintf(buff, sizeof(buff), "%d.%d.%d", majorVer, minorVer, patchVer);
    returnValueStr = env->NewStringUTF((const char*)buff);

    return returnValueStr;
}