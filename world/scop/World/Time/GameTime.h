#pragma once

class Time
{
public:
	static void initialize();
	static void update();
	__forceinline static float DeltaTime() { return DeltaTimeValue; }


private:
	static LARGE_INTEGER CpuFrequency;
	static LARGE_INTEGER PrevFrequency;
	static LARGE_INTEGER CurrentFrequency;
	static float DeltaTimeValue;
};

