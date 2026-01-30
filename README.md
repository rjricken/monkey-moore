# Monkey-Moore

![CI Status](https://github.com/rjricken/monkey-moore/actions/workflows/ci.yml/badge.svg)
![License](https://img.shields.io/badge/license-GPLv3-blue)
![C++ Standard](https://img.shields.io/badge/C%2B%2B-14-blue.svg)

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

Monkey-Moore uses **wxWidgets** for its cross-platform GUI. To build the project on Linux, you must satisfy the dependencies and use CMake.

### 1. Install Dependencies
You will need a C++ compiler, CMake, and the wxWidgets 3.2 development libraries.

```bash
sudo apt-get install build-essential cmake libwxgtk3.2-dev
```

### 2. Compile

Generate the Makefile and compile the application:

```bash
cd build
cmake ..
make
```

### 3. Running the application

Once compiled, you can run the executable directly:

```bash
./MonkeyMoore
```

## Linux Compatibility & Known Issues

While the core search logic is fully functional on Linux, the GUI (originally designed for Windows) has some known visual quirks on modern Linux desktop environments:
- **Dark Mode Conflicts**: The UI does not currently support system-wide dark modes, which may result in unreadable text.
    - Workaround: Force the application to use a light theme by running it with the following environment variable:

        ```bash
        GTK_THEME=Adwaita:light ./MonkeyMoore
        ```
- **UI Scaling**: Buttons may appear too small or have cropped icons on certain resolutions.
- **Table Builder**: The Table Builder feature is currently non-functional on Linux.

_These issues are being tracked and will be addressed in future updates._
