#pragma once
// utils wictexture위해 추가
#include <InitGuid.h>
#include <dinput.h>
#include <dsound.h>

#include <windows.h>
#include <math.h>
#include "DirectXMath.h"
#include "SimpleMath.h"
#include "WICTextureLoader11.h"

using int8 = __int8;
using int16 = __int16;
using int32 = __int32;
using int64 = __int64;

using uint8 = unsigned __int8;
using uint16 = unsigned __int16;
using uint32 = unsigned __int32;
using uint64 = unsigned __int64;

using vec2 = DirectX::SimpleMath::Vector2;
using vec3 = DirectX::SimpleMath::Vector3;
using vec4 = DirectX::SimpleMath::Vector4;
using color = DirectX::XMFLOAT4;

using Mat = DirectX::SimpleMath::Matrix;

struct Vertex {
	vec3 pos;
	vec4 color;
};

struct VertexUV {
	vec3 pos;
	vec2 uv;
};

struct MVP 
{
	Mat model = Mat::Identity;
	Mat view = Mat::Identity;
	Mat proj = Mat::Identity;
};

#include <map>
#include <vector>
#include <string>
#include <stack>
#include <iostream>
#include <fstream>
#include <tuple>
#include <utility>
#include <set>
#include <list>
#include <queue>

using namespace std;

#include <assert.h>


#include <wrl.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXTex/DirectXTex.h>
#include <DirectXTex/DirectXTex.inl>
using namespace DirectX;
using namespace Microsoft::WRL;

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(linker, "/entry:wWinMainCRTStartup /subsystem:console")

#ifdef _DEBUG
#pragma comment(lib, "DirectXTex\\DirectXTex_debug.lib")
#else
#pragma comment(lib, "DirectXTex\\DirectXTex.lib")
#endif

#define CHECK(p)	assert(SUCCEEDED(p))

#define WATER_HEIGHT 15
#define WATER_SPEED 0.02

enum BlockType {
	WATER = -5,
	GLASS,
	TRANSPARENCY_RED,
	TRANSPARENCY_GREEN,
	TRANSPARENCY_BLUE,
	AIR = 0,
	GRASS,
	OAK_LOG,
	OAK_LEAVES,
	BLACK_CONCRETE,
	BLUE_CONCRETE,
	BROWN_CONCRETE,
	CYAN_CONCRETE,
	GRAY_CONCRETE,
	GREEN_CONCRETE,
	LIGHT_BLUE_CONCRETE,
	LIGHT_GRAY_CONCRETE,
	LIME_CONCRETE,
	MAGENTA_CONCRETE,
	ORANGE_CONCRETE,
	PINK_CONCRETE,
	PURPLE_CONCRETE,
	RED_CONCRETE,
	WHITE_CONCRETE,
	YELLOW_CONCRETE,
};

static const vector<vector<int>> blockOffset = {
	//top, side, bottom
	{	0,	0,	0}, // grass
	{	1,	2,	1}, // oak log
	{	3,	3,	3}, // oak leaves
	{	4,	4,	4}, // black concrete
	{	5,	5,	5}, // blue concrete
	{	6,	6,	6}, // brown concrete
	{	7,	7,	7}, // cyan concrete
	{	8,	8,	8}, // gray concrete
	{	9,	9,	9}, // green concrete
	{	10,	10,	10}, // light blue concrete
	{	11,	11,	11}, // light gray concrete
	{	12,	12,	12}, // lime concrete
	{	13,	13,	13}, // magenta concrete
	{	14,	14,	14}, // orange concrete
	{	15,	15,	15}, // pink concrete
	{	16,	16,	16}, // purple concrete
	{	17,	17,	17}, // red concrete
	{	18,	18,	18}, // white concrete
	{	19,	19,	19}, // yellow concrete
	{	20, 20, 20} // glass
};

enum class CamType {
	NORMAL,
	REFLECTION_XZ,
};

extern float sun_radian;

class TestCam;
class DeferredGraphics;
class Entity;
class Terrain;

extern shared_ptr<TestCam> cam;
extern shared_ptr<DeferredGraphics> d_graphic;
extern shared_ptr<Entity> entity;
extern Terrain* p_terrain;

extern HWND hWnd;
extern bool under_water;
extern float delta_time;
extern bool in_water;
extern int w_width;
extern int w_height;
extern bool first_view;


inline void printVec3(string const& name, vec3 const& v) {
	cout << name << " : " << v.x << ' ' << v.y << ' ' << v.z << endl;
}

inline void printVec4(string const& name, vec4 const& v) {
	cout << name << " : " << v.x << ' ' << v.y << ' ' << v.z << endl;
}

static bool test_flag = false;