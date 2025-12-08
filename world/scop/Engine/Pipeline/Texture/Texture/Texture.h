#pragma once
class Texture
{
public:
	Texture(
		ComPtr<ID3D11Device> device,
		wstring const& path
	);
	Texture(
		ComPtr<ID3D11Device>& device,
		ComPtr<ID3D11DeviceContext>& context,
		string const& path,
		int mip_level
	);
	Texture(
		ComPtr<ID3D11Device>& device,
		const wchar_t* filename,
		bool isCubeMap
	);
	~Texture() = default;
	ComPtr<ID3D11ShaderResourceView> getComPtr() const;
	
private:
	Texture(Texture const&) = delete;
	Texture& operator=(Texture const&) = delete;

private:
	ComPtr<ID3D11Device> device;
	ComPtr<ID3D11ShaderResourceView> sharder_resource_view;
};

