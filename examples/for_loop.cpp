#include <cstdint>

extern "C"
{
    float get_result(const float *v, uint64_t N)
    {
        float out = 0;
        for (std::size_t i = 0; i < N; ++i)
        {
            out += v[i];
        }
        return out;
    }
}
