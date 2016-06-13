/******************************************************************************
 * Inovesa - Inovesa Numerical Optimized Vlasov-Equation Solver Algorithms   *
 * Copyright (c) 2014-2016: Patrik Schönfeldt                                 *
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

#ifndef WAKEIMPEDANCEMAP_HPP
#define WAKEIMPEDANCEMAP_HPP

#include "SM/WakeKickMap.hpp"
#include "ElectricField.hpp"
#include "impedances/Impedance.hpp"

namespace vfps
{

class WakePotentialMap : public WakeKickMap
{
public:
    WakePotentialMap(PhaseSpace* in, PhaseSpace* out,
                     const meshindex_t xsize,
                     const meshindex_t ysize,
                     ElectricField* field,
                     const InterpolationType it,
                     bool interpol_clamp);

public:
    /**
     * @brief update implements WakeKickMap
     */
    void update();

private:
    ElectricField* _field;
};

} // namespace vfps

#endif // WAKEIMPEDANCEMAP_HPP