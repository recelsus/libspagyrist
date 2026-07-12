#include "spagyrist/validation.hpp"

#include <string>

namespace spagyrist {
namespace {

void add_error(validation_result& result, error_code code, std::string path, std::string message)
{
    result.errors.push_back(validation_error{
        .code = code,
        .path = std::move(path),
        .message = std::move(message),
    });
}

void validate_inline(
    const inline_element& value,
    const std::string& path,
    validation_result& result)
{
    switch (value.type) {
    case inline_element::kind::text:
        break;
    case inline_element::kind::strong:
    case inline_element::kind::emphasis:
        for (std::size_t i = 0; i < value.content.size(); ++i) {
            validate_inline(value.content[i], path + ".content[" + std::to_string(i) + "]", result);
        }
        break;
    case inline_element::kind::inline_code:
        break;
    case inline_element::kind::link:
        if (value.text.empty()) {
            add_error(result, error_code::invalid_document, path + ".text", "link text must not be empty");
        }
        break;
    case inline_element::kind::line_break:
        break;
    }
}

void validate_inline_list(
    const std::vector<inline_element>& values,
    const std::string& path,
    validation_result& result)
{
    for (std::size_t i = 0; i < values.size(); ++i) {
        validate_inline(values[i], path + "[" + std::to_string(i) + "]", result);
    }
}

void validate_blocks(
    const std::vector<block>& blocks,
    const std::string& path,
    validation_result& result);

void validate_table_row(
    const block::table_row& row,
    const std::string& path,
    validation_result& result)
{
    for (std::size_t i = 0; i < row.size(); ++i) {
        validate_inline_list(row[i], path + "[" + std::to_string(i) + "]", result);
    }
}

void validate_block(
    const block& value,
    const std::string& path,
    validation_result& result)
{
    switch (value.type) {
    case block::kind::heading:
        if (value.level < 1 || value.level > 6) {
            add_error(result, error_code::invalid_document, path + ".level", "heading level must be between 1 and 6");
        }
        validate_inline_list(value.content, path + ".content", result);
        break;
    case block::kind::paragraph:
        validate_inline_list(value.content, path + ".content", result);
        break;
    case block::kind::unordered_list:
    case block::kind::ordered_list:
        for (std::size_t i = 0; i < value.items.size(); ++i) {
            validate_blocks(value.items[i].blocks, path + ".items[" + std::to_string(i) + "].blocks", result);
        }
        break;
    case block::kind::blockquote:
        validate_blocks(value.blocks, path + ".blocks", result);
        break;
    case block::kind::code_block:
        break;
    case block::kind::table: {
        if (value.caption) {
            validate_inline_list(*value.caption, path + ".caption", result);
        }
        validate_table_row(value.header, path + ".header", result);
        const auto expected = value.header.size();
        for (std::size_t i = 0; i < value.rows.size(); ++i) {
            if (expected != 0 && value.rows[i].size() != expected) {
                add_error(result, error_code::invalid_document, path + ".rows[" + std::to_string(i) + "]", "table row cell count must match header cell count");
            }
            validate_table_row(value.rows[i], path + ".rows[" + std::to_string(i) + "]", result);
        }
        break;
    }
    case block::kind::horizontal_rule:
        break;
    }
}

void validate_blocks(
    const std::vector<block>& blocks,
    const std::string& path,
    validation_result& result)
{
    for (std::size_t i = 0; i < blocks.size(); ++i) {
        validate_block(blocks[i], path + "[" + std::to_string(i) + "]", result);
    }
}

} // namespace

validation_result validate(const candidate& value)
{
    validation_result result;
    if (value.id.empty()) {
        add_error(result, error_code::invalid_candidate, "candidate.id", "candidate id must not be empty");
    }
    if (value.title.empty()) {
        add_error(result, error_code::invalid_candidate, "candidate.title", "candidate title must not be empty");
    }
    return result;
}

validation_result validate(const candidate_list& value)
{
    validation_result result;
    for (std::size_t i = 0; i < value.candidates.size(); ++i) {
        auto candidate_result = validate(value.candidates[i]);
        for (auto& error : candidate_result.errors) {
            const auto local_path = error.path.starts_with("candidate.")
                ? error.path.substr(std::string{"candidate."}.size())
                : error.path;
            error.path = "candidates[" + std::to_string(i) + "]." + local_path;
            result.errors.push_back(std::move(error));
        }
    }
    return result;
}

validation_result validate(const document& value)
{
    validation_result result;
    if (value.metadata.title.empty()) {
        add_error(result, error_code::invalid_document, "document.metadata.title", "document title must not be empty");
    }
    validate_blocks(value.blocks, "document.blocks", result);
    return result;
}

} // namespace spagyrist
