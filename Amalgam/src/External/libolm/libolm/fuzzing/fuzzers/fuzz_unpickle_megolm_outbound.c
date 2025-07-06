#include <olm/outbound_group_session.h>

#include "fuzzing.h"

int main(int argc, const char *argv[]) {
    if (argc != 1) {
      printf("Usage: %s <input_file\n", argv[0]);
      exit(3);
    }

    void *session_buffer = malloc(olm_outbound_group_session_size());
    OlmOutboundGroupSession *session = olm_outbound_group_session(session_buffer);

    int pickle_fd = STDIN_FILENO;
    uint8_t *pickle_buffer;
    ssize_t pickle_length = check_errno("Error reading message file",
                                        read_file(pickle_fd, &pickle_buffer));

    check_outbound_group_session(
        session, "Error unpickling outbound group session",
        olm_unpickle_outbound_group_session(session, "", 0, pickle_buffer,
                                            pickle_length));

    free(session_buffer);
    free(pickle_buffer);

    return EXIT_SUCCESS;
}
