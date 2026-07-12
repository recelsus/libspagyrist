#include "spagyrist/renderer.hpp"

#include <algorithm>
#include <string>

namespace spagyrist {
namespace {

void append_inline_markdown(std::string& output, const inline_element& value)
{
    switch (value.type) {
    case inline_element::kind::text:
        output += value.text;
        break;
    case inline_element::kind::strong:
        output += "**";
        for (const auto& child : value.content) {
            append_inline_markdown(output, child);
        }
        output += "**";
        break;
    case inline_element::kind::emphasis:
        output += '*';
        for (const auto& child : value.content) {
            append_inline_markdown(output, child);
        }
        output += '*';
        break;
    case inline_element::kind::inline_code:
        output += '`' + value.text + '`';
        break;
    case inline_element::kind::link:
        if (value.url) {
            output += '[' + value.text + "](" + *value.url + ')';
        } else {
            output += value.text;
        }
        break;
    case inline_element::kind::line_break:
        output += "  \n";
        break;
    }
}

std::string inline_markdown(const std::vector<inline_element>& values)
{
    std::string output;
    for (const auto& value : values) {
        append_inline_markdown(output, value);
    }
    return output;
}

void append_blocks_markdown(std::string& output, const std::vector<block>& blocks, std::size_t indent);

void append_block_markdown(std::string& output, const block& value, std::size_t indent)
{
    const std::string padding(indent, ' ');

    switch (value.type) {
    case block::kind::heading:
        output += std::string(std::clamp<int>(value.level, 1, 6), '#') + ' ' + inline_markdown(value.content) + "\n\n";
        break;
    case block::kind::paragraph:
        output += padding + inline_markdown(value.content) + "\n\n";
        break;
    case block::kind::unordered_list:
        for (const auto& item : value.items) {
            output += padding + "- ";
            if (!item.blocks.empty() && item.blocks.front().type == block::kind::paragraph) {
                output += inline_markdown(item.blocks.front().content) + '\n';
                if (item.blocks.size() > 1) {
                    std::vector<block> rest(item.blocks.begin() + 1, item.blocks.end());
                    append_blocks_markdown(output, rest, indent + 2);
                }
            } else {
                output += '\n';
                append_blocks_markdown(output, item.blocks, indent + 2);
            }
        }
        output += '\n';
        break;
    case block::kind::ordered_list: {
        auto number = value.start.value_or(1);
        for (const auto& item : value.items) {
            output += padding + std::to_string(number++) + ". ";
            if (!item.blocks.empty() && item.blocks.front().type == block::kind::paragraph) {
                output += inline_markdown(item.blocks.front().content) + '\n';
            } else {
                output += '\n';
                append_blocks_markdown(output, item.blocks, indent + 3);
            }
        }
        output += '\n';
        break;
    }
    case block::kind::blockquote:
        for (const auto& nested : value.blocks) {
            auto nested_output = std::string{};
            append_block_markdown(nested_output, nested, 0);
            output += "> ";
            for (const auto ch : nested_output) {
                output += ch;
                if (ch == '\n' && !nested_output.empty()) {
                    output += "> ";
                }
            }
        }
        output += '\n';
        break;
    case block::kind::code_block:
        output += "```";
        if (value.language) {
            output += *value.language;
        }
        output += '\n' + value.code_content;
        if (!value.code_content.empty() && value.code_content.back() != '\n') {
            output += '\n';
        }
        output += "```\n\n";
        break;
    case block::kind::table:
        if (value.caption) {
            output += inline_markdown(*value.caption) + "\n\n";
        }
        output += '|';
        for (const auto& cell : value.header) {
            output += ' ' + inline_markdown(cell) + " |";
        }
        output += "\n|";
        for (std::size_t i = 0; i < value.header.size(); ++i) {
            output += " --- |";
        }
        output += '\n';
        for (const auto& row : value.rows) {
            output += '|';
            for (const auto& cell : row) {
                output += ' ' + inline_markdown(cell) + " |";
            }
            output += '\n';
        }
        output += '\n';
        break;
    case block::kind::horizontal_rule:
        output += "---\n\n";
        break;
    }
}

void append_blocks_markdown(std::string& output, const std::vector<block>& blocks, std::size_t indent)
{
    for (const auto& value : blocks) {
        append_block_markdown(output, value, indent);
    }
}

} // namespace

std::string render_markdown(const document& value)
{
    std::string output;
    append_blocks_markdown(output, value.blocks, 0);
    return output;
}

} // namespace spagyrist

