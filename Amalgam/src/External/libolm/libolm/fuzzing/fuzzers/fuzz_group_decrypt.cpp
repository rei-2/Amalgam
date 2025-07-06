#include "olm/olm.hh"

#include "fuzzing.hh"

#ifndef __AFL_FUZZ_TESTCASE_LEN
  ssize_t fuzz_len;
  #define __AFL_FUZZ_TESTCASE_LEN fuzz_len
  unsigned char fuzz_buf[1024000];
  #define __AFL_FUZZ_TESTCASE_BUF fuzz_buf
  #define __AFL_FUZZ_INIT() void sync(void);
  #define __AFL_LOOP(x) ((fuzz_len = read(0, fuzz_buf, sizeof(fuzz_buf))) > 0 ? 1 : 0)
  #define __AFL_INIT() sync()
#endif

__AFL_FUZZ_INIT();

int main(int argc, const char *argv[]) {
    if (argc <= 2) {
        const char * message = "Usage: decrypt <pickle_key> <group_session>\n";
        (void)write(STDERR_FILENO, message, strlen(message));
        exit(3);
    }

    const char * key = argv[1];
    size_t key_length = strlen(key);


    int session_fd = check_errno(
        "Error opening session file", open(argv[2], O_RDONLY)
    );

    uint8_t *session_buffer;
    ssize_t session_length = check_errno(
        "Error reading session file", read_file(session_fd, &session_buffer)
    );

    uint8_t session_memory[olm_inbound_group_session_size()];
    OlmInboundGroupSession * session = olm_inbound_group_session(session_memory);
    check_error(
        olm_inbound_group_session_last_error,
        session,
        "Error unpickling session",
        olm_unpickle_inbound_group_session(
            session, key, key_length, session_buffer, session_length
        )
    );

#ifdef __AFL_HAVE_MANUAL_CONTROL
    __AFL_INIT();
#endif

    size_t test_case_buf_len = 1024;
    uint8_t * message_buffer = (uint8_t *) malloc(test_case_buf_len);
    uint8_t * tmp_buffer = (uint8_t *) malloc(test_case_buf_len);

    while (__AFL_LOOP(10000)) {
        size_t message_length = __AFL_FUZZ_TESTCASE_LEN;

        if (message_length > test_case_buf_len) {
            message_buffer = (uint8_t *)realloc(message_buffer, message_length);
            tmp_buffer = (uint8_t *)realloc(tmp_buffer, message_length);

            if (!message_buffer || !tmp_buffer) return 1;
        }

        memcpy(message_buffer, __AFL_FUZZ_TESTCASE_BUF, message_length);
        memcpy(tmp_buffer, message_buffer, message_length);

        size_t max_length = check_error(
            olm_inbound_group_session_last_error,
            session,
            "Error getting plaintext length",
            olm_group_decrypt_max_plaintext_length(
                session, tmp_buffer, message_length
            )
        );

        uint8_t plaintext[max_length];

        uint32_t ratchet_index;

        size_t length = check_error(
            olm_inbound_group_session_last_error,
            session,
            "Error decrypting message",
            olm_group_decrypt(
                session,
                message_buffer, message_length,
                plaintext, max_length, &ratchet_index
            )
        );

        (void)write(STDOUT_FILENO, plaintext, length);
        (void)write(STDOUT_FILENO, "\n", 1);
    }

    free(session_buffer);
    free(message_buffer);
    free(tmp_buffer);

    return EXIT_SUCCESS;
}
