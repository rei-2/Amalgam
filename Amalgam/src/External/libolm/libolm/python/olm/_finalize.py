# The MIT License (MIT)
# Copyright (c) 2010 Benjamin Peterson <benjamin@python.org>

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
# DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
# OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
# OR OTHER DEALINGS IN THE SOFTWARE.

"""Finalization with weakrefs

This is designed for avoiding __del__.
"""

import sys
import traceback
import weakref

__author__ = "Benjamin Peterson <benjamin@python.org>"


class OwnerRef(weakref.ref):
    """A simple weakref.ref subclass, so attributes can be added."""
    pass


def _run_finalizer(ref):
    """Internal weakref callback to run finalizers"""
    del _finalize_refs[id(ref)]
    finalizer = ref.finalizer
    item = ref.item
    try:
        finalizer(item)
    except Exception:  # pragma: no cover
        print("Exception running {}:".format(finalizer), file=sys.stderr)
        traceback.print_exc()


_finalize_refs = {}


def track_for_finalization(owner, item, finalizer):
    """Register an object for finalization.

    ``owner`` is the the object which is responsible for ``item``.
    ``finalizer`` will be called with ``item`` as its only argument when
    ``owner`` is destroyed by the garbage collector.
    """
    ref = OwnerRef(owner, _run_finalizer)
    ref.item = item
    ref.finalizer = finalizer
    _finalize_refs[id(ref)] = ref
