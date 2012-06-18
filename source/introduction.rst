Introduction
============

An Introduction to libuv is a small set of tutorials about using :term:`libuv` as
a high performance evented I/O library which offers the same API on Windows and Unix and TODO

This book is meant to cover the main areas of libuv, but is not a comprehensive
reference discussing every function and structure.

Who this book is for
--------------------

If you are reading this book, you are either a systems programmer, creating
low-level programs such as daemons or network services and clients. You have
found that the event loop approach is well suited for your application and
decided to use libuv.

You may also be a node.js module writer, who wants to wrap platform APIs
written in C or C++ with a set of (a)synchronous APIs that are exposed to
JavaScript. 

This book assumes that you are comfortable with the C programming language.

What is libuv?
--------------

When the :term:`node.js` project began in 2009 as a JavaScript environment
decoupled from the browser. Using Google's V8 and Marc Lehmann's libev (TODO:
links) node.js combined a model of I/O -- evented -- with a language that was
well suited to the style of programming due to the way it had been shaped by
browsers. As node.js grew in popularity, there was a need to make it run on
Windows, but libev ran only on Unix. Windows equivalent of kernel notification
mechanisms like kqueue or (e)poll is IOCP. libuv is an abstraction around libev
or IOCP depending on the platform, providing users an API based on libev.

libuv offers core utilities like timers, non-blocking networking support,
asynchronous file system access, child processes and more in an **always
asynchronous** manner. It uses a :doc:`**event-loop** <eventloops>` driven
process to function in a single thread.

TODO clear up about async and threads

Code
----

All the code from this book is included as part of the source of the book on
Github. Clone/Download the book (TODO: link) and run `make` in the `code/`
folder to compile all the examples. This book and the code is based on libuv
version `node-v0.7.9` and a version is included in the `code/` folder.
