#pragma once

struct VertexWater;
class DeferredGraphics;
template<typename T> class Buffer;

class WaterChunk
{
public:
	WaterChunk();
	~WaterChunk();
	void update(ComPtr<ID3D11Device> const& device);
	void setBuffer(ComPtr<ID3D11DeviceContext> const& context);
	void reset();

public:
	uint32 vertices_idx = 0;
	vector<VertexWater> vertices;
	vector<uint32> indices;
	bool render_flag = 0;

private:
	shared_ptr<Buffer<VertexWater>> v_buffer;
	shared_ptr<Buffer<uint32>> i_buffer;
};

