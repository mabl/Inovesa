/******************************************************************************
 * Inovesa - Inovesa Numerical Optimized Vlasov-Equation Solver Application   *
 * Copyright (c) 2013-2017: Patrik Schönfeldt                                 *
 * Copyright (c) 2014-2017: Karlsruhe Institute of Technology                 *
 *                                                                            *
 * This file is part of Inovesa.                                              *
 * Inovesa is free software: you can redistribute it and/or modify            *
 * it under the terms of the GNU General Public License as published by       *
 * the Free Software Foundation, either version 3 of the License, or          *
 * (at your option) any later version.                                        *
 *                                                                            *
 * Inovesa is distributed in the hope that it will be useful,                 *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
 * GNU General Public License for more details.                               *
 *                                                                            *
 * You should have received a copy of the GNU General Public License          *
 * along with Inovesa.  If not, see <http://www.gnu.org/licenses/>.           *
 ******************************************************************************/

#ifndef ROTATIONMAP_HPP
#define ROTATIONMAP_HPP

#include <array>

#include "defines.hpp"
#include "SM/SourceMap.hpp"
#include "IO/Display.hpp"

using std::modf;

namespace vfps
{

class RotationMap : public SourceMap
{
public:
    /**
     * @brief RotationMap
     * @param in data source
     * @param out data target
     * @param xsize horizontal size of thne mesh
     * @param ysize vertical size of thne mesh
     * @param angle to rotate the phase space
     * @param it number of points to use for (1D) interpolation
     * @param interpol_clamped turn clamping on and off
     * @param rt choose between real and Manhattan style
     * @param rotmapsize size of rotation map (can be 0, _size or _size/2)
     *
     * Saturation only makes sense with quadratic/cubic interpolation.
     * Enabling it with linear (or without)
     * interpolation currently crashes the program.
     */
    RotationMap(std::shared_ptr<PhaseSpace> in,
                std::shared_ptr<PhaseSpace> out,
                const meshindex_t xsize, const meshindex_t ysize,
                const meshaxis_t angle, const InterpolationType it,
                const bool interpol_clamped,
                const RotationCoordinates rt, const size_t rotmapsize=0);

    ~RotationMap() noexcept;

    /**
     * @brief apply overrides HM::apply() by an optimized implementation
     */
    void apply() override;

    /**
     * @brief apply overrides HM::apply() by an optimized implementation
     */
    PhaseSpace::Position apply(const PhaseSpace::Position pos) const override;

private:
    const uint32_t _rotmapsize;

    const bool _clamp;

    void genHInfo(meshindex_t q_i, meshindex_t p_i, hi* myhinfo);

    const RotationCoordinates _rt;
    const meshaxis_t _cos_dt;
    const meshaxis_t _sin_dt;

    #ifdef INOVESA_USE_CL
    void genCode4SM4sat();

    void genCode4Rotation();

    cl_int2 imgsize;

    cl_float2 rot;
    #endif // INOVESA_USE_CL
};

}

#endif // ROTATIONMAP_HPP
