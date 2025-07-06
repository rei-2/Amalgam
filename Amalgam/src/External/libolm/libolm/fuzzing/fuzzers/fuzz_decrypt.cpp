#include "olm/olm.hh"

#include "fuzzing.hh"

int main(int argc, const char *argv[]) {
    if (argc <= 3) {
        const char * message = "Usage: decrypt: <session_key> <session_file>"
            " <message_type>\n";
        (void)write(STDERR_FILENO, message, strlen(message));
        exit(3);
    }

    const char * key = argv[1];
    size_t key_length = strlen(key);


    int session_fd = check_errno(
        "Error opening session file", open(argv[2], O_RDONLY)
    );

    int message_type = atoi(argv[3]);

    uint8_t *session_buffer;
    ssize_t session_length = check_errno(
        "Error reading session file", read_file(session_fd, &session_buffer)
    );

    int message_fd = STDIN_FILENO;
    uint8_t * message_buffer;
    ssize_t message_length = check_errno(
        "Error reading message file", read_file(message_fd, &message_buffer)
    );

    uint8_t * tmp_buffer = (uint8_t *) malloc(message_length);
    memcpy(tmp_buffer, message_buffer, message_length);

    uint8_t session_memory[olm_session_size()];
    OlmSession * session = olm_session(session_memory);
    check_session(session, "Error unpickling session", olm_unpickle_session(
        session, key, key_length, session_buffer, session_length
    ));

    size_t max_length = check_session(
        session,
        "Error getting plaintext length",
        olm_decrypt_max_plaintext_length(
            session, message_type, tmp_buffer, message_length
        )
    );

    uint8_t plaintext[max_length];

    size_t length = check_session(
        session, "Error decrypting message", olm_decrypt(
            session, message_type,
            message_buffer, message_length,
            plaintext, max_length
        )
    );

    (void)write(STDOUT_FILENO, plaintext, length);
    (void)write(STDOUT_FILENO, "\n", 1);

    free(session_buffer);
    free(message_buffer);
    free(tmp_buffer);

    return EXIT_SUCCESS;
}
