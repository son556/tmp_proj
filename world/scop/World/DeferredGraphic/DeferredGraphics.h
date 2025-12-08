#pragma once


class DeferredBuffer;

class DeferredGraphics
{
public:
	DeferredGraphics(
		HWND hwnd, 
		UINT width, 
		UINT height
	);
	~DeferredGraphics();
	void setViewPort(D3D11_VIEWPORT const& view_port);
	void renderBegin(
		DeferredBuffer* d_buffer = nullptr,
		ComPtr<ID3D11DepthStencilView> dsv = nullptr,
		bool reset_rtv = true,
		bool reset_dsv = true
	);
	void renderBegin(int cnt,
		ID3D11RenderTargetView** rtvs,
		ComPtr<ID3D11DepthStencilView> dsv,
		bool reset_render_target,
		bool reset_depth_stencil_view
	);
	void renderEnd();
	ComPtr<ID3D11Device> getDevice();
	ComPtr<ID3D11DeviceContext> getContext();
	ComPtr<ID3D11DepthStencilView> getDSV();

private:
	DeferredGraphics() = delete;
	DeferredGraphics(DeferredGraphics const&) = delete;
	DeferredGraphics& operator=(DeferredGraphics const&) = delete;

private:
	void createSwapChainAndDevice();
	void createRTV();
	void createDepthStencilView();
	void setViewPort();

private:
	HWND hWnd;
	UINT width;
	UINT height;
	D3D11_VIEWPORT view_port = {0, };
	ComPtr<IDXGISwapChain> swap_chain;
	ComPtr<ID3D11Device> device;
	ComPtr<ID3D11DeviceContext> context;
	float clear_color[4] = { 0.f, 0.f, 0.f, 1.f };
	ComPtr<ID3D11DepthStencilView> DSV;
	ComPtr<ID3D11RenderTargetView> defer_rtv;
};

