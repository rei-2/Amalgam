Tracing
=======

To see what crypto functions are being called with what input run

.. code:: bash

    gdb --batch -x tracing/trace.gdb ./build/test_ratchet | grep "^[- ]" | tr "{}" "[]" | tracing/graph.py
