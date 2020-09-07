# reallocator

Experiments with a dynamic arena allocation-style allocator. Effectively
provides a dynamically growing arena.

Preliminary, non-scientific results:

* The `Reallocator` performs better than a thin shim on top of `malloc` and
  `free` _only_ when we're not optimizing. When we compile under `-O3`, there is
  no difference. This is likely because we're still frequently performing
  dynamic allocation under the `ReallocatorQueue`.

* The `ArenaAllocator` outperforms the `Reallocator` and
  `NormalAllocator`. Sometimes the known approach is the right approach :)

## License

MIT Open Source licensed, refer to the `LICENSE` file.
