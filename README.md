# libspagyrist

libspagyrist is a C++ library for search candidates, selection, document
models, rendering, and output.

Applications convert fetched data into libspagyrist's common structures.
libspagyrist then handles selection, rendering, and output.

## Features

- Common search candidate structure: `spagyrist::candidate`
- Selection: built-in selector, `fzf` selector, number selector, fallback selector
- Common document structure: `spagyrist::document`
- Inline / block based document model
- Markdown / plain text / terminal rendering
- stdout / editor output
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

When building libspagyrist as the top-level project, a small fixed-data client
is also built for quick manual checks. It is disabled by default when
libspagyrist is used as a subproject.

```sh
./build/examples/spagyrist_fixed_client --list
./build/examples/spagyrist_fixed_client --version
./build/examples/spagyrist_fixed_client --info
./build/examples/spagyrist_fixed_client --select first --format plain
./build/examples/spagyrist_fixed_client --select fzf --format terminal
printf '2\n' | ./build/examples/spagyrist_fixed_client --select number --format markdown
VISUAL=true ./build/examples/spagyrist_fixed_client --select first --output editor
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

## Output

The default output target is stdout. When editor output is explicitly selected,
libspagyrist resolves an editor in this order:

```text
VISUAL -> EDITOR -> nvim -> vim -> vi -> nano -> stdout
```

Editor-specific options such as `nvim -R` can be configured through `VISUAL` or
`EDITOR`. Passing an editor command directly as an output target, such as
`--output "nvim -R"`, is out of scope. Client CLIs should expose only the output
target, such as `--output editor`.

## Help Text

Client applications can compose libspagyrist's common help fragments into their
own help output. Options are also exposed as list APIs, so additions to
selectors, formats, or outputs can be reflected by updating the libspagyrist
lists.

```cpp
std::cout << "Client options:\n";
std::cout << "  --lang <code>  Client-specific language option.\n";
std::cout << "\n";
std::cout << spagyrist::cli_help_text();
```

Use these APIs when a client wants finer control over layout:

- `spagyrist::selector_help_text()`
- `spagyrist::format_help_text()`
- `spagyrist::output_help_text()`
- `spagyrist::selector_cli_help_option()`
- `spagyrist::format_cli_help_option()`
- `spagyrist::output_cli_help_option()`
- `spagyrist::version_cli_help_option()`
- `spagyrist::info_cli_help_option()`
- `spagyrist::selector_help_options()`
- `spagyrist::format_help_options()`
- `spagyrist::output_help_options()`

## Version And Runtime Info

Client applications can show libspagyrist's version and the features available
in the current runtime environment.

```cpp
std::cout << spagyrist::version_text() << '\n';
std::cout << spagyrist::runtime_info_text();
```

`runtime_info_text()` reports selector and editor status. Selector status covers
the built-in `builtin` and `number` selectors plus detection of the
external `fzf` command. Editor status covers `VISUAL`, `EDITOR`, `nvim`, `vim`,
`vi`, and `nano` in resolution order. Use these APIs for finer control:

- `spagyrist::version()`
- `spagyrist::version_text()`
- `spagyrist::selector_runtime_statuses()`
- `spagyrist::editor_runtime_statuses()`
- `spagyrist::format_runtime_statuses()`
- `spagyrist::runtime_info_text()`

## Selector

libspagyrist provides a built-in selector that does not depend on external
commands.

For standard clients, prefer the built-in selector as the default and fall back
to the number selector when it is not available, such as in a non-TTY
environment. `fzf` remains available as an explicitly selected external
selector.

```cpp
std::vector<spagyrist::candidate> candidates = /* application data */;

spagyrist::builtin_selector primary;
spagyrist::number_selector fallback;

auto selected = spagyrist::select_candidate_with_fallback(
    primary,
    fallback,
    candidates);
```

You can also explicitly use `fzf` and fall back to the number selector only when
`fzf` is unavailable or fails internally.

```cpp
spagyrist::fzf_selector primary;
spagyrist::number_selector fallback;

auto selected = spagyrist::select_candidate_with_fallback(
    primary,
    fallback,
    candidates);
```

Cancellation is not treated as a fallback condition. Fallback happens only when
a selector is unavailable or fails internally.

Use `select_candidate_result()` when detailed selector status matters.

- `selected`: selection completed
- `no_selection`: no candidates
- `cancelled`: Escape, Ctrl-C, EOF, or selector-side cancellation
- `unavailable`: required environment or external command is unavailable
- `error`: selector internal error, terminal setup failure, read/write failure, and similar failures
- `invalid_selection`: the selector returned an out-of-range candidate index

`select_candidate()` is the compatibility API and returns only the selected
value derived from the detailed result. For the `fzf` selector, a missing
executable is `unavailable`, while abnormal exits and invalid output are
`error`. Invalid or out-of-range input in the number selector asks for input
again instead of becoming an immediate final status.

Built-in selector keys:

- Text input: update the search query
- Backspace: delete a character
- Up / Down or Ctrl-P / Ctrl-N: move the current selection
- Enter: confirm selection
- Escape / Ctrl-C / EOF: cancel

Unicode handling is byte-based and preserves UTF-8 strings. The built-in
selector appends structurally complete UTF-8 byte sequences to the query as one
character input and does not treat incomplete UTF-8 input as text.
Grapheme-aware cursor movement, full-width character measurement, and Unicode
normalization are outside the initial implementation scope.

## Candidate Text

Selectors use display and search strings projected from `candidate`. Projection
does not modify the original `candidate`; it sanitizes only the text that will
be used for terminal display and search.

- C0 control characters, ESC, and DEL are replaced with spaces
- ANSI escape sequences from candidate data are not treated as terminal control
- Match positions for search text and display text are kept separate
- Matches in non-displayed fields, such as descriptions or URLs, do not create invalid highlights
- `candidate.preview` is optional and is used only by selectors that support previews

ANSI decoration is applied after safe display truncation. Library-generated
ANSI sequences are not truncated in the middle, and color output is reset by
the end of each rendered line.
The built-in selector maps matcher byte positions back to UTF-8 code point
units in the display string before highlighting. Multiple matched bytes inside
the same code point are highlighted once. Invalid UTF-8 candidate text does not
crash rendering; structurally unreadable byte sequences are treated as
single-byte units. Display width limits are byte-based and do not fully account
for terminal cell width of wide characters.

When `candidate.preview` is present, the fzf selector converts it into a form
that can be passed to fzf preview. Missing previews are ignored. The built-in
selector does not render previews.
The fzf selector limits matching to the candidate text shown in fzf and does
not include preview text in matching. The matching algorithm itself is still
fzf's own matcher.

## Matcher

The current matcher is a greedy subsequence matcher.

- Byte-based
- Not fzy-compatible
- Does not search for the optimal alignment when repeated characters exist
- Long match spans remain matched, but receive a score penalty
- Consecutive matches, prefix matches, word boundaries, and case boundaries are rewarded

This keeps abbreviation search useful for long paths and function names while
ranking closer and more consecutive matches higher.

`match_result::positions` contains byte offsets in the candidate string.
`ranked_candidate::search_positions` contains byte offsets in
`candidate_text::search`, while `ranked_candidate::display_positions` contains
byte offsets in `candidate_text::display`.

## License

libspagyrist is licensed under the MIT License. See `LICENSE` for details.

Third-party notices are listed in `THIRD_PARTY_NOTICES.md`.
