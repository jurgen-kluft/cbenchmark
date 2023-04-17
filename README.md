# cbenchmark

WIP!!!

Cross platform benchmark library

- Windows
- Mac OS

## Extending to other platforms

To extend for an additional platform, e.g. Linux, add:

- TODO

## How to use 


## Benchmark Registration

You do need to register your benchmark suite to be part of the list of benchmarks to run.

```c++
BENCHMARK_SUITE_LIST(cBenchMarks);

BENCHMARK_SUITE_DECLARE(cBenchMarks, doubly_linked_list);

```

