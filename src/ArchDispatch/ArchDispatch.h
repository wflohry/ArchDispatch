#ifndef ARCH_DISPATCH_DISPATCHER_H
#define ARCH_DISPATCH_DISPATCHER_H

#include <stdexcept>
#include <string>

namespace ArchDispatch
{

// Matches those in simdpp (but avoiding bringing in header dependency)
enum class Architecture
{
    NONE_NULL = 0,
    SSE2      = 1 << 1,
    SSE4_1    = 1 << 4,
    AVX       = 1 << 6,
    AVX2      = 1 << 7
};

std::string  get_name(Architecture arch);
std::string  detect_supported_lib(const std::string &lib_base_name);
Architecture detect_architecture();
}        // namespace ArchDispatch

#ifdef __unix__
#	include <dlfcn.h>
#	include <string>
#	include <memory>

namespace ArchDispatch
{

class Dispatcher
{
  public:
    Dispatcher(const std::string &lib_base_name)
    {
        name = detect_supported_lib(lib_base_name);
        if (!name.empty())
        {
            handle = std::unique_ptr<void, void (*)(void *)>(dlopen(name.c_str(), RTLD_LAZY), &close);
        }
    }

    std::string get_lib_name() const
    {
        return this->name;
    }

    operator bool() const
    {
        return !!handle;
    }

    template <typename Func>
    Func load(const std::string &function_name)
    {
        if (!handle)
        {
            throw std::runtime_error("No active library");
        }
        if (Func out = reinterpret_cast<Func>(dlsym(handle.get(), function_name.data())))
        {
            return out;
        }
        throw std::runtime_error(get_error());
    }

    static std::string get_error()
    {
        if (const char *error = dlerror())
        {
            return std::string(error);
        }
        return std::string("Unknown error occurred");
    }

    ~Dispatcher()
    {
    }

  private:
    static void close(void *ptr)
    {
        if (!!ptr)
        {
            dlclose(ptr);
        }
    }
    std::string                             name;
    std::unique_ptr<void, void (*)(void *)> handle = {nullptr, &close};
};
}        // namespace ArchDispatch
#endif

#endif
