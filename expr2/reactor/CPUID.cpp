// Copyright 2016 The SwiftShader Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "CPUID.hpp"

#if defined(_WIN32)
#	ifndef WIN32_LEAN_AND_MEAN
#		define WIN32_LEAN_AND_MEAN
#	endif
#	include <windows.h>
#	include <intrin.h>
#	include <float.h>
#else
#	include <unistd.h>
#	include <sched.h>
#	include <sys/types.h>
#endif

namespace rr {

bool CPUID::MMX = detectMMX();
bool CPUID::CMOV = detectCMOV();
bool CPUID::SSE = detectSSE();
bool CPUID::SSE2 = detectSSE2();
bool CPUID::SSE3 = detectSSE3();
bool CPUID::SSSE3 = detectSSSE3();
bool CPUID::SSE4_1 = detectSSE4_1();
bool CPUID::F16C = detectF16C();

bool CPUID::enableMMX = true;
bool CPUID::enableCMOV = true;
bool CPUID::enableSSE = true;
bool CPUID::enableSSE2 = true;
bool CPUID::enableSSE3 = true;
bool CPUID::enableSSSE3 = true;
bool CPUID::enableSSE4_1 = true;
bool CPUID::enableAVX = true;
bool CPUID::enableF16C = true;

void CPUID::setEnableMMX(bool enable)
{
	enableMMX = enable;

	if(!enableMMX)
	{
		enableSSE = false;
		enableSSE2 = false;
		enableSSE3 = false;
		enableSSSE3 = false;
		enableSSE4_1 = false;
	}
}

void CPUID::setEnableCMOV(bool enable)
{
	enableCMOV = enable;

	if(!CMOV)
	{
		enableSSE = false;
		enableSSE2 = false;
		enableSSE3 = false;
		enableSSSE3 = false;
		enableSSE4_1 = false;
	}
}

void CPUID::setEnableSSE(bool enable)
{
	enableSSE = enable;

	if(enableSSE)
	{
		enableMMX = true;
		enableCMOV = true;
	}
	else
	{
		enableSSE2 = false;
		enableSSE3 = false;
		enableSSSE3 = false;
		enableSSE4_1 = false;
	}
}

void CPUID::setEnableSSE2(bool enable)
{
	enableSSE2 = enable;

	if(enableSSE2)
	{
		enableMMX = true;
		enableCMOV = true;
		enableSSE = true;
	}
	else
	{
		enableSSE3 = false;
		enableSSSE3 = false;
		enableSSE4_1 = false;
	}
}

void CPUID::setEnableSSE3(bool enable)
{
	enableSSE3 = enable;

	if(enableSSE3)
	{
		enableMMX = true;
		enableCMOV = true;
		enableSSE = true;
		enableSSE2 = true;
	}
	else
	{
		enableSSSE3 = false;
		enableSSE4_1 = false;
	}
}

void CPUID::setEnableSSSE3(bool enable)
{
	enableSSSE3 = enable;

	if(enableSSSE3)
	{
		enableMMX = true;
		enableCMOV = true;
		enableSSE = true;
		enableSSE2 = true;
		enableSSE3 = true;
	}
	else
	{
		enableSSE4_1 = false;
	}
}

void CPUID::setEnableSSE4_1(bool enable)
{
	enableSSE4_1 = enable;

	if(enableSSE4_1)
	{
		enableMMX = true;
		enableCMOV = true;
		enableSSE = true;
		enableSSE2 = true;
		enableSSE3 = true;
		enableSSSE3 = true;
	}
}

void CPUID::setEnableAVX(bool enable)
{
	enableAVX = enable;

	if(enableAVX)
	{
		enableMMX = true;
		enableCMOV = true;
		enableSSE = true;
		enableSSE2 = true;
		enableSSE3 = true;
		enableSSSE3 = true;
		enableSSE4_1 = true;
	}
}

void CPUID::setEnableF16C(bool enable)
{
	enableF16C = enable;
}

static void cpuid(int eax_ebx_ecx_edx[4], int eax, int ecx = 0)
{
#if defined(__i386__) || defined(__x86_64__)
#	if defined(_WIN32)
	__cpuidex(eax_ebx_ecx_edx, eax, ecx);
#	else
	__asm volatile("cpuid"
	               : "=a"(eax_ebx_ecx_edx[0]), "=b"(eax_ebx_ecx_edx[1]), "=c"(eax_ebx_ecx_edx[2]), "=d"(eax_ebx_ecx_edx[3])
	               : "a"(eax), "c"(ecx));
#	endif
#else
	eax_ebx_ecx_edx[0] = 0;
	eax_ebx_ecx_edx[1] = 0;
	eax_ebx_ecx_edx[2] = 0;
	eax_ebx_ecx_edx[3] = 0;
#endif
}

bool CPUID::detectMMX()
{
	int registers[4];
	cpuid(registers, 1);
	return MMX = (registers[3] & 0x00800000) != 0;
}

bool CPUID::detectCMOV()
{
	int registers[4];
	cpuid(registers, 1);
	return CMOV = (registers[3] & 0x00008000) != 0;
}

bool CPUID::detectSSE()
{
	int registers[4];
	cpuid(registers, 1);
	return SSE = (registers[3] & 0x02000000) != 0;
}

bool CPUID::detectSSE2()
{
	int registers[4];
	cpuid(registers, 1);
	return SSE2 = (registers[3] & 0x04000000) != 0;
}

bool CPUID::detectSSE3()
{
	int registers[4];
	cpuid(registers, 1);
	return SSE3 = (registers[2] & 0x00000001) != 0;
}

bool CPUID::detectSSSE3()
{
	int registers[4];
	cpuid(registers, 1);
	return SSSE3 = (registers[2] & 0x00000200) != 0;
}

bool CPUID::detectSSE4_1()
{
	int registers[4];
	cpuid(registers, 1);
	return SSE4_1 = (registers[2] & 0x00080000) != 0;
}

bool CPUID::detectF16C()
{
	int registers[4];
	cpuid(registers, 1);
	return F16C = (registers[2] & 0x20000000) != 0;
}

bool CPUID::supportsAVX2()
{
	int eax_ebx_ecx_edx[4];
	cpuid(eax_ebx_ecx_edx, 1);
	// Test bits 12 (FMA), 27 (OSXSAVE), and 28 (AVX) of ECX
	bool osxsave_avx_fma = (eax_ebx_ecx_edx[2] & 0x18001000) == 0x18001000;

	// AVX is a prerequisite of AVX2 and must be checked for first according to the Intel Software Developer's Manual.
	// OSXSAVE ensures the operating system can save/restore ymm registers on context switches.
	// FMA support is often considered an integral part of AVX2.
	if(!osxsave_avx_fma)
	{
		return false;
	}

	cpuid(eax_ebx_ecx_edx, 7, 0);  // Valid if AVX is supported
	return (eax_ebx_ecx_edx[1] & 0x00000020) != 0;
}

}  // namespace rr
