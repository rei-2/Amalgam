#include "olm/olm.hh"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stddef.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


ssize_t read_file(
    int fd,
    uint8_t **buffer
) {
    size_t buffer_size = 4096;
    size_t buffer_pos = 0;
    uint8_t * current_buffer = (uint8_t *) malloc(buffer_size);
    if (!current_buffer) return -1;

    while (1) {
        ssize_t count = read(
            fd, current_buffer + buffer_pos, buffer_size - buffer_pos
        );

        if (count < 0) break;  // A read error happened, so just fail immediately.

        if (count == 0) {
            // Nothing more left to read. We downsize the buffer to fit the
            // data exactly, unless no data was read at all, in which case we
            // skip the downsizing.

            if (buffer_pos != 0) {
                current_buffer = (uint8_t *) realloc(current_buffer, buffer_pos);
                if (!current_buffer) break;
            }

            // The read was successful so we return the allocated buffer.
            *buffer = current_buffer;
            return buffer_pos;
        }

        buffer_pos += count;

        // We've reached capacity, so enlarge the buffer.
        if (buffer_pos == buffer_size) {
            buffer_size *= 2;
            uint8_t * new_buffer = (uint8_t *) realloc(current_buffer, buffer_size);
            if (!new_buffer) break;
            current_buffer = new_buffer;
        }
    }

    free(current_buffer);
    return -1;
}

template<typename T>
T check_errno(
    const char * message,
    T value
) {
    if (value == T(-1)) {
        perror(message);
        exit(1);
    }
    return value;
}

template<typename T, typename F>
size_t check_error(
    F f,
    T * object,
    const char * message,
    size_t value
) {
    if (value == olm_error()) {
        const char * olm_message = f(object);
        (void)write(STDERR_FILENO, message, strlen(message));
        (void)write(STDERR_FILENO, ": ", 2);
        (void)write(STDERR_FILENO, olm_message, strlen(olm_message));
        (void)write(STDERR_FILENO, "\n", 1);

        exit(2);
    }
    return value;
}

size_t check_session(
    OlmSession * session,
    const char * message,
    size_t value
) {
    return check_error(olm_session_last_error, session, message, value);
}
