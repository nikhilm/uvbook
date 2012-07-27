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

libuv's thread API is also very limited since the semantics and syntax of
threads are different on all platforms, with different levels of completeness.

This chapter makes the following assumption: **There is only one event loop,
running in one thread (the main thread)**. No other thread interacts
with the event loop (except using ``uv_async_send``). :doc:`multiple` covers
running event loops in different threads and managing them.

Core thread operations
----------------------

There isn't much here, you just start a thread using ``uv_thread_create()`` and
wait for it to close using ``uv_thread_join()``.

.. _thread-create-example:

.. rubric:: thread-create/main.c
.. literalinclude:: ../code/thread-create/main.c
    :linenos:
    :lines: 26-37
    :emphasize-lines: 3-7

.. tip::

    ``uv_thread_t`` is just an alias for ``pthread_t`` on Unix, but this is an
    implementation detail, avoid depending on it to always be true.

The second parameter is the function which will serve as the entry point for
the thread, the last parameter is a ``void *`` argument which can be used to pass
custom parameters to the thread. The function ``hare`` will now run in a separate
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

The mutex functions are a **direct** map to the pthread equivalents.

.. rubric:: libuv mutex functions
.. literalinclude:: ../libuv/include/uv.h
    :lines: 1578-1582

The ``uv_mutex_init()`` and ``uv_mutex_trylock()`` functions will return 0 on
success, -1 on error instead of error codes.

If `libuv` has been compiled with debugging enabled, ``uv_mutex_destroy()``,
``uv_mutex_lock()`` and ``uv_mutex_unlock()`` will ``abort()`` on error.
Similarly ``uv_mutex_trylock()`` will abort if the error is anything *other
than* ``EAGAIN``.

Recursive mutexes are supported by some platforms, but you should not rely on
them. The BSD mutex implementation will raise an error if a thread which has
locked a mutex attempts to lock it again. For example, a construct like::

    uv_mutex_lock(a_mutex);
    uv_thread_create(thread_id, entry, (void *)a_mutex);
    uv_mutex_lock(a_mutex);
    // more things here

can be used to wait until another thread initializes some stuff and then
unlocks ``a_mutex`` but will lead to your program crashing if in debug mode, or
otherwise behaving wrongly.

.. tip::

    Mutexes on linux support attributes for a recursive mutex, but the API is
    not exposed via libuv.

Locks
~~~~~

Read-write locks are the other synchronization primitive supported.

Others
~~~~~~

Semaphores and condition variables are not implemented yet. Their are a couple
of patches for condition variable support in libuv TODO link, but since the
Windows condition variable system is available only from Windows Vista and
Windows Server 2008 onwards [#]_, its not in libuv yet.

libuv work queue
----------------

``uv_queue_work()`` is a convenience function that allows an application to run
a task in a separate thread, and have a callback that is triggered when the
task is done. A seemingly simple function, what makes ``uv_queue_work()``
tempting is that it allows potentially any third-party libraries to be used
with the event-loop paradigm. When you use event loops, it is *imperative to
make sure that no function which runs periodically in the loop thread blocks
when performing I/O or is a serious CPU hog*, because this means the loop slows
down and events are not being dealt with at full capacity.

But a lot of existing code out there features blocking functions (for example
a routine which performs I/O under the hood) to be used with threads if you
want responsiveness (the classic one thread per client server model), and
getting them to play with an event loop library generally involves rolling your
own system of running the task in a separate thread.  libuv just provides
a convenient abstraction for this.

Here is a simple example inspired by "node.js is cancer" [#]_. We are going to
calculate fibonacci numbers, sleeping a bit along the way, but run it in
a separate thread so that the blocking and CPU bound task does not prevent the
event loop from performing other activities.

.. literalinclude:: ../code/queue-work/main.c
    :linenos:
    :lines: 17-29

The actual task function is simple, nothing to show that it is going to be
run in a separate thread. The ``uv_work_t`` structure is the clue. You can pass
arbitrary data through it using the ``void* data`` field and use it to
communicate to and from the thread. But be sure you are using proper locks if
you are changing things while both threads may be running.

The trigger is ``uv_queue_work``:

.. literalinclude:: ../code/queue-work/main.c
    :linenos:
    :lines: 31-44
    :emphasize-lines: 40

The thread function will be launched in a separate thread, passed the
``uv_work_t`` structure and once the function returns, the *after* function
will be called, again with the same structure.

For writing wrappers to blocking libraries, a common :doc:`pattern <patterns>`
is to use a baton to exchange data.

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

.. rubric:: Footnotes
.. [#] http://msdn.microsoft.com/en-us/library/windows/desktop/ms683469(v=vs.85).aspx
.. [#] https://raw.github.com/teddziuba/teddziuba.github.com/master/_posts/2011-10-01-node-js-is-cancer.html
