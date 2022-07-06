#include <ArchDispatch/ArchDispatch.h>
#include <iostream>
#include <vector>

namespace
{
std::pair<std::vector<float>, float> get_value(std::size_t N)
{
    float              result = 0.f;
    std::vector<float> values(N, 0.f);
    for (std::size_t i = 0; i < N; ++i)
    {
        const float v = static_cast<float>(i);
        values[i]     = v;
        result += v;
    }
    return {values, result};
}

int example_1(const std::string &libname)
{
    ArchDispatch::Dispatcher dispatcher(libname);

    if (!dispatcher)
    {
        std::cerr << "Unable to load " << libname << " (" << dispatcher.get_lib_name() << ")" << std::endl;
        std::cerr << dispatcher.get_error() << std::endl;
        return 1;
    }
    else
    {
        std::cout << "Successfully loaded " << dispatcher.get_lib_name() << std::endl;
    }

    typedef float (*RunLoop)(const float *, uint64_t);
    RunLoop run_loop = dispatcher.load<RunLoop>("get_result");
    if (!!run_loop)
    {
        const std::size_t N      = 256;
        const auto        data   = get_value(N);
        const float       result = run_loop(data.first.data(), N);
        const float       error  = std::abs(data.second - result);
        std::cout << result << " vs " << data.second << std::endl;
        if (error < 1E-6)
        {
            std::cout << "Success" << std::endl;
        }
        else
        {
            std::cerr << "Error" << std::endl;
        }
    }
    else
    {
        std::cerr << "Unable to load function" << std::endl;
        return 1;
    }
    return 0;
}

int example_2(const std::string &libname, int argc, char **argv)
{
    typedef int (*Main_t)(int, char **);
    const auto result_1 = ArchDispatch::run_main(libname, "simd_main", argc, argv);
    const auto result_2 = ArchDispatch::run_func<Main_t>(libname, "simd_main", argc, argv);

    for (auto &&[code, err] : {result_1, result_2})
    {
        if (!err.empty() || 0 != code)
        {
            std::cerr << "Received error: " << err << std::endl;
            return code;
        }
    }

    std::cout << "Example 2: Success" << std::endl;

    return 0;
}

}        // namespace

int main(int argc, char **argv)
{
#ifdef __unix__
    const std::string libname = "libfor_loop";
#else
    const std::string libname = "for_loop";
#endif

    example_1(libname);
    example_2(libname, argc, argv);

    return 0;
}
