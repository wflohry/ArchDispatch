#include "ArchDispatch.h"
#include <array>
#include <filesystem>
#include <simdpp/dispatch/arch.h>

namespace
{
constexpr std::array<ArchDispatch::Architecture, 5> supported = {
    ArchDispatch::Architecture::AVX2,
    ArchDispatch::Architecture::AVX,
    ArchDispatch::Architecture::SSE4_1,
    ArchDispatch::Architecture::SSE2,
    ArchDispatch::Architecture::NONE_NULL};

constexpr int get_mask()
{
    int out = 0;
    for (size_t i = 0; i < supported.size(); ++i)
    {
        out |= static_cast<int>(supported[i]);
    }
    return out;
}

ArchDispatch::Architecture apply_mask(simdpp::Arch arch)
{
    constexpr int arch_mask = get_mask();
    return static_cast<ArchDispatch::Architecture>(static_cast<int>(arch) & arch_mask);
}

}        // namespace

#if defined(__unix__)

namespace
{
static const char *ext = ".so";
}        // namespace

#	if ((__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 8)) && (__i386__ || __amd64__)

#		include <simdpp/dispatch/get_arch_gcc_builtin_cpu_supports.h>

ArchDispatch::Architecture ArchDispatch::detect_architecture()
{
    return apply_mask(simdpp::get_arch_gcc_builtin_cpu_supports());
}

#	else        // clang

#		include <simdpp/dispatch/get_arch_linux_cpuinfo.h>
ArchDispatch::Architecture ArchDispatch::detect_architecture()
{
    return apply_mask(simdpp::get_arch_linux_cpuinfo());
}
#	endif        // gnu / clang

#elif defined(_MSC_VER) || defined(__clang__) || defined(__INTEL_COMPILER)

namespace
{
static const char *ext = ".dll";
}

#	define LIBSIMDPP_SIMD_H
#	include <simdpp/dispatch/get_arch_raw_cpuid.h>
ArchDispatch::Architecture ArchDispatch::detect_architecture()
{
    return apply_mask(simdpp::get_arch_raw_cpuid());
}
#endif

std::string ArchDispatch::detect_supported_lib(const std::string &lib_base_name, format_name_t format_name, void *user_data)
{
    const auto detected = detect_architecture();
    for (auto &&arch : supported)
    {
        if (arch != ArchDispatch::Architecture::NONE_NULL && !(static_cast<int>(detected) & static_cast<int>(arch)))
        {
            continue;
        }
        const auto name = format_name(lib_base_name, arch, user_data);
        const auto filename = std::filesystem::absolute(name);
        if (std::filesystem::exists(filename))
        {
#if defined(_MSC_VER)
            std::filesystem::current_path(filename.parent_path());
#endif
            return filename.generic_string();
        }
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
    return ".";
}

std::string ArchDispatch::format_name_folder(const std::string &lib_base_name, Architecture arch, void *user_data)
{
    (void) user_data;
    const auto relative_path = std::string(lib_base_name).append(ext);
    if (arch == ArchDispatch::Architecture::NONE_NULL){
        return relative_path;
    }
    return get_name(arch).append("/").append(relative_path);
}

std::string ArchDispatch::format_name_suffix(const std::string &lib_base_name, Architecture arch, void *user_data)
{
    (void) user_data;
    return std::string(lib_base_name).append(get_name(arch)).append(ext);
}
