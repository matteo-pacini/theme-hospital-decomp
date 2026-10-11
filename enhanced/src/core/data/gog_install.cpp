#include "core/data/gog_install.h"

#include "core/data/fs.h"
#include "core/data/sha256.h"

#include <miniz_tinfl.h>

#include <cstring>
#include <fstream>
#include <system_error>
#include <vector>

namespace th
{

namespace
{

// A zlib chunk of the installer: its "zlb\x1a" magic's offset and the size
// it inflates to.
struct ManifestChunk {
    uint64_t offset;
    uint64_t size;
};

#include "core/data/gog_manifest.inc"

constexpr size_t kBlock = 1 << 16;
constexpr size_t kStreamBuffer = 1 << 20;

// A file stream with a kStreamBuffer buffer in place of the library's default
// of a few KB: the number of calls that reach the OS then no longer depends on
// it. Under the browser each one is proxied to the main thread and waits for a
// frame, so an install with the default made thousands of them.
template <class Stream> class BufferedFile : private std::vector<char>, public Stream
{
  public:
    BufferedFile() : std::vector<char>(kStreamBuffer)
    {
        // Before open(): the library ignores a buffer set on an open file.
        this->rdbuf()->pubsetbuf(std::vector<char>::data(), static_cast<std::streamsize>(kStreamBuffer));
    }

    BufferedFile(const std::filesystem::path &path, std::ios::openmode mode) : BufferedFile()
    {
        this->open(path, mode | std::ios::binary);
    }
};
constexpr char kChunkMagic[4] = {'z', 'l', 'b', 0x1a};
constexpr std::string_view kInstallerName = "setup_theme_hospital_v3_(28027).exe";
constexpr std::string_view kMarker = "install.ok";

// Progress and cancellation of one call.
class Job
{
  public:
    Job(const InstallOptions &options, uint64_t total) : options_(options), total_(total) {}

    void advance(uint64_t bytes)
    {
        done_ += bytes;
        if (options_.progress)
            options_.progress(done_, total_);
    }

    bool cancelled() const { return options_.cancel && options_.cancel->load(); }

  private:
    const InstallOptions &options_;
    uint64_t total_;
    uint64_t done_ = 0;
};

InstallResult failed(std::string error)
{
    InstallResult r;
    r.error = std::move(error);
    return r;
}

InstallResult cancelled_result()
{
    InstallResult r;
    r.cancelled = true;
    r.error = "cancelled";
    return r;
}

InstallResult succeeded()
{
    InstallResult r;
    r.ok = true;
    return r;
}

uint64_t manifest_total()
{
    uint64_t total = 0;
    for (const ManifestFile &f : kManifestFiles)
        total += f.size;
    return total;
}

// One manifest file being written: counts and hashes what it is given.
class Output
{
  public:
    explicit Output(const std::filesystem::path &path) : file_(path, std::ios::trunc) {}

    bool write(const uint8_t *data, size_t size)
    {
        file_.write(reinterpret_cast<const char *>(data), static_cast<std::streamsize>(size));
        sha_.update(data, size);
        size_ += size;
        return static_cast<bool>(file_);
    }

    bool is_open() { return file_.is_open(); }

    // Closes the file and says what is wrong with it: "" if it holds `file`.
    std::string finish(const ManifestFile &file)
    {
        file_.close();
        if (!file_)
            return std::string(file.path) + ": cannot be written";
        if (size_ != file.size)
            return std::string(file.path) + ": size " + std::to_string(size_) + ", expected " +
                   std::to_string(file.size);
        if (to_hex(sha_.final()) != file.sha256)
            return std::string(file.path) + ": checksum differs";
        return {};
    }

  private:
    BufferedFile<std::ofstream> file_;
    Sha256 sha_;
    uint64_t size_ = 0;
};

// The work folder: `dest`.partial, until it replaces `dest`.
class Staging
{
  public:
    explicit Staging(const std::filesystem::path &dest)
        : dest_(dest), partial_(dest.string() + ".partial"), old_(dest.string() + ".old")
    {
    }

    const std::filesystem::path &partial() const { return partial_; }

    std::string begin()
    {
        std::error_code ec;
        std::filesystem::remove_all(partial_, ec);
        std::filesystem::create_directories(partial_, ec);
        if (ec)
            return partial_.string() + ": " + ec.message();
        return {};
    }

    void discard()
    {
        std::error_code ec;
        std::filesystem::remove_all(partial_, ec);
    }

    // Writes install.ok last, then puts the work folder in place of `dest`.
    // A `dest` that was there is kept until the new one is in place.
    std::string commit()
    {
        std::ofstream marker(partial_ / std::string(kMarker));
        marker << kManifestVersion << '\n';
        marker.close();
        if (!marker)
            return (partial_ / std::string(kMarker)).string() + ": cannot be written";

        std::error_code ec;
        std::filesystem::remove_all(old_, ec);
        bool had = std::filesystem::exists(dest_, ec);
        if (had) {
            std::filesystem::rename(dest_, old_, ec);
            if (ec)
                return dest_.string() + ": " + ec.message();
        }
        std::filesystem::rename(partial_, dest_, ec);
        if (ec) {
            std::string why = partial_.string() + ": " + ec.message();
            if (had)
                std::filesystem::rename(old_, dest_, ec);
            return why;
        }
        std::filesystem::remove_all(old_, ec);
        return {};
    }

  private:
    std::filesystem::path dest_;
    std::filesystem::path partial_;
    std::filesystem::path old_;
};

std::filesystem::path out_path(const Staging &staging, const ManifestFile &file)
{
    return staging.partial() / std::filesystem::path(std::string(file.path));
}

// Inflates the zlib stream after the magic at `chunk.offset` into `out`.
std::string inflate_chunk(std::ifstream &in, const ManifestChunk &chunk, Output &out, Job &job)
{
    in.clear();
    in.seekg(static_cast<std::streamoff>(chunk.offset));
    char magic[4];
    if (!in.read(magic, 4) || std::memcmp(magic, kChunkMagic, 4) != 0)
        return "no chunk at offset " + std::to_string(chunk.offset);

    tinfl_decompressor inflator;
    tinfl_init(&inflator);
    std::vector<uint8_t> input(kBlock);
    std::vector<uint8_t> dict(TINFL_LZ_DICT_SIZE);
    size_t in_pos = 0;
    size_t in_end = 0;
    size_t dict_pos = 0;
    for (;;) {
        if (job.cancelled())
            return "cancelled";
        if (in_pos == in_end) {
            in.read(reinterpret_cast<char *>(input.data()), static_cast<std::streamsize>(input.size()));
            in_end = static_cast<size_t>(in.gcount());
            in_pos = 0;
            if (in_end == 0)
                return "the installer ends inside a chunk";
        }
        size_t in_size = in_end - in_pos;
        size_t out_size = dict.size() - dict_pos;
        tinfl_status status =
            tinfl_decompress(&inflator, input.data() + in_pos, &in_size, dict.data(), dict.data() + dict_pos, &out_size,
                             TINFL_FLAG_PARSE_ZLIB_HEADER | TINFL_FLAG_HAS_MORE_INPUT);
        in_pos += in_size;
        if (!out.write(dict.data() + dict_pos, out_size))
            return "cannot be written";
        job.advance(out_size);
        dict_pos = (dict_pos + out_size) & (dict.size() - 1);
        if (status == TINFL_STATUS_DONE)
            return {};
        if (status < TINFL_STATUS_DONE)
            return "damaged zlib data";
    }
}

// Writes one manifest file out of the installer's chunks.
std::string extract_file(std::ifstream &in, const ManifestFile &file, const std::filesystem::path &path, Job &job)
{
    std::error_code ec;
    std::filesystem::create_directories(path.parent_path(), ec);
    Output out(path);
    if (!out.is_open())
        return std::string(file.path) + ": cannot be written";
    for (uint32_t i = 0; i < file.chunk_count; i++) {
        std::string why = inflate_chunk(in, kManifestChunks[file.first_chunk + i], out, job);
        if (!why.empty())
            return std::string(file.path) + ": " + why;
    }
    return out.finish(file);
}

// Feeds the rest of `in` to `sha` and counts it in `size`. False if stopped.
bool hash_stream(std::ifstream &in, Sha256 &sha, uint64_t &size, Job &job)
{
    std::vector<uint8_t> block(kBlock);
    while (in) {
        if (job.cancelled())
            return false;
        in.read(reinterpret_cast<char *>(block.data()), static_cast<std::streamsize>(block.size()));
        auto got = static_cast<size_t>(in.gcount());
        sha.update(block.data(), got);
        size += got;
        job.advance(got);
    }
    return true;
}

// The installer's size and sha256 against GOG's v3 (28027) one.
std::string check_installer(const std::filesystem::path &path, Job &job)
{
    std::error_code ec;
    uint64_t size = std::filesystem::file_size(path, ec);
    if (ec)
        return path.string() + ": " + ec.message();
    std::string what = path.string() + ": not GOG's " + std::string(kInstallerName);
    if (size != kInstallerSize)
        return what + " (size " + std::to_string(size) + ", expected " + std::to_string(kInstallerSize) + ")";

    BufferedFile<std::ifstream> in(path, std::ios::in);
    Sha256 sha;
    uint64_t hashed = 0;
    if (!hash_stream(in, sha, hashed, job))
        return "cancelled";
    if (to_hex(sha.final()) != kInstallerSha256)
        return what + " (checksum differs)";
    return {};
}

// Copies a source file into `out` in blocks.
std::string copy_into(std::ifstream &in, Output &out, Job &job)
{
    std::vector<uint8_t> block(kBlock);
    while (in) {
        if (job.cancelled())
            return "cancelled";
        in.read(reinterpret_cast<char *>(block.data()), static_cast<std::streamsize>(block.size()));
        auto got = static_cast<size_t>(in.gcount());
        if (!out.write(block.data(), got))
            return "cannot be written";
        job.advance(got);
    }
    return {};
}

std::string copy_file(const std::filesystem::path &source, const ManifestFile &file, const std::filesystem::path &path,
                      Job &job)
{
    BufferedFile<std::ifstream> in(source, std::ios::in);
    if (!in)
        return std::string(file.path) + ": cannot be read from " + source.string();
    std::error_code ec;
    std::filesystem::create_directories(path.parent_path(), ec);
    Output out(path);
    if (!out.is_open())
        return std::string(file.path) + ": cannot be written";
    std::string why = copy_into(in, out, job);
    if (!why.empty())
        return std::string(file.path) + ": " + why;
    return out.finish(file);
}

// The manifest's files found under `folder`, each with the size it should
// have: every one must be there, whatever its case.
std::string find_in_folder(const std::filesystem::path &folder, std::vector<std::filesystem::path> &found)
{
    for (const ManifestFile &file : kManifestFiles) {
        auto path = resolve_ci(folder, file.path);
        std::error_code ec;
        if (!path || !std::filesystem::is_regular_file(*path, ec))
            return std::string(file.path) + ": missing in " + folder.string();
        uint64_t size = std::filesystem::file_size(*path, ec);
        if (ec || size != file.size)
            return std::string(file.path) + ": size " + std::to_string(size) + ", expected " +
                   std::to_string(file.size);
        found.push_back(*path);
    }
    return {};
}

// Finishes an install whose files are written: put it in place or give it up.
InstallResult conclude(Staging &staging, const std::string &error, const Job &job)
{
    if (!error.empty() || job.cancelled()) {
        staging.discard();
        return job.cancelled() ? cancelled_result() : failed(error);
    }
    std::string why = staging.commit();
    if (!why.empty()) {
        staging.discard();
        return failed(why);
    }
    return succeeded();
}

} // namespace

std::span<const ManifestFile> manifest_files()
{
    return kManifestFiles;
}

uint32_t manifest_version()
{
    return kManifestVersion;
}

InstallResult install_from_installer(const std::filesystem::path &installer, const std::filesystem::path &dest,
                                     const InstallOptions &options)
{
    Job job(options, kInstallerSize + manifest_total());
    std::string error = check_installer(installer, job);
    if (!error.empty())
        return job.cancelled() ? cancelled_result() : failed(error);

    Staging staging(dest);
    error = staging.begin();
    if (!error.empty())
        return failed(error);
    BufferedFile<std::ifstream> in(installer, std::ios::in);
    if (!in)
        error = installer.string() + ": cannot be read";
    for (const ManifestFile &file : kManifestFiles) {
        if (!error.empty() || job.cancelled())
            break;
        error = extract_file(in, file, out_path(staging, file), job);
    }
    return conclude(staging, error, job);
}

InstallResult install_from_folder(const std::filesystem::path &folder, const std::filesystem::path &dest,
                                  const InstallOptions &options)
{
    std::vector<std::filesystem::path> sources;
    std::string error = find_in_folder(folder, sources);
    if (!error.empty())
        return failed(error);

    Job job(options, manifest_total());
    Staging staging(dest);
    error = staging.begin();
    if (!error.empty())
        return failed(error);
    for (size_t i = 0; i < sources.size() && error.empty() && !job.cancelled(); i++)
        error = copy_file(sources[i], kManifestFiles[i], out_path(staging, kManifestFiles[i]), job);
    return conclude(staging, error, job);
}

InstallResult verify_install(const std::filesystem::path &dest, const InstallOptions &options)
{
    Job job(options, manifest_total());
    for (const ManifestFile &file : kManifestFiles) {
        auto path = resolve_ci(dest, file.path);
        BufferedFile<std::ifstream> in;
        if (path)
            in.open(*path, std::ios::binary);
        if (!in)
            return failed(std::string(file.path) + ": missing in " + dest.string());
        Sha256 sha;
        uint64_t size = 0;
        if (!hash_stream(in, sha, size, job))
            return cancelled_result();
        if (size != file.size)
            return failed(std::string(file.path) + ": size " + std::to_string(size) + ", expected " +
                          std::to_string(file.size));
        if (to_hex(sha.final()) != file.sha256)
            return failed(std::string(file.path) + ": checksum differs");
    }
    return succeeded();
}

bool is_installed(const std::filesystem::path &dest)
{
    std::ifstream marker(dest / std::string(kMarker));
    uint32_t version = 0;
    if (!(marker >> version) || version != kManifestVersion)
        return false;
    for (const ManifestFile &file : kManifestFiles) {
        std::error_code ec;
        uint64_t size = std::filesystem::file_size(dest / std::filesystem::path(std::string(file.path)), ec);
        if (ec || size != file.size)
            return false;
    }
    return true;
}

} // namespace th
