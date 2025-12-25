#pragma once

#include "WorldUtils.h"

namespace TestPrint
{
	inline void PrintIndex3(Index3 idx3)
	{
		cout << "x: " << idx3.x << ", y: " << idx3.y << ", z: " << idx3.z << endl;
	}

	inline void PrintIndex2(Index2 idx2)
	{
		cout << "x: " << idx2.x << ", y: " << idx2.y << endl;
	}

	inline void PrintWidx(const WorldIndex& widx)
	{
		cout << "chunk idx {" << " x: " << widx.c_idx.x << " z: " << widx.c_idx.y << " }" << endl;
		cout << "block idx { " << " x: " << widx.b_idx.x << " y: " << widx.b_idx.y << " z: " << widx.b_idx.z << " }" << endl << endl;
	}
}

