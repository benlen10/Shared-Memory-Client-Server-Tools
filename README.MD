# Shared Memory Client & Server

### Description:
- Based on a project for CS537 - Operating Systems at The University of Wisconsin Madison
- Client and server that communicate through a shared memory page to display statistics about the client processes. All of the processes are running on the same machine. Each client process will periodically write to the shared memory page with updates about its recent behavior

### Client Output Example:
5, pid : 14084, birth : Fri Oct 13 22:59:27 2017, elapsed : 34 s 5.4670 ms, c1
5, pid : 14085, birth : Fri Oct 13 22:59:27 2017, elapsed : 34 s 5.1960 ms, c2
5, pid : 14276, birth : Fri Oct 13 23:00:02 2017, elapsed : 0 s 0.0010 ms, c3