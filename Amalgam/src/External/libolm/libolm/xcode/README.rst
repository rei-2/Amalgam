OLMKit
======

OLMKit exposes an Objective-C wrapper to libolm.

The original work by Chris Ballinger can be found at https://github.com/chrisballinger/OLMKit.

Installation
------------
You can embed OLMKit to your application project with CocoaPods. The pod for
the latest OLMKit release is::

    pod 'OLMKit'

Or you can use Swift Package Manager with the URL::

    https://gitlab.matrix.org/matrix-org/olm

Development
-----------
Run `pod install` and open `OLMKit.xcworkspace` with Xcode.

The project contains only tests files. The libolm and the Objective-C wrapper source files are loaded via the OLMKit CocoaPods pod.

To add a new source file, add it to the file system and run `pod update` to make CocoaPods insert it into OLMKit.xcworkspace. 

Development alternative
-----------------------
Based on the Swift package, you can build source files and run tests directly from the command line: `swift test`.

Release
-------
See ../README.rst for the release of the CocoaPod and the Swift package.
