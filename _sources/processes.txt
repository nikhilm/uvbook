Processes
=========

libuv offers considerable child process management, abstracting the platform
differences and allowing communication with the child process using streams or
named pipes.

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
``uv_spawn`` uses ``execvp`` internally, there is no need to supply the full
path. Finally as per underlying conventions, the arguments array *has* to be
one larger than the number of arguments, with the last element being ``NULL``.

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

Changing the process parameters
-------------------------------

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
    :lines: 12-27
    :emphasize-lines: 9,16

Just remember that the watcher is still monitoring the child, so your program
won't exit. Use ``uv_unref()`` if you want to be more *fire-and-forget*.

Signals and termination
-----------------------

libuv wraps the standard ``kill(2)`` system call on Unix and implements one
with similar semantics on Windows, with *one caveat*. ``uv_kill`` on Windows
only supports ``SIGTERM``, ``SIGINT`` and ``SIGKILL``, all of which lead to
termination of the process. The signature of ``uv_kill`` is::

    uv_err_t uv_kill(int pid, int signum);

For processes started using libuv, you may use ``uv_process_kill`` instead,
which accepts the ``uv_process_t`` watcher as the first argument, rather than
the pid. In this case, **remember to call** ``uv_close`` on the watcher.

Process I/O
-----------

libuv allows child processes to share the input/output streams or file
descriptors of the parent. It also supports named pipes as an IPC mechanism
amongst the processes. In addition, libuv allows pipes to be written across an
IPC border TODO

Inherited I/O
+++++++++++++

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
script prints gets sent to the client. By using processes, we can offload the
read/write buffering to the operating system, so in terms of convenience this
is great. Just be warned that creating processes is a costly task.

Pipes
+++++

uv_write2 TODO chapter name
+++++++++++++++++++++++++++

does ev limitation of ev_child only on default loop extend to libuv? yes it
does for now
