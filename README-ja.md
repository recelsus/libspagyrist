# libspagyrist

libspagyrist は、検索候補、選択、文書モデル、レンダリング、出力を扱うC++ ライブラリ。 \
利用側アプリケーションが取得したデータをlibspagyristの共通構造へ変換し、libspagyristが選択、レンダリング、出力します。

## Features

- 検索候補の共通構造: `spagyrist::candidate`
- 選択処理: `fzf` selector、number selector、fallback selector
- 文書の共通構造: `spagyrist::document`
- inline / block ベースの文書モデル
- Markdown / plain text / terminal 向けレンダリング
- stdout 出力
- CMake package install

## Responsibility

libspagyrist が扱うもの:

- candidate
- document
- selector
- renderer
- output
- validation

libspagyrist が扱わないもの:

- HTTP request
- JSON / HTML parser
- データ元などの固有処理
- 取得元ごとの検索 API
- 取得元ごとの記事、ページ、データ変換

## Build

必要なもの:

- C++20 対応 compiler
- CMake 3.20 以上

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

install 後は CMake の `find_package(spagyrist REQUIRED)` から利用できます。

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

利用側アプリケーションでは、取得元の検索結果を `spagyrist::candidate` に変換し、
選択されたデータを `spagyrist::document` に変換します。

## Selector

`fzf` selector を優先し、利用できない場合に number selector へ fallback できます。

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
