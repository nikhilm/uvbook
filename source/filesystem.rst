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

.. rubric:: file-read/main.c - opening a file
.. literalinclude:: ../code/file-read/main.c
    :linenos:
    :lines: 39-48
    :emphasize-lines: 2

The `result` field of a `uv_fs_t` is the file descriptor in case of the
`uv_fs_open` callback. If the file is successfully opened, we start reading it.

.. warning::

    The `uv_fs_req_cleanup()` function must be called to free internal memory
    allocations in libuv.

.. rubric:: file-read/main.c - read callback
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

.. rubric:: file-read/main.c - write callback
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

We set the dominos rolling in `main()`:

.. rubric:: file-read/main.c
.. literalinclude:: ../code/file-read/main.c
    :linenos:
    :lines: 50-54
    :emphasize-lines: 2

Filesystem operations
---------------------

All the standard filesystem operations like `unlink`, `rmdir`, `stat` are
supported asynchronously and have intuitive argument order. They follow the
same patterns as the read/write/open calls, returning the result in the
`uv_fs_t.result` field. The full list:

.. rubric:: Filesystem operations
.. literalinclude:: ../libuv/include/uv.h
    :lines: 1390-1466

Buffers and Streams
-------------------

The basic I/O tool in libuv is the stream (`uv_stream_t`). TCP sockets, UDP
sockets, named pipes for file I/O and IPC are all treated as stream subclasses.

Streams are initialized using custom functions for each subclass, then operated
upon using

.. code-block:: c

    int uv_read_start(uv_stream_t*, uv_alloc_cb alloc_cb, uv_read_cb read_cb);
    int uv_read_stop(uv_stream_t*);
    int uv_write(uv_write_t* req, uv_stream_t* handle,
                uv_buf_t bufs[], int bufcnt, uv_write_cb cb);

The stream based functions are simpler to use than the filesystem ones and
libuv will automatically keep reading from a stream when `uv_read_start()` is
called once, until `uv_read_stop()` is called.

The discrete unit of data is the buffer -- `uv_buf_t`. This is simply
a collection of a pointer to bytes (`uv_buf_t.base`) and the length
(`uv_buf_t.len`). The `uv_buf_t` is lightweight and passed around by value.
What does require management is the actual bytes, which have to be allocated
and freed by the application.

To demonstrate streams using pipes, here is a simple tee utility using libuv.
Doing all operations asynchronously shows the power of evented I/O. The two
writes won't block each other, but we've to be careful to copy over the buffer
data to ensure we don't free a buffer until it has been written.

The program is to be executed as:

    ./uvtee <output_file>

We start of opening pipes on the files we require. libuv pipes to a file are
opened as bidirectional by default.

.. rubric:: uvtee/main.c - read on pipes
.. literalinclude:: ../code/uvtee/main.c
    :linenos:
    :lines: 62-81
    :emphasize-lines: 4,5,15

The third argument of `uv_pipe_init()` should be set to 1 for IPC using named
pipes. This is covered in :doc:`processes`. The `uv_pipe_open()` call
associates the file descriptor with the file.

We start monitoring `stdin`. The `alloc_buffer` callback is invoked as new
buffers are required to hold incoming data. `read_stdin` will be called with
these buffers.

.. rubric:: uvtee/main.c - reading buffers
.. literalinclude:: ../code/uvtee/main.c
    :linenos:
    :lines: 19-22,44-60

The standard `malloc` is sufficient here, but you can use any memory allocation
scheme. For example, node.js uses its own slab allocator which associates
buffers with V8 objects.

The read callback `nread` parameter is -1 on any error. This error might be
EOF, in which case we close all the streams, using the generic close function
`uv_close()` which deals with the handle based on its internal type.
Otherwise `nread` is a non-negative number and we can attempt to write that
many bytes to the output streams. Finally remember that buffer allocation and
deallocation is application responsibility, so we free the data.

.. rubric:: uvtee/main.c - Write to pipe
.. literalinclude:: ../code/uvtee/main.c
    :linenos:
    :lines: 9-13,23-42

`write_data()` makes a copy of the buffer obtained from read. Again, this
buffer does not get passed through to the callback trigged on write completion.
To get around this we wrap a write request and a buffer in `write_req_t` and
unwrap it in the callbacks.

File modification notification
------------------------------

TODO rework section title
