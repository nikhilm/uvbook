Event loops
===========

Basics of the evented I/O model
Event loop lifecycle

Hello World
-----------

With the basics out of the way, lets write our first libuv program. It does
nothing, except start a loop which will exit immediately.

TODO caption program with location in code folder

.. literalinclude:: ../code/helloworld/main.c
    :linenos:

This program quits immediately because it has no events to process. A libuv
event loop has to be told to watch out for events using the various API
functions.

Watchers
Semantics of event loops
Reference counting Each watcher (TODO what is a watcher)
increments the refer
