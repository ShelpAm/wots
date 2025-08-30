#pragma once
#include <filesystem>
#include <spdlog/spdlog.h>

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
};

class Make_directory : public Task {
  public:
    Make_directory(fs::path dir) : dir_(std::move(dir)) {}
    void run() override
    {
        spdlog::info("Creating directory: {}", dir_.string());
        fs::create_directory(dir_);
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
        spdlog::info("Creating symlink: {} -> {}", to_.string(),
                     from_.string());
        // TODO(shelpam): not portable, sometimes should use
        // create_directory_symlink
        fs::create_symlink(to_, from_);
    }

  private:
    fs::path from_;
    fs::path to_;
};

} // namespace wots
