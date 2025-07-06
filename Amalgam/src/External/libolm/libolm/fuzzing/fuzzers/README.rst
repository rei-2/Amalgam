Fuzzers
=======

This directory contains a collection of fuzzing tools. Each tests a different
entry point to the code.

Usage notes:

1. Install AFL:

   .. code::

      apt-get install afl

2. Build the fuzzers:

   .. code::

      make fuzzers

3. Some of the tests (eg ``fuzz_decrypt`` and ``fuzz_group_decrypt``) require a
   session file. You can create one by pickling an Olm session.

4. Make some work directories:

   .. code::

      mkdir -p fuzzing/in fuzzing/out

5. Generate starting input:

   .. code::

      echo "Test" > fuzzing/in/test

6. Run the test under ``afl-fuzz``:

   .. code::

      afl-fuzz -i fuzzing/in -o fuzzing/out -- \
         ./build/fuzzers/fuzz_<fuzzing_tool> [<test args>]

7. To resume with the data produced by an earlier run:

   .. code::

       afl-fuzz -i- -o existing_output_dir [...etc...]

8. If it shows failures, pipe the failure case into
   ``./build/fuzzers/debug_<fuzzing_tool>``, fix, and repeat.
