Multiple event loops
====================

It is possible to use multiple event loops in the same thread. But this usually
makes no sense since the `uv_run()` call of one loop will block and stop the
other loop from running at all. With a careful combination of `uv_run(loop,
UV_RUN_ONCE)` you could do some really fun things though.

Modality
--------

You can use multiple loops to create a 'modal' step in your program, where the
second event loop 'pauses' the first event loop until some action occurs (a
user presses Return or you get a new event or something). An

One loop per thread
-------------------

This is the 'standard model', no different from spawning multiple processes
like we did in the :doc:`processes` chapter.

Using two loops for synchronization
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

There is a very specific use-case where two event loops can be used as
a synchronization mechanism in place of conditional variables. I used it in
`node-taglib <https://github.com/nikhilm/node-taglib>`_. libuv did not have
condition variable support then, and I've kept it that way for now to allow
it to work with earlier node versions. The specific use case is:

1. The *main thread* calls a blocking function in a *worker thread* using
   `uv_queue_work()`.
2. The *worker thread* has to call a custom function. The catch is that the
   custom function *has to run on the main thread*.
3. The *worker thread* has to wait until this function returns.

The condition variable approach is:

1. The worker thread doesn't directly call the custom function. It instead
   creates a `uv_async_t` handler. The callback for this handler calls the
   custom function.
2. Initializes a condition variable.
3. It uses `uv_async_send()` to get the main thread (where the event loop runs)
   to invoke the function on its behalf.
4. Waits on the condition variable.
5. The callback calls the custom function, then signals the condition variable
   which lets the worker thread continue.

The event loop implementation instead:

1. Creates a new event loop in the worker thread.
2. Associates a `uv_async_t` with this new loop.
3. Passes this handler to the `main thread` through the original `uv_async_t`
   handler's `data` field.
4. `uv_run()` the new event loop, which now blocks because the async handler
   has incremented it's :ref:`refcount <reference-count>`.
5. The callback in the main thread calls the custom function, then uses
   `uv_async_send()` to signal the async handler on the new loop.
6. The callback for this async handler simply closes the handler itself, the
   new loop's refcount drops to zero, `uv_run()` returns and the worker thread
   can continue.
