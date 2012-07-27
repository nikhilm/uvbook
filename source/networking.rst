Networking
==========

Networking in libuv is not much different from directly using the BSD socket
interface, some things are easier, all are non-blocking but the concepts stay
the same. In addition libuv offers utility functions to abstract the annoying,
repetitive and low-level tasks like setting up sockets using the BSD socket
structures, DNS lookup, and tweaking various socket parameters.

The ``uv_tcp_t`` and ``uv_udp_t`` structures are used for network I/O.

TCP
---

Server sockets proceed by:

1. ``uv_tcp_init`` the TCP watcher.
2. ``uv_tcp_bind`` it.
3. Call ``uv_listen`` on the watcher to have a callback invoked whenever a new
   connection is established by a client.
4. Use ``uv_accept`` to accept the connection.
5. Use :ref:`stream operations <buffers-and-streams>` to communicate with the
   client.

Here is a simple echo server

.. rubric:: tcp-echo-server/main.c
.. literalinclude:: ../code/tcp-echo-server/main.c
    :linenos:
    :lines: 50-
    :emphasize-lines: 4-5,7-9

You can see the utility function ``uv_ip4_addr`` being used to convert from
a human readable IP address, port pair to the sockaddr_in structure required by
the BSD socket APIs. The reverse can be obtained using ``uv_ip4_name``.

.. NOTE::

    In case it wasn't obvious there are ``uv_ip6_*`` analogues for the ip4
    functions.

Most of the setup functions are normal functions since its all CPU-bound.
``uv_listen`` is where we return to libuv's callback style. The second
arguments is the backlog queue -- the maximum length of queued connections.

When a connection is initiated by clients, the callback is required to set up
a watcher for the client socket and associate the watcher using ``uv_accept``.
In this case we also establish interest in reading from this stream.

.. rubric:: tcp-echo-server/main.c
.. literalinclude:: ../code/tcp-echo-server/main.c
    :linenos:
    :lines: 34-48
    :emphasize-lines: 9-10

The remaining set of functions is very similar to the streams example and can
be found in the code. Just remember to call ``uv_close`` when the socket isn't
required. This can be done even in the ``uv_listen`` callback if you are not
interested in accepting the connection.

TCP on the client-side
----------------------

Where you do bind/listen/accept, on the client side its simply a matter of
calling ``uv_tcp_connect``. The same ``uv_connect_cb`` style callback of
``uv_listen`` is used by ``uv_tcp_connect``. Try::

    uv_tcp_t socket;
    uv_tcp_init(loop, &socket);

    uv_connect_t connect;

    struct sockaddr_in dest = uv_ip4_addr("127.0.0.1", 80);

    uv_tcp_connect(&connect, &socket, dest, on_connect);

where ``on_connect`` will be called after the connection is established.

Querying DNS
------------

libuv provides asynchronous DNS resolution. For this it provides its own
``getaddrinfo`` replacement, backed by `c-ares`_. In the callback you can
perform normal socket operations on the retrieved addresses. Let's connect to
Freenode to see an example of DNS resolution.

.. rubric:: dns/main.c
.. literalinclude:: ../code/dns/main.c
    :linenos:
    :lines: 61-
    :emphasize-lines: 12

If ``uv_getaddrinfo`` returns non-zero, something went wrong in the setup and
your callback won't be invoked at all. All arguments can be freed immediately
after ``uv_getaddrinfo`` returns. The `hostname`, `servname` and `hints`
structures are documented in `the getaddrinfo man page <getaddrinfo>`_.

In the resolver callback, you can pick any IP from the linked list of ``struct
addrinfo(s)``. This also demonstrates ``uv_tcp_connect``. It is necessary to
call ``uv_freeaddrinfo`` in the callback.

.. rubric:: dns/main.c
.. literalinclude:: ../code/dns/main.c
    :linenos:
    :lines: 41-59
    :emphasize-lines: 8,16

.. _c-ares: http://c-ares.haxx.se
.. _getaddrinfo: http://www.kernel.org/doc/man-pages/online/pages/man3/getaddrinfo.3.html

TODO UDP, multicast
