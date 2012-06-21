Threads
=======

Wait a minute? Why are we on threads? Aren't event loops supposed to be **the
way** to do *web-scale programming*? Well no. Threads are still the medium in
which the processor does its job, and threads are mighty useful sometimes, even
though you might have to wade through scary looking mutexes. TODO rework this
a bit

Threads are used internally to fake the asynchronous nature of all the system
calls. libuv also uses threads to allow you, the application, to perform a task
asynchronously that is actually blocking, by spawning a thread and collecting
the result when it is done.

Today there are two predominant thread libraries. The Windows threads
implementation and `pthreads`_. libuv's thread API is analogous to
the pthread API and often has similar semantics.

A notable aspect of libuv's thread facilities is that it is a self contained
section within libuv. Whereas other features intimately depend on the event
loop and callback principles, threads are complete agnostic, they block as
required, signal errors directly via return values and, as shown in the
:ref:`first example <thread-create-example>`, don't even require a running
event loop.

This chapter makes the following assumption: **There is only one event loop,
running in one thread (the main thread)**. No other thread interacts
with the event loop (except using `uv_async_send`). :doc:`multiple` covers
running event loops in different threads and managing them.

Core thread operations
----------------------

There isn't much here, you just start a thread using `uv_thread_create()` and
wait for it to close using `uv_thread_join()`.

.. _thread-create-example:

.. rubric:: thread-create/main.c
.. literalinclude:: ../code/thread-create/main.c
    :linenos:
    :lines: 26-37
    :emphasize-lines: 3-7

.. tip::

    `uv_thread_t` is just an alias for `pthread_t` on Unix, but this is an
    implementation detail, avoid depending on it to always be true.

The second parameter is the function which will serve as the entry point for
the thread, the last parameter is a `void *` argument which can be used to pass
custom parameters to the thread. The function `hare` will now run in a separate
thread, scheduled pre-emptively by the operating system:

.. rubric:: thread-create/main.c
.. literalinclude:: ../code/thread-create/main.c
    :linenos:
    :lines: 6-14
    :emphasize-lines: 2

Unlike ``pthread_join()`` which allows the target thread to pass back a value to
the calling thread using a second parameter, ``uv_thread_join()`` does not. To
send values use :ref:`inter-thread-communication`.

Synchronization Primitives
--------------------------

This section is purposely spartan. This book is not about threads, so I only
catalogue any surprises in the libuv APIs here. For the rest you can look at
the pthreads `man pages <pthreads>`_

Mutexes
~~~~~~~

Recursive mutexes not allowed

Locks
~~~~~

Others
~~~~~~

Semaphores not dne
The problem with condition variables

libuv work queue
----------------

uv_queue_work

.. _inter-thread-communication:

Inter-thread communication
--------------------------

inter thread communication using uv_async
synchronization
parallel downloads example?

note about recursive mutexes not supported
note about bad error of just aborting
uv_once

.. _pthreads: http://man7.org/linux/man-pages/man7/pthreads.7.html
