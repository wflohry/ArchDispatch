#include <cstdint>

#if defined(__GNUC__) || defined(__clang__)
#	define VISIBILITY_HIDDEN __attribute__((visibility("hidden")))
#	define VISIBILITY_VISIBLE __attribute__((visibility("default")))
#elif defined(_MSC_VER)
#	define VISIBILITY_HIDDEN
#	define VISIBILITY_VISIBLE __declspec(dllexport)
#endif

extern "C"
{
    VISIBILITY_VISIBLE float get_result(const float *v, uint64_t N)
    {
        float out = 0;
        for (std::size_t i = 0; i < N; ++i)
        {
            out += v[i];
        }
        return out;
    }

    VISIBILITY_VISIBLE int simd_main(int argc, char **argv)
    {
        return 0;
    }
}
