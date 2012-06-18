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

Here is a simple example of reading a file.

.. literalinclude:: ../code/file-read/main.c
    :linenos:
    :emphasize-lines: 6-7,11,20,24,26,31,35

TODO don't include whole at one point, part by part explanations, especially
`result` field.

.. note::

    Error handling

Filesystem operations
---------------------

File modification notification
------------------------------

TODO rework section title

buffers and streams
Local I/O
