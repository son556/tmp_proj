#pragma once


#include "WorldUtils.h"
template<typename T> class Buffer;
class DeferredGraphics;

class Sun
{
public:
	Sun(UINT radius);
	~Sun() = default;
	shared_ptr<Buffer<VertexColor>> getVertexBuffer();
	shared_ptr<Buffer<uint32>> getIndexBuffer();

private:
	Sun() = delete;
	Sun(Sun const&) = delete;
	Sun& operator=(Sun const&) = delete;

private:
	shared_ptr<Buffer<VertexColor>> vbuffer;
	shared_ptr<Buffer<uint32>> ibuffer;
};

