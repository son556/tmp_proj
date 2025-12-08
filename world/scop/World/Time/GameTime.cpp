#include "pch.h"
#include "GameTime.h"

LARGE_INTEGER Time::CpuFrequency = {};
LARGE_INTEGER Time::PrevFrequency = {};
LARGE_INTEGER Time::CurrentFrequency = {};
float Time::DeltaTimeValue = 0.0f;

void Time::initialize()
{
	// Cpu 고유 진동수
	QueryPerformanceFrequency(&CpuFrequency);

	// 프로그램이 시작 했을 때 현재 진동수
	QueryPerformanceCounter(&PrevFrequency);
}

void Time::update()
{
	QueryPerformanceCounter(&CurrentFrequency);
	float differenceFrequency
		= static_cast<float>(CurrentFrequency.QuadPart 
			- PrevFrequency.QuadPart);
	DeltaTimeValue = 
		differenceFrequency / static_cast<float>(CpuFrequency.QuadPart);
	PrevFrequency.QuadPart = CurrentFrequency.QuadPart;
}

