Patterns
========

Idle watcher pattern

uv_ref/unref and allowing loop to quit

The general pattern involves passing around
a baton structure [#]_ to get data to and fro between your application and the
blocking function call::

    TODO above

.. [#] I was first introduced to the term baton in this context, in Konstantin
       Käfer's excellent slide set. http://kkaefer.github.com/node-cpp-modules/#baton
