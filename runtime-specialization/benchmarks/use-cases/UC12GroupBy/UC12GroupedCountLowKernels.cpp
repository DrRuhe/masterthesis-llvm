#include "UC12Kernels.h"
#include "ClangRuntimeSpecializer.h"
#include <cstring>
#include <vector>
#include <stdexcept>

// grouped_count: row_stride, key_offset, n_buckets are specialization constants.
// No value column needed — increments a count bucket per key.
void grouped_count(const uint8_t* rows, int64_t n_rows,
                   int row_stride, int key_offset,
                   int n_buckets, int64_t* count_buckets) {
    for (int64_t i = 0; i < n_rows; ++i) {
        const uint8_t* row_ptr = rows + (int64_t)i * row_stride;

        int32_t key;
        __builtin_memcpy(&key, row_ptr + key_offset, sizeof(int32_t));
        int bucket = ((key % n_buckets) + n_buckets) % n_buckets;

        count_buckets[bucket] += 1;
    }
}

GroupedCountLowSpecialized create_grouped_count_low_specialized(
        int row_stride, int key_offset, int n_buckets) {
    auto* RS = clangRuntimeSpecializer::ClangRuntimeSpecializer::init();
    auto lam = [row_stride, key_offset, n_buckets]
               (const uint8_t* rows, int64_t n_rows, int64_t* out) {
        grouped_count(rows, n_rows, row_stride, key_offset, n_buckets, out);
    };
    return RS->specializeLambda<void>(lam);
}

void validate_grouped_count_low_specialized(int row_stride, int key_offset, int n_buckets) {
    constexpr int N_TEST = 1000;
    std::vector<uint8_t> test_data(N_TEST * row_stride, 0);

    for (int i = 0; i < N_TEST; ++i) {
        int32_t key = (int32_t)(i % n_buckets);
        __builtin_memcpy(test_data.data() + i * row_stride + key_offset, &key, sizeof(int32_t));
    }

    std::vector<int64_t> ref_buckets(n_buckets, 0);
    grouped_count(test_data.data(), N_TEST, row_stride, key_offset, n_buckets,
                  ref_buckets.data());

    auto spec = create_grouped_count_low_specialized(row_stride, key_offset, n_buckets);

    std::vector<int64_t> spec_buckets(n_buckets, 0);
    spec(test_data.data(), N_TEST, spec_buckets.data());

    for (int b = 0; b < n_buckets; ++b) {
        if (ref_buckets[b] != spec_buckets[b]) {
            throw std::runtime_error(
                "validate_grouped_count_low_specialized: bucket mismatch at index " +
                std::to_string(b));
        }
    }
}
