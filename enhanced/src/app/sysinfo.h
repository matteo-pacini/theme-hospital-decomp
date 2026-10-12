// What a debug report says about the machine and the build: the parts that
// need no SDL (the rest is App::system_text).

#pragma once

#include <string>
#include <string_view>

namespace th::sysinfo
{

// PRETTY_NAME of an os-release file's text (else NAME and VERSION), without
// quotes; empty when it has neither.
std::string parse_os_release(std::string_view text);

// The processor's name from /proc/cpuinfo's text: "model name" on x86,
// else "Model name", "Hardware" or "Model"; empty when none is there.
std::string parse_cpuinfo_model(std::string_view text);

// "16 GiB", "7.6 GiB" or "512 MiB" for a size in MiB.
std::string format_mib(int mib);

// `path` with the home folder `home` (trailing separators ignored) shown as
// "~", so that a report shared in public names no user; `path` unchanged
// when it is not under `home` or `home` is empty or a root.
std::string without_home(std::string_view path, std::string_view home);

// The operating system's name and version, and the kernel or build, from the
// running system ("unknown" where it cannot be read). The browser build
// reports the user agent instead (web::user_agent).
std::string os_description();

// The processor's model name, or "unknown".
std::string cpu_model();

// The instruction set the program was compiled for.
const char *architecture();

} // namespace th::sysinfo
