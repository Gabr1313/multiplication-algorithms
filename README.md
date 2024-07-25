# Multiplication Algorithm

In this repository, I share my implementation of famous multiplication algorithms written in `C`.  
So far, I have implemented:
- Naif: O(n^2)
- Karatsuba: O(n^(log2(3)))

## Building
Refer to the `Makefile` for building instructions.  
There are 3 modes available:
- `debug` (default): Includes flags for GCC debugging.
- `release`: Includes flags for a release build.
- `fast`: Includes flags for a release build with statically linked library and without assertions.

There are 3 buildable executables:

```sh
mkdir ./obj # required for building
make MODE=release gen
make clean MODE=fast naif
make clean MODE=fast karatsuba
```

## Testing
To test the multiplication of numbers with approximately 1000 base-2 digits, run:
```sh
./test.sh 1000
```
