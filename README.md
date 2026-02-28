# Monkey-Moore

![CI Status](https://github.com/rjricken/monkey-moore/actions/workflows/ci.yml/badge.svg)
![License](https://img.shields.io/badge/license-GPLv3-blue)
![C++ Standard](https://img.shields.io/badge/C%2B%2B-17-blue.svg)

**Monkey-Moore** is a high-performance relative search utility designed specifically for ROM hacking, data mining, and reverse engineering.

By leveraging an optimized, multi-threaded implementation of the **Boyer-Moore string search algorithm**, it performs rapid pattern matching across large binary files. Monkey-Moore offers extensive configuration options, making it the ideal tool for locating text or data in proprietary formats where the encoding is unknown.

## Overview: What is Relative Searching?

Relative searching is a specialized technique used to discover data that cannot be found via standard hexadecimal or ASCII text searches.

Unlike traditional methods that scan for exact byte values, a relative search identifies data based on the **internal pattern of differences** between values. This allows you to locate text regardless of how it is encoded, provided the mathematical relationship between characters remains consistent.

### Example
Imagine a text string encoded in a non-standard ASCII variation where every character's value is decremented by 2.
* **Standard Text:** `"code"`
* **Encoded Values:** `"ambc"`

A standard text search for "code" would fail. However, Monkey-Moore analyzes the **relative difference** between bytes (e.g., the distance between 'c' and 'o', 'o' and 'd'). Since this relational pattern is identical in both the standard and encoded versions, Monkey-Moore will successfully locate the string.

## Key Features

* **Pattern Flexibility:** Full wildcard support allows for matching complex or partially known sequences.
* **8-bit & 16-bit Support:** Search across different character widths to accommodate various system architectures (e.g., NES vs. SNES/GBA).
* **Endianness Selector:** Toggle between Big-Endian and Little-Endian formats for accurate 16-bit searches.
* **Value Scan:** Input raw numerical sequences directly; the tool automatically infers the underlying relative differences to locate matching patterns.
* **Custom Character Sequences:** Define custom character sets to target specific languages, such as Japanese (Kana/Kanji).

## Performance

Monkey-Moore is built for speed on modern hardware:

* **Boyer-Moore Algorithm:** The search engine utilizes a preprocessing step to generate a skip-table. This allows the tool to intelligently bypass large sections of the target file that cannot contain a match, significantly outperforming brute-force methods.
* **Multi-Threading:** Workloads are distributed across all available CPU cores. This parallel processing drastically reduces the time required to scan large ROMs or massive binary blobs.

## Building from Source (Linux)

Monkey-Moore uses **CMake** for its build system and **wxWidgets** for the cross-platform GUI. A `Justfile` is included for convenience if you use the [Just command runner](https://github.com/casey/just).

### 1. Install Dependencies
You will need a C++17 compliant compiler, CMake (3.19+), and the wxWidgets development libraries.

```bash
sudo apt-get install build-essential cmake libwxgtk3.2-dev
```

### 2. Compile & Run (using Just)

If you have `just` installed, building is simple:

```bash
just configure-release
just build-release
just run-release
```

### 3. Compile (standard CMake)

If you do not use `just`, you can build manually:

```bash
# Configure (Release mode recommended for performance)
cmake --preset release

# Compile
cmake --build --preset release
```

### 4. Running the Application

The compiled binary is located in the `src/gui` subdirectory of your build folder:

```bash
./build-release/src/gui/monkey-moore-gui
```
## Unit Tests

Monkey-Moore uses **Catch2** for unit testing the core search algorithms and multithreaded search engine.

```bash
# Run tests via Just
just test

# Or manually via CTest
ctest --preset debug
```
