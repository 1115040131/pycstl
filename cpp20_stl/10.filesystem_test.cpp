#include <algorithm>
#include <filesystem>
#include <fstream>
#include <regex>

#include <fmt/base.h>
#include <fmt/format.h>
#include <gtest/gtest.h>

namespace fmt {

template <>
struct formatter<std::filesystem::path> : formatter<std::string> {
    template <typename FormatContext>
    auto format(const std::filesystem::path& path, FormatContext& ctx) const {
        return format_to(ctx.out(), "{}", path.string());
    }
};

}  // namespace fmt

namespace pyc {

namespace fs = std::filesystem;

TEST(FilesystemTest, FormatTest) {
    constexpr std::string_view kFileName = "cpp20_stl/resource/the-raven.txt";
    fs::path dir{kFileName};
    EXPECT_TRUE(fs::exists(dir));
    EXPECT_EQ(fmt::format("{}", dir), "cpp20_stl/resource/the-raven.txt");

    // 路径的不同部分
    EXPECT_EQ(dir, "cpp20_stl/resource/the-raven.txt");
    EXPECT_EQ(dir.stem(), "the-raven");
    EXPECT_EQ(dir.extension(), ".txt");
    EXPECT_EQ(dir.filename(), "the-raven.txt");
    EXPECT_EQ(dir.parent_path(), "cpp20_stl/resource");
    fmt::print("canonical: {}\n", fs::canonical(dir));
}

TEST(FilesystemTest, PathTest) {
    fs::path dir{"testdir"};

    fmt::println("current_path: {}", fs::current_path());
    fmt::println("absolute(p): {}", fs::absolute(dir));
    // /= 操作符将一个字符串追加到路径字符串的末尾，并返回一个新的路径对象
    EXPECT_EQ(dir /= "append.txt", "testdir/append.txt");
    EXPECT_TRUE(
        fs::equivalent("cpp20_stl/resource/the-raven.txt", "cpp20_stl/resource/../resource/the-raven.txt"));
    EXPECT_THROW([[maybe_unused]] auto is_equal = fs::equivalent("testdir/foo.txt", "testdir/../testdir/foo.txt"),
                 fs::filesystem_error);
}

#pragma region ls

// 返回文件类型的字符表示
char type_char(const fs::file_status& fstat) {
    if (fs::is_symlink(fstat)) {
        return 'l';
    } else if (fs::is_directory(fstat)) {
        return 'd';
    } else if (fs::is_character_file(fstat)) {
        return 'c';
    } else if (fs::is_block_file(fstat)) {
        return 'b';
    } else if (fs::is_fifo(fstat)) {
        return 'p';
    } else if (fs::is_socket(fstat)) {
        return 's';
    } else if (fs::is_other(fstat)) {
        return 'o';
    } else if (fs::is_regular_file(fstat)) {
        return '-';
    }
    return '?';
}

std::string rwx(const fs::perms& fperm) {
    auto bit2char = [&fperm](fs::perms bit, char c) { return (fperm & bit) == fs::perms::none ? '-' : c; };
    return {
        bit2char(fs::perms::owner_read, 'r'),  bit2char(fs::perms::owner_write, 'w'),
        bit2char(fs::perms::owner_exec, 'x'),  bit2char(fs::perms::group_read, 'r'),
        bit2char(fs::perms::group_write, 'w'), bit2char(fs::perms::group_exec, 'x'),
        bit2char(fs::perms::others_read, 'r'), bit2char(fs::perms::others_write, 'w'),
        bit2char(fs::perms::others_exec, 'x'),
    };
}

std::string size_string(const uintmax_t fsize) {
    constexpr const uintmax_t kilo{1024};
    constexpr const uintmax_t mega{1024 * kilo};
    constexpr const uintmax_t giga{1024 * mega};
    if (fsize >= giga) {
        return fmt::format("{}G", (fsize + giga / 2) / giga);
    } else if (fsize >= mega) {
        return fmt::format("{}M", (fsize + mega / 2) / mega);
    } else if (fsize >= kilo) {
        return fmt::format("{}K", (fsize + kilo / 2) / kilo);
    }
    return fmt::format("{}B", fsize);
}

void print_dir(const fs::directory_entry& entry) {
    const auto fpath{entry.path()};
    const auto fstat{entry.symlink_status()};
    const auto fperm{fstat.permissions()};
    const uintmax_t fsize{fs::is_regular_file(fstat) ? fs::file_size(fpath) : 0};
    const auto fn{fpath.filename()};

    std::string suffix{};
    if (fs::is_symlink(fstat)) {
        suffix = " -> " + fs::read_symlink(fpath).string();
    } else if (fs::is_directory(entry)) {
        suffix = "/";
    } else if ((fperm & fs::perms::owner_exec) != fs::perms::none) {
        suffix = "*";
    }
    fmt::println("{}{} {} {}{}", type_char(fstat), rwx(fperm), size_string(fsize), fn, suffix);
}

// ls 命令实现
void ls_impl(const fs::path& dir = fs::current_path()) {
    std::vector<fs::directory_entry> entries{};
    for (const auto& entry : fs::directory_iterator(dir)) {
        entries.push_back(entry);
    }
    std::ranges::sort(entries, {}, [](const fs::directory_entry& entry) {
        std::string s;
        std::ranges::transform(entry.path().filename().string(), std::back_inserter(s), ::tolower);
        return s;
    });
    for (const auto& entry : entries) {
        print_dir(entry);
    }
}

TEST(FilesystemTest, ListTest) { ls_impl("cpp20_stl"); }

#pragma endregion

#pragma region rename

// 对文件名字符串应用正则表达式替换
std::string replace_str(std::string str, const std::vector<std::pair<std::regex, std::string>>& replacements) {
    for (const auto& [pattern, replace] : replacements) {
        str = std::regex_replace(str, pattern, replace);
    }
    return str;
}

void rename_impl(const fs::path& dir, const std::vector<std::pair<std::regex, std::string>>& patterns) {
    for (const auto& entry : fs::directory_iterator(dir)) {
        fs::path fpath{entry.path()};
        std::string rname = replace_str(fpath.filename().string(), patterns);
        if (rname != fpath.filename()) {
            fs::path rpath{fpath.parent_path() / rname};
            if (fs::exists(rpath)) {
                fmt::println("Error: cannot rename {} -> {}, file exists", fpath.filename(), rpath.filename());
            } else {
                fs::rename(fpath, rpath);
                fmt::println("{} -> {}", fpath.filename(), rpath.filename());
            }
        }
    }
}

TEST(FilesystemTest, RenameTest) {
    std::string_view old_name{"test.txt"};
    std::string_view new_name{"test.TXT"};

    std::ofstream(old_name.data());
    EXPECT_TRUE(fs::exists(old_name));
    EXPECT_FALSE(fs::exists(new_name));

    fs::path dir = fs::current_path();
    rename_impl(dir, {{std::regex{".txt"}, ".TXT"}});

    EXPECT_FALSE(fs::exists(old_name));
    EXPECT_TRUE(fs::exists(new_name));

    rename_impl(dir, {{std::regex{".TXT"}, ".txt"}});
    EXPECT_TRUE(fs::exists(old_name));
    EXPECT_FALSE(fs::exists(new_name));
}

#pragma endregion

}  // namespace pyc
