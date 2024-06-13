#include <immintrin.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint32_t b32;
typedef uint64_t u64;
typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;
typedef float    f32;
typedef double   f64;

#define WIDTH 4

#include <time.h>
#ifdef _WIN32
#include <windows.h>
#endif

static inline f64 clock_start(void)
{
#ifdef _WIN32
    return (f64)timeGetTime() / 1000;
#else
    struct timespec ts = {0};
    int ret = clock_gettime(CLOCK_MONOTONIC, &ts);
    return (f64)ts.tv_sec + ts.tv_nsec*1e-9;
#endif
}

static inline f64 clock_elapsed(f64 start)
{
    return clock_start() - start;
}

u64 sum_scalar(u64 n)
{
	f64 t = clock_start();
	u64 sum = 0;
	for (u64 i = 0; i < n; i++) {
		sum += i;
	}
	printf("Summing took ~%fms\n", clock_elapsed(t));
	return sum;
}

u64 sum_wide(u64 n)
{
	f64 t = clock_start();
	u64 m = n/WIDTH*WIDTH;
	u64 sum[WIDTH] = { 0 };
	for (u64 i = 0; i < m; i += WIDTH) {
		sum[0] += i + 0;
		sum[1] += i + 1;
		sum[2] += i + 2;
		sum[3] += i + 3;
	}
	u64 s = sum[0] + sum[1] + sum[2] + sum[3];
	for (u64 i = 0; i < n - m; i++) {
		s += m + i;
	}
	printf("Summing took ~%fms\n", clock_elapsed(t));
	return s;
}

u64 sum_simd(u64 n)
{
	f64 t = clock_start();
	u64 m = n/WIDTH*WIDTH;
	u64 sums[WIDTH];
	__m256i sum = _mm256_setzero_si256();
	__m256i summands = _mm256_set_epi64x(0, 1, 2, 3);
	for (u64 i = 0; i < m; i += WIDTH) {
		__m256i tmp = _mm256_set1_epi64x(i);
		tmp = _mm256_add_epi64(tmp, summands);
		sum = _mm256_add_epi64(sum, tmp);
	}
	_mm256_storeu_si256((void*)sums, sum);
	u64 s = sums[0] + sums[1] + sums[2] + sums[3];
	for (u64 i = 0; i < n - m; i++) {
		s += m + i;
	}
	printf("Summing took ~%fms\n", clock_elapsed(t));
	return s;
}

u64 sum_simd_wide(u64 n)
{
	f64 t = clock_start();
	u64 m = n/(WIDTH*WIDTH)*(WIDTH*WIDTH);
	u64 u64sums[WIDTH];
	__m256i m256sums[WIDTH] = { _mm256_setzero_si256() };
	__m256i tmps[WIDTH];
	__m256i summands = _mm256_set_epi64x(0, 1, 2, 3);
	for (u64 i = 0; i < m; i += WIDTH*WIDTH) {
		tmps[0] = _mm256_set1_epi64x(i + 0*WIDTH);
		tmps[1] = _mm256_set1_epi64x(i + 1*WIDTH);
		tmps[2] = _mm256_set1_epi64x(i + 2*WIDTH);
		tmps[3] = _mm256_set1_epi64x(i + 3*WIDTH);

		tmps[0] = _mm256_add_epi64(tmps[0], summands);
		tmps[1] = _mm256_add_epi64(tmps[1], summands);
		tmps[2] = _mm256_add_epi64(tmps[2], summands);
		tmps[3] = _mm256_add_epi64(tmps[3], summands);

		m256sums[0] = _mm256_add_epi64(m256sums[0], tmps[0]);
		m256sums[1] = _mm256_add_epi64(m256sums[1], tmps[1]);
		m256sums[2] = _mm256_add_epi64(m256sums[2], tmps[2]);
		m256sums[3] = _mm256_add_epi64(m256sums[3], tmps[3]);
	}
	__m256i m256sum = _mm256_add_epi64(_mm256_add_epi64(m256sums[0], m256sums[1]), _mm256_add_epi64(m256sums[2], m256sums[3]));
	_mm256_storeu_si256((void*)u64sums, m256sum);
	u64 s = u64sums[0] + u64sums[1] + u64sums[2] + u64sums[3];
	for (u64 i = 0; i < n - m; i++) {
		s += m + i;
	}
	printf("Summing took ~%fms\n", clock_elapsed(t));
	return s;
}

int main(int argc, char const *argv[])
{
	if (argc < 2) {
		printf("Please provide the amount of numbers to sum together");
		return 1;
	}
	u64 n = atoll(argv[1]);
	printf("---\nScalar:\n");
	u64 s1 = sum_scalar(n);
	printf("---\nWide:\n");
	u64 s2 = sum_wide(n);
	printf("---\nSIMD:\n");
	u64 s3 = sum_simd(n);
	printf("---\nSIMD Wide:\n");
	u64 s4 = sum_simd_wide(n);
	assert(s1 == s2);
	assert(s1 == s3);
	assert(s1 == s4);
	printf("---\nSum: %llu\n", s1);
	return 0;
}
