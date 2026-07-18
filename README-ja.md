# libspagyrist

libspagyrist は、検索候補、選択、文書モデル、レンダリング、出力を扱うC++ ライブラリ。 \
利用側アプリケーションが取得したデータをlibspagyristの共通構造へ変換し、libspagyristが選択、レンダリング、出力します。

## Features

- 検索候補の共通構造: `spagyrist::candidate`
- 選択処理: built-in selector、auto selector、`fzf` selector、number selector、fallback selector
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

トップレベルでビルドする場合、簡易動作確認用の固定 client も作成します。
サブプロジェクトとして利用する場合はデフォルトで無効。

```sh
./build/examples/spagyrist_fixed_client --list
./build/examples/spagyrist_fixed_client --select first --format plain
./build/examples/spagyrist_fixed_client --select fzf --format terminal
printf '2\n' | ./build/examples/spagyrist_fixed_client --select number --format markdown
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

標準の選択方式として、外部コマンドに依存しない built-in selector を提供します。

`auto_selector`はbuilt-in selectorを優先し、非TTY環境などで利用できない場合は
number selectorにfallbackします。`fzf` などは明示的に選択する外部selectorとして
利用できます。

```cpp
std::vector<spagyrist::candidate> candidates = /* application data */;

spagyrist::auto_selector selector;
auto selected = spagyrist::select_candidate(selector, candidates);
```

`fzf`を明示的に使い、失敗時だけnumber selectorへfallbackすることもできます。

```cpp
spagyrist::fzf_selector primary;
spagyrist::number_selector fallback;

auto selected = spagyrist::select_candidate_with_fallback(
    primary,
    fallback,
    candidates);
```

キャンセルは fallback の理由として扱いません。利用不可能またはselector内部エラーの
場合にのみfallbackします。

詳細状態が必要な場合は `select_candidate_result()` を利用します。

- `selected`: 正常選択
- `no_selection`: 候補なし
- `cancelled`: Escape、Ctrl-C、EOF、またはselector側のキャンセル
- `unavailable`: 実行環境や外部コマンドが利用不可
- `error`: selector内部エラー、端末初期化失敗、読み込み/書き込み失敗など
- `invalid_selection`: selectorが候補範囲外のindexを返した場合

`select_candidate()` は互換用APIで、詳細状態APIの結果から選択結果のみを返します。
`fzf` selectorでは、実行ファイルが見つからない場合は `unavailable`、異常終了や
不正出力は `error` として扱います。number selectorの不正入力や範囲外入力は
即時の最終状態にせず、再入力を求めます。

内蔵 selector の基本操作:

- 文字入力: 検索 query を更新
- Backspace: 文字削除
- Up / Down または Ctrl-P / Ctrl-N: 候補移動
- Enter: 選択確定
- Escape / Ctrl-C / EOF: キャンセル

Unicode は UTF-8 文字列を破壊しない byte-based 処理です。書記素単位のカーソル移動、
全角幅計算、Unicode 正規化は初期実装では対象外です。

## Candidate Text

selectorで利用する表示用文字列と検索用文字列は、`candidate` から投影して作成します。
この投影処理では、元の `candidate` は変更せず、端末表示へ渡す文字列だけを安全化します。

- C0制御文字、ESC、DELは空白へ置換
- 候補由来のANSIエスケープシーケンスは端末制御として扱わない
- 検索用文字列上の一致位置と表示用文字列上の一致位置を分離
- 説明やURLなど表示されていないフィールドだけに一致した場合、不正な強調表示を行わない
- `candidate.preview` は任意フィールドで、preview対応selectorだけが利用する

ANSI装飾は、表示文字列を安全に切り詰めた後に付与します。ライブラリが生成したANSI
シーケンスを途中で切断せず、カラー表示時は行末までにリセットします。

`candidate.preview` が存在する場合、fzf selectorはfzfのpreviewへ渡せる形式に変換します。
previewが存在しない場合は無視されます。built-in selectorではpreview表示を行いません。
fzf selectorでは、検索対象をfzfに表示している候補文字列へ限定し、preview本文は検索対象に含めません。
ただし、実際のmatchingアルゴリズムはfzf側のものです。

## Matcher

現在のmatcherは、greedyなsubsequence matcherです。

- byte-based
- fzy互換ではない
- 同じ文字が複数存在する場合、最適一致位置を探索しない
- 長い一致範囲は不一致にせず、スコア減点として扱う
- 連続一致、前方一致、単語境界、case boundaryを加点する

このため、長いパスや関数名の略称検索は候補として残ります。ただし、より近い一致や
連続した一致の方が高くrankされます。

## License

libspagyrist is licensed under the MIT License. See `LICENSE` for details.

Third-party notices are listed in `THIRD_PARTY_NOTICES.md`.
