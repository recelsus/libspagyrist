#include "spagyrist/spagyrist.hpp"

#include "test_support.hpp"

#include <string>

namespace {

spagyrist::document sample_document()
{
    spagyrist::document doc;
    doc.metadata.title = "Linux";
    doc.blocks.push_back(spagyrist::block::heading(
        1,
        {spagyrist::inline_element::text_node("Linux")}));
    doc.blocks.push_back(spagyrist::block::paragraph({
        spagyrist::inline_element::text_node("A "),
        spagyrist::inline_element::strong({spagyrist::inline_element::text_node("portable")}),
        spagyrist::inline_element::text_node(" operating system family."),
    }));
    doc.blocks.push_back(spagyrist::block::code("uname -a\n", "sh"));
    return doc;
}

void markdown_renderer_keeps_markup()
{
    const auto rendered = spagyrist::render(sample_document(), spagyrist::format::markdown);

    SPAGYRIST_CHECK(rendered.find("# Linux") != std::string::npos);
    SPAGYRIST_CHECK(rendered.find("**portable**") != std::string::npos);
    SPAGYRIST_CHECK(rendered.find("```sh") != std::string::npos);
}

void plain_renderer_removes_markup()
{
    const auto rendered = spagyrist::render(sample_document(), spagyrist::format::plain);

    SPAGYRIST_CHECK(rendered.find("Linux") != std::string::npos);
    SPAGYRIST_CHECK(rendered.find("portable") != std::string::npos);
    SPAGYRIST_CHECK(rendered.find("**portable**") == std::string::npos);
    SPAGYRIST_CHECK(rendered.find("```") == std::string::npos);
}

void terminal_renderer_is_available_as_separate_format()
{
    const auto rendered = spagyrist::render(sample_document(), spagyrist::format::terminal);

    SPAGYRIST_CHECK(rendered.find("Linux") != std::string::npos);
    SPAGYRIST_CHECK(rendered.find("uname -a") != std::string::npos);
}

} // namespace

void run_renderer_tests()
{
    markdown_renderer_keeps_markup();
    plain_renderer_removes_markup();
    terminal_renderer_is_available_as_separate_format();
}

