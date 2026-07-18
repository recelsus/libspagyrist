#include "selector/terminal.hpp"

#include "test_support.hpp"

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

void terminal_input_parses_basic_keys()
{
    using spagyrist::detail::terminal_key;

    SPAGYRIST_CHECK(spagyrist::detail::parse_terminal_input("a").key == terminal_key::character);
    SPAGYRIST_CHECK(spagyrist::detail::parse_terminal_input("a").value == 'a');
    SPAGYRIST_CHECK(spagyrist::detail::parse_terminal_input("\n").key == terminal_key::enter);
    SPAGYRIST_CHECK(spagyrist::detail::parse_terminal_input("\177").key == terminal_key::backspace);
    SPAGYRIST_CHECK(spagyrist::detail::parse_terminal_input("\003").key == terminal_key::ctrl_c);
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
    SPAGYRIST_CHECK(read.value == 'x');
}

} // namespace

void run_terminal_tests()
{
    terminal_reports_invalid_fd_as_not_tty();
    terminal_size_returns_empty_for_invalid_fd();
    raw_terminal_mode_does_not_throw_for_invalid_fd();
    terminal_input_parses_basic_keys();
    terminal_input_parses_arrow_keys();
    terminal_input_reads_from_fd();
}
