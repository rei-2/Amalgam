/* Copyright 2021 The Matrix.org Foundation C.I.C.
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

#include <string.h>

#include "olm/pickle_encoding.h"

size_t add_junk_suffix_to_pickle(void const * key, size_t key_length,
    void * pickled, size_t pickled_length, size_t junk_length)
{
    size_t raw_length = _olm_enc_input((uint8_t const *) key, key_length,
        (uint8_t *) pickled, pickled_length, nullptr);

    /* Insert junk. */
    size_t new_length = raw_length + junk_length;
    uint8_t * pos = (uint8_t *) pickled + raw_length;
    while (junk_length--) {
        *pos++ = 255;
    }

    void * dest = _olm_enc_output_pos((uint8_t *) pickled, new_length);
    memmove(dest, pickled, new_length);

    return _olm_enc_output(
        (uint8_t const *) key, key_length, (uint8_t *) pickled, new_length);
}
