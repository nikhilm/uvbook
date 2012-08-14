Utilities
=========

This chapter catalogues tools and techniques which are useful for common tasks.
The `libev man page`_ already covers some patterns which can be adopted to
libuv through simple API changes.  It also covers parts of the libuv API that
don't require entire chapters dedicated to them.

Timers
------

Check & Prepare watchers
------------------------

TODO

External I/O with polling
-------------------------

TODO

Loading libraries
-----------------

TODO

Idle watcher pattern
--------------------

The callbacks of idle watchers are only invoked when the event loop has no
other pending events. In such a situation they are invoked once every iteration
of the loop. The idle callback can be used to perform some very low priority
activity. For example, you could dispatch a summary of the daily application
performance to the developers for analysis during periods of idleness, or use
the application's CPU time to perform SETI calculations :) An idle watcher is
also useful in a GUI application. Say you are using an event loop for a file
download. If the TCP socket is still being established and no other events are
present your event loop will pause (**block**), which means your progress bar
will freeze and the user will think the application crashed. In such a case
queue up and idle watcher to keep the UI operational.

.. rubric:: idle-compute/main.c
.. literalinclude:: ../code/idle-compute/main.c
    :linenos:
    :lines: 5-9, 32-
    :emphasize-lines: 38

Here we initialize the idle watcher and queue it up along with the actual
events we are interested in. ``crunch_away`` will now be called repeatedly
until the user types something and presses Return. Then it will be interrupted
for a brief amount as the loop deals with the input data, after which it will
keep calling the idle callback again.

.. rubric:: idle-compute/main.c
.. literalinclude:: ../code/idle-compute/main.c
    :linenos:
    :lines: 10-19

Event loop reference count
--------------------------

The event loop only runs as long as there are active watchers. This system
works by having every watcher increase the reference count of the event loop
when it is started and decreasing the reference count when stopped. It is also
possible to manually change the reference count of handles using::

    void uv_ref(uv_handle_t*);
    void uv_unref(uv_handle_t*);

These functions can be used to allow a loop to exit even when a watcher is
active or to use custom objects to keep the loop alive.

The former can be used with interval timers. You might have a garbage collector
which runs every X seconds, or your network service might send a heartbeat to
others periodically, but you don't want to have to stop them along all clean
exit paths or error scenarios. Or you want the program to exit when all your
other watchers are done. In that case just unref the timer immediately after
creation so that if it is the only watcher running then ``uv_run`` will still
exit.

The later is used in node.js where some libuv methods are being bubbled up to
the JS API. A ``uv_handle_t`` (the superclass of all watchers) is created per
JS object and can be ref/unrefed.

.. rubric:: ref-timer/main.c
.. literalinclude:: ../code/ref-timer/main.c
    :linenos:
    :lines: 5-8, 17-
    :emphasize-lines: 9

We initialize the garbage collector timer, then immediately ``unref`` it.
Observe how after 9 seconds, when the fake job is done, the program
automatically exits, even though the garbage collector is still running.

.. _baton:

Passing data to worker thread
-----------------------------

When using ``uv_queue_work`` you'll usually need to pass complex data through
to the worker thread. The solution is to use a ``struct`` and set
``uv_work_t.data`` to point to it. A slight variation is to have the
``uv_work_t`` itself as the first member of this struct (called a baton [#]_).
This allows cleaning up the work request and all the data in one free call.

.. code-block:: c
    :linenos:
    :emphasize-lines: 2

    struct ftp_baton {
        uv_work_t req;
        char *host;
        int port;
        char *username;
        char *password;
    }

.. code-block:: c
    :linenos:
    :emphasize-lines: 2

    ftp_baton *baton = (ftp_baton*) malloc(sizeof(ftp_baton));
    baton->req.data = (void*) baton;
    baton->host = strdup("my.webhost.com");
    baton->port = 21;
    // ...

    uv_queue_work(loop, &baton->req, ftp_session, ftp_cleanup);

Here we create the baton and queue the task.

Now the task function can extract the data it needs:

.. code-block:: c
    :linenos:
    :emphasize-lines: 2, 12

    void ftp_session(uv_work_t *req) {
        ftp_baton *baton = (ftp_baton*) req->data;

        fprintf(stderr, "Connecting to %s\n", baton->host);
    }

    void ftp_cleanup(uv_work_t *req) {
        ftp_baton *baton = (ftp_baton*) req->data;

        free(baton->host);
        // ...
        free(baton);
    }

We then free the baton which also frees the watcher.

.. [#] I was first introduced to the term baton in this context, in Konstantin
       Käfer's excellent slides on writing node.js bindings --
       http://kkaefer.github.com/node-cpp-modules/#baton

.. _libev man page: http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#COMMON_OR_USEFUL_IDIOMS_OR_BOTH
