#pragma once
#include <filesystem>
#include <spdlog/spdlog.h>
#include <wots-cli/wots.hpp>

namespace wots {

namespace fs = std::filesystem;

class Task {
  public:
    Task() = default;
    Task(Task const &) = default;
    Task(Task &&) = delete;
    Task &operator=(Task const &) = default;
    Task &operator=(Task &&) = delete;
    virtual ~Task() = default;
    virtual void run() = 0;
    virtual void log() = 0;
};

class Make_directory : public Task {
  public:
    Make_directory(fs::path dir) : dir_(std::move(dir)) {}
    void run() override
    {
        fs::create_directory(dir_);
    }
    void log() override
    {
        spdlog::info("Creating directory: {}", dir_.string());
    }

  private:
    fs::path dir_;
};
class Make_symlink : public Task {
  public:
    Make_symlink(fs::path from, fs::path to)
        : from_(std::move(from)), to_(std::move(to))
    {
    }
    void run() override
    {
        if (fs::is_regular_file(to_)) {
            fs::create_symlink(to_, from_);
        }
        else if (fs::is_directory(to_)) {
            fs::create_directory_symlink(to_, from_);
        }
        else {
            throw Unsupported_filetype_error{};
        }
    }
    void log() override
    {
        spdlog::info("Creating symlink: {} -> {}", from_.string(),
                     to_.string());
    }
    [[nodiscard]] fs::path const &from() const
    {
        return from_;
    }
    [[nodiscard]] fs::path const &to() const
    {
        return to_;
    }

  private:
    fs::path from_;
    fs::path to_;
};

} // namespace wots
