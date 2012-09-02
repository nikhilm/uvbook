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

These are available only on Unix, ``uv_spawn`` will fail on Windows with
``UV_ENOTSUP``.

* ``UV_PROCESS_WINDOWS_VERBATIM_ARGUMENTS`` - No quoting or escaping of
  ``uv_process_options_t.args`` is done on Windows. Ignored on Unix.
* ``UV_PROCESS_DETACHED`` - Starts the child process in a new session, which
  will keep running after the parent process exits. See example below.

Detaching processes
-------------------


Let's create a simple online programming contest platform (called uvcoder).
When a contestant submits C code, you want to compile it using `gcc`. `gcc`
exits with success if the compilation succeeded.


streams
named pipes IPC
does ev limitation of ev_child only on default loop extend to libuv? yes it
does for now
