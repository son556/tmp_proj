#pragma once

#include "WorldUtils.h"

class DeferredGraphics;
class VertexShader;
template<typename T> class Buffer;

class TPChunk
{
public:
	TPChunk();
	~TPChunk();
	void render(
		ComPtr<ID3D11DeviceContext> const& context,
		bool flag_up
	);
	void update(
		ComPtr<ID3D11Device> const& device,
		bool flag_up
	);
	void reset();

private:
	TPChunk(TPChunk const&) = delete;
	TPChunk operator=(TPChunk const&) = delete;

public:
	uint32 vertices_idx_up = 0;
	uint32 vertices_idx_down = 0;
	bool render_up_flag = false;
	bool render_down_flag = false;
	vector<VertexColor> vertices_up;
	vector<uint32> indices_up;
	vector<VertexColor> vertices_down;
	vector<uint32> indices_down;

private:
	shared_ptr<Buffer<VertexColor>> tp_up_vbuffer;
	shared_ptr<Buffer<uint32>> tp_up_ibuffer;
	shared_ptr<Buffer<VertexColor>> tp_down_vbuffer;
	shared_ptr<Buffer<uint32>> tp_down_ibuffer;
};

