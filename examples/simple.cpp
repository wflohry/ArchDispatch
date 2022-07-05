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
}        // namespace

int main(int argc, char **argv)
{
#ifdef __unix__
    const std::string libname = "libfor_loop";
#else
    const std::string libname = "for_loop";
#endif

    ArchDispatch::Dispatcher dispatcher(libname);

    if (!dispatcher)
    {
        std::cerr << "Unable to load " << libname << " (" << dispatcher.get_lib_name() << ")" << std::endl;
        std::cerr << dispatcher.get_error() << std::endl;
        return 1;
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
