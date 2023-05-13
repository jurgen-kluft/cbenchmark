# cbenchmark

Cross platform benchmark library

- Windows
- Mac OS

## Extending to other platforms

To extend for an additional platform, e.g. Linux, add:

- TODO

## How to use 


## Benchmark Registration

You don't need to register your benchmark suite, fixture or unit to be part of the list of benchmarks to run, they are automatically registered at initialization time. If you however want to disable a benchmark suite, fixture or benchmark, you can do so by adding one of the following macro's to your code:

```c++

BM_SUITE_DISABLE;
BM_FIXTURE_DISABLE;
BM_UNIT_DISABLE;
    
```


## Internals

A lot of effort has been put into removing nearly all dynamic heap allocations from the library, most of the heap allocations are done upfront.
Internally it mainly uses the ForwardAllocator and a ScratchAllocator to allocate and deallocate memory. Also for a benchmark that uses multiple
threads each thread is given its own ForwardAllocator with a dedicated block of memory.

