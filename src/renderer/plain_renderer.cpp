#include "spagyrist/renderer.hpp"

#include <string>

namespace spagyrist {
namespace {

void append_inline_plain(std::string& output, const inline_element& value)
{
    switch (value.type) {
    case inline_element::kind::text:
    case inline_element::kind::inline_code:
    case inline_element::kind::link:
        output += value.text;
        break;
    case inline_element::kind::strong:
    case inline_element::kind::emphasis:
        for (const auto& child : value.content) {
            append_inline_plain(output, child);
        }
        break;
    case inline_element::kind::line_break:
        output += '\n';
        break;
    }
}

std::string inline_plain(const std::vector<inline_element>& values)
{
    std::string output;
    for (const auto& value : values) {
        append_inline_plain(output, value);
    }
    return output;
}

void append_blocks_plain(std::string& output, const std::vector<block>& blocks, std::size_t indent);

void append_block_plain(std::string& output, const block& value, std::size_t indent)
{
    const std::string padding(indent, ' ');

    switch (value.type) {
    case block::kind::heading:
        output += padding + inline_plain(value.content) + "\n\n";
        break;
    case block::kind::paragraph:
        output += padding + inline_plain(value.content) + "\n\n";
        break;
    case block::kind::unordered_list:
        for (const auto& item : value.items) {
            output += padding + "- ";
            if (!item.blocks.empty() && item.blocks.front().type == block::kind::paragraph) {
                output += inline_plain(item.blocks.front().content) + '\n';
                if (item.blocks.size() > 1) {
                    std::vector<block> rest(item.blocks.begin() + 1, item.blocks.end());
                    append_blocks_plain(output, rest, indent + 2);
                }
            } else {
                output += '\n';
                append_blocks_plain(output, item.blocks, indent + 2);
            }
        }
        output += '\n';
        break;
    case block::kind::ordered_list: {
        auto number = value.start.value_or(1);
        for (const auto& item : value.items) {
            output += padding + std::to_string(number++) + ". ";
            if (!item.blocks.empty() && item.blocks.front().type == block::kind::paragraph) {
                output += inline_plain(item.blocks.front().content) + '\n';
            } else {
                output += '\n';
                append_blocks_plain(output, item.blocks, indent + 3);
            }
        }
        output += '\n';
        break;
    }
    case block::kind::blockquote:
        append_blocks_plain(output, value.blocks, indent + 2);
        break;
    case block::kind::code_block:
        output += padding + value.code_content;
        if (!output.empty() && output.back() != '\n') {
            output += '\n';
        }
        output += '\n';
        break;
    case block::kind::table:
        if (value.caption) {
            output += padding + inline_plain(*value.caption) + "\n";
        }
        for (std::size_t i = 0; i < value.header.size(); ++i) {
            if (i != 0) {
                output += '\t';
            }
            output += inline_plain(value.header[i]);
        }
        output += '\n';
        for (const auto& row : value.rows) {
            for (std::size_t i = 0; i < row.size(); ++i) {
                if (i != 0) {
                    output += '\t';
                }
                output += inline_plain(row[i]);
            }
            output += '\n';
        }
        output += '\n';
        break;
    case block::kind::horizontal_rule:
        output += "\n";
        break;
    }
}

void append_blocks_plain(std::string& output, const std::vector<block>& blocks, std::size_t indent)
{
    for (const auto& value : blocks) {
        append_block_plain(output, value, indent);
    }
}

} // namespace

std::string render_plain(const document& value)
{
    std::string output;
    append_blocks_plain(output, value.blocks, 0);
    return output;
}

} // namespace spagyrist

