#pragma once

#include "spagyrist/candidate.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace spagyrist {

struct document_metadata {
    std::string title;
    std::optional<std::string> source;
    std::optional<std::string> url;
    std::optional<std::string> language;
    std::optional<std::string> description;
    std::vector<std::string> authors;
    std::optional<std::string> created_at;
    std::optional<std::string> updated_at;
    metadata_map extra;
};

struct inline_element {
    enum class kind {
        text,
        strong,
        emphasis,
        inline_code,
        link,
        line_break,
    };

    kind type{kind::text};
    std::string text;
    std::optional<std::string> url;
    std::vector<inline_element> content;

    [[nodiscard]] static inline_element text_node(std::string value);
    [[nodiscard]] static inline_element strong(std::vector<inline_element> value);
    [[nodiscard]] static inline_element emphasis(std::vector<inline_element> value);
    [[nodiscard]] static inline_element code(std::string value);
    [[nodiscard]] static inline_element link(std::string label, std::string target);
    [[nodiscard]] static inline_element line_break();
};

struct block {
    enum class kind {
        heading,
        paragraph,
        unordered_list,
        ordered_list,
        blockquote,
        code_block,
        table,
        horizontal_rule,
    };

    struct list_item {
        std::vector<block> blocks;
    };

    using table_cell = std::vector<inline_element>;
    using table_row = std::vector<table_cell>;

    kind type{kind::paragraph};

    std::uint8_t level{1};
    std::vector<inline_element> content;

    std::vector<list_item> items;
    std::optional<std::size_t> start;

    std::vector<block> blocks;

    std::optional<std::string> language;
    std::string code_content;

    std::optional<std::vector<inline_element>> caption;
    table_row header;
    std::vector<table_row> rows;

    [[nodiscard]] static block heading(std::uint8_t level, std::vector<inline_element> content);
    [[nodiscard]] static block paragraph(std::vector<inline_element> content);
    [[nodiscard]] static block unordered_list(std::vector<list_item> items);
    [[nodiscard]] static block ordered_list(std::vector<list_item> items, std::size_t start = 1);
    [[nodiscard]] static block quote(std::vector<block> blocks);
    [[nodiscard]] static block code(std::string content, std::optional<std::string> language = std::nullopt);
    [[nodiscard]] static block table(table_row header, std::vector<table_row> rows);
    [[nodiscard]] static block horizontal_rule();
};

struct document {
    document_metadata metadata;
    std::vector<block> blocks;
};

} // namespace spagyrist
