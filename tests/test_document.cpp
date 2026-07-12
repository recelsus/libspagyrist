#include "spagyrist/spagyrist.hpp"

#include "test_support.hpp"

namespace {

void document_with_wikipedia_like_shape_passes_validation()
{
    auto doc = spagyrist::document{};
    doc.metadata.title = "Linux";
    doc.metadata.source = "Wikipedia";
    doc.metadata.url = "https://en.wikipedia.org/wiki/Linux";
    doc.metadata.language = "en";
    doc.metadata.description = "Family of Unix-like operating systems";
    doc.metadata.extra.emplace("source_page_id", "6097297");
    doc.metadata.extra.emplace("wikibase_item", "Q388");

    doc.blocks.push_back(spagyrist::block::heading(
        1,
        {spagyrist::inline_element::text_node("Linux")}));
    doc.blocks.push_back(spagyrist::block::paragraph({
        spagyrist::inline_element::text_node("Linux is a family of "),
        spagyrist::inline_element::link("Unix-like", "https://en.wikipedia.org/wiki/Unix-like"),
        spagyrist::inline_element::text_node(" operating systems."),
    }));

    spagyrist::block::list_item item;
    item.blocks.push_back(spagyrist::block::paragraph({
        spagyrist::inline_element::strong({spagyrist::inline_element::text_node("Portable")}),
        spagyrist::inline_element::text_node(" across many hardware platforms."),
    }));
    doc.blocks.push_back(spagyrist::block::unordered_list({item}));

    doc.blocks.push_back(spagyrist::block::code("uname -a\n", "sh"));

    SPAGYRIST_CHECK(spagyrist::validate(doc).ok());
}

void invalid_document_reports_heading_level()
{
    auto doc = spagyrist::document{};
    doc.metadata.title = "Invalid";
    doc.blocks.push_back(spagyrist::block::heading(
        9,
        {spagyrist::inline_element::text_node("Too deep")}));

    const auto result = spagyrist::validate(doc);

    SPAGYRIST_CHECK(!result.ok());
    SPAGYRIST_CHECK(result.errors.size() == 1);
    SPAGYRIST_CHECK(result.errors[0].path == "document.blocks[0].level");
}

void invalid_document_reports_table_width_mismatch()
{
    auto doc = spagyrist::document{};
    doc.metadata.title = "Table";

    spagyrist::block::table_row header{
        {spagyrist::inline_element::text_node("Version")},
        {spagyrist::inline_element::text_node("Date")},
    };
    std::vector<spagyrist::block::table_row> rows{
        {
            {spagyrist::inline_element::text_node("1.0")},
        },
    };

    doc.blocks.push_back(spagyrist::block::table(header, rows));

    const auto result = spagyrist::validate(doc);

    SPAGYRIST_CHECK(!result.ok());
    SPAGYRIST_CHECK(result.errors.size() == 1);
    SPAGYRIST_CHECK(result.errors[0].path == "document.blocks[0].rows[0]");
}

} // namespace

void run_document_tests()
{
    document_with_wikipedia_like_shape_passes_validation();
    invalid_document_reports_heading_level();
    invalid_document_reports_table_width_mismatch();
}

