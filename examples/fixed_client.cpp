#include "spagyrist/spagyrist.hpp"

#include <iostream>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

class first_selector final : public spagyrist::selector {
public:
    std::optional<std::size_t>
    select(std::span<const spagyrist::candidate> candidates) override
    {
        if (candidates.empty()) {
            return std::nullopt;
        }
        return 0;
    }
};

struct fixture_item {
    spagyrist::candidate candidate;
    spagyrist::document document;
};

spagyrist::candidate make_candidate(
    std::string id,
    std::string title,
    std::string subtitle,
    std::string description,
    std::string url)
{
    spagyrist::candidate value;
    value.id = std::move(id);
    value.title = std::move(title);
    value.subtitle = std::move(subtitle);
    value.description = std::move(description);
    value.url = std::move(url);
    return value;
}

spagyrist::document make_document(
    const spagyrist::candidate& candidate,
    std::string language,
    std::vector<std::string> paragraphs)
{
    spagyrist::document document;
    document.metadata.title = candidate.title;
    document.metadata.source = "fixed-client";
    document.metadata.url = candidate.url;
    document.metadata.language = std::move(language);
    document.metadata.description = candidate.description;

    document.blocks.push_back(spagyrist::block::heading(
        1,
        {spagyrist::inline_element::text_node(candidate.title)}));

    for (auto& paragraph : paragraphs) {
        document.blocks.push_back(spagyrist::block::paragraph({
            spagyrist::inline_element::text_node(std::move(paragraph)),
        }));
    }

    return document;
}

std::vector<fixture_item> fixtures()
{
    std::vector<fixture_item> output;

    auto linux_item = make_candidate(
        "linux",
        "Linux",
        "Operating system kernel",
        "A Unix-like operating system family used across servers and devices.",
        "https://example.test/linux");
    linux_item.preview =
        "Linux\n"
        "\n"
        "Operating system kernel\n"
        "A Unix-like operating system family used across servers and devices.\n"
        "\n"
        "Preview text is supplied by candidate.preview and is used only by preview-capable selectors.";
    output.push_back(fixture_item{
        .candidate = linux_item,
        .document = make_document(
            linux_item,
            "en",
            {
                "Linux is a fixed fixture item for libspagyrist selector testing.",
                "This entry is useful for checking ranking, selection, and rendering.",
            }),
    });

    auto freebsd = make_candidate(
        "freebsd",
        "FreeBSD",
        "Unix-like operating system",
        "A free and open source operating system descended from BSD.",
        "https://example.test/freebsd");
    freebsd.preview =
        "FreeBSD\n"
        "\n"
        "Unix-like operating system\n"
        "This preview demonstrates that fzf can show selector-specific candidate details.";
    output.push_back(fixture_item{
        .candidate = freebsd,
        .document = make_document(
            freebsd,
            "en",
            {
                "FreeBSD is included to test non-Linux search candidates.",
                "It provides another Unix-like entry with a different title shape.",
            }),
    });

    auto darwin = make_candidate(
        "darwin",
        "Darwin",
        "Apple operating system core",
        "The open source Unix-like core used by Apple's operating systems.",
        "https://example.test/darwin");
    darwin.preview =
        "Darwin\n"
        "\n"
        "Apple operating system core\n"
        "Search for Apple in fzf to see this candidate ranked highly while the preview remains available.";
    output.push_back(fixture_item{
        .candidate = darwin,
        .document = make_document(
            darwin,
            "en",
            {
                "Darwin is a fixture for checking descriptions and subtitles.",
                "It is intentionally short and deterministic.",
            }),
    });

    auto plan9 = make_candidate(
        "plan9",
        "Plan 9",
        "Distributed operating system",
        "A research operating system from Bell Labs.",
        "https://example.test/plan9");
    plan9.preview =
        "Plan 9\n"
        "\n"
        "Distributed operating system\n"
        "This shorter entry is useful for checking preview rendering with compact candidate text.";
    output.push_back(fixture_item{
        .candidate = plan9,
        .document = make_document(
            plan9,
            "en",
            {
                "Plan 9 gives the fixed client a shorter title with a digit.",
                "It is useful for basic fuzzy matching checks.",
            }),
    });

    return output;
}

std::vector<spagyrist::candidate> candidates_from(const std::vector<fixture_item>& items)
{
    std::vector<spagyrist::candidate> output;
    output.reserve(items.size());
    for (const auto& item : items) {
        output.push_back(item.candidate);
    }
    return output;
}

std::optional<spagyrist::format> parse_format(std::string_view value)
{
    if (value == "terminal") {
        return spagyrist::format::terminal;
    }
    if (value == "markdown") {
        return spagyrist::format::markdown;
    }
    if (value == "plain") {
        return spagyrist::format::plain;
    }
    return std::nullopt;
}

void print_usage(std::ostream& output)
{
    output
        << "Usage: spagyrist_fixed_client [options]\n"
        << "\n"
        << "Options:\n"
        << "  --select <auto|builtin|fzf|number|first>  Selector. Default: auto\n"
        << "  --format <terminal|markdown|plain>        Output format. Default: terminal\n"
        << "  --list                                    Print fixed candidates and exit\n"
        << "  -h, --help                                Show this help\n";
}

} // namespace

int main(int argc, char** argv)
{
    std::string selector_name{"auto"};
    auto output_format = spagyrist::format::terminal;
    bool list_only = false;

    for (int i = 1; i < argc; ++i) {
        const std::string_view arg{argv[i]};
        if (arg == "-h" || arg == "--help") {
            print_usage(std::cout);
            return 0;
        }
        if (arg == "--list") {
            list_only = true;
            continue;
        }
        if (arg == "--select" || arg == "--format") {
            if (i + 1 >= argc) {
                std::cerr << "missing value for " << arg << '\n';
                return 2;
            }
            const std::string_view value{argv[++i]};
            if (arg == "--select") {
                selector_name = value;
            } else {
                const auto parsed = parse_format(value);
                if (!parsed) {
                    std::cerr << "invalid format: " << value << '\n';
                    return 2;
                }
                output_format = *parsed;
            }
            continue;
        }
        std::cerr << "unknown option: " << arg << '\n';
        return 2;
    }

    const auto items = fixtures();
    auto candidates = candidates_from(items);

    if (list_only) {
        for (const auto& candidate : candidates) {
            std::cout << candidate.id << '\t' << candidate.title << '\n';
        }
        return 0;
    }

    std::optional<spagyrist::selection> selected;
    if (selector_name == "auto") {
        spagyrist::auto_selector selector;
        selected = spagyrist::select_candidate(selector, candidates);
    } else if (selector_name == "builtin") {
        spagyrist::builtin_selector selector;
        selected = spagyrist::select_candidate(selector, candidates);
    } else if (selector_name == "fzf") {
        spagyrist::fzf_selector selector;
        selected = spagyrist::select_candidate(selector, candidates);
    } else if (selector_name == "number") {
        spagyrist::number_selector selector;
        selected = spagyrist::select_candidate(selector, candidates);
    } else if (selector_name == "first") {
        first_selector selector;
        selected = spagyrist::select_candidate(selector, candidates);
    } else {
        std::cerr << "invalid selector: " << selector_name << '\n';
        return 2;
    }

    if (!selected) {
        std::cerr << "selection cancelled or unavailable\n";
        return 1;
    }

    std::cout << spagyrist::render(items[selected->index].document, output_format);
    return 0;
}
