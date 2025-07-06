/* Copyright 2015 OpenMarket Ltd
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
#ifndef OLM_BASE64_HH_
#define OLM_BASE64_HH_

#include <cstddef>
#include <cstdint>

// Note: exports in this file are only for unit tests.  Nobody else should be
// using this externally
#include "olm/olm_export.h"

namespace olm {

/**
 * The number of bytes of unpadded base64 needed to encode a length of input.
 */
OLM_EXPORT std::size_t encode_base64_length(
    std::size_t input_length
);

/**
 * Encode the raw input as unpadded base64.
 * Writes encode_base64_length(input_length) bytes to the output buffer.
 * The input can overlap with the last three quarters of the output buffer.
 * That is, the input pointer may be output + output_length - input_length.
 */
OLM_EXPORT std::uint8_t * encode_base64(
    std::uint8_t const * input, std::size_t input_length,
    std::uint8_t * output
);

/**
 * The number of bytes of raw data a length of unpadded base64 will encode to.
 * Returns std::size_t(-1) if the length is not a valid length for base64.
 */
OLM_EXPORT std::size_t decode_base64_length(
    std::size_t input_length
);

/**
 * Decodes the unpadded base64 input to raw bytes.
 * Writes decode_base64_length(input_length) bytes to the output buffer.
 * The output can overlap with the first three quarters of the input buffer.
 * That is, the input pointers and output pointer may be the same.
 *
 * Returns the number of bytes of raw data the base64 input decoded to. If the
 * input length supplied is not a valid length for base64, returns
 * std::size_t(-1) and does not decode.
 */
OLM_EXPORT std::size_t decode_base64(
    std::uint8_t const * input, std::size_t input_length,
    std::uint8_t * output
);

} // namespace olm


#endif /* OLM_BASE64_HH_ */
