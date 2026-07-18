#include "selector/builtin/terminal.hpp"

#include "test_support.hpp"

#include <stdexcept>
#include <string>
#include <string_view>
#include <unistd.h>

namespace {

void terminal_reports_invalid_fd_as_not_tty()
{
    SPAGYRIST_CHECK(!spagyrist::detail::is_terminal(-1));
}

void terminal_size_returns_empty_for_invalid_fd()
{
    const auto size = spagyrist::detail::query_terminal_size(-1);

    SPAGYRIST_CHECK(!size.has_value());
}

void raw_terminal_mode_does_not_throw_for_invalid_fd()
{
    spagyrist::detail::raw_terminal_mode mode{-1};

    SPAGYRIST_CHECK(!mode.enabled());
    SPAGYRIST_CHECK(!mode.error().empty());
}

void terminal_screen_guard_reports_invalid_fd()
{
    spagyrist::detail::terminal_screen_guard screen{-1};

    SPAGYRIST_CHECK(!screen.enabled());
    SPAGYRIST_CHECK(!screen.error().empty());
}

void terminal_write_all_writes_complete_value()
{
    int fds[2]{};
    SPAGYRIST_CHECK(pipe(fds) == 0);

    spagyrist::detail::write_terminal_all(fds[1], "hello");
    close(fds[1]);

    char buffer[8]{};
    const auto read_size = read(fds[0], buffer, sizeof(buffer));
    close(fds[0]);

    const auto output = std::string{buffer, static_cast<std::size_t>(read_size)};
    SPAGYRIST_CHECK(read_size == 5);
    SPAGYRIST_CHECK(output == "hello");
}

void terminal_write_all_reports_invalid_fd()
{
    bool threw = false;
    try {
        spagyrist::detail::write_terminal_all(-1, "x");
    } catch (const std::runtime_error&) {
        threw = true;
    }

    SPAGYRIST_CHECK(threw);
    SPAGYRIST_CHECK(!spagyrist::detail::try_write_terminal_all(-1, "x"));
}

void terminal_input_parses_basic_keys()
{
    using spagyrist::detail::terminal_key;

    SPAGYRIST_CHECK(spagyrist::detail::parse_terminal_input("a").key == terminal_key::character);
    SPAGYRIST_CHECK(spagyrist::detail::parse_terminal_input("a").value == "a");
    SPAGYRIST_CHECK(spagyrist::detail::parse_terminal_input("\n").key == terminal_key::enter);
    SPAGYRIST_CHECK(spagyrist::detail::parse_terminal_input("\177").key == terminal_key::backspace);
    SPAGYRIST_CHECK(spagyrist::detail::parse_terminal_input("\003").key == terminal_key::ctrl_c);
    SPAGYRIST_CHECK(spagyrist::detail::parse_terminal_input("\016").key == terminal_key::ctrl_n);
    SPAGYRIST_CHECK(spagyrist::detail::parse_terminal_input("\020").key == terminal_key::ctrl_p);
    SPAGYRIST_CHECK(spagyrist::detail::parse_terminal_input("\033").key == terminal_key::escape);
}

void terminal_input_parses_arrow_keys()
{
    using spagyrist::detail::terminal_key;

    SPAGYRIST_CHECK(spagyrist::detail::parse_terminal_input("\033[A").key == terminal_key::arrow_up);
    SPAGYRIST_CHECK(spagyrist::detail::parse_terminal_input("\033[B").key == terminal_key::arrow_down);
    SPAGYRIST_CHECK(spagyrist::detail::parse_terminal_input("\033[C").key == terminal_key::unknown);
}

void terminal_input_reads_from_fd()
{
    using spagyrist::detail::terminal_key;

    int fds[2]{};
    SPAGYRIST_CHECK(pipe(fds) == 0);
    const char input = 'x';
    SPAGYRIST_CHECK(write(fds[1], &input, 1) == 1);
    close(fds[1]);

    const auto read = spagyrist::detail::read_terminal_input(fds[0]);
    close(fds[0]);

    SPAGYRIST_CHECK(read.key == terminal_key::character);
    SPAGYRIST_CHECK(read.value == "x");
}

void terminal_input_parses_utf8_characters()
{
    using spagyrist::detail::terminal_key;

    const auto two_byte = spagyrist::detail::parse_terminal_input("\xc3\xa9");
    const auto three_byte = spagyrist::detail::parse_terminal_input("\xe3\x81\x82");
    const auto four_byte = spagyrist::detail::parse_terminal_input("\xf0\x9f\x98\x80");

    SPAGYRIST_CHECK(two_byte.key == terminal_key::character);
    SPAGYRIST_CHECK(two_byte.value == "\xc3\xa9");
    SPAGYRIST_CHECK(three_byte.key == terminal_key::character);
    SPAGYRIST_CHECK(three_byte.value == "\xe3\x81\x82");
    SPAGYRIST_CHECK(four_byte.key == terminal_key::character);
    SPAGYRIST_CHECK(four_byte.value == "\xf0\x9f\x98\x80");
}

void terminal_input_rejects_invalid_utf8()
{
    using spagyrist::detail::terminal_key;

    SPAGYRIST_CHECK(spagyrist::detail::parse_terminal_input("\x80").key == terminal_key::unknown);
    SPAGYRIST_CHECK(spagyrist::detail::parse_terminal_input("\xe3\x81").key == terminal_key::unknown);
}

void terminal_input_reads_utf8_from_fd()
{
    using spagyrist::detail::terminal_key;

    int fds[2]{};
    SPAGYRIST_CHECK(pipe(fds) == 0);
    constexpr std::string_view input{"\xe3\x81\x82"};
    SPAGYRIST_CHECK(write(fds[1], input.data(), input.size()) == static_cast<ssize_t>(input.size()));
    close(fds[1]);

    const auto read = spagyrist::detail::read_terminal_input(fds[0]);
    close(fds[0]);

    SPAGYRIST_CHECK(read.key == terminal_key::character);
    SPAGYRIST_CHECK(read.value == input);
}

void terminal_input_handles_incomplete_utf8_from_fd()
{
    using spagyrist::detail::terminal_key;

    int fds[2]{};
    SPAGYRIST_CHECK(pipe(fds) == 0);
    const char input = '\xe3';
    SPAGYRIST_CHECK(write(fds[1], &input, 1) == 1);
    close(fds[1]);

    const auto read = spagyrist::detail::read_terminal_input(fds[0]);
    close(fds[0]);

    SPAGYRIST_CHECK(read.key == terminal_key::unknown);
}

void terminal_input_reads_single_escape_as_escape()
{
    using spagyrist::detail::terminal_key;

    int fds[2]{};
    SPAGYRIST_CHECK(pipe(fds) == 0);
    const char input = '\033';
    SPAGYRIST_CHECK(write(fds[1], &input, 1) == 1);
    close(fds[1]);

    const auto read = spagyrist::detail::read_terminal_input(fds[0]);
    close(fds[0]);

    SPAGYRIST_CHECK(read.key == terminal_key::escape);
}

} // namespace

void run_terminal_tests()
{
    terminal_reports_invalid_fd_as_not_tty();
    terminal_size_returns_empty_for_invalid_fd();
    raw_terminal_mode_does_not_throw_for_invalid_fd();
    terminal_screen_guard_reports_invalid_fd();
    terminal_write_all_writes_complete_value();
    terminal_write_all_reports_invalid_fd();
    terminal_input_parses_basic_keys();
    terminal_input_parses_arrow_keys();
    terminal_input_reads_from_fd();
    terminal_input_parses_utf8_characters();
    terminal_input_rejects_invalid_utf8();
    terminal_input_reads_utf8_from_fd();
    terminal_input_handles_incomplete_utf8_from_fd();
    terminal_input_reads_single_escape_as_escape();
}
