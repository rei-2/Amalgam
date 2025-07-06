#include "fuzzing.hh"
#include "olm/account.hh"
#include "olm/olm.h"

size_t fuzz_unpickle_account(
    OlmAccount * account, void * pickled, size_t pickled_length
) {
    olm::Account & object = *reinterpret_cast<olm::Account *>(account);
    std::uint8_t * const pos = reinterpret_cast<std::uint8_t *>(pickled);
    std::uint8_t * const end = pos + pickled_length;

    if (!unpickle(pos, end, object)) {
        if (object.last_error == OlmErrorCode::OLM_SUCCESS) {
            object.last_error = OlmErrorCode::OLM_CORRUPTED_PICKLE;
        }
        return std::size_t(-1);
    }

    return pickled_length;
}

int main(int argc, const char * argv[]) {
    int pickle_fd = STDIN_FILENO;
    uint8_t * pickle_buffer;
    ssize_t pickle_length = check_errno(
        "Error reading pickle file", read_file(pickle_fd, &pickle_buffer));

    void * account_buf = malloc(olm_account_size());
    if (!account_buf) {
        return 3;
    }
    OlmAccount * account = olm_account(account_buf);

    check_error(olm_account_last_error, account, "Error unpickling account",
        fuzz_unpickle_account(account, pickle_buffer, pickle_length));

    free(pickle_buffer);
    free(account);

    return EXIT_SUCCESS;
}
