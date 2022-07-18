# Synchronized-Stack-w-Processes
I created a stack the is synchronized between different processes with shared memory.  
To make the memory shared I used mmap and to synchronize I locked files using fcntl.
## About the Files
### stack.hpp and stack.cpp:
Contain the stack data stracture and methods.
### client.cpp:
The client side.
### server.cpp:
The server side.
### part5.cpp:
Implementation of malloc and free using mmap.
