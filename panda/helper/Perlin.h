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

#ifndef HELPER_PERLIN_H
#define HELPER_PERLIN_H

#include <panda/helper/system/Config.h>
#include <QPointF>
#include <QVector2D>
#include <QVector3D>
#include <cstdint>

namespace panda
{

namespace helper
{

class Perlin
{
 public:
	Perlin(uint8_t octaves = 4);
	Perlin(uint8_t octaves, int32_t seed);

	void setSeed(int32_t seed);
	uint8_t getOctaves() const { return m_octaves; }
	void setOctaves(uint8_t octaves) { m_octaves = octaves; }

	/// Class Perlin look: fractal Brownian motion by summing 'mOctaves' worth of noise
	PReal fBm(PReal v) const;
	PReal fBm(PReal x, PReal y) const;
	PReal fBm(const QVector2D &v) const { return fBm(v.x(), v.y()); }
	PReal fBm(const QPointF &v) const { return fBm(v.x(), v.y()); }
	PReal fBm(PReal x, PReal y, PReal z) const;
	PReal fBm(const QVector3D &v) const { return fBm(v.x(), v.y(), v.z()); }

	/// Derivative of fractal Brownian motion, corresponding with the values returned by fBm()
	QVector2D dfBm(PReal x, PReal y) const;
	QVector2D dfBm(const QVector2D &v) const { return dfBm(v.x(), v.y()); }
	QVector3D dfBm(PReal x, PReal y, PReal z) const;
	QVector3D dfBm(const QVector3D &v) const { return dfBm(v.x(), v.y(), v.z()); }

	/// Calculates a single octave of noise
	PReal noise(PReal x) const;
	PReal noise(PReal x, PReal y) const;
	PReal noise(PReal x, PReal y, PReal z) const;

	/// Calculates the derivative of a single octave of noise
	QVector2D dnoise(PReal x, PReal y) const;
	QVector3D dnoise(PReal x, PReal y, PReal z) const;

 private:
	void initPermutationTable();

	PReal grad(int32_t hash, PReal x) const;
	PReal grad(int32_t hash, PReal x, PReal y) const;
	PReal grad(int32_t hash, PReal x, PReal y, PReal z) const;

	uint8_t m_octaves;
	int32_t	m_seed;

	uint8_t	m_perms[512];
};

} // namespace helper

} // namespace panda

#endif // HELPER_PERLIN_H
