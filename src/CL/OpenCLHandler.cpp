/******************************************************************************/
/* Inovesa - Inovesa Numerical Optimized Vlasov-Equation Solver Algorithms   */
/* Copyright (c) 2014-2016: Patrik Schönfeldt                                 */
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

#ifdef INOVESA_USE_CL

#include "CL/OpenCLHandler.hpp"
#include "IO/Display.hpp"
#ifdef __linux__
#include <GL/glx.h>
#endif
#if defined(__APPLE__) || defined(__MACOSX)
#include <OpenCL/cl_gl.h>
#else
#include <CL/cl_gl.h>
#endif

void OCLH::prepareCLEnvironment(bool glsharing)
{
    cl::Platform::get(&OCLH::platforms);

    std::string available_clversion;
    std::string needed_clversion = "OpenCL 1.";

    unsigned int plati = 0;
    std::string cl_plat_vendor;
    for (unsigned int p=0; p<OCLH::platforms.size(); p++) {
        OCLH::platforms[p].getInfo(CL_PLATFORM_VERSION,&available_clversion);
        if (available_clversion.substr(0,needed_clversion.length()) == needed_clversion) {
                        // stick with AMD (if available)
                        if (cl_plat_vendor != "Advanced Micro Devices, Inc.") {
                plati = p;
                cl_plat_vendor = OCLH::platforms[p].getInfo<CL_PLATFORM_VENDOR>();
            }
        }
    }

#ifdef INOVESA_USE_CLFFT
    clfftInitSetupData(&fft_setup);
    clfftSetup(&fft_setup);
#endif // INOVESA_USE_CLFFT

    glsharing=false;
    if (glsharing) {
        #ifdef __linux__
        cl_context_properties properties[] = {
                        CL_GL_CONTEXT_KHR,
                        (cl_context_properties)glXGetCurrentContext(),
                        CL_GLX_DISPLAY_KHR,
                        (cl_context_properties)glXGetCurrentDisplay(),
                        CL_CONTEXT_PLATFORM,
                        (cl_context_properties)(OCLH::platforms[plati])(),
                        0
        };
        #else
        cl_context_properties properties[] = {
        };
        #endif
        OCLH::context = cl::Context(CL_DEVICE_TYPE_ALL, properties);
    } else {
        cl_context_properties properties[] = {
                        CL_CONTEXT_PLATFORM,
                        (cl_context_properties)(OCLH::platforms[plati])(),
                        0
                     };
        OCLH::context = cl::Context(CL_DEVICE_TYPE_ALL, properties);
    }

        OCLH::devices = OCLH::context.getInfo<CL_CONTEXT_DEVICES>();
}

void
OCLH::prepareCLDevice(unsigned int device)
{
    OCLH::queue = cl::CommandQueue(OCLH::context, OCLH::devices[device]);
    devicetype = OCLH::devices[device].getInfo<CL_DEVICE_TYPE>();
    // cl_VENDOR_gl_sharing is present, when string contains the substring
    OCLH::ogl_sharing
                    = OCLH::devices[device].getInfo<CL_DEVICE_EXTENSIONS>().find(
                            "_gl_sharing") != std::string::npos;
    std::string devicename;
    OCLH::devices[device].getInfo<std::string>(CL_DEVICE_NAME,&devicename);
    vfps::Display::printText("Initialized \""+devicename+"\" for use with OpenCL.");
}

cl::Program OCLH::prepareCLProg(std::string code)
{
    code = datatype_aliases()+custom_datatypes+code;
    cl::vector<std::string> codevec;
    codevec.push_back(code);
    cl::Program::Sources source(codevec);
    cl::Program p(OCLH::context, source);
    try {
            p.build(OCLH::devices);
    } catch (cl::Error &e) {
        e.what();
        std::cout	<< "===== OpenCL Code =====\n"
                                << code << std::endl;
        std::cout	<< "===== OpenCL Build Log =====\n"
                                << p.getBuildInfo<CL_PROGRAM_BUILD_LOG>(OCLH::devices[0])
                                << std::endl;
    }

return p;
}

void OCLH::teardownCLEnvironment()
{
#ifdef INOVESA_USE_CLFFT
    clfftTeardown();
#endif // INOVESA_USE_CLFFT
}

void OCLH::listCLDevices()
{
    std::cout << "OpenCL device options available on this computer:" << std::endl
              << " 0: (Do not use OpenCL.)" << std::endl;
    for (unsigned int p=0; p<OCLH::platforms.size(); p++) {
        cl_context_properties tmp_properties[] =
            { CL_CONTEXT_PLATFORM, (cl_context_properties)(OCLH::platforms[p])(), 0};
        cl::Context tmp_context = cl::Context(CL_DEVICE_TYPE_ALL, tmp_properties);
        cl::vector<cl::Device> tmp_devices = tmp_context.getInfo<CL_CONTEXT_DEVICES>();
        for (unsigned int d=0; d<OCLH::devices.size(); d++) {
            std::string tmpdevicetype;
            switch(tmp_devices[d].getInfo<CL_DEVICE_TYPE>()) {
            case CL_DEVICE_TYPE_CPU:
                tmpdevicetype = "CPU";
                break;
            case CL_DEVICE_TYPE_GPU:
                tmpdevicetype = "GPU";
                break;
            default:
                tmpdevicetype = "unknown";
                break;
            }
            std::cout << " " << d+1 << ": "
                      << tmpdevicetype << ", "
                      << tmp_devices[d].getInfo<CL_DEVICE_MAX_MEM_ALLOC_SIZE>()/0x100000 << " MiB, "
                      << tmp_devices[d].getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>() << "CU "
                      << "(" << tmp_devices[d].getInfo<CL_DEVICE_NAME>() << ")"
                      #ifdef DEBUG
                      << std::endl
                      << "\ton \"" <<  OCLH::platforms[p].getInfo<CL_PLATFORM_NAME>() << "\""
                      << " by \"" <<  OCLH::platforms[p].getInfo<CL_PLATFORM_VENDOR>() << '\"'
                      << std::endl
                      << "\toffering \"" << tmp_devices[d].getInfo<CL_DEVICE_VERSION>() << '\"'
                      << " with " << tmp_devices[d].getInfo<CL_DEVICE_EXTENSIONS>()
                      #endif // DEBUG
                      << std::endl;
        }
    }
}

bool OCLH::active;

cl::vector<cl::Platform> OCLH::platforms;

cl::Context OCLH::context;

cl::vector<cl::Device> OCLH::devices;

cl_device_type OCLH::devicetype;

cl::CommandQueue OCLH::queue;

bool OCLH::ogl_sharing;

#ifdef INOVESA_USE_CLFFT
clfftSetupData OCLH::fft_setup;
#endif // INOVESA_USE_CLFFT

const std::string OCLH::custom_datatypes = R"(
        typedef struct { data_t real; data_t imag; } impedance_t;
        inline impedance_t cmult(const impedance_t a, const impedance_t b) {
            impedance_t rv;
            rv.real = a.real*b.real - a.imag*b.imag;
            rv.imag = a.imag*b.real + a.real*b.imag;
            return rv;
        }

        )";

std::string OCLH::datatype_aliases()
{
    std::string code;
    if (std::is_same<vfps::meshdata_t,float>::value) {
    code +=
        "typedef float data_t;\n"
        "typedef float2 data2_t;\n"
        "typedef float3 data3_t;\n"
        "typedef float4 data4_t;\n"
        "float mult(float x, float y);"
        "float mult(float x, float y) { return x*y; }\n";
    } else  if (std::is_same<vfps::meshdata_t,double>::value) {
    code +=
        "typedef double data_t;\n"
        "typedef double2 data2_t;\n"
        "typedef double3 data3_t;\n"
        "typedef double4 data4_t;\n"
        "double mult(double x, double y);"
        "double mult(double x, double y) { return x*y; }\n";
    } else {
        std::stringstream fxp_fracpart;
        fxp_fracpart << FXP_FRACPART;

        code +=    "__constant int fracpart="+fxp_fracpart.str()+";\n";
        #if FXP_FRACPART < 31
        if (std::is_same<vfps::meshdata_t,vfps::fixp32>::value) {
        code +=
            "typedef int data_t;\n"
            "typedef int2 data2_t;\n"
            "typedef int3 data3_t;\n"
            "typedef int4 data4_t;\n"
            "int mult(int x, int y){return ((long)(x)*(long)(y))>>fracpart;}\n";
        } else
        #endif
        if (std::is_same<vfps::meshdata_t,vfps::fixp64>::value) {
        code +=
            "typedef long data_t;\n"
            "typedef long2 data2_t;\n"
            "typedef long3 data3_t;\n"
            "typedef long4 data4_t;\n"
            "long mult(long x, long y) {"
            "return ((mul_hi(x,y) << (64-fracpart)) + ((x*y) >> fracpart));}\n";
        }
    }
    return code;
}

#endif
