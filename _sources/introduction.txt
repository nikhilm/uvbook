Introduction
============

This 'book' is a small set of tutorials about using libuv_ as
a high performance evented I/O library which offers the same API on Windows and Unix.

It is meant to cover the main areas of libuv, but is not a comprehensive
reference discussing every function and data structure. The `official libuv
documentation`_ is included directly in the libuv header file.

.. _official libuv documentation: https://github.com/joyent/libuv/blob/master/include/uv.h

This book is still a work in progress, so sections may be incomplete, but
I hope you will enjoy it as it grows.

Who this book is for
--------------------

If you are reading this book, you are either:

1) a systems programmer, creating low-level programs such as daemons or network
   services and clients. You have found that the event loop approach is well
   suited for your application and decided to use libuv.

2) a node.js module writer, who wants to wrap platform APIs
   written in C or C++ with a set of (a)synchronous APIs that are exposed to
   JavaScript. You will use libuv purely in the context of node.js. For
   this you will require some other resources as the book does not cover parts
   specific to v8/node.js.

This book assumes that you are comfortable with the C programming language.

Background
----------

The node.js_ project began in 2009 as a JavaScript environment decoupled
from the browser. Using Google's V8_ and Marc Lehmann's libev_, node.js
combined a model of I/O -- evented -- with a language that was well suited to
the style of programming; due to the way it had been shaped by browsers. As
node.js grew in popularity, it was important to make it work on Windows, but
libev ran only on Unix. The Windows equivalent of kernel event notification
mechanisms like kqueue or (e)poll is IOCP. libuv was an abstraction around libev
or IOCP depending on the platform, providing users an API based on libev.
In the node-v0.9.0 version of libuv `libev was removed`_.

Since then libuv has continued to mature and become a high quality standalone
library for system programming. Users outside of node.js include Mozilla's
Rust_ programming language, and a variety_ of language bindings.

The first independently release version of libuv was 0.10.2.

Code
----

All the code from this book is included as part of the source of the book on
Github. `Clone`_/`Download`_ the book and run ``make`` in the ``code/``
folder to compile all the examples. This book and the code is based on libuv
version `v0.10.3`_ and a version is included in the ``libuv/`` folder
which will be compiled automatically.

.. _Clone: https://github.com/nikhilm/uvbook
.. _Download: https://github.com/nikhilm/uvbook/downloads
.. _v0.10.3: https://github.com/joyent/libuv/tags
.. _V8: http://code.google.com/p/v8/
.. _libev: http://software.schmorp.de/pkg/libev.html
.. _libuv: https://github.com/joyent/libuv
.. _node.js: http://www.nodejs.org
.. _libev was removed: https://github.com/joyent/libuv/issues/485
.. _Rust: http://rust-lang.org
.. _variety: https://github.com/joyent/libuv/wiki/Projects-that-use-libuv
