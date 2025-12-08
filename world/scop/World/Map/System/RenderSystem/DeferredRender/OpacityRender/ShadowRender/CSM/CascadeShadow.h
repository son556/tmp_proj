#pragma once

class DeferredGraphics;
class DepthMap;
class ConstantBuffer;
class MapUtils;
class DeferredBuffer;

class CascadeShadow
{
public:
	CascadeShadow(
		UINT width, UINT height, MapUtils* m_info);
	ComPtr<ID3D11ShaderResourceView> getSRV();
	ComPtr<ID3D11DepthStencilView> getDSV();
	void setFrustumVertices(
		vec3 const& coord,
		int vertex_idx,
		int frustum_idx
	);
	void updateCBuffer(CamType type);
	shared_ptr<ConstantBuffer> getCBuffer();
	MVP const& getMVP();
	D3D11_VIEWPORT getViewPort();
	shared_ptr<DeferredBuffer> getDBuffer();
	~CascadeShadow() = default;

private:
	CascadeShadow() = delete;
	CascadeShadow(CascadeShadow const&) = delete;
	CascadeShadow& operator=(CascadeShadow const&) = delete;

private:
	MapUtils* m_info;
	shared_ptr<DeferredBuffer> d_buffer;
	shared_ptr<DepthMap> depth_buffer;
	float frustum_vertices[2][4][3];
	shared_ptr<ConstantBuffer> cbuffer;
	MVP mvp;
	float width;
	float height;
};

