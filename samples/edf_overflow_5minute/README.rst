.. zephyr:code-sample:: edf_overflow
   :name: EDF Overflow

   Demonstrate and handle an Earliest Deadline First (EDF) scheduling overflow condition.

Overview
********

A sample application that demonstrates how the Zephyr RTOS handles an Earliest 
Deadline First (EDF) scheduling overflow. It configures real-time threads with 
conflicting deadlines to intentionally trigger and log an overflow condition, 
making it useful for verifying deterministic deadline-miss handlers.

Building and Running
********************

This application can be built and executed on QEMU as follows:

.. zephyr-app-commands::
   :zephyr-app: samples/edf_overflow_5minute
   :board: qemu_cortex_m3
   :goals: run
   :compact:

To build for another board, change "qemu_x86" above to that board's name.

Sample Output
=============
qemu-system-arm: warning: nic stellaris_enet.0 has no peer
*** Booting Zephyr OS build c1f1215991a8 ***
[00:00:00.000,000] <dbg> foo.main: Main: start

[00:06:00.010,000] <dbg> foo.thread_1_entry: thread_1: Started

[00:06:00.010,000] <dbg> foo.thread_2_entry: thread_2: Started with deadline 25154781 ticks

[00:06:00.060,000] <dbg> foo.thread_2_entry: thread_2: Finished

[00:06:00.060,000] <dbg> foo.thread_1_entry: thread_1: Started with deadline 1073741823 ticks

[00:06:10.060,000] <dbg> foo.thread_1_entry: thread_1: Finished