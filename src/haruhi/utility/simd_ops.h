/* vim:ts=4
 *
 * Copyleft 2008…2012  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef HARUHI__UTILITY__SIMD_OPS_H__INCLUDED
#define HARUHI__UTILITY__SIMD_OPS_H__INCLUDED

// Standard:
#include <cstddef>

// System:
#ifdef HARUHI_SSE1
#include <xmmintrin.h>
#endif

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/utility/fast_pow.h>


/*
 * IMPORTANT: These functions work fast if provided array sizes are
 * divisible by 4. There is serious performance regression otherwise.
 */

namespace SIMD {

#define CAST_TO_MM128(x) reinterpret_cast<__m128*> (x)
#define VECSIZE 4


/**
 * Computes target = 0
 * Arrays must be 16-byte aligned.
 *
 * \param	size Number of floats in arrays.
 */
static inline void
clear_buffer (float* target, std::size_t size) noexcept
{
#ifdef HARUHI_SSE1
	__m128* xt = CAST_TO_MM128 (target);
	for (std::size_t i = 0; i < size / VECSIZE; ++i)
	{
		*xt = _mm_xor_ps (*xt, *xt);
		++xt;
	}
	for (std::size_t i = size / VECSIZE * VECSIZE; i < size; ++i)
		target[i] = 0.0f;
#elif defined(HARUHI_IEEE754)
	bzero (target, sizeof (float) * size);
#else
	std::fill (target, target + size, 0.0);
#endif
}


/**
 * Computes target = -target
 * Arrays must be 16-byte aligned.
 *
 * \param	size Number of floats in arrays.
 */
static inline void
negate_buffer (float* target, std::size_t size) noexcept
{
#ifdef HARUHI_SSE1
# ifdef HARUHI_IEEE754
	union { int i; float f; } u = { i: 1 << 31 };
	__m128 st = _mm_set_ps1 (u.f);
	__m128* xt = CAST_TO_MM128 (target);
	for (std::size_t i = 0; i < size / VECSIZE; ++i)
	{
		*xt = _mm_xor_ps (*xt, st);
		++xt;
	}
# else
	__m128* xt = CAST_TO_MM128 (target);
	for (std::size_t i = 0; i < size / VECSIZE; ++i)
	{
		*xt = _mm_sub_ps (_mm_setzero_ps(), *xt);
		++xt;
	}
# endif
	for (std::size_t i = size / VECSIZE * VECSIZE; i < size; ++i)
		target[i] = -target[i];
#else
	for (std::size_t i = 0; i < size; ++i)
		target[i] = -target[i];
#endif
}


/**
 * Computes target = scalar
 * Arrays must be 16-byte aligned.
 *
 * \param	size Number of floats in arrays.
 */
static inline void
fill_buffer (float* target, std::size_t size, float scalar) noexcept
{
#ifdef HARUHI_SSE1
	__m128* xt = CAST_TO_MM128 (target);
	for (std::size_t i = 0; i < size / VECSIZE; ++i)
	{
		*xt = _mm_set_ps1 (scalar);
		++xt;
	}
	for (std::size_t i = size / VECSIZE * VECSIZE; i < size; ++i)
		target[i] = scalar;
#else
	std::fill (target, target + size, scalar);
#endif
}


/**
 * Computes target = source
 * Arrays must be 16-byte aligned.
 *
 * \param	size Number of floats in arrays.
 */
static inline void
copy_buffer (float* target, float* source, std::size_t size) noexcept
{
#ifdef HARUHI_SSE1
	__m128* xs = CAST_TO_MM128 (source);
	__m128* xt = CAST_TO_MM128 (target);
	for (std::size_t i = 0; i < size / VECSIZE; ++i)
	{
		*xt = *xs;
		++xs;
		++xt;
	}
	for (std::size_t i = size / VECSIZE * VECSIZE; i < size; ++i)
		target[i] = source[i];
#else
	memcpy (target, source, sizeof (float) * size);
#endif
}


/**
 * Computes target = target + source
 * Arrays must be 16-byte aligned.
 *
 * \param	size Number of floats in arrays.
 */
static inline void
add_buffers (float* target, float* source, std::size_t size) noexcept
{
#ifdef HARUHI_SSE1
	__m128* xs = CAST_TO_MM128 (source);
	__m128* xt = CAST_TO_MM128 (target);
	for (std::size_t i = 0; i < size / VECSIZE; ++i)
	{
		*xt = _mm_add_ps (*xt, *xs);
		++xs;
		++xt;
	}
	for (std::size_t i = size / VECSIZE * VECSIZE; i < size; ++i)
		target[i] += source[i];
#else
	for (std::size_t i = 0; i < size; ++i)
		target[i] += source[i];
#endif
}


/**
 * Computes target = target + source * attenuation
 * Arrays must be 16-byte aligned.
 *
 * \param	size Number of floats in arrays.
 */
static inline void
add_buffers (float* target, float* source, float attenuate_source, std::size_t size) noexcept
{
#ifdef HARUHI_SSE1
	__m128* xs = CAST_TO_MM128 (source);
	__m128* xt = CAST_TO_MM128 (target);
	__m128 att = _mm_set_ps1 (attenuate_source);
	for (std::size_t i = 0; i < size / VECSIZE; ++i)
	{
		*xt = _mm_add_ps (*xt, _mm_mul_ps (att, *xs));
		++xs;
		++xt;
	}
	for (std::size_t i = size / VECSIZE * VECSIZE; i < size; ++i)
		target[i] += attenuate_source * source[i];
#else
	for (std::size_t i = 0; i < size; ++i)
		target[i] += attenuate_source * source[i];
#endif
}


/**
 * Computes target = target + scalar
 * Arrays must be 16-byte aligned.
 *
 * \param	size Number of floats in arrays.
 */
static inline void
add_scalar_to_buffer (float* target, std::size_t size, float scalar) noexcept
{
#ifdef HARUHI_SSE1
	__m128 xs = _mm_set_ps1 (scalar);
	__m128* xt = CAST_TO_MM128 (target);
	for (std::size_t i = 0; i < size / VECSIZE; ++i)
	{
		*xt = _mm_add_ps (*xt, xs);
		++xt;
	}
	for (std::size_t i = size / VECSIZE * VECSIZE; i < size; ++i)
		target[i] += scalar;
#else
	for (std::size_t i = 0; i < size; ++i)
		target[i] += scalar;
#endif
}


/**
 * Computes target = target - source
 * Arrays must be 16-byte aligned.
 *
 * \param	size Number of floats in arrays.
 */
static inline void
sub_buffers (float* target, float* source, std::size_t size) noexcept
{
#ifdef HARUHI_SSE1
	__m128* xs = CAST_TO_MM128 (source);
	__m128* xt = CAST_TO_MM128 (target);
	for (std::size_t i = 0; i < size / VECSIZE; ++i)
	{
		*xt = _mm_sub_ps (*xt, *xs);
		++xs;
		++xt;
	}
	for (std::size_t i = size / VECSIZE * VECSIZE; i < size; ++i)
		target[i] += source[i];
#else
	for (std::size_t i = 0; i < size; ++i)
		target[i] -= source[i];
#endif
}


/**
 * Computes target = target * source
 * Arrays must be 16-byte aligned.
 *
 * \param	size Number of floats in arrays.
 */
static inline void
multiply_buffers (float* target, float* source, std::size_t size) noexcept
{
#ifdef HARUHI_SSE1
	__m128* xs = CAST_TO_MM128 (source);
	__m128* xt = CAST_TO_MM128 (target);
	for (std::size_t i = 0; i < size / VECSIZE; ++i)
	{
		*xt = _mm_mul_ps (*xt, *xs);
		++xs;
		++xt;
	}
	for (std::size_t i = size / VECSIZE * VECSIZE; i < size; ++i)
		target[i] *= source[i];
#else
	for (std::size_t i = 0; i < size; ++i)
		target[i] *= source[i];
#endif
}


/**
 * Computes target = target * scalar
 * Arrays must be 16-byte aligned.
 *
 * \param	size Number of floats in arrays.
 */
static inline void
multiply_buffer_by_scalar (float* target, std::size_t size, float scalar) noexcept
{
#ifdef HARUHI_SSE1
	__m128 xs = _mm_set_ps1 (scalar);
	__m128* xt = CAST_TO_MM128 (target);
	for (std::size_t i = 0; i < size / VECSIZE; ++i)
	{
		*xt = _mm_mul_ps (*xt, xs);
		++xt;
	}
	for (std::size_t i = size / VECSIZE * VECSIZE; i < size; ++i)
		target[i] *= scalar;
#else
	for (std::size_t i = 0; i < size; ++i)
		target[i] *= scalar;
#endif
}


/**
 * Computes target = target * source * scalar
 * Arrays must be 16-byte aligned.
 *
 * \param	size Number of floats in arrays.
 */
static inline void
multiply_buffers_and_by_scalar (float* target, float* source, std::size_t size, float scalar) noexcept
{
#ifdef HARUHI_SSE1
	__m128 xscalar = _mm_set_ps1 (scalar);
	__m128* xs = CAST_TO_MM128 (source);
	__m128* xt = CAST_TO_MM128 (target);
	for (std::size_t i = 0; i < size / VECSIZE; ++i)
	{
		*xt = _mm_mul_ps (*xt, _mm_mul_ps (*xs, xscalar));
		++xs;
		++xt;
	}
	for (std::size_t i = size / VECSIZE * VECSIZE; i < size; ++i)
		target[i] *= source[i] * scalar;
#else
	for (std::size_t i = 0; i < size; ++i)
		target[i] *= source[i] * scalar;
#endif
}


/**
 * Computes target1 = target1 * source * scalar, target2 = target2 * source * scalar
 * Arrays must be 16-byte aligned.
 *
 * \param	size Number of floats in arrays.
 */
static inline void
multiply_buffers_and_by_scalar (float* target1, float* target2, float* source, std::size_t size, float scalar) noexcept
{
#ifdef HARUHI_SSE1
	__m128 xscalar = _mm_set_ps1 (scalar);
	__m128 tmp;
	__m128* xs = CAST_TO_MM128 (source);
	__m128* xt1 = CAST_TO_MM128 (target1);
	__m128* xt2 = CAST_TO_MM128 (target2);
	for (std::size_t i = 0; i < size / VECSIZE; ++i)
	{
		tmp = _mm_mul_ps (*xs, xscalar);
		*xt1 = _mm_mul_ps (*xt1, tmp);
		*xt2 = _mm_mul_ps (*xt2, tmp);
		++xs;
		++xt1;
		++xt2;
	}
	for (std::size_t i = size / VECSIZE * VECSIZE; i < size; ++i)
	{
		float k = source[i] * scalar;
		target1[i] *= k;
		target2[i] *= k;
	}
#else
	for (std::size_t i = 0; i < size; ++i)
	{
		float k = source[i] * scalar;
		target1[i] *= k;
		target2[i] *= k;
	}
#endif
}


/**
 * Computes target = target^power
 * Arrays must be 16-byte aligned.
 *
 * \param	size Number of floats in arrays.
 */
static inline void
power_buffers (float* target, float* power, std::size_t size) noexcept
{
#ifdef HARUHI_HAS_SSE_POW
	const float* target_end = target + size;
	for (float *x = target, *y = power; x < target_end; x += 4, y += 4)
		SSEPow::vec4_pow (x, x, y);
	for (std::size_t i = size / VECSIZE * VECSIZE; i < size; ++i)
		target[i] = SSEPow::pow (target[i], power[i]);
#else
	for (std::size_t i = 0; i < size; ++i)
		target[i] = FastPow::pow (target[i], power[i]);
#endif
}


/**
 * Computes target = target^scalar
 * Arrays must be 16-byte aligned.
 *
 * \param	size Number of floats in arrays.
 */
static inline void
power_buffer_to_scalar (float* target, std::size_t size, float scalar) noexcept
{
#ifdef HARUHI_HAS_SSE_POW
	const float* target_end = target + size;
	for (float* x = target; x < target_end; x += 4)
		SSEPow::vec4_pow_to_scalar (x, x, scalar);
	for (std::size_t i = size / VECSIZE * VECSIZE; i < size; ++i)
		target[i] = SSEPow::pow (target[i], scalar);
#else
	for (std::size_t i = 0; i < size; ++i)
		target[i] = FastPow::pow (target[i], scalar);
#endif
}


#undef CAST_TO_MM128
#undef VECSIZE

} // namespace SIMD

#endif

