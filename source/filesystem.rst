Filesystem
==========

Simple filesystem read/write is achieved using the `uv_fs_*` functions and the
`uv_fs_t` struct.

.. note::

    The libuv filesystem operations are different from :doc:`socket operations
    <networking>`. Socket operations use the non-blocking operations provided
    by the operating system. Filesystem operations use blocking functions
    internally, but invoke these functions in a thread pool and notify watchers
    registered with the event loop when application interaction is required.

.. note::

    The fs operations are actually a part of `libeio` on Unix systems. `libeio`
    is a separate library written by the author of `libev`. TODO link

All filesystem functions have two forms - *synchronous* and *asynchronous*.

The *synchronous* forms automatically get called (and **block**) if no callback
is specified. The return value of functions is the equivalent Unix return value
(usually 0 on success, -1 on error).

The *asynchronous* form is called when a callback is passed and the return
value is 0.

Reading/Writing files
---------------------

A file descriptor is obtained using

.. code-block:: c

    int uv_fs_open(uv_loop_t* loop, uv_fs_t* req, const char* path, int flags, int mode, uv_fs_cb cb)

`flags` and `mode` are standard
`Unix flags <http://man7.org/linux/man-pages/man2/open.2.html>`_.
libuv takes care of converting to the appropriate Windows flags.

File descriptors are closed using

.. code-block:: c

    int uv_fs_close(uv_loop_t* loop, uv_fs_t* req, uv_file file,

Filesystem operation callbacks have the signature:

.. code-block:: c

    void callback(uv_fs_t* req);

Let's see a simple implementation of `cat`. We start with registering
a callback for when the file is opened:

.. literalinclude:: ../code/file-read/main.c
    :linenos:
    :lines: 39-48
    :emphasize-lines: 2

The `result` field of a `uv_fs_t` is the file descriptor in case of the
`uv_fs_open` callback. If the file is successfully opened, we start reading it.

.. warning::

    The `uv_fs_req_cleanup()` function must be called to free internal memory
    allocations in libuv.

.. literalinclude:: ../code/file-read/main.c
    :linenos:
    :lines: 24-37
    :emphasize-lines: 6,9,12

In the case of a read call, you should pass an *initialized* buffer which will
be filled with data before the read callback is triggered.

In the read callback the `result` field is 0 for EOF, -1 for error and the
number of bytes read on success.

Here you see a common pattern when writing asynchronous programs. The
`uv_fs_close()` call is performed synchronously. *Usually tasks which are
one-off, or are done as part of the startup or shutdown stage are performed
synchronously, since we are interested in fast I/O when the program is going
about its primary task and dealing with multiple I/O sources*. For solo tasks
the performance difference usually is negligible and may lead to simpler code.

We can generalize the pattern that the actual return value of the original
system call is stored in `uv_fs_t.result`.

Filesystem writing is similarly simple using `uv_fs_write()`.  *Your callback
will be triggered after the write is complete*.  In our case the callback
simply drives the next read. Thus read and write proceed in lockstep via
callbacks.

.. literalinclude:: ../code/file-read/main.c
    :linenos:
    :lines: 14-22
    :emphasize-lines: 7

.. note::

    The error usually stored in `errno` TODO link to errno.h can be accessed
    from `uv_fs_t.errorno`, but converted to a standard `UV_*` error code.
    There is currently no way to directly extract a string error message from
    the `errorno` field.

.. warning::

    Due to the way filesystems and disk drives are configured for performance,
    a write that 'succeeds' may not be committed to disk yet. See
    `uv_fs_fsync` for stronger guarantees.

Filesystem operations
---------------------

File modification notification
------------------------------

TODO rework section title

buffers and streams
Local I/O
