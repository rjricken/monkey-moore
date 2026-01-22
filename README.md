# Monkey-Moore

Monkey-Moore is a high-performance relative search utility designed for data hacking and reverse engineering. By leveraging an optimized version of the Boyer-Moore string search algorithm, it performs rapid pattern matching across large files. The tool offers a wide range of configuration options to maximize the success of finding data in unknown or proprietary encodings.

## Overview

Relative searching is a specialized method used to discover data that cannot be found through standard hex or text searches. Unlike traditional methods that look for exact byte values, a relative search identifies data based on its internal pattern. This allows you to find text or data regardless of how it is encoded, as long as the mathematical relationship between the values remains consistent.

### Example

Imagine a sequence of characters encoded in a non-standard variation of ASCII where every character's value has been decreased by 2. The word `"code"` would appear in the file as `"ambc"`.

A standard search for "code" would fail, but Monkey-Moore would successfully locate it by analyzing the relative differences between the characters rather than the values themselves.

## Key Features
- **Pattern Flexibility**: Full support for wildcards to increase the chances of matching longer text sequences.
- **8 and 16-bit Support**: Capability to search across different character widths.
- **Endianness Selector**: Toggle between Big-Endian and Little-Endian formats for 16-bit searches.
- **Value Scan**: Perform relative searches by providing raw numerical sequences; the tool automatically infers the underlying relative differences to locate matching patterns.
- **Custom Character Sequences**: Define your own sequences to target specific languages sucb as Japanese.

## Performance

The core of Monkey-Moore is built for speed:
- **Boyer-Moore Algorithm**: The search engine uses a preprocessing step to analyze the search pattern and create a skip-table. This allows the tool to bypass large sections of the target file that cannot possibly contain a match, significantly outperforming brute-force search methods.
- **Multi-Threading**: Monkey-Moore is designed for modern hardware. It utilizes multi-threading to distribute the workload across every available CPU core. This parallel processing drastically reduces the time required to scan large ROMs or binary blobs.

## Compatibility & Prerequisites

Monkey-Moore is a cross-platform application built using the wxWidgets GUI framework. It is currently optimized and supported for Windows and Linux environments.

### Dependencies

To build or run the project from source, the following libraries are required:
* [wxWidgets](https://github.com/wxWidgets/wxWidgets) 3.0+: Used for the cross-platform native graphical user interface.
* [XML Parser](http://www.applied-mathematics.net/tools/xmlParser.html) 2.40+: Handles user configuration and customization files.
