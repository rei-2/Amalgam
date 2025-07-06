#include "olm/message.hh"
#include "fuzzing.hh"

int main(int argc, const char *argv[]) {
    int message_fd = STDIN_FILENO;
    uint8_t * message_buffer;
    ssize_t message_length = check_errno(
        "Error reading message file", read_file(message_fd, &message_buffer)
    );
    olm::MessageReader * reader = new olm::MessageReader;
    decode_message(*reader, message_buffer, message_length, 8);
    free(message_buffer);
    delete reader;

    return EXIT_SUCCESS;
}
