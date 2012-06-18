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

Reading/Writing files
----------------------

Filesystem operations
---------------------

File modification notification
------------------------------

TODO rework section title

buffers and streams
Local I/O
