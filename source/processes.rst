Processes
=========

libuv offers considerable child process management, abstracting the platform
differences and allowing communication with the child process using streams or
named pipes.

A common idiom in Unix is for every process to do one thing and do it well. In
such a case, a process often uses multiple child processes to achieve tasks
(similar to using pipes in shells). A multi-process model with messages
may also be easier to reason about compared to one with threads and shared
memory.

A common refrain against event-based programs is that they cannot take
advantage of multiple cores in modern computers. In a multi-threaded program
the kernel can perform scheduling and assign different threads to different
cores, improving performance. But an event loop has only one thread.  The
workaround can be to launch multiple processes instead, with each process
running an event loop, and each process getting assigned to a separate CPU
core.

Spawning child processes
------------------------

The simplest case is when you simply want to launch a process and know when it
exits. This is achieved using ``uv_spawn``.

.. rubric:: spawn/main.c
.. literalinclude:: ../code/spawn/main.c
    :linenos:
    :lines: 5-7,13-
    :emphasize-lines: 11,13-17

The ``uv_process_t`` struct only acts as the watcher, all options are set via
``uv_process_options_t``. To simply launch a process, you need to set only the
``file`` and ``args`` fields. ``file`` is the program to execute. Since
``uv_spawn`` uses execvp_ internally, there is no need to supply the full
path. Finally as per underlying conventions, **the arguments array has to be
one larger than the number of arguments, with the last element being NULL**.

.. _execvp: http://www.kernel.org/doc/man-pages/online/pages/man3/exec.3.html

After the call to ``uv_spawn``, ``uv_process_t.pid`` will contain the process
ID of the child process.

The exit callback will be invoked with the *exit status* and the type of *signal*
which caused the exit.

.. rubric:: spawn/main.c
.. literalinclude:: ../code/spawn/main.c
    :linenos:
    :lines: 9-12
    :emphasize-lines: 3

It is **required** to close the process watcher after the process exits.

Changing process parameters
---------------------------

Before the child process is launched you can control the execution environment
using fields in ``uv_process_options_t``.

Change execution directory
++++++++++++++++++++++++++

Set ``uv_process_options_t.cwd`` to the corresponding directory.

Set environment variables
+++++++++++++++++++++++++

``uv_process_options_t.env`` is an array of strings, each of the form
``VAR=VALUE`` used to set up the environment variables for the process. Set
this to ``NULL`` to inherit the environment from the parent (this) process.

Option flags
++++++++++++

Setting ``uv_process_options_t.flags`` to a bitwise OR of the following flags,
modifies the child process behaviour:

* ``UV_PROCESS_SETUID`` - sets the child's execution user ID to ``uv_process_options_t.uid``.
* ``UV_PROCESS_SETGID`` - sets the child's execution group ID to ``uv_process_options_t.gid``.

Changing the UID/GID is only supported on Unix, ``uv_spawn`` will fail on
Windows with ``UV_ENOTSUP``.

* ``UV_PROCESS_WINDOWS_VERBATIM_ARGUMENTS`` - No quoting or escaping of
  ``uv_process_options_t.args`` is done on Windows. Ignored on Unix.
* ``UV_PROCESS_DETACHED`` - Starts the child process in a new session, which
  will keep running after the parent process exits. See example below.

Detaching processes
-------------------

Passing the flag ``UV_PROCESS_DETACHED`` can be used to launch daemons, or
child processes which are independent of the parent so that the parent exiting
does not affect it.

.. rubric:: detach/main.c
.. literalinclude:: ../code/detach/main.c
    :linenos:
    :lines: 9-30
    :emphasize-lines: 12,19

Just remember that the watcher is still monitoring the child, so your program
won't exit. Use ``uv_unref()`` if you want to be more *fire-and-forget*.

Signals and termination
-----------------------

libuv wraps the standard ``kill(2)`` system call on Unix and implements one
with similar semantics on Windows, with *one caveat*: ``uv_kill`` on Windows
only supports ``SIGTERM``, ``SIGINT`` and ``SIGKILL``, all of which lead to
termination of the process. The signature of ``uv_kill`` is::

    uv_err_t uv_kill(int pid, int signum);

For processes started using libuv, you may use ``uv_process_kill`` instead,
which accepts the ``uv_process_t`` watcher as the first argument, rather than
the pid. In this case, **remember to call** ``uv_close`` on the watcher.

Child Process I/O
-----------------

A normal, newly spawned process has its own set of file descriptors, with 0,
1 and 2 being ``stdin``, ``stdout`` and ``stderr`` respectively. Sometimes you
may want to share file descriptors with the child. For example, perhaps your
applications launches a sub-command and you want any errors to go in the log
file, but ignore ``stdout``. For this you'd like to have ``stderr`` of the
child to be displayed. In this case, libuv supports *inheriting* file
descriptors. In this sample, we invoke the test program, which is:

.. rubric:: proc-streams/test.c
.. literalinclude:: ../code/proc-streams/test.c

The actual program ``proc-streams`` runs this while inheriting only ``stderr``.
The file descriptors of the child process are set using the ``stdio`` field in
``uv_process_options_t``. First set the ``stdio_count`` field to the number of
file descriptors being set. ``uv_process_options_t.stdio`` is an array of
``uv_stdio_container_t``, which is:

.. literalinclude:: ../libuv/include/uv.h
    :lines: 1188-1195

where flags can have several values. Use ``UV_IGNORE`` if it isn't going to be
used. If the first three ``stdio`` fields are marked as ``UV_IGNORE`` they'll
redirect to ``/dev/null``.

Since we want to pass on an existing descriptor, we'll use ``UV_INHERIT_FD``.
Then we set the ``fd`` to ``stderr``.

.. rubric:: proc-streams/main.c
.. literalinclude:: ../code/proc-streams/main.c
    :linenos:
    :lines: 15-17,27-
    :emphasize-lines: 6,10,11,12

If you run ``proc-stream`` you'll see that only the line "This is stderr" will
be displayed. Try marking ``stdout`` as being inherited and see the output.

It is dead simple to apply this redirection to streams.  By setting ``flags``
to ``UV_INHERIT_STREAM`` and setting ``data.stream`` to the stream in the
parent process, the child process can treat that stream as standard I/O. This
can be used to implement something like CGI_.

.. _CGI: http://en.wikipedia.org/wiki/Common_Gateway_Interface

A sample CGI script/executable is:

.. rubric:: cgi/tick.c
.. literalinclude:: ../code/cgi/tick.c

The CGI server combines the concepts from this chapter and :doc:`networking` so
that every client is sent ten ticks after which that connection is closed.

.. rubric:: cgi/main.c
.. literalinclude:: ../code/cgi/main.c
    :linenos:
    :lines: 47,53-62
    :emphasize-lines: 5

Here we simply accept the TCP connection and pass on the socket (*stream*) to
``invoke_cgi_script``.

.. rubric:: cgi/main.c
.. literalinclude:: ../code/cgi/main.c
    :linenos:
    :lines: 16, 25-45
    :emphasize-lines: 8-9,17-18

The ``stdout`` of the CGI script is set to the socket so that whatever our tick
script prints, gets sent to the client. By using processes, we can offload the
read/write buffering to the operating system, so in terms of convenience this
is great. Just be warned that creating processes is a costly task.

.. _pipes:

Pipes
-----

libuv's ``uv_pipe_t`` structure is slightly confusing to Unix programmers,
because it immediately conjures up ``|`` and `pipe(7)`_. But ``uv_pipe_t`` is
not related to anonymous pipes, rather it has two uses:

#. Stream API - It acts as the concrete implementation of the ``uv_stream_t``
   API for providing a FIFO, streaming interface to local file I/O. This is
   performed using ``uv_pipe_open`` as covered in :ref:`buffers-and-streams`.
   You could also use it for TCP/UDP, but there are already convenience functions
   and structures for them.

#. IPC mechanism - ``uv_pipe_t`` can be backed by a `Unix Domain Socket`_ or
   `Windows Named Pipe`_ to allow multiple processes to communicate. This is
   discussed below.

.. _pipe(7): http://www.kernel.org/doc/man-pages/online/pages/man7/pipe.7.html
.. _Unix Domain Socket: http://www.kernel.org/doc/man-pages/online/pages/man7/unix.7.html
.. _Windows Named Pipe: http://msdn.microsoft.com/en-us/library/windows/desktop/aa365590(v=vs.85).aspx

Parent-child IPC
++++++++++++++++

A parent and child can have one or two way communication over a pipe created by
settings ``uv_stdio_container_t.flags`` to a bit-wise combination of
``UV_CREATE_PIPE`` and ``UV_READABLE_PIPE`` or ``UV_WRITABLE_PIPE``. The
read/write flag is from the perspective of the child process.

Arbitrary process IPC
+++++++++++++++++++++

Since domain sockets [#]_ can have a well known name and a location in the
file-system they can be used for IPC between unrelated processes. The D-BUS_
system used by open source desktop environments uses domain sockets for event
notification. Various applications can then react when a contact comes online
or new hardware is detected. The MySQL server also runs a domain socket on
which clients can interact with it.

.. _D-BUS: http://www.freedesktop.org/wiki/Software/dbus

When using domain sockets, a client-server pattern is usually followed with the
creator/owner of the socket acting as the server. After the initial setup,
messaging is no different from TCP, so we'll re-use the echo server example.

.. rubric:: pipe-echo-server/main.c
.. literalinclude:: ../code/pipe-echo-server/main.c
    :linenos:
    :lines: 56-
    :emphasize-lines: 5,9,13

We name the socket ``echo.sock`` which means it will be created in the local
directory. This socket now behaves no different from TCP sockets as far as
the stream API is concerned. You can test this server using `netcat`_::

    $ nc -U /path/to/echo.sock

A client which wants to connect to a domain socket will use::

    void uv_pipe_connect(uv_connect_t *req, uv_pipe_t *handle, const char *name, uv_connect_cb cb);

where ``name`` will be ``echo.sock`` or similar.

.. _netcat: http://netcat.sf.net

Sending file descriptors over pipes
+++++++++++++++++++++++++++++++++++

The cool thing about domain sockets is that file descriptors can be exchanged
between processes by sending them over a domain socket. This allows processes
to hand off their I/O to other processes. Applications include load-balancing
servers, worker processes and other ways to make optimum use of CPU.

.. warning::

    On Windows, only file descriptors representing TCP sockets can be passed
    around.

To demonstrate, we will look at a echo server implementation that hands of
clients to worker processes in a round-robin fashion. This program is a bit
involved, and while only snippets are included in the book, it is recommended
to read the full code to really understand it.

The worker process is quite simple, since the file-descriptor is handed over to
it by the master.

.. rubric:: multi-echo-server/worker.c
.. literalinclude:: ../code/multi-echo-server/worker.c
    :linenos:
    :lines: 7-9,53-
    :emphasize-lines: 7-9

``queue`` is the pipe connected to the master process on the other end, along
which new file descriptors get sent. We use the ``read2`` function to express
interest in file descriptors. It is important to set the ``ipc`` argument of
``uv_pipe_init`` to 1 to indicate this pipe will be used for inter-process
communication! Since the master will write the file handle to the standard
input of the worker, we connect the pipe to ``stdin`` using ``uv_pipe_open``.

.. rubric:: multi-echo-server/worker.c
.. literalinclude:: ../code/multi-echo-server/worker.c
    :linenos:
    :lines: 36-52
    :emphasize-lines: 9

Although ``accept`` seems odd in this code, it actually makes sense. What
``accept`` traditionally does is get a file descriptor (the client) from
another file descriptor (The listening socket). Which is exactly what we do
here. Fetch the file descriptor (``client``) from ``queue``. From this point
the worker does standard echo server stuff.

Turning now to the master, let's take a look at how the workers are launched to
allow load balancing.

.. rubric:: multi-echo-server/main.c
.. literalinclude:: ../code/multi-echo-server/main.c
    :linenos:
    :lines: 6-13

The ``child_worker`` structure wraps the process, and the pipe between the
master and the individual process.

.. rubric:: multi-echo-server/main.c
.. literalinclude:: ../code/multi-echo-server/main.c
    :linenos:
    :lines: 49,61-93
    :emphasize-lines: 15,18-19

In setting up the workers, we use the nifty libuv function ``uv_cpu_info`` to
get the number of CPUs so we can launch an equal number of workers. Again it is
important to initialize the pipe acting as the IPC channel with the third
argument as 1. We then indicate that the child process' ``stdin`` is to be
a readable pipe (from the point of view of the child). Everything is
straightforward till here. The workers are launched and waiting for file
descriptors to be written to their pipes.

It is in ``on_new_connection`` (the TCP infrastructure is initialized in
``main()``), that we accept the client socket and pass it along to the next
worker in the round-robin.

.. rubric:: multi-echo-server/main.c
.. literalinclude:: ../code/multi-echo-server/main.c
    :linenos:
    :lines: 29-47
    :emphasize-lines: 9,12-13

Again, the ``uv_write2`` call handles all the abstraction and it is simply
a matter of passing in the file descriptor as the right argument. With this our
multi-process echo server is operational.

TODO what do the write2/read2 functions do with the buffers?

----

.. [#] In this section domain sockets stands in for named pipes on Windows as
    well.
