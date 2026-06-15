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
   :zephyr-app: samples/edf_overflow
   :board: qemu_cortex_m3
   :goals: run
   :compact:

To build for another board, change "qemu_cortex_m3" above to that board's name.

Sample Output
=============
[142/144] To exit from QEMU enter: 'CTRL+a, x'[QEMU] CPU: cortex-m3
qemu-system-arm: warning: nic stellaris_enet.0 has no peer
*** Booting Zephyr OS build 8b5ef0e4d7b9 ***
[00:00:00.000,000] <dbg> foo.main: Main: start

[00:00:00.000,000] <dbg> foo.main: Main: starting thread1

[00:00:00.000,000] <dbg> foo.thread_2_entry: thread_2: Started with deadline 1073757778 ticks

[00:00:00.050,000] <dbg> foo.thread_2_entry: thread_2: Finished

[00:00:00.050,000] <dbg> foo.thread_1_entry: thread_1: Started with deadline 2147499308 ticks

[00:00:00.100,000] <dbg> foo.thread_1_entry: thread_1: Finished