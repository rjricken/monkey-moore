# Monkey-Moore

Monkey-Moore is a powerful relative searcher for datahacking purposes. The core algorithm is based on the Boyer-Moore string search algorithm and it builds on top of it to perform fast searches over large files, supporting a wide range of options to further enhance the chances of finding what is being looked for.

## Overview

A relative search is a method used to find data that can't be found with regular methods. The difference is that a relative search matches data based on its pattern rather than based solely on its value. That means data can be found regarless of how it's encoded, as long as it maintains a pattern that can be observed.

A simple example would be of a sequence of characters encoded in a variation of the ASCII character set, where every character has its value decreased in 2. The word "code" would now look like "ambc", but Monkey-Moore would find it just fine by comparing the differences between each character instead of each character's value.

Monkey-Moore supports wildcards to further increase the flexibility of finding possible matches, and automatically optimizes textual searches containing only alphabetical characters.

Other features included:
* 8 and 16-bit searches
* Endianess selector
* Value scan relative searching
* Custom character sequences

## Performance

The core search algorithm is based on Boyer-Moore, which makes use of preprocessing step on the search pattern to create static data about it to be later used to speed the search by skipping parts of the target file.

Monkey-Moore also makes use of multi-threading to split the search among multiple CPU cores and thus cut the time needed to perform a search, specially in larger files, but as many cores as there are available in the machine it's running on.

## Prerequisites

Monkey-Moore is a multiplatform application built on top of the wxWidgets GUI toolset. Currently, only Windows and Linux are supported.

The dependencies are:
* [wxWidgets](https://github.com/wxWidgets/wxWidgets) 3.0+
* [XML Parser](http://www.applied-mathematics.net/tools/xmlParser.html) 2.40+