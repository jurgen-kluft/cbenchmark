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

You don't need to register your benchmark suite to be part of the list of benchmarks to run.
They are automatically registered at initialization time, if you however want to disable a benchmark suite, fixture or benchmark, you can do so by adding the following macro to your code:

```c++

BM_SUITE_DISABLE;
BM_FIXTURE_DISABLE;
BM_UNIT_DISABLE;
    
```

```c++
BENCHMARK_SUITE_LIST;

```

