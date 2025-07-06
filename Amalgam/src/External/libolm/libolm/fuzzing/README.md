# Directory structure

- `fuzzers/`: Sources for the fuzzing harnesses.
- `corpora/`: Contains the fuzzing corpora and assorted tools. The corpora are
  filed under a directory with the same name as the fuzzing harness. Each of
  those directories also contains the following:

  - `in/`: Contains the actual corpus test cases.
  - `tools/`: Any tools useful for that particular harness. A good example
    would be a binary which generates seed test cases.
