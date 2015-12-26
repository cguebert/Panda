// This is a conversion from the libCinder source, removing the need for their Vec types.

/*
 Copyright (c) 2010, The Barbarian Group
 All rights reserved.

 Redistribution and use in source and binary forms, with or without modification, are permitted provided that
 the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and
	the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
	the following disclaimer in the documentation and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
*/

#include <cmath>

#include <panda/helper/Perlin.h>
#include <random>

namespace panda
{

namespace helper
{

using types::Point;

static inline PReal fade(PReal t) { return t * t * t * (t * (t * 6 - 15) + 10); }
static inline PReal dfade(PReal t) { return 30.0f * t * t * (t * (t - 2.0f) + 1.0f); }
inline PReal nlerp(PReal t, PReal a, PReal b) { return a + t * (b - a); }

Perlin::Perlin(uint8_t octaves)
	: m_octaves(octaves)
	, m_seed(0x214)
{
	initPermutationTable();
}

Perlin::Perlin(uint8_t octaves, int32_t seed)
	: m_octaves(octaves)
	, m_seed(seed)
{
	initPermutationTable();
}

void Perlin::initPermutationTable()
{
	std::mt19937 gen;
	std::uniform_int_distribution<> dist(0, 255);
	gen.seed(m_seed);

	for(size_t t = 0; t < 256; ++t)
		m_perms[t] = m_perms[t + 256] = dist(gen);
}

void Perlin::setSeed(int32_t seed)
{
	if(m_seed != seed)
	{
		m_seed = seed;
		initPermutationTable();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// fBm
PReal Perlin::fBm(PReal v) const
{
	PReal result = 0.0f;
	PReal amp = 0.5f;

	for(uint8_t i = 0; i < m_octaves; i++)
	{
		result += noise(v) * amp;
		v *= 2.0f;
		amp *= 0.5f;
	}

	return result;
}

PReal Perlin::fBm(PReal x, PReal y) const
{
	PReal result = 0.0f;
	PReal amp = 0.5f;

	for(uint8_t i = 0; i < m_octaves; i++)
	{
		result += noise(x, y) * amp;
		x *= 2.0f; y *= 2.0f;
		amp *= 0.5f;
	}

	return result;
}

PReal Perlin::fBm(PReal x, PReal y, PReal z) const
{
	PReal result = 0.0f;
	PReal amp = 0.5f;

	for(uint8_t i = 0; i < m_octaves; i++)
	{
		result += noise(x, y, z) * amp;
		x *= 2.0f; y *= 2.0f; z *= 2.0f;
		amp *= 0.5f;
	}

	return result;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// dfBm
Point Perlin::dfBm(PReal x, PReal y) const
{
	Point result;
	PReal amp = 0.5f;

	for(uint8_t i = 0; i < m_octaves; i++)
	{
		result += dnoise(x, y) * amp;
		x *= 2.0f; y *= 2.0f;
		amp *= 0.5f;
	}

	return result;
}

Perlin::P3D Perlin::dfBm(PReal x, PReal y, PReal z) const
{
	P3D result;
	PReal amp = 0.5f;

	for(uint8_t i = 0; i < m_octaves; i++)
	{
		auto noise = dnoise(x, y, z);

		for (int j = 0; j < 3; ++j)
			result[j] += noise[j] * amp;
		x *= 2.0f; y *= 2.0f; z *= 2.0f;
		amp *= 0.5f;
	}

	return result;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// noise
PReal Perlin::noise(PReal x) const
{
	int32_t X = ((int32_t)floorf(x)) & 255;
	x -= floorf(x);
	PReal u = fade(x);
	int32_t A = m_perms[X], AA = m_perms[A], B = m_perms[X+1], BA = m_perms[B];

	return nlerp(u, grad(m_perms[AA], x), grad(m_perms[BA], x-1));
}

PReal Perlin::noise(PReal x, PReal y) const
{
	int32_t X = ((int32_t)floorf(x)) & 255, Y = ((int32_t)floorf(y)) & 255;
	x -= floorf(x); y -= floorf(y);
	PReal	u = fade(x), v = fade(y);
	int32_t A = m_perms[X]+Y, AA = m_perms[A], AB = m_perms[A+1],
	B = m_perms[X+1]+Y, BA = m_perms[B], BB = m_perms[B+1];

	return nlerp(v, nlerp(u, grad(m_perms[AA], x  , y ),
							 grad(m_perms[BA], x-1, y )),
					nlerp(u, grad(m_perms[AB], x  , y-1),
							 grad(m_perms[BB], x-1, y-1)));
}

PReal Perlin::noise(PReal x, PReal y, PReal z) const
{
	// These floors need to remain that due to behavior with negatives.
	int32_t X = ((int32_t)floorf(x)) & 255, Y = ((int32_t)floorf(y)) & 255, Z = ((int32_t)floorf(z)) & 255;
	x -= floorf(x); y -= floorf(y); z -= floorf(z);
	PReal	u = fade(x), v = fade(y), w = fade(z);
	int32_t A = m_perms[X  ]+Y, AA = m_perms[A]+Z, AB = m_perms[A+1]+Z,
	B = m_perms[X+1]+Y, BA = m_perms[B]+Z, BB = m_perms[B+1]+Z;

	PReal a = grad(m_perms[AA  ], x  , y  , z  );
	PReal b = grad(m_perms[BA  ], x-1, y  , z  );
	PReal c = grad(m_perms[AB  ], x  , y-1, z  );
	PReal d = grad(m_perms[BB  ], x-1, y-1, z  );
	PReal e = grad(m_perms[AA+1], x  , y  , z-1);
	PReal f = grad(m_perms[BA+1], x-1, y  , z-1);
	PReal g = grad(m_perms[AB+1], x  , y-1, z-1);
	PReal h = grad(m_perms[BB+1], x-1, y-1, z-1);

	return	nlerp(w, nlerp(v, nlerp(u, a, b),
							  nlerp(u, c, d)),
					 nlerp(v, nlerp(u, e, f),
							  nlerp(u, g, h)));
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// dnoise
// Credit for the ideas for analytical Perlin derivatives below are due to Iñigo Quílez
Point Perlin::dnoise(PReal x, PReal y) const
{
	int32_t X = ((int32_t)x) & 255, Y = ((int32_t)y) & 255;
	x -= floorf(x); y -= floorf(y);
	PReal u = fade(x), v = fade(y);
	PReal du = dfade(x), dv = dfade(y);
	int32_t A = m_perms[X  ]+Y, AA = m_perms[A]+0, AB = m_perms[A+1]+0,
		B = m_perms[X+1]+Y, BA = m_perms[B]+0, BB = m_perms[B+1]+0;

	if(du < 0.000001f) du = 1.0f;
	if(dv < 0.000001f) dv = 1.0f;

	PReal a = grad(m_perms[AA], x  , y  );
	PReal b = grad(m_perms[BA], x-1, y  );
	PReal c = grad(m_perms[AB], x  , y-1);
	PReal d = grad(m_perms[BB], x-1, y-1);
	
	const PReal k1 =   b - a;
	const PReal k2 =   c - a;
	const PReal k4 =   a - b - c + d;

	return Point(du * (k1 + k4 * v), dv * (k2 + k4 * u));
}

Perlin::P3D Perlin::dnoise(PReal x, PReal y, PReal z) const
{
	int32_t X = ((int32_t)floorf(x)) & 255, Y = ((int32_t)floorf(y)) & 255, Z = ((int32_t)floorf(z)) & 255;
	x -= floorf(x); y -= floorf(y); z -= floorf(z);
	PReal u = fade(x), v = fade(y), w = fade(z);
	PReal du = dfade(x), dv = dfade(y), dw = dfade(z);
	int32_t A = m_perms[X  ]+Y, AA = m_perms[A]+Z, AB = m_perms[A+1]+Z,
		B = m_perms[X+1]+Y, BA = m_perms[B]+Z, BB = m_perms[B+1]+Z;

	if(du < 0.000001f) du = 1.0f;
	if(dv < 0.000001f) dv = 1.0f;
	if(dw < 0.000001f) dw = 1.0f;

	PReal a = grad(m_perms[AA  ], x  , y  , z  );
	PReal b = grad(m_perms[BA  ], x-1, y  , z  );
	PReal c = grad(m_perms[AB  ], x  , y-1, z  );
	PReal d = grad(m_perms[BB  ], x-1, y-1, z  );
	PReal e = grad(m_perms[AA+1], x  , y  , z-1);
	PReal f = grad(m_perms[BA+1], x-1, y  , z-1);
	PReal g = grad(m_perms[AB+1], x  , y-1, z-1);
	PReal h = grad(m_perms[BB+1], x-1, y-1, z-1);

	const PReal k1 =   b - a;
	const PReal k2 =   c - a;
	const PReal k3 =   e - a;
	const PReal k4 =   a - b - c + d;
	const PReal k5 =   a - c - e + g;
	const PReal k6 =   a - b - e + f;
	const PReal k7 =  -a + b + c - d + e - f - g + h;

	return P3D{ du * (k1 + k4*v + k6*w + k7*v*w),
		dv * (k2 + k5*w + k4*u + k7*w*u),
		dw * (k3 + k6*u + k5*v + k7*u*v) };
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// grad

PReal Perlin::grad(int32_t hash, PReal x) const
{
	int32_t h = hash & 15;                   // CONVERT LO 4 BITS OF HASH CODE
	PReal	u = h<8 ? x : 0,                 // INTO 12 GRADIENT DIRECTIONS.
			v = h<4 ? 0 : h==12||h==14 ? x : 0;
	return ((h&1) == 0 ? u : -u) + ((h&2) == 0 ? v : -v);
}

PReal Perlin::grad(int32_t hash, PReal x, PReal y) const
{
	int32_t h = hash & 15;                   // CONVERT LO 4 BITS OF HASH CODE
	PReal	u = h<8 ? x : y,                 // INTO 12 GRADIENT DIRECTIONS.
			v = h<4 ? y : h==12||h==14 ? x : 0;
	return ((h&1) == 0 ? u : -u) + ((h&2) == 0 ? v : -v);
}

PReal Perlin::grad(int32_t hash, PReal x, PReal y, PReal z) const
{
	int32_t h = hash & 15;                 // CONVERT LO 4 BITS OF HASH CODE
	PReal u = h<8 ? x : y,                 // INTO 12 GRADIENT DIRECTIONS.
		 v = h<4 ? y : h==12||h==14 ? x : z;
	return ((h&1) == 0 ? u : -u) + ((h&2) == 0 ? v : -v);
}



} // namespace helper

} // namespace panda
