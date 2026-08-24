# Chordec

> [!NOTE]
> This is unfinished project, and will release soon 😁

A program to detect any chords on music


## Build Information

- Using make : 

```bash
make test
```

- Using cmake : 

    - For testing
    ```bash
    mkdir build && cd build
    cmake .. -DBUILD_TESTS=ON
    cmake --build .
    ```

    - For main
    ```bash
    mkdir build && cd build
    cmake .. -DBUILD_MAIN=ON
    cmake --build .
    ```