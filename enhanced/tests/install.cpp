// Installing the game data (core/data/gog_install.cpp): SHA-256, the
// manifest, installs from a folder and from GOG's installer.
//
// The folder tests make their work folders under build/tmp (or
// TH_TEST_SCRATCH). The installer test runs when TH_GOG_SETUP names GOG's
// setup_theme_hospital_v3_(28027).exe.

#include "common.h"

#include "core/data/fs.h"
#include "core/data/gamedata.h"
#include "core/data/gog_install.h"
#include "core/data/sha256.h"

#include <algorithm>
#include <atomic>
#include <cctype>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>

namespace fs = std::filesystem;

namespace
{

std::string sha_hex(std::string_view text)
{
    th::Sha256 sha;
    sha.update(text.data(), text.size());
    return th::to_hex(sha.final());
}

fs::path scratch_dir(const char *name)
{
    const char *base = std::getenv("TH_TEST_SCRATCH");
    fs::path dir = fs::path(base && *base ? base : "build/tmp") / name;
    std::error_code ec;
    fs::remove_all(dir, ec);
    fs::create_directories(dir, ec);
    return dir;
}

std::string lower(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return char(std::tolower(c)); });
    return s;
}

// Links every manifest file of `data_dir` into `folder` under a lower case
// name, except `skip`, plus a file the manifest does not name.
void link_source(const fs::path &data_dir, const fs::path &folder, std::string_view skip = {})
{
    std::error_code ec;
    for (const th::ManifestFile &file : th::manifest_files()) {
        if (file.path == skip)
            continue;
        auto from = th::resolve_ci(data_dir, file.path);
        CHECK(from);
        if (!from)
            return;
        fs::path to = folder / lower(std::string(file.path));
        fs::create_directories(to.parent_path(), ec);
        fs::create_symlink(fs::absolute(*from), to, ec);
        CHECK(!ec);
    }
    std::ofstream(folder / "junk.txt") << "not a game file";
    std::ofstream(folder / "data" / "extra.dat") << "not a game file either";
}

// Every manifest file of `dir` holds what the same file of `data_dir` does.
void check_same(const fs::path &data_dir, const fs::path &dir)
{
    for (const th::ManifestFile &file : th::manifest_files()) {
        auto want = th::resolve_ci(data_dir, file.path);
        auto got = th::resolve_ci(dir, file.path);
        CHECK(want);
        CHECK(got);
        if (!want || !got)
            return;
        auto a = th::read_file(*want);
        auto b = th::read_file(*got);
        CHECK(a && b && *a == *b);
        if (!a || !b || *a != *b)
            std::fprintf(stderr, "%s differs\n", std::string(file.path).c_str());
    }
}

bool contains(const std::string &text, std::string_view what)
{
    return text.find(what) != std::string::npos;
}

// The shortest manifest file: a corrupted copy of it is made cheaply.
std::string_view smallest_file()
{
    auto files = th::manifest_files();
    return std::min_element(files.begin(), files.end(), [](const auto &a, const auto &b) { return a.size < b.size; })
        ->path;
}

} // namespace

void test_sha256_vectors()
{
    CHECK(sha_hex("") == "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
    CHECK(sha_hex("abc") == "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
    CHECK(sha_hex("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq") ==
          "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1");
    CHECK(sha_hex("abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmnoijklmnopjklmnopqklmnopqrlmnopqrs"
                  "mnopqrstnopqrstu") == "cf5b16a778af8380036ce59e7b0492370b249b11e8f07a51afac45037afee9d1");

    th::Sha256 sha;
    std::string block(1000, 'a');
    for (int i = 0; i < 1000; i++)
        sha.update(block.data(), block.size());
    CHECK(th::to_hex(sha.final()) == "cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0");

    // The same message in uneven pieces, across the 64-byte block edge.
    std::string text(200, 'x');
    th::Sha256 whole;
    whole.update(text.data(), text.size());
    th::Sha256 parts;
    for (size_t at = 0, step = 1; at < text.size(); at += step, step += 7)
        parts.update(text.data() + at, std::min(step, text.size() - at));
    CHECK(whole.final() == parts.final());
}

void test_manifest_covers_data()
{
    auto files = th::manifest_files();
    CHECK(!files.empty());
    for (const th::ManifestFile &file : files) {
        CHECK(std::none_of(file.path.begin(), file.path.end(), [](unsigned char c) { return std::islower(c); }));
        CHECK(file.sha256.size() == 64);
        CHECK(file.chunk_count > 0);
    }
    // Sorted, so no file is there twice.
    CHECK(std::is_sorted(files.begin(), files.end(), [](const auto &a, const auto &b) { return a.path < b.path; }));
    for (std::string_view rel : th::required_data_files()) {
        bool found = std::any_of(files.begin(), files.end(),
                                 [&](const th::ManifestFile &f) { return lower(std::string(f.path)) == rel; });
        CHECK(found);
    }
}

void test_install_from_folder(const char *dir)
{
    fs::path root = scratch_dir("install_folder");
    link_source(dir, root / "src");
    fs::path dest = root / "dest";
    CHECK(!th::is_installed(dest));

    uint64_t last_done = 0;
    uint64_t total = 0;
    bool monotone = true;
    th::InstallOptions options;
    options.progress = [&](uint64_t done, uint64_t of) {
        monotone = monotone && done >= last_done && done <= of;
        last_done = done;
        total = of;
    };
    auto start = std::chrono::steady_clock::now();
    th::InstallResult r = th::install_from_folder(root / "src", dest, options);
    double seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
    std::fprintf(stderr, "install from a folder: %.1f s\n", seconds);
    CHECK(r.ok);
    if (!r.ok)
        std::fprintf(stderr, "%s\n", r.error.c_str());
    CHECK(monotone);
    CHECK(total != 0 && last_done == total);
    CHECK(!fs::exists(root / "dest.partial"));
    CHECK(!th::resolve_ci(dest, "junk.txt"));
    CHECK(!th::resolve_ci(dest, "data/extra.dat"));
    CHECK(th::resolve_ci(dest, "install.ok"));
    check_same(dir, dest);

    CHECK(th::is_installed(dest));
    CHECK(th::verify_install(dest).ok);

    // A file cut short is seen by both checks, and named.
    auto victim = th::resolve_ci(dest, smallest_file());
    CHECK(victim);
    if (victim)
        fs::resize_file(*victim, fs::file_size(*victim) - 1);
    CHECK(!th::is_installed(dest));
    th::InstallResult v = th::verify_install(dest);
    CHECK(!v.ok);
    CHECK(contains(v.error, smallest_file()));
    CHECK(contains(v.error, "size"));

    std::error_code ec;
    fs::remove_all(root, ec);
}

// A source whose smallest file is corrupted, or missing from it.
void test_install_refused(const char *dir, bool corrupt)
{
    fs::path root = scratch_dir(corrupt ? "install_corrupt" : "install_missing");
    std::string bad(smallest_file());
    link_source(dir, root / "src", bad);
    if (corrupt) {
        auto original = th::read_file(*th::resolve_ci(dir, bad));
        CHECK(original);
        if (!original)
            return;
        (*original)[0] ^= 0xff;
        std::ofstream out(root / "src" / lower(bad), std::ios::binary);
        out.write(reinterpret_cast<const char *>(original->data()), static_cast<std::streamsize>(original->size()));
    }

    // An install that is there already is left as it was.
    fs::path dest = root / "dest";
    fs::create_directories(dest);
    std::ofstream(dest / "keep.txt") << "kept";

    th::InstallResult r = th::install_from_folder(root / "src", dest);
    CHECK(!r.ok);
    CHECK(!r.cancelled);
    CHECK(contains(r.error, bad));
    CHECK(contains(r.error, corrupt ? "checksum" : "missing"));
    CHECK(fs::exists(dest / "keep.txt"));
    CHECK(!fs::exists(root / "dest.partial"));

    std::error_code ec;
    fs::remove_all(root, ec);
}

void test_install_refused_corrupt(const char *dir)
{
    test_install_refused(dir, true);
}

void test_install_refused_missing(const char *dir)
{
    test_install_refused(dir, false);
}

void test_install_cancelled(const char *dir)
{
    fs::path root = scratch_dir("install_cancel");
    link_source(dir, root / "src");
    fs::path dest = root / "dest";
    std::atomic<bool> cancel = false;
    th::InstallOptions options;
    options.cancel = &cancel;
    options.progress = [&](uint64_t, uint64_t) { cancel = true; };
    th::InstallResult r = th::install_from_folder(root / "src", dest, options);
    CHECK(!r.ok);
    CHECK(r.cancelled);
    CHECK(!fs::exists(dest));
    CHECK(!fs::exists(root / "dest.partial"));
    std::error_code ec;
    fs::remove_all(root, ec);
}

void test_install_from_installer(const char *dir)
{
    const char *setup = std::getenv("TH_GOG_SETUP");
    if (!setup || !*setup)
        return;
    fs::path root = scratch_dir("install_setup");
    fs::path dest = root / "dest";

    uint64_t last_done = 0;
    uint64_t total = 0;
    th::InstallOptions options;
    options.progress = [&](uint64_t done, uint64_t of) {
        last_done = done;
        total = of;
    };
    auto start = std::chrono::steady_clock::now();
    th::InstallResult r = th::install_from_installer(setup, dest, options);
    double seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
    std::fprintf(stderr, "install from the installer: %.1f s\n", seconds);
    CHECK(r.ok);
    if (!r.ok)
        std::fprintf(stderr, "%s\n", r.error.c_str());
    CHECK(total != 0 && last_done == total);
    CHECK(!fs::exists(root / "dest.partial"));
    CHECK(th::is_installed(dest));
    check_same(dir, dest);

    // Not the installer: refused before anything is written.
    fs::path other = root / "setup_theme_hospital_v3_(28027).exe";
    std::ofstream(other) << "not the installer";
    fs::path dest2 = root / "dest2";
    th::InstallResult bad = th::install_from_installer(other, dest2);
    CHECK(!bad.ok);
    CHECK(!fs::exists(dest2));
    CHECK(!fs::exists(root / "dest2.partial"));

    std::error_code ec;
    fs::remove_all(root, ec);
}
