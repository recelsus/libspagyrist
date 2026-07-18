#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <termios.h>

namespace spagyrist::detail {

struct terminal_size {
    std::size_t rows{};
    std::size_t columns{};
};

enum class terminal_key {
    character,
    backspace,
    enter,
    escape,
    ctrl_c,
    ctrl_n,
    ctrl_p,
    arrow_up,
    arrow_down,
    unknown,
    end_of_input,
};

struct terminal_input {
    terminal_key key{terminal_key::unknown};
    std::string value{};
};

class raw_terminal_mode {
public:
    explicit raw_terminal_mode(int fd);

    raw_terminal_mode(const raw_terminal_mode&) = delete;
    raw_terminal_mode& operator=(const raw_terminal_mode&) = delete;

    raw_terminal_mode(raw_terminal_mode&& other) noexcept;
    raw_terminal_mode& operator=(raw_terminal_mode&& other) noexcept;

    ~raw_terminal_mode();

    [[nodiscard]] bool enabled() const noexcept;
    [[nodiscard]] const std::string& error() const noexcept;

    void restore() noexcept;

private:
    int fd_{-1};
    bool enabled_{false};
    std::string error_;
    termios original_{};
};

class terminal_screen_guard {
public:
    explicit terminal_screen_guard(int fd);

    terminal_screen_guard(const terminal_screen_guard&) = delete;
    terminal_screen_guard& operator=(const terminal_screen_guard&) = delete;

    ~terminal_screen_guard();

    [[nodiscard]] bool enabled() const noexcept;
    [[nodiscard]] const std::string& error() const noexcept;

private:
    int fd_{-1};
    bool screen_enabled_{false};
    bool cursor_hidden_{false};
    std::string error_;
};

[[nodiscard]] bool is_terminal(int fd) noexcept;
[[nodiscard]] std::optional<terminal_size> query_terminal_size(int fd) noexcept;
void write_terminal_all(int fd, std::string_view value);
[[nodiscard]] bool try_write_terminal_all(int fd, std::string_view value) noexcept;
[[nodiscard]] terminal_input parse_terminal_input(std::string_view input) noexcept;
[[nodiscard]] terminal_input read_terminal_input(int fd);

} // namespace spagyrist::detail
