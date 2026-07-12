#include "spagyrist/document.hpp"

#include <utility>

namespace spagyrist {

inline_element inline_element::text_node(std::string value)
{
    inline_element node;
    node.type = kind::text;
    node.text = std::move(value);
    return node;
}

inline_element inline_element::strong(std::vector<inline_element> value)
{
    inline_element node;
    node.type = kind::strong;
    node.content = std::move(value);
    return node;
}

inline_element inline_element::emphasis(std::vector<inline_element> value)
{
    inline_element node;
    node.type = kind::emphasis;
    node.content = std::move(value);
    return node;
}

inline_element inline_element::code(std::string value)
{
    inline_element node;
    node.type = kind::inline_code;
    node.text = std::move(value);
    return node;
}

inline_element inline_element::link(std::string label, std::string target)
{
    inline_element node;
    node.type = kind::link;
    node.text = std::move(label);
    node.url = std::move(target);
    return node;
}

inline_element inline_element::line_break()
{
    inline_element node;
    node.type = kind::line_break;
    return node;
}

block block::heading(std::uint8_t level, std::vector<inline_element> content)
{
    block value;
    value.type = kind::heading;
    value.level = level;
    value.content = std::move(content);
    return value;
}

block block::paragraph(std::vector<inline_element> content)
{
    block value;
    value.type = kind::paragraph;
    value.content = std::move(content);
    return value;
}

block block::unordered_list(std::vector<list_item> items)
{
    block value;
    value.type = kind::unordered_list;
    value.items = std::move(items);
    return value;
}

block block::ordered_list(std::vector<list_item> items, std::size_t start)
{
    block value;
    value.type = kind::ordered_list;
    value.start = start;
    value.items = std::move(items);
    return value;
}

block block::quote(std::vector<block> blocks)
{
    block value;
    value.type = kind::blockquote;
    value.blocks = std::move(blocks);
    return value;
}

block block::code(std::string content, std::optional<std::string> language)
{
    block value;
    value.type = kind::code_block;
    value.code_content = std::move(content);
    value.language = std::move(language);
    return value;
}

block block::table(table_row header, std::vector<table_row> rows)
{
    block value;
    value.type = kind::table;
    value.header = std::move(header);
    value.rows = std::move(rows);
    return value;
}

block block::horizontal_rule()
{
    block value;
    value.type = kind::horizontal_rule;
    return value;
}

} // namespace spagyrist
