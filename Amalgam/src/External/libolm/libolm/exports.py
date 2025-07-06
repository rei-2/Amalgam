#!/usr/bin/env python3

import sys
import re
import json

expr = re.compile(r"(_*olm_[^( ]*)\(")

exports = {'_free', '_malloc'}

for f in sys.argv[1:]:
    with open(f) as fp:
        for line in fp:
            matches = expr.search(line)
            if matches is not None:
                exports.add('_%s' % (matches.group(1),))

json.dump(sorted(exports), sys.stdout)
