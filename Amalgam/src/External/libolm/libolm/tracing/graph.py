#! /usr/bin/env python

import sys
import yaml
import array

class Call(object):
    def __init__(self, call):
        self.func, = call
        args = dict(call[self.func])
        self.output = array.array("B", args.pop("output")).tostring()
        self.inputs = {
            name: array.array("B", args[name]).tostring()
            for name in args
            if not name.endswith("_length")
        }
        self.bind = {}

    def expr(self, stream, indent="  ", level=""):
        stream.write(self.func + "(\n")
        for name, value in self.inputs.items():
            stream.write(level + indent + name + "=")
            self.bind.get(name, Literal(value)).expr(
                stream, indent, level + indent
            )
            stream.write(",\n")
        stream.write(level + ")")


class Literal(str):
    def expr(self, stream, indent, level):
        stream.write("\"" + self.encode("hex") + "\"")


class Slice(object):
    def __init__(self, thing, start, end):
        self.thing = thing
        self.start = start
        self.end = end

    def expr(self, stream, indent="  ", level=""):
        self.thing.expr(stream, indent, level)
        stream.write("[%d:%d]" % (self.start, self.end))


class Concat(list):
    def expr(self, stream, indent="  ", level=""):
        stream.write("concat(\n")
        for thing in self:
            stream.write(level + indent)
            thing.expr(stream, indent, level + indent)
            stream.write(",\n")
        stream.write(level + ")")


calls = [Call(c) for c in yaml.load(sys.stdin)]

outputs = {}

for call in calls:
    for i in range(8, len(call.output)):
        outputs.setdefault(call.output[i - 8: i], []).append(call)

for call in calls:
    for name, value in call.inputs.items():
        for bind in outputs.get(value[:8], ()):
            if value == bind.output:
                call.bind[name] = bind
            else:
                for end in range(len(value), len(bind.output) + 1):
                    start = end - len(value)
                    if value == bind.output[start:end]:
                        call.bind[name] = Slice(bind, start, end)
        if not name in call.bind:
            i = 0
            j = 1
            k = 0
            concat = Concat()
            while i < len(value):
                for bind in outputs.get(value[i:i+8], ()):
                    if value[i:].startswith(bind.output):
                        if k != i:
                            concat.append(Literal(value[k:i]))
                        concat.append(bind)
                        j = len(bind.output)
                        k = i + j
                        break
                i += j
                j = 1
            if concat:
                if k != i:
                    concat.append(Literal(value[k:i]))
                call.bind[name] = concat

for call in calls:
    if call.func.startswith("h"):
        sys.stdout.write("\"" + call.output.encode("hex") + "\" = ")
        call.expr(sys.stdout)
        sys.stdout.write("\n")

