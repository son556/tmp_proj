#pragma once


#include "TPChunk.h"
#include "WaterChunk.h"

struct Chunk
{
public: // create
	Chunk();
	~Chunk();
	void setGeoRender(
		ComPtr<ID3D11DeviceContext> const& context
	);
	void setShadowRender(
		ComPtr<ID3D11DeviceContext> const& context
	);
	void createGeoBuffer(
		ComPtr<ID3D11Device> const& device,
		vector<VertexGeo> const& vertices,
		vector<uint32> const& indices
	);
	void createShadowBuffer(
		ComPtr<ID3D11Device> const& device,
		vector<VertexShadow> const& vertices,
		vector<uint32> const& indices
	);
	void reset();
	void setPos(Index2 const& c_pos);

public:
	int16 max_h = -1;
	vec3 start_pos; // 0 0 0 번째 블록의 가운데 위치
	Index2 chunk_pos; // 청크의 시작위치
	uint32 vertices_idx;
	uint32 shadow_idx;
	bool render_flag;
	TPChunk tp_chunk; // 반투명 처리
	WaterChunk w_chunk; // 물 청크

private:
	shared_ptr<Buffer<VertexGeo>> geo_vbuffer;
	shared_ptr<Buffer<uint32>> geo_ibuffer;
	shared_ptr<Buffer<VertexShadow>> shadow_vbuffer;
	shared_ptr<Buffer<uint32>> shadow_ibuffer;
};

