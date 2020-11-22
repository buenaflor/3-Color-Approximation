# 3-Color-Approximation
Approximate if graph is 3-colorable in C using multiple processes (implementation with semaphore and shared memory and circular buffer)

# Supervisor - Generator

The generator program takes a graph as input. The program repeatedly generates a random solution
to the problem and writes its result to the circular buffer. It repeats this procedure until it is notified by the supervisor to terminate.
The generators report their solutions to the supervisor by means of a circular buffer.

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
