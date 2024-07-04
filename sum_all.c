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

#if defined(_WIN32) || defined(__WIN32__)
#include <intrin.h>
#include <windows.h>
static u64 os_timer_freq(void)
{
    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);
    return freq.QuadPart;
}
static u64 os_timer(void)
{
    LARGE_INTEGER freq;
    QueryPerformanceCounter(&freq);
    return freq.QuadPart;
}
#else
#include <x86intrin.h>
#include <sys/time.h>
static u64 os_timer_freq(void)
{
    return 1000000;
}
static u64 os_timer(void)
{
    struct timeval time;
    gettimeofday(&time, 0);
    return os_timer_freq()*(u64)time.tv_sec + (u64)time.tv_usec;
}
#endif

static inline u64 cpu_timer(void)
{
    return __rdtsc();
}

static u64 cpu_timer_freq(void)
{
    u64 millisToWait = 100;
    u64 osFreq       = os_timer_freq();
    u64 cpuStart     = cpu_timer();
    u64 osStart      = os_timer();
    u64 osEnd        = 0;
    u64 osElapsed    = 0;
    u64 osWaitTime   = osFreq * millisToWait / 1000;
    while(osElapsed < osWaitTime) {
        osEnd     = os_timer();
        osElapsed = osEnd - osStart;
    }
    u64 cpuEnd     = cpu_timer();
    u64 cpuElapsed = cpuEnd - cpuStart;
    u64 cpu_freq = 0;
    if (osElapsed) cpu_freq = osFreq * cpuElapsed / osElapsed;
    return cpu_freq;
}

static f64 tsc_to_ms(u64 tsc)
{
	static u64 freq = 0;
	if (!freq) freq = cpu_timer_freq();
	return 1000.0 * (f64)tsc / (f64)freq;
}

u64 sum_scalar(u64 start, u64 end)
{
	u64 t0 = cpu_timer();
	u64 sum = 0;
	for (u64 i = start; i < end; i++) sum += i&0xff;
	u64 t1 = cpu_timer();
	printf("Summing took ~%fms\n", tsc_to_ms(t1 - t0));
	return sum;
}

u64 sum_wide(u64 start, u64 end)
{
	u64 t0 = cpu_timer();
	u64 n = end/WIDTH*WIDTH;
	u64 sum[WIDTH] = { 0 };
	for (u64 i = start; i < n; i += WIDTH) {
		sum[0] += (i + 0)&0xff;
		sum[1] += (i + 1)&0xff;
		sum[2] += (i + 2)&0xff;
		sum[3] += (i + 3)&0xff;
	}
	u64 s = sum[0] + sum[1] + sum[2] + sum[3];
	for (u64 i = 0; i < end - n; i++) {
		s += (n + i)&0xff;
	}
	u64 t1 = cpu_timer();
	printf("Summing took ~%fms\n", tsc_to_ms(t1 - t0));
	return s;
}

u64 sum_simd(u64 start, u64 end)
{
	u64 t0 = cpu_timer();
	u64 n = end/WIDTH*WIDTH;
	u64 sums[WIDTH];
	__m256i sum      = _mm256_setzero_si256();
	__m256i summands = _mm256_set_epi64x(0, 1, 2, 3);
	__m256i mask     = _mm256_set_epi64x(0xff, 0xff, 0xff, 0xff);
	for (u64 i = start; i < n; i += WIDTH) {
		__m256i tmp = _mm256_set1_epi64x(i);
		tmp = _mm256_add_epi64(tmp, summands);
		tmp = _mm256_and_si256(tmp, mask);
		sum = _mm256_add_epi64(sum, tmp);
	}
	_mm256_storeu_si256((void*)sums, sum);
	u64 s = sums[0] + sums[1] + sums[2] + sums[3];
	for (u64 i = 0; i < end - n; i++) {
		s += (n + i)&0xff;
	}
	u64 t1 = cpu_timer();
	printf("Summing took ~%fms\n", tsc_to_ms(t1 - t0));
	return s;
}

u64 sum_simd_wide(u64 start, u64 end)
{
	u64 t0 = cpu_timer();
	u64 n  = end/(WIDTH*WIDTH)*(WIDTH*WIDTH);
	u64 u64sums[WIDTH];
	__m256i m256sums[WIDTH] = { _mm256_setzero_si256() };
	u64 inc_val = WIDTH*WIDTH;
	__m256i inc = _mm256_castpd_si256(_mm256_broadcast_sd((f64*)&inc_val));
	u64 tmp_vals[]      = { 0, 1, 2, 3 };
	__m256i tmps[WIDTH] = {
		_mm256_castpd_si256(_mm256_broadcast_sd((f64*)&tmp_vals[0])),
		_mm256_castpd_si256(_mm256_broadcast_sd((f64*)&tmp_vals[1])),
		_mm256_castpd_si256(_mm256_broadcast_sd((f64*)&tmp_vals[2])),
		_mm256_castpd_si256(_mm256_broadcast_sd((f64*)&tmp_vals[3])),
	};
	u64 widths_vals[WIDTH*WIDTH] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
	__m256i widths[WIDTH] = {
		_mm256_loadu_si256((__m256i*)&widths_vals[0*WIDTH]),
		_mm256_loadu_si256((__m256i*)&widths_vals[1*WIDTH]),
		_mm256_loadu_si256((__m256i*)&widths_vals[2*WIDTH]),
		_mm256_loadu_si256((__m256i*)&widths_vals[3*WIDTH]),
	};
	u64 mask_val = 0xff;
	__m256i mask = _mm256_castpd_si256(_mm256_broadcast_sd((f64*)&mask_val));
	for (u64 i = start; i < n; i += WIDTH*WIDTH) {
		tmps[0] = _mm256_and_si256(widths[0], mask);
		tmps[1] = _mm256_and_si256(widths[1], mask);
		tmps[2] = _mm256_and_si256(widths[2], mask);
		tmps[3] = _mm256_and_si256(widths[3], mask);

		m256sums[0] = _mm256_add_epi64(m256sums[0], tmps[0]);
		m256sums[1] = _mm256_add_epi64(m256sums[1], tmps[1]);
		m256sums[2] = _mm256_add_epi64(m256sums[2], tmps[2]);
		m256sums[3] = _mm256_add_epi64(m256sums[3], tmps[3]);

		widths[0] = _mm256_add_epi64(widths[0], inc);
		widths[1] = _mm256_add_epi64(widths[1], inc);
		widths[2] = _mm256_add_epi64(widths[2], inc);
		widths[3] = _mm256_add_epi64(widths[3], inc);
	}
	__m256i m256sum = _mm256_add_epi64(_mm256_add_epi64(m256sums[0], m256sums[1]), _mm256_add_epi64(m256sums[2], m256sums[3]));
	_mm256_storeu_si256((void*)u64sums, m256sum);
	u64 s = u64sums[0] + u64sums[1] + u64sums[2] + u64sums[3];
	for (u64 i = 0; i < end - n; i++) {
		s += (n + i)&0xff;
	}
	u64 t1 = cpu_timer();
	printf("Summing took ~%fms\n", tsc_to_ms(t1 - t0));
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
	u64 s1 = sum_scalar(0, n);
	printf("---\nWide:\n");
	u64 s2 = sum_wide(0, n);
	printf("---\nSIMD:\n");
	u64 s3 = sum_simd(0, n);
	printf("---\nSIMD Wide:\n");
	u64 s4 = sum_simd_wide(0, n);
	// printf("s1: %lld, s2: %lld, s3: %lld, s4: %lld\n", s1, s2, s3, s4);
	assert(s1 == s2);
	assert(s1 == s3);
	assert(s1 == s4);
	printf("---\nSum: %llu\n", s1);
	return 0;
}
