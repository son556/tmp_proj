#pragma once


template<typename Data>
class Buffer
{
public:
	Buffer(
		ComPtr<ID3D11Device> device,
		Data const* data_array,
		uint32 count,
		UINT bind_flag,
		bool dynamic_flag = false
	)
		: device(device)
	{
		this->stride = sizeof(Data);
		this->count = count;
		
		D3D11_BUFFER_DESC desc = {};
		desc.ByteWidth = this->stride * this->count;
		if (dynamic_flag == false)
			desc.Usage = D3D11_USAGE_IMMUTABLE;
		else {
			this->dynamic_flag = true;
			desc.Usage = D3D11_USAGE_DYNAMIC;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		}
		desc.BindFlags = bind_flag;

		D3D11_SUBRESOURCE_DATA data = {};
		data.pSysMem = data_array;
		HRESULT hr = this->device->CreateBuffer(
			&desc,
			&data,
			this->buffer.GetAddressOf()
		);
		CHECK(hr);
	}

	void update(
		const vector<Data>& resource,
		ComPtr<ID3D11DeviceContext> context
		) {
		if (this->dynamic_flag == false)
			return;
		D3D11_MAPPED_SUBRESOURCE subresource;
		ZeroMemory(&subresource, sizeof(subresource));
		HRESULT hr = context->Map(
			this->buffer.Get(),
			0,
			D3D11_MAP_WRITE_DISCARD,
			0,
			&subresource
		);
		CHECK(hr);
		void* test_ptr = memcpy(subresource.pData, resource.data(),
			sizeof(Data) * resource.size());
		if (!(test_ptr))
			cout << "constant buffer memcpy error" << endl;
		context->Unmap(
			this->buffer.Get(),
			0
		);
	}
	Buffer(Buffer const&) = delete;
	~Buffer() {};
	Buffer& operator=(Buffer const&) = delete;
	ComPtr<ID3D11Buffer> getComPtr() const { return this->buffer; }
	uint32 getStride() const { return this->stride; }
	uint32 getOffset() const { return this->offset; }
	uint32 getCount() const { return this->count; }
private:
	ComPtr<ID3D11Device> device;
	ComPtr<ID3D11Buffer> buffer;
	uint32 stride = 0;
	uint32 offset = 0;
	uint32 count = 0;
	bool dynamic_flag = false;
};