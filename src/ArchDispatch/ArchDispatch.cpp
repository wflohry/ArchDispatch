#include "ArchDispatch.h"
#include <filesystem>

namespace
{
constexpr std::array<ArchDispatch::Architecture, 4> supported = {
    ArchDispatch::Architecture::AVX2,
    ArchDispatch::Architecture::AVX,
    ArchDispatch::Architecture::SSE4_1,
    ArchDispatch::Architecture::SSE2};

constexpr int get_mask()
{
    int out = 0;
    for (size_t i = 0; i < supported.size(); ++i)
    {
        out |= static_cast<int>(supported[i]);
    }
    return out;
}

constexpr int arch_mask = get_mask();

}        // namespace

#if __unix__
#	include <simdpp/dispatch/get_arch_linux_cpuinfo.h>

namespace
{
static const char *ext = ".so";
}        // namespace

ArchDispatch::Architecture ArchDispatch::detect_architecture()
{
    const auto detected = simdpp::get_arch_linux_cpuinfo();

    return static_cast<Architecture>(static_cast<int>(detected) & arch_mask);
}
#endif

std::string ArchDispatch::detect_supported_lib(const std::string &lib_base_name)
{
    const auto detected = detect_architecture();
    for (auto &&arch : supported)
    {
        if (!(static_cast<int>(detected) & static_cast<int>(arch)))
        {
            continue;
        }
        std::string name = std::string(lib_base_name).append(get_name(arch)).append(ext);
        if (std::filesystem::exists(name))
        {
            return std::filesystem::absolute(name);
        }
    }

    const auto without = std::string(lib_base_name).append(ext);
    if (std::filesystem::exists(without))
    {
        return std::filesystem::absolute(without);
    }

    return "";
}

std::string ArchDispatch::get_name(Architecture arch)
{
    switch (arch)
    {
        case Architecture::NONE_NULL:
            break;
        case Architecture::AVX2:
            return "AVX2";
        case Architecture::AVX:
            return "AVX";
        case Architecture::SSE4_1:
            return "SSE4_1";
        case Architecture::SSE2:
            return "SSE2";
    }
    return "";
}
