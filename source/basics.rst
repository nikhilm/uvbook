Basics of libuv
===============

libuv enforces an **asynchronous**, **event-driven** style of programming.  Its
core job is to provide an event loop and callback based notifications of I/O
and other activities.  libuv offers core utilities like timers, non-blocking
networking support, asynchronous file system access, child processes and more.

Event loops
-----------

In event-driven programming, an application expresses interest in certain events
and respond to them when they occur. The responsibility of gathering events
from the operating system or monitoring other sources of events is handled by
libuv, and the user can register callbacks to be invoked when an event occurs.
The event-loop usually keeps running *forever*. In pseudocode:

.. code-block:: python

    while there are still events to process:
        e = get the next event
        if there is a callback associated with e:
            call the callback

Some examples of events are:

* File is ready for writing
* A socket has data ready to be read
* A timer has timed out

This event loop is encapsulated by ``uv_run()`` -- the end-all function when using
libuv.

The most common activity of systems programs is to deal with input and output,
rather than a lot of number-crunching. The problem with using conventional
input/output functions (``read``, ``fprintf``, etc.) is that they are
**blocking**. The actual write to a hard disk or reading from a network, takes
a disproportionately long time compared to the speed of the processor. The
functions don't return until the task is done, so that your program is doing
nothing. For programs which require high performance this is a major roadblock
as other activities and other I/O operations are kept waiting.

One of the standard solutions is to use threads. Each blocking I/O operation is
started in a separate thread (or in a thread pool). When the blocking function
gets invoked in the thread, the processor can schedule another thread to run,
which actually needs the CPU.

The approach followed by libuv uses another style, which is the **asynchronous,
non-blocking** style. Most modern operating systems provide event notification
subsystems. For example, a normal ``read`` call on a socket would block until
the sender actually sent something. Instead, the application can request the
operating system to watch the socket and put an event notification in the
queue. The application can inspect the events at its convenience (perhaps doing
some number crunching before to use the processor to the maximum) and grab the
data. It is **asynchronous** because the application expressed interest at one
point, then used the data at another point (in time and space). It is
**non-blocking** because the application process was free to do other tasks.
This fits in well with libuv's event-loop approach, since the operating system
events can be treated as just another libuv event. The non-blocking ensures
that other events can continue to be handled as fast they come in [#]_.

.. NOTE::

    How the I/O is run in the background is not of our concern, but due to the
    way our computer hardware works, with the thread as the basic unit of the
    processor, libuv and OSes will usually run background/worker threads and/or
    polling to perform tasks in a non-blocking manner.

Bert Belder, one of the libuv core developers has a small video explaining the
architecture of libuv and its background. If you have no prior experience with
either libuv or libev, it is a quick, useful watch.

.. raw:: html

    <iframe width="560" height="315"
    src="https://www.youtube-nocookie.com/embed/nGn60vDSxQ4" frameborder="0"
    allowfullscreen></iframe>

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
``uv_default_loop()``. You should use this loop if you only want a single
loop.

.. note::

    node.js uses the default loop as its main loop. If you are writing bindings
    you should be aware of this.

Error handling
--------------

libuv functions which may fail return ``-1`` on error. The error code itself is
set on the event loop as ``last_err``. Use ``uv_last_error(loop)`` to get
a ``uv_err_t`` which has a ``code`` member with the error code. ``code`` is an
enumeration of ``UV_*`` as defined here:

.. rubric:: libuv error codes
.. literalinclude:: ../libuv/include/uv.h
    :lines: 69-127

You can use the ``uv_strerror(uv_err_t)`` and ``uv_err_name(uv_err_t)`` functions
to get a ``const char *`` describing the error or the error name respectively.

Async callbacks have a ``status`` argument as the last argument. Use this instead
of the return value.

Watchers
--------

Watchers are how users of libuv express interest in particular events. Watchers
are opaque structs named as ``uv_TYPE_t`` where type signifies what the watcher
is used for. A full list of watchers supported by libuv is:

.. rubric:: libuv watchers
.. literalinclude:: ../libuv/include/uv.h
    :lines: 190-207

.. note::

    All watcher structs are subclasses of ``uv_handle_t`` and often referred to
    as **handles** in libuv and in this text.

Watchers are setup by a corresponding::

    uv_TYPE_init(uv_TYPE_t*)

function.

.. note::

    Some watcher initialization functions require the loop as a first argument.

A watcher is set to actually listen for events by invoking::

    uv_TYPE_start(uv_TYPE_t*, callback)

and stopped by calling the corresponding::

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
watcher life cycle and see how ``uv_run()`` will now block because a watcher is
present. The idle watcher is stopped when the count is reached and ``uv_run()``
exits since no event watchers are active.

.. rubric:: idle-basic/main.c
.. literalinclude:: ../code/idle-basic/main.c
    :emphasize-lines: 6,10,14-17

void \*data pattern

note about not necessarily creating type structs on the stack

----

.. [#] Depending on the capacity of the hardware of course.
