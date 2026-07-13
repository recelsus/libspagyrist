# libspagyrist

libspagyrist is a C++ library for search candidates, selection, document
models, rendering, and output.

Applications convert fetched data into libspagyrist's common structures.
libspagyrist then handles selection, rendering, and output.

## Features

- Common search candidate structure: `spagyrist::candidate`
- Selection: `fzf` selector, number selector, fallback selector
- Common document structure: `spagyrist::document`
- Inline / block based document model
- Markdown / plain text / terminal rendering
- stdout output
- CMake package install

## Responsibility

libspagyrist handles:

- candidate
- document
- selector
- renderer
- output
- validation

libspagyrist does not handle:

- HTTP requests
- JSON / HTML parsers
- Source-specific behavior
- Source-specific search APIs
- Source-specific article, page, or data conversion

## Build

Requirements:

- C++20 compiler
- CMake 3.20 or newer

```sh
mkdir -p build
cd build
cmake ..
make
```

```sh
ctest --output-on-failure
```

## Install

```sh
mkdir -p build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/path/to/install
make
cmake --install .
```

After installation, use the library from CMake with
`find_package(spagyrist REQUIRED)`.

```cmake
find_package(spagyrist REQUIRED)

target_link_libraries(your_app
  PRIVATE
    spagyrist::spagyrist
)
```

## Usage

```cpp
#include <spagyrist/spagyrist.hpp>

#include <iostream>
#include <vector>

int main()
{
    spagyrist::document document;
    document.metadata.title = "Example";
    document.metadata.source = "fixture";
    document.metadata.language = "en";

    document.blocks.push_back(spagyrist::block::heading(
        1,
        {spagyrist::inline_element::text_node("Example")}));

    document.blocks.push_back(spagyrist::block::paragraph({
        spagyrist::inline_element::text_node("This document was built by an application."),
    }));

    std::cout << spagyrist::render(document, spagyrist::format::markdown);
}
```

Applications convert source search results into `spagyrist::candidate`, then
convert selected data into `spagyrist::document`.

## Selector

Use the `fzf` selector first and fall back to the number selector when `fzf` is
not available.

```cpp
spagyrist::fzf_selector primary;
spagyrist::number_selector fallback;

auto selected = spagyrist::select_candidate_with_fallback(
    primary,
    fallback,
    candidates);
```

## License

libspagyrist is licensed under the MIT License. See `LICENSE` for details.

Third-party notices are listed in `THIRD_PARTY_NOTICES.md`.
