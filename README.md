# psr_exercise

```sh
git clone git@github.com:DublerIno/psr_exercise.git
```

## PSR Exercise 2: memory allocation and mutex

The implementation is in `Core/Src/app_threadx.c`. Three threads each receive
a 1024-byte stack allocated with `tx_byte_allocate()` from the application byte
pool. The pool itself has static backing storage; the thread stacks are allocated
dynamically from it.

Thread 1 creates the global `led_mutex` before its loop, then resumes threads 2
and 3. All three have equal priority and use the same entry function with different
LED indices. Each waits for the mutex, turns its green LED on, sleeps for one
second while holding the mutex, turns the LED off, and releases the mutex.
Only one green LED can be on at a time. The one-second mutex demonstration extends
the earlier single-thread 500 ms blinking exercise.

### Allocation overhead

Immediately before and after each stack allocation, `tx_byte_pool_info_get()`
reads the available bytes. No other thread is running during these measurements.

```text
consumed bytes = available_before - available_after
overhead bytes = consumed bytes - requested bytes
```

For this project's 32-bit STM32 target and bundled ThreadX allocator, each
1024-byte allocation should consume 1032 bytes: **8 bytes overhead** (a 4-byte next
block pointer and a 4-byte allocation marker/pool pointer). The request is already
aligned, and the pool has enough space to split each block. Other request sizes
can also incur alignment padding or an unused remainder too small to split.
The code measures the result instead of hard-coding 8.

Open the board's ST-LINK virtual COM port at **115200 baud, 8 data bits, no parity,
1 stop bit, no flow control**, then reset/run the board. The existing BSP routes
`printf()` to COM1 (USART3). Expected output with the current 50 KiB pool:

```text
LED1 stack: before=51192, after=50160, requested=1024, used=1032, overhead=8 bytes
LED2 stack: before=50160, after=49128, requested=1024, used=1032, overhead=8 bytes
LED3 stack: before=49128, after=48096, requested=1024, used=1032, overhead=8 bytes
```

The pool reserves 8 bytes for its end marker at creation, separately from the
per-allocation overhead. The three stacks therefore consume 3096 bytes in total:
3072 bytes requested plus 24 bytes overhead. The global arrays `available_before`,
`available_after`, and `allocation_overhead` also expose the results in the debugger.

### TraceX demonstration

Event tracing is enabled in `Core/Inc/tx_user.h`. The application enables its
64000-byte, word-aligned `tracex_buffer` before allocating the stacks.

1. Build and launch the existing STM32CubeIDE Debug configuration.
2. Run for about 4 seconds, then suspend the target. The trace is circular, so
   leaving it running longer can overwrite the startup allocation events.
3. In the debugger's GDB console, export the complete buffer as a binary file
   (the end address is exclusive):

   ```gdb
   dump binary memory /tmp/PSR_Ex_2.trx (char *)&tracex_buffer (char *)&tracex_buffer + sizeof(tracex_buffer)
   ```

4. Open that file in TraceX. Inspect the byte allocation and thread creation
   events, followed by creation of `Shared LED mutex`, mutex gets/puts, and thread
   sleep/suspend/resume events for `LED1 green`, `LED2 green`, and `LED3 green`.
   While the owner sleeps with its LED on, the other two threads wait for the
   mutex. Ownership transfers after the owner wakes, switches its LED off, and
   releases the mutex.

These are expected hardware results, not a captured run. Capture the terminal
output and TraceX view on the board for the exercise presentation.
