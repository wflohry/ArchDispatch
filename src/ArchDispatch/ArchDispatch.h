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
            handle = std::unique_ptr<void, Deleter>(dlopen(name.c_str(), RTLD_LAZY));
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
        if (handle)
        {
            if (Func out = reinterpret_cast<Func>(dlsym(handle.get(), function_name.data())))
            {
                return out;
            }
        }
        return nullptr;
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
    struct Deleter
    {
        void operator()(void *ptr)
        {
            if (!!ptr)
            {
                dlclose(ptr);
            }
        }
    };
    std::string                    name;
    std::unique_ptr<void, Deleter> handle;
};

}        // namespace ArchDispatch
#elif defined(_MSC_VER)

#include <windows.h>
#include <type_traits>

namespace ArchDispatch
{

    class Dispatcher
    {
    public:
        Dispatcher(const std::string& lib_base_name)
        {
            name = detect_supported_lib(lib_base_name);
            if (!name.empty())
            {
                handle = LoadLibrary(name.c_str()); // std::unique_ptr<void, Deleter>(dlopen(name.c_str(), RTLD_LAZY));
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
        Func load(const std::string& function_name)
        {
            if (handle)
            {
                if (Func out = reinterpret_cast<Func>(GetProcAddress(handle, function_name.c_str())))
                {
                    return out;
                }
            }
            return nullptr;
        }

        static std::string get_error()
        {
            return std::string("Unknown error occurred");
        }

        ~Dispatcher()
        {
        }

    private:
        std::string name;
        HINSTANCE handle = nullptr;
    };
}
#endif



namespace ArchDispatch
{

/**
 * @brief run_func Load the library, run a single function, and then close the library
 * @param lib_base_name Library name to be passed to arch dispatch.
 * @param function_name unction name within library
 * @param args Args to be passed to function
 * @return Pair containing output or error string, if it exists
 */
template <typename Func, typename... Args>
inline auto run_func(const std::string &lib_base_name, const std::string &function_name, Args &&...args) -> std::pair<std::invoke_result_t<Func, Args...>, std::string>
{
    using Ret       = std::invoke_result_t<Func, Args...>;
    auto dispatcher = ArchDispatch::Dispatcher(lib_base_name);
    if (dispatcher)
    {
        if (auto func = dispatcher.load<Func>(function_name))
        {
            return {func(std::forward<Args>(args)...), ""};
        }
    }

    if constexpr (std::is_default_constructible_v<Ret>)
    {
        return {Ret{}, dispatcher.get_error()};
    }
    else
    {
        throw std::runtime_error(dispatcher.get_error());
    }
}

/**
 * @brief run_main Run the main function from a library for whole-program dynamic dispatch
 * @param lib_base_name Library name to be passed to arch dispatch.
 * @param main_function_name Function name within library
 * @param argc Argument count from main
 * @param argv Arguments from main
 * @return Pair containing error code from "main" or error string, if it exists
 */
inline std::pair<int, std::string> run_main(const std::string &lib_base_name, const std::string &main_function_name, int argc, char **argv)
{
    typedef int (*main_func_t)(int argc, char **argv);
    return run_func<main_func_t>(lib_base_name, main_function_name, argc, argv);
}

}        // namespace ArchDispatch

#endif
