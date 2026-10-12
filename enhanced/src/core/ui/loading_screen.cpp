#include "core/ui/loading_screen.h"

#include "core/data/gamedata.h"
#include "core/ui/layer.h"

#include <cstring>

namespace th
{

bool load_loading_screen(const std::filesystem::path &dir, LoadingScreen &out, std::string *error)
{
    auto dat = load_data_file(dir, "qdata/load01v.dat", error);
    auto pal = dat ? load_data_file(dir, "qdata/load01v.pal", error) : std::nullopt;
    if (!dat || !pal)
        return false;
    if (dat->size() < size_t(kUiWidth) * size_t(kUiHeight) || !palette_from_vga(*pal, out.palette)) {
        if (error)
            *error = "load01v: not a full-screen picture";
        return false;
    }
    out.pixels.resize(kUiWidth, kUiHeight);
    for (int y = 0; y < kUiHeight; y++)
        std::memcpy(out.pixels.row(y), dat->data() + size_t(y) * size_t(kUiWidth), size_t(kUiWidth));
    return true;
}

} // namespace th
