#include "atomic_file.h"

#include <cstdio>
#include <fstream>
#include <system_error>

namespace shared::persistence {

namespace fs = std::filesystem;

bool atomic_write(const fs::path& target, const std::string& content)
{
    std::error_code ec;

    // Ensure the parent directory exists (first write to a fresh addon dir).
    const fs::path dir = target.has_parent_path() ? target.parent_path() : fs::path(".");
    if (!dir.empty() && !fs::exists(dir, ec))
    {
        fs::create_directories(dir, ec);
        if (ec) { return false; }
    }

    // Temp file in the SAME directory, so the final rename stays on one volume
    // (a cross-device rename is not atomic and may fail). Name is unique per
    // target so concurrent writers to different files don't collide.
    const fs::path tmp = dir / (target.filename().string() + ".tmp");

    {
        // Binary + truncate: write bytes verbatim, no newline translation.
        std::ofstream out(tmp, std::ios::binary | std::ios::trunc);
        if (!out) { return false; }
        out.write(content.data(), static_cast<std::streamsize>(content.size()));
        out.flush();
        if (!out) { fs::remove(tmp, ec); return false; }
    } // stream closed here — bytes handed to the OS before we rename.

    // Atomic replace. std::filesystem::rename overwrites an existing target.
    fs::rename(tmp, target, ec);
    if (ec)
    {
        fs::remove(tmp, ec); // best-effort cleanup; don't leak the temp file.
        return false;
    }
    return true;
}

std::optional<std::string> read_file(const fs::path& path)
{
    std::ifstream in(path, std::ios::binary);
    if (!in) { return std::nullopt; }

    std::string content(
        (std::istreambuf_iterator<char>(in)),
        std::istreambuf_iterator<char>());
    return content;
}

} // namespace shared::persistence
