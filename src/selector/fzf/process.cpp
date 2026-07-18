#include "process.hpp"

#include <cerrno>
#include <csignal>
#include <cstring>
#include <stdexcept>
#include <sys/wait.h>
#include <unistd.h>
#include <utility>

namespace spagyrist::detail::fzf {
namespace {

class file_descriptor {
public:
    explicit file_descriptor(int value = -1)
        : value_(value)
    {
    }

    file_descriptor(const file_descriptor&) = delete;
    file_descriptor& operator=(const file_descriptor&) = delete;

    file_descriptor(file_descriptor&& other) noexcept
        : value_(other.release())
    {
    }

    file_descriptor& operator=(file_descriptor&& other) noexcept
    {
        if (this != &other) {
            reset(other.release());
        }
        return *this;
    }

    ~file_descriptor()
    {
        reset();
    }

    [[nodiscard]] int get() const noexcept
    {
        return value_;
    }

    [[nodiscard]] int release() noexcept
    {
        const auto value = value_;
        value_ = -1;
        return value;
    }

    void reset(int value = -1) noexcept
    {
        if (value_ >= 0) {
            close(value_);
        }
        value_ = value;
    }

private:
    int value_;
};

std::pair<file_descriptor, file_descriptor> make_pipe()
{
    int fds[2]{};
    if (pipe(fds) != 0) {
        throw std::runtime_error(std::string{"pipe failed: "} + std::strerror(errno));
    }
    return {file_descriptor{fds[0]}, file_descriptor{fds[1]}};
}

void write_all(int fd, std::string_view input)
{
    while (!input.empty()) {
        const auto written = write(fd, input.data(), input.size());
        if (written < 0) {
            if (errno == EINTR) {
                continue;
            }
            if (errno == EPIPE) {
                return;
            }
            throw std::runtime_error(std::string{"write failed: "} + std::strerror(errno));
        }
        input.remove_prefix(static_cast<std::size_t>(written));
    }
}

std::string read_all(int fd)
{
    std::string output;
    char buffer[4096]{};
    while (true) {
        const auto count = read(fd, buffer, sizeof(buffer));
        if (count < 0) {
            if (errno == EINTR) {
                continue;
            }
            throw std::runtime_error(std::string{"read failed: "} + std::strerror(errno));
        }
        if (count == 0) {
            break;
        }
        output.append(buffer, static_cast<std::size_t>(count));
    }
    return output;
}

} // namespace

process_result run_process(
    const std::string& executable,
    const std::vector<std::string>& arguments,
    std::string_view input)
{
    auto [stdin_read, stdin_write] = make_pipe();
    auto [stdout_read, stdout_write] = make_pipe();
    auto [stderr_read, stderr_write] = make_pipe();

    const auto pid = fork();
    if (pid < 0) {
        throw std::runtime_error(std::string{"fork failed: "} + std::strerror(errno));
    }

    if (pid == 0) {
        dup2(stdin_read.get(), STDIN_FILENO);
        dup2(stdout_write.get(), STDOUT_FILENO);
        dup2(stderr_write.get(), STDERR_FILENO);

        stdin_read.reset();
        stdin_write.reset();
        stdout_read.reset();
        stdout_write.reset();
        stderr_read.reset();
        stderr_write.reset();

        std::vector<char*> argv;
        argv.reserve(arguments.size() + 2);
        argv.push_back(const_cast<char*>(executable.c_str()));
        for (const auto& argument : arguments) {
            argv.push_back(const_cast<char*>(argument.c_str()));
        }
        argv.push_back(nullptr);

        execvp(executable.c_str(), argv.data());
        _exit(127);
    }

    stdin_read.reset();
    stdout_write.reset();
    stderr_write.reset();

    write_all(stdin_write.get(), input);
    stdin_write.reset();

    auto output = read_all(stdout_read.get());
    stdout_read.reset();
    auto error = read_all(stderr_read.get());
    stderr_read.reset();

    int status{};
    while (waitpid(pid, &status, 0) < 0) {
        if (errno != EINTR) {
            throw std::runtime_error(std::string{"waitpid failed: "} + std::strerror(errno));
        }
    }

    if (WIFEXITED(status)) {
        return process_result{
            .exit_code = WEXITSTATUS(status),
            .output = std::move(output),
            .error = std::move(error),
        };
    }
    if (WIFSIGNALED(status)) {
        return process_result{
            .exit_code = 128 + WTERMSIG(status),
            .output = std::move(output),
            .error = std::move(error),
        };
    }
    return process_result{
        .exit_code = 1,
        .output = std::move(output),
        .error = std::move(error),
    };
}

} // namespace spagyrist::detail::fzf
