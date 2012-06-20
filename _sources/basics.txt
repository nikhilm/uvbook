Basics of libuv
===============

libuv offers core utilities like timers, non-blocking networking support,
asynchronous file system access, child processes and more in an **always
asynchronous** manner. It uses a driven process to function in a single thread.

TODO clear up about async and threads

Async
-----

Event loops
-----------

libuv is based on an *event-driven* model of programming. The idea is to
express interest in certain events and respond to them when they occur. The
responsibility of gathering events from the operating system or monitoring
other sources of events is handled by libuv, and the user can register
callbacks to be invoked when an event occurs. An event-loop is a loop that
keeps running *forever*, watching out for events and notifying the user via
callbacks. In pseudocode:

.. code-block:: python

    while there are still events to process:
        e = get the next event
        if there is a callback associated with e:
            call the callback

Some examples of events are:

* File is ready for writing
* A socket has data ready to be read
* A timer has timed out

This event loop is encapsulated by `uv_run()` the end-all function when using
libuv. A standard program using libuv

Basics of the evented I/O model
Event loop lifecycle

Hello World
-----------

With the basics out of the way, lets write our first libuv program. It does
nothing, except start a loop which will exit immediately.

.. rubric:: helloworld/main.c
.. literalinclude:: ../code/helloworld/main.c
    :linenos:

This program quits immediately because it has no events to process. A libuv
event loop has to be told to watch out for events using the various API
functions.

Default loop
++++++++++++

A default loop is provided by libuv and can be accessed using
`uv_default_loop()`. You should use this loop if you only want a single
loop.

.. note::

    node.js uses the default loop as its main loop. If you are writing bindings
    you should be aware of this.

TODO does ev limitation of ev_child only on default loop extend to libuv? Check
this

Watchers
--------

Watchers are how users of libuv express interest in particular events. Watchers
are opaque structs named as `uv_TYPE_t` where type signifies what the watcher
is used for. A full list of watchers supported by libuv is:

.. rubric:: libuv watchers
.. literalinclude:: ../libuv/include/uv.h
    :lines: 184-202

.. note::

    All watcher structs are subclasses of `uv_handle_t` and often referred to
    as **handles** in libuv and in this text.

Watchers are setup by a corresponding

.. code-block:: c

    uv_TYPE_init(uv_TYPE_t*)

function.

.. note::

    Some watcher initialization functions require the loop as a first argument.

A watcher is set to actually listen for events by invoking

.. code-block:: c

    uv_TYPE_start(uv_TYPE_t*, callback)

and stopped by calling the corresponding

.. code-block:: c

    uv_TYPE_stop(uv_TYPE_t*)

Callbacks are functions which are called by libuv whenever an event the watcher
is interested in has taken place. Application specific logic will usually be
implemented in the callback. For example, an IO watcher's callback will receive
the data read from a file, a timer callback will be triggered on timeout and so
on.

Idling
++++++

Here is an example of using a watcher. An idle watcher's callback is repeatedly
called. There are some deeper semantics, discussed in :doc:`utilities`, but
we'll ignore them for now. Let's just use an idle watcher to look at the
watcher life cycle and see how `uv_run()` will now block because a watcher is
present. The idle watcher is stopped when the count is reached and `uv_run()`
exits since no event watchers are active.

.. rubric:: idle-basic/main.c
.. literalinclude:: ../code/idle-basic/main.c
    :emphasize-lines: 6,10,14-17

Reference counting
------------------

TODO move section to some other chapter?

The event loop only runs (i.e. `uv_run()` blocks) as long as their are active
watchers. This system works by having every watcher increase the reference
count of the event loop when it is started and decreasing the reference count
when stopped. It is also possible to manually change the reference count of
:term:`handles <handle>` using:

.. code-block::

    void uv_ref(uv_handle_t*);
    void uv_unref(uv_handle_t*);

This is mostly used when a loop shouldn't wait around for certain watchers,
even if they are active. See :doc:`patterns` for an example.

Semantics of event loops

void \*data pattern

note about not necessarily creating type structs on the stack
