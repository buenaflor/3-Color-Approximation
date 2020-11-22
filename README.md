# 3-Color-Approximation
Approximate if graph is 3-colorable in C using multiple processes (implementation with semaphore and shared memory and circular buffer)

## Supervisor - Generator

The supervisor is in charge of setting up the shared memory and the semaphores and initializes the circular buffer in order to communicate with the generators. It waits for the generators to write solutions to the circular buffer and reads and prints them.

The generator program takes a graph as input and repeatedly generates a random solution for the 3-color problem and writes its result to the circular buffer. This procedure repeats until it is notified by the supervisor to terminate.

The program will terminate upon CTRL-C or a solution with 0 edges have been found. In this case, the graph is 3-colorable.

### Usage

Invocation of the supervisor:
```sh
$ ./supervisor
[./supervisor] Solution with 2 edges: 0-2 1-2
[./supervisor] Solution with 1 edges: 0-2
```

Invocation of the generator:
```sh
$ ./generator 0-1 0-3 0-4 1-2 1-3 1-4 1-5 2-4 2-5 3-4 4-5
```

Invocation of multiple generators:
```sh
$ for i in {1..10}; do (./generator 0-1 0-3 0-4 1-2 1-3 1-4 1-5 2-4 2-5 3-4 4-5 &); done
```
