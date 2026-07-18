#include "terminal.hpp"

#include "utf8.hpp"

#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <string>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <utility>

namespace spagyrist::detail {
namespace {

std::string errno_message(std::string_view prefix)
{
    std::string output{prefix};
    output += ": ";
    output += std::strerror(errno);
    return output;
}

bool write_terminal_all_impl(int fd, std::string_view value, std::string* error) noexcept
{
    while (!value.empty()) {
        const auto written = write(fd, value.data(), value.size());
        if (written < 0) {
            if (errno == EINTR) {
                continue;
            }
            if (error != nullptr) {
                *error = errno_message("terminal write failed");
            }
            return false;
        }
        if (written == 0) {
            if (error != nullptr) {
                *error = "terminal write failed: wrote 0 bytes";
            }
            return false;
        }
        value.remove_prefix(static_cast<std::size_t>(written));
    }
    return true;
}

std::optional<char> read_byte_with_timeout(int fd, long timeout_usec)
{
    while (true) {
        fd_set read_set;
        FD_ZERO(&read_set);
        FD_SET(fd, &read_set);

        timeval timeout{};
        timeout.tv_sec = 0;
        timeout.tv_usec = timeout_usec;

        const auto ready = select(fd + 1, &read_set, nullptr, nullptr, &timeout);
        if (ready < 0) {
            if (errno == EINTR) {
                continue;
            }
            throw std::runtime_error(errno_message("select failed"));
        }
        if (ready == 0) {
            return std::nullopt;
        }

        char ch{};
        const auto count = read(fd, &ch, 1);
        if (count < 0) {
            if (errno == EINTR) {
                continue;
            }
            throw std::runtime_error(errno_message("read failed"));
        }
        if (count == 0) {
            return std::nullopt;
        }
        return ch;
    }
}

} // namespace

raw_terminal_mode::raw_terminal_mode(int fd)
    : fd_(fd)
{
    if (fd_ < 0) {
        error_ = "invalid file descriptor";
        return;
    }

    if (tcgetattr(fd_, &original_) != 0) {
        error_ = errno_message("tcgetattr failed");
        return;
    }

    auto raw = original_;
    raw.c_iflag &= static_cast<tcflag_t>(~(BRKINT | ICRNL | INPCK | ISTRIP | IXON));
    raw.c_oflag &= static_cast<tcflag_t>(~OPOST);
    raw.c_cflag |= CS8;
    raw.c_lflag &= static_cast<tcflag_t>(~(ECHO | ICANON | IEXTEN | ISIG));
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    if (tcsetattr(fd_, TCSAFLUSH, &raw) != 0) {
        error_ = errno_message("tcsetattr failed");
        return;
    }

    enabled_ = true;
}

raw_terminal_mode::raw_terminal_mode(raw_terminal_mode&& other) noexcept
    : fd_(other.fd_)
    , enabled_(other.enabled_)
    , error_(std::move(other.error_))
    , original_(other.original_)
{
    other.fd_ = -1;
    other.enabled_ = false;
}

raw_terminal_mode&
raw_terminal_mode::operator=(raw_terminal_mode&& other) noexcept
{
    if (this != &other) {
        restore();
        fd_ = other.fd_;
        enabled_ = other.enabled_;
        error_ = std::move(other.error_);
        original_ = other.original_;
        other.fd_ = -1;
        other.enabled_ = false;
    }
    return *this;
}

raw_terminal_mode::~raw_terminal_mode()
{
    restore();
}

bool raw_terminal_mode::enabled() const noexcept
{
    return enabled_;
}

const std::string& raw_terminal_mode::error() const noexcept
{
    return error_;
}

void raw_terminal_mode::restore() noexcept
{
    if (!enabled_) {
        return;
    }

    static_cast<void>(tcsetattr(fd_, TCSAFLUSH, &original_));
    enabled_ = false;
}

terminal_screen_guard::terminal_screen_guard(int fd)
    : fd_(fd)
{
    if (fd_ < 0) {
        error_ = "invalid file descriptor";
        return;
    }
    if (!write_terminal_all_impl(fd_, "\033[?1049h", &error_)) {
        return;
    }
    screen_enabled_ = true;

    if (!write_terminal_all_impl(fd_, "\033[?25l", &error_)) {
        return;
    }
    cursor_hidden_ = true;
}

terminal_screen_guard::~terminal_screen_guard()
{
    if (cursor_hidden_) {
        static_cast<void>(try_write_terminal_all(fd_, "\033[?25h"));
    }
    if (screen_enabled_) {
        static_cast<void>(try_write_terminal_all(fd_, "\033[?1049l"));
    }
}

bool terminal_screen_guard::enabled() const noexcept
{
    return screen_enabled_ && cursor_hidden_;
}

const std::string& terminal_screen_guard::error() const noexcept
{
    return error_;
}

bool is_terminal(int fd) noexcept
{
    return fd >= 0 && isatty(fd) == 1;
}

std::optional<terminal_size> query_terminal_size(int fd) noexcept
{
    if (fd < 0) {
        return std::nullopt;
    }

    winsize size{};
    if (ioctl(fd, TIOCGWINSZ, &size) != 0 || size.ws_row == 0 || size.ws_col == 0) {
        return std::nullopt;
    }

    return terminal_size{
        .rows = size.ws_row,
        .columns = size.ws_col,
    };
}

void write_terminal_all(int fd, std::string_view value)
{
    std::string error;
    if (!write_terminal_all_impl(fd, value, &error)) {
        throw std::runtime_error(error);
    }
}

bool try_write_terminal_all(int fd, std::string_view value) noexcept
{
    return write_terminal_all_impl(fd, value, nullptr);
}

terminal_input parse_terminal_input(std::string_view input) noexcept
{
    if (input.empty()) {
        return terminal_input{.key = terminal_key::end_of_input};
    }

    const auto first = static_cast<unsigned char>(input.front());
    if (first == 0x03) {
        return terminal_input{.key = terminal_key::ctrl_c};
    }
    if (first == 0x0e) {
        return terminal_input{.key = terminal_key::ctrl_n};
    }
    if (first == 0x10) {
        return terminal_input{.key = terminal_key::ctrl_p};
    }
    if (first == '\r' || first == '\n') {
        return terminal_input{.key = terminal_key::enter};
    }
    if (first == 0x7f || first == 0x08) {
        return terminal_input{.key = terminal_key::backspace};
    }
    if (first == 0x1b) {
        if (input == "\x1b[A") {
            return terminal_input{.key = terminal_key::arrow_up};
        }
        if (input == "\x1b[B") {
            return terminal_input{.key = terminal_key::arrow_down};
        }
        if (input.size() == 1) {
            return terminal_input{.key = terminal_key::escape};
        }
        return terminal_input{.key = terminal_key::unknown};
    }
    if (first >= 0x20 && is_complete_utf8_code_point(input)) {
        return terminal_input{.key = terminal_key::character, .value = std::string{input}};
    }
    return terminal_input{.key = terminal_key::unknown};
}

terminal_input read_terminal_input(int fd)
{
    char ch{};
    while (true) {
        const auto count = read(fd, &ch, 1);
        if (count < 0) {
            if (errno == EINTR) {
                continue;
            }
            throw std::runtime_error(errno_message("read failed"));
        }
        if (count == 0) {
            return terminal_input{.key = terminal_key::end_of_input};
        }
        break;
    }

    if (ch != '\x1b') {
        std::string sequence;
        sequence += ch;
        const auto length = utf8_code_point_length(static_cast<unsigned char>(ch));
        if (length == 0) {
            return terminal_input{.key = terminal_key::unknown};
        }
        for (std::size_t i = 1; i < length; ++i) {
            const auto next = read_byte_with_timeout(fd, 10000);
            if (!next) {
                return terminal_input{.key = terminal_key::unknown};
            }
            sequence += *next;
        }
        return parse_terminal_input(sequence);
    }

    char sequence[3]{ch, 0, 0};
    for (std::size_t i = 1; i < 3; ++i) {
        const auto next = read_byte_with_timeout(fd, 10000);
        if (!next) {
            return parse_terminal_input(std::string_view{sequence, i});
        }
        sequence[i] = *next;
    }
    return parse_terminal_input(std::string_view{sequence, 3});
}

} // namespace spagyrist::detail
