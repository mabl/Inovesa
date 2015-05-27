/******************************************************************************/
/* Inovesa - Inovesa Numerical Optimized Vlesov-Equation Solver Application   */
/* Copyright (c) 2014-2015: Patrik Schönfeldt                                 */
/*                                                                            */
/* This file is part of Inovesa.                                              */
/* Inovesa is free software: you can redistribute it and/or modify            */
/* it under the terms of the GNU General Public License as published by       */
/* the Free Software Foundation, either version 3 of the License, or          */
/* (at your option) any later version.                                        */
/*                                                                            */
/* Inovesa is distributed in the hope that it will be useful,                 */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of             */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              */
/* GNU General Public License for more details.                               */
/*                                                                            */
/* You should have received a copy of the GNU General Public License          */
/* along with Inovesa.  If not, see <http://www.gnu.org/licenses/>.           */
/******************************************************************************/

#include <climits>
#include <iostream>
#ifdef INOVESA_USE_PNG
#include <png++/png.hpp>
#endif
#include <sstream>

#include "defines.hpp"
#include "Display.hpp"
#include "PhaseSpace.hpp"
#include "CL/CLProgs.hpp"
#include "CL/OpenCLHandler.hpp"
#include "HM/FokkerPlanckMap.hpp"
#include "HM/Identity.hpp"
#include "HM/KickMap.hpp"
#include "HM/RotationMap.hpp"
#include "IO/HDF5File.hpp"
#include "IO/ProgramOptions.hpp"

using namespace vfps;

int main(int argc, char** argv)
{
	ProgramOptions opts;

	try {
		if (!opts.parse(argc,argv)) {
			return EXIT_SUCCESS;
		}
	} catch(std::exception& e) {
		std::cerr << "error: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	#ifdef INOVESA_USE_CL
	OCLH::active = (opts.getCLDevice() >= 0);
	if (OCLH::active) {
		try {
			prepareCLEnvironment();
		} catch (cl::Error& e) {
			std::cerr << e.what() << std::endl;
			std::cout << "Will fall back to sequential version."
					  << std::endl;
			OCLH::active = false;
		}
	}
	if (OCLH::active) {
		if (opts.getCLDevice() == 0) {
			listCLDevices();
			return EXIT_SUCCESS;
		} else {
			try {
				prepareCLDevice(opts.getCLDevice()-1);
				prepareCLProgs();
			} catch (cl::Error& e) {
				std::cerr << e.what() << std::endl;
				std::cout << "Will fall back to sequential version."
						  << std::endl;
				OCLH::active = false;
			}
		}
	}
	#endif // INOVESA_USE_CL


	meshindex_t ps_size;
	#ifdef INOVESA_USE_PNG
	bool usepng = true;
	// load pattern to start with
	png::image<png::gray_pixel_16> image;
	if (opts.getStartDistPNG() != "/dev/null") {
		try {
			image.read(opts.getStartDistPNG());
		} catch ( const png::std_error &e ) {
			std::cerr << e.what() << std::endl;
			usepng = false;
		}
		catch ( const png::error &e ) {
			std::cerr << "Problem loading " << opts.getStartDistPNG()
					  << ": " << e.what() << std::endl;
			usepng = false;
		}

		if (usepng) {
			if (image.get_width() == image.get_height()) {
				ps_size = image.get_width();
			} else {
				std::cerr << "Phase space has to be quadratic. Please adjust "
						  << opts.getStartDistPNG() << std::endl;
				usepng = false;
			}
		}
	} else {
		usepng = false;
	}
	if (!usepng)
	#endif // INOVESA_USE_PNG
	{
	}
	ps_size = 256;

	PhaseSpace mesh(ps_size,-10.0,10.0,-10.0,10.0);

	#ifdef INOVESA_USE_PNG
	if (usepng) {
		for (unsigned int x=0; x<ps_size; x++) {
			for (unsigned int y=0; y<ps_size; y++) {
				mesh[x][y] = image[ps_size-y-1][x]/float(UINT16_MAX);
			}
		}
	} else
	#endif // INOVESA_USE_PNG
	{
		for (int x=0; x<int(ps_size); x++) {
			for (int y=0; y<int(ps_size); y++) {
				mesh[x][y] = std::exp(-std::pow((x-int(ps_size/2))
												/(ps_size/10.),2)/2
									  -std::pow((y-int(ps_size/2))
												/(ps_size/10.),2)/2);
			}
		}
	}

	HDF5File file(opts.getOutFile(),ps_size);

	PhaseSpace mesh_rotated(mesh);

	#ifdef INOVESA_USE_GUI
	Display* display = nullptr;
	if (opts.showPhaseSpace()) {
		display = new Display();
	}
	#endif

	const unsigned int steps = opts.getSteps();
	const unsigned int outstep = opts.getOutSteps();
	const float rotations = opts.getNRotations();
	const double f_s = opts.getSyncFreq();
	const double t_d = opts.getDampingTime();

	/* angle of one rotation step (in rad)
	 * (angle = 2*pi corresponds to 1 synchrotron period)
	 */
	const double angle = 2*M_PI/steps;

	const double e0 = 2.0/(f_s*t_d*steps);

	if (!usepng) {
		KickMap km(&mesh_rotated,&mesh,ps_size,ps_size,
				   HeritageMap::InterpolationType::cubic);
		km.laser(2.0,4.0,0.2);
		km.apply();
	}

	std::cout << "Building FokkerPlanckMap." << std::endl;
	FokkerPlanckMap fpm(&mesh_rotated,&mesh,ps_size,ps_size,
						FokkerPlanckMap::FPType::full,e0,
						FokkerPlanckMap::DerivationType::cubic);

	std::cout << "Building RotationMap." << std::endl;
	RotationMap rm(&mesh,&mesh_rotated,ps_size,ps_size,angle,
				   HeritageMap::InterpolationType::cubic,
				   RotationMap::RotationCoordinates::norm_pm1,true);

	#ifdef INOVESA_USE_CL
	if (OCLH::active) {
		mesh.syncCLMem(vfps::PhaseSpace::clCopyDirection::cpu2dev);
	}
	#endif // INOVESA_USE_CL
	std::cout << "Starting the simulation." << std::endl;
	for (unsigned int i=0;i<=steps*rotations;i++) {
		if (i%outstep == 0) {
			#ifdef INOVESA_USE_CL
			if (OCLH::active) {
				mesh.syncCLMem(vfps::PhaseSpace::clCopyDirection::dev2cpu);
			}
			#endif // INOVESA_USE_CL
			file.append(&mesh);
			#ifdef INOVESA_USE_GUI
			if (opts.showPhaseSpace()) {
				display->createTexture(&mesh);
				display->draw();
				display->delTexture();
			} else
			#endif
			{
			std::cout << static_cast<float>(i)/steps << '/'
					  << rotations << std::endl;
			}
		}
		rm.apply();
		fpm.apply();
	}
	#ifdef INOVESA_USE_CL
	if (OCLH::active) {
		OCLH::queue.flush();
	}
	#endif // INOVESA_USE_CL

	return EXIT_SUCCESS;
}

