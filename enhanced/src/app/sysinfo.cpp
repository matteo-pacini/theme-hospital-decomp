#include "app/sysinfo.h"

#include <cstdio>
#include <format>
#include <fstream>
#include <sstream>

#if defined(_WIN32)
#include <windows.h>
#elif defined(__APPLE__)
#include <sys/sysctl.h>
#elif !defined(__EMSCRIPTEN__)
#include <sys/utsname.h>
#endif

namespace th::sysinfo
{

namespace
{

std::string_view trim(std::string_view s)
{
    while (!s.empty() && (s.front() == ' ' || s.front() == '\t'))
        s.remove_prefix(1);
    while (!s.empty() && (s.back() == ' ' || s.back() == '\t' || s.back() == '\r'))
        s.remove_suffix(1);
    return s;
}

// The value of `key` in lines of KEY=VALUE (os-release) or KEY : VALUE
// (cpuinfo), first match; `sep` is the separator character.
std::string find_value(std::string_view text, std::string_view key, char sep)
{
    while (!text.empty()) {
        size_t eol = text.find('\n');
        std::string_view line = text.substr(0, eol);
        text = eol == std::string_view::npos ? std::string_view() : text.substr(eol + 1);
        size_t at = line.find(sep);
        if (at == std::string_view::npos || trim(line.substr(0, at)) != key)
            continue;
        std::string_view value = trim(line.substr(at + 1));
        if (value.size() >= 2 && (value.front() == '"' || value.front() == '\'') && value.back() == value.front())
            value = value.substr(1, value.size() - 2);
        return std::string(value);
    }
    return {};
}

#if !defined(_WIN32) && !defined(__APPLE__) && !defined(__EMSCRIPTEN__)
std::string read_text(const char *path)
{
    std::ifstream in(path);
    std::ostringstream text;
    text << in.rdbuf();
    return text.str();
}
#endif

} // namespace

std::string parse_os_release(std::string_view text)
{
    std::string pretty = find_value(text, "PRETTY_NAME", '=');
    if (!pretty.empty())
        return pretty;
    std::string name = find_value(text, "NAME", '=');
    std::string version = find_value(text, "VERSION", '=');
    return version.empty() ? name : name + " " + version;
}

std::string parse_cpuinfo_model(std::string_view text)
{
    for (const char *key : {"model name", "Model name", "Hardware", "Model"}) {
        std::string value = find_value(text, key, ':');
        if (!value.empty())
            return value;
    }
    return {};
}

std::string format_mib(int mib)
{
    if (mib < 1024)
        return std::format("{} MiB", mib);
    if (mib % 1024 == 0)
        return std::format("{} GiB", mib / 1024);
    return std::format("{:.1f} GiB", double(mib) / 1024.0);
}

std::string without_home(std::string_view path, std::string_view home)
{
    while (home.size() > 1 && (home.back() == '/' || home.back() == '\\'))
        home.remove_suffix(1);
    if (home.size() <= 1 || !path.starts_with(home))
        return std::string(path);
    std::string_view rest = path.substr(home.size());
    if (!rest.empty() && rest.front() != '/' && rest.front() != '\\')
        return std::string(path);
    return "~" + std::string(rest);
}

const char *architecture()
{
#if defined(__x86_64__) || defined(_M_X64)
    return "x86_64";
#elif defined(__aarch64__) || defined(_M_ARM64)
    return "arm64";
#elif defined(__i386__) || defined(_M_IX86)
    return "x86";
#elif defined(__arm__)
    return "arm";
#elif defined(__wasm32__)
    return "wasm32";
#else
    return "unknown";
#endif
}

#if defined(_WIN32)

std::string os_description()
{
    using RtlGetVersionFn = LONG(WINAPI *)(OSVERSIONINFOW *);
    HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
    FARPROC proc = ntdll ? GetProcAddress(ntdll, "RtlGetVersion") : nullptr;
    if (!proc)
        return "Windows (version unknown)";
    OSVERSIONINFOW info{};
    info.dwOSVersionInfoSize = sizeof info;
    if (reinterpret_cast<RtlGetVersionFn>(reinterpret_cast<void *>(proc))(&info) != 0)
        return "Windows (version unknown)";
    return std::format("Windows NT {}.{} build {}", info.dwMajorVersion, info.dwMinorVersion, info.dwBuildNumber);
}

std::string cpu_model()
{
    char name[256];
    DWORD size = sizeof name;
    if (RegGetValueA(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", "ProcessorNameString",
                     RRF_RT_REG_SZ, nullptr, name, &size) != ERROR_SUCCESS)
        return "unknown";
    return std::string(trim(name));
}

#elif defined(__APPLE__)

namespace
{

std::string sysctl_string(const char *name)
{
    size_t size = 0;
    if (sysctlbyname(name, nullptr, &size, nullptr, 0) != 0 || size == 0)
        return {};
    std::string value(size, '\0');
    if (sysctlbyname(name, value.data(), &size, nullptr, 0) != 0)
        return {};
    value.resize(size ? size - 1 : 0);
    return value;
}

} // namespace

std::string os_description()
{
    std::string version = sysctl_string("kern.osproductversion");
    std::string build = sysctl_string("kern.osversion");
    if (version.empty())
        return "macOS (version unknown)";
    return build.empty() ? "macOS " + version : std::format("macOS {} (build {})", version, build);
}

std::string cpu_model()
{
    std::string name = sysctl_string("machdep.cpu.brand_string");
    return name.empty() ? "unknown" : name;
}

#elif defined(__EMSCRIPTEN__)

std::string os_description()
{
    return "browser";
}

std::string cpu_model()
{
    return "unknown";
}

#else

std::string os_description()
{
    std::string name = parse_os_release(read_text("/etc/os-release"));
    if (name.empty())
        name = parse_os_release(read_text("/usr/lib/os-release"));
    if (name.empty())
        name = "Linux";
    utsname u{};
    if (uname(&u) == 0)
        name += std::format(" (kernel {})", u.release);
    return name;
}

std::string cpu_model()
{
    std::string name = parse_cpuinfo_model(read_text("/proc/cpuinfo"));
    return name.empty() ? "unknown" : name;
}

#endif

} // namespace th::sysinfo
