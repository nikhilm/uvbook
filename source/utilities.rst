Utilities
=========

This chapter catalogues tools and techniques which are useful for common tasks.
The `libev man page`_ already covers some patterns which can be adopted to
libuv through simple API changes.  It also covers parts of the libuv API that
don't require entire chapters dedicated to them.

idle watchers
timers
check & prepare watchers

Idle watcher pattern
--------------------

Idle watchers are invoked

Reference count tweaking
------------------------

Async baton
-----------

The general pattern involves passing around
a baton structure [#]_ to get data to and fro between your application and the
blocking function call::

    TODO above

.. [#] I was first introduced to the term baton in this context, in Konstantin
       Käfer's excellent slide set. http://kkaefer.github.com/node-cpp-modules/#baton

.. _libev man page: http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#COMMON_OR_USEFUL_IDIOMS_OR_BOTH
