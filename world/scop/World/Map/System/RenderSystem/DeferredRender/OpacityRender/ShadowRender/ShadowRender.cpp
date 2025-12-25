#include "pch.h"
#include "ShadowRender.h"
#include "MapUtils.h"
#include "DeferredGraphics.h"
#include "DeferredBuffer.h"
#include "RasterizerState.h"
#include "VertexShader.h"
#include "PixelShader.h"
#include "InputLayout.h"
#include "InputLayouts.h"
#include "ConstantBuffer.h"
#include "Chunk.h"
#include "DepthMap.h"
#include "TextureArray.h"
#include "StructuredBuffer.h"
#include "CascadeShadow.h"
#include "Block.h"
#include "Buffer.h"
#include "TestCam.h"
#include "Entity.h"

ShadowRender::ShadowRender(MapUtils* minfo)
{
	this->m_info = minfo;
	this->d_buffer = make_shared<DeferredBuffer>(1);
	this->d_buffer->setRTVsAndSRVs(
		d_graphic->getDevice(),
		this->m_info->width,
		this->m_info->height
	);
	this->rasterizer_state = make_shared<RasterizerState>(
		d_graphic->getDevice(),
		D3D11_FILL_SOLID,
		D3D11_CULL_BACK
	);
	this->s_vertex_shader = make_shared<VertexShader>(
		d_graphic->getDevice(),
		L"shader/World/DeferredShader/Shadow/CSM/ShadowVS.hlsl",
		"main",
		"vs_5_0"
	);
	this->s_pixel_shader = make_shared<PixelShader>(
		d_graphic->getDevice(),
		L"shader/World/DeferredShader/Shadow/CSM/ShadowPS.hlsl",
		"main",
		"ps_5_0"
	);
	this->s_input_layout = make_shared<InputLayout>(
		d_graphic->getDevice(),
		InputLayouts::layout_pns.data(),
		InputLayouts::layout_pns.size(),
		this->s_vertex_shader->getBlob()
	);
	this->vertex_shader = make_shared<VertexShader>(
		d_graphic->getDevice(),
		L"shader/World/DeferredShader/Shadow/CombineShadowVS.hlsl",
		"main",
		"vs_5_0"
	);
	this->pixel_shader = make_shared<PixelShader>(
		d_graphic->getDevice(),
		L"shader/World/DeferredShader/Shadow/CombineShadowPS.hlsl",
		"main",
		"ps_5_0"
	);
	
	vector<VertexDefer> vertices;
	vector<uint32> indices;
	Block::makeBox(1, vertices, indices);
	this->vbuffer = make_shared<Buffer<VertexDefer>>(
		d_graphic->getDevice(),
		vertices.data(),
		vertices.size(),
		D3D11_BIND_VERTEX_BUFFER
	);
	this->ibuffer = make_shared<Buffer<uint32>>(
		d_graphic->getDevice(),
		indices.data(),
		indices.size(),
		D3D11_BIND_INDEX_BUFFER
	);
	this->input_layout = make_shared<InputLayout>(
		d_graphic->getDevice(),
		InputLayouts::layout_pt.data(),
		InputLayouts::layout_pt.size(),
		this->vertex_shader->getBlob()
	);
	vector<ComPtr<ID3D11ShaderResourceView>> srvs_vec;
	for (int i = 0; i < this->split_cnt; i++) {
		this->csms.push_back(
			make_shared<CascadeShadow>(2048, 2048, this->m_info));
		srvs_vec.push_back(this->csms.back()->getSRV());
	}
	this->tex2d_arr = make_shared<TextureArray>(
		d_graphic->getDevice(),
		d_graphic->getContext(),
		srvs_vec,
		DXGI_FORMAT_R32_FLOAT
	);

	this->mvps.resize(this->split_cnt);
	this->structured_buffer = make_shared<StructuredBuffer>(
		d_graphic->getDevice().Get(),
		d_graphic->getContext().Get(),
		static_cast<void *>(this->mvps.data()),
		static_cast<uint32>(sizeof(MVP)),
		static_cast<uint32>(this->mvps.size()), 0, 0
	);
	this->ps_cbuffer = make_shared<ConstantBuffer>(
		d_graphic->getDevice(),
		d_graphic->getContext(),
		this->frustum_split
	);
	this->s_rasterizer_state = make_shared<RasterizerState>(
		d_graphic->getDevice(),
		D3D11_FILL_SOLID,
		D3D11_CULL_BACK
	);
	this->devideFrustum();
}

void ShadowRender::setPipe()
{
	ComPtr<ID3D11DeviceContext> context =
		d_graphic->getContext();
	context->IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	uint32 stride = this->vbuffer->getStride();
	uint32 offset = this->vbuffer->getOffset();
	context->IASetVertexBuffers(
		0, 1, this->vbuffer->getComPtr().GetAddressOf(), &stride, &offset
	);
	context->IASetIndexBuffer(
		this->ibuffer->getComPtr().Get(),
		DXGI_FORMAT_R32_UINT,
		0
	);
	context->IASetInputLayout(this->input_layout->getComPtr().Get());
	context->VSSetShader(
		this->vertex_shader->getComPtr().Get(),
		nullptr,
		0
	);
	context->RSSetState(this->rasterizer_state->getComPtr().Get());
	context->PSSetShader(
		this->pixel_shader->getComPtr().Get(),
		nullptr,
		0
	);
	context->PSSetConstantBuffers(0, 1,
		this->ps_cbuffer->getComPtr().GetAddressOf());
}

void ShadowRender::setCSMPipe()
{
	ComPtr<ID3D11DeviceContext> context =
		d_graphic->getContext();
	context->IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetInputLayout(this->s_input_layout->getComPtr().Get());

	context->VSSetShader(
		this->s_vertex_shader->getComPtr().Get(),
		nullptr,
		0
	);
	context->RSSetState(this->s_rasterizer_state->getComPtr().Get());
	context->PSSetShader(
		this->s_pixel_shader->getComPtr().Get(),
		nullptr,
		0
	);
}

void ShadowRender::devideFrustum() // view space
{
	float p_near = 0.1f;
	float p_far = 280;
	float r = w_width / w_height;
	float fov = cam->getFOV();
	float theta = XMConvertToRadians(fov / 2);

	vector<tuple<float, float, float>> tmp;
	tmp.resize(this->split_cnt + 1);
	float t = 0.99;
	t = 0.6;
	int csm_idx = 0;
	float ss = this->split_cnt;
	for (int i = 0; i < this->split_cnt + 1; i++) {
		float c_log = log(p_near * pow(p_far / p_near, i / ss));
		float c_uni = p_near + (p_far - p_near) * (i / ss);
		float c = t * c_log + (1.0 - t) * c_uni;
		float h = c * tan(theta);
		float w = h * r;

		this->frustum_split.vz_arr[i] = vec4(c, c, c, c);
		if (i == 0) {
			this->csms[csm_idx]->setFrustumVertices(vec3(-w, h, c), 0, 0);
			this->csms[csm_idx]->setFrustumVertices(vec3(w, h, c), 1, 0);
			this->csms[csm_idx]->setFrustumVertices(vec3(w, -h, c), 2, 0);
			this->csms[csm_idx]->setFrustumVertices(vec3(-w, -h, c), 3, 0);
		}
		else if (i == this->split_cnt) {
			this->csms[csm_idx]->setFrustumVertices(vec3(-w, h, c), 0, 1);
			this->csms[csm_idx]->setFrustumVertices(vec3(w, h, c), 1, 1);
			this->csms[csm_idx]->setFrustumVertices(vec3(w, -h, c), 2, 1);
			this->csms[csm_idx]->setFrustumVertices(vec3(-w, -h, c), 3, 1);
		}
		else {
			this->csms[csm_idx]->setFrustumVertices(vec3(-w, h, c), 0, 1);
			this->csms[csm_idx]->setFrustumVertices(vec3(w, h, c), 1, 1);
			this->csms[csm_idx]->setFrustumVertices(vec3(w, -h, c), 2, 1);
			this->csms[csm_idx]->setFrustumVertices(vec3(-w, -h, c), 3, 1);
			csm_idx++;
			this->csms[csm_idx]->setFrustumVertices(vec3(-w, h, c), 0, 0);
			this->csms[csm_idx]->setFrustumVertices(vec3(w, h, c), 1, 0);
			this->csms[csm_idx]->setFrustumVertices(vec3(w, -h, c), 2, 0);
			this->csms[csm_idx]->setFrustumVertices(vec3(-w, -h, c), 3, 0);
		}
	}
	for (int i = this->split_cnt + 1; i < 8; i++)
		this->frustum_split.vz_arr[i] = vec4(p_far, p_far, p_far, p_far);
}

void ShadowRender::render(CamType type)
{
	ComPtr<ID3D11DeviceContext> context =
		d_graphic->getContext();

	d_graphic->renderBegin(this->d_buffer.get());

	// shadow csm
	vec3 ld = this->m_info->light_dir;
	this->frustum_split.light_dir = vec4(ld.x, ld.y, ld.z, this->split_cnt);
	this->frustum_split.view = cam->getMVP(type).view.Transpose();
	this->ps_cbuffer->update(this->frustum_split);


	this->setPipe();
	context->PSSetShaderResources(
		0, 1,
		this->structured_buffer->GetSRV().GetAddressOf()
	);
	for (int i = 1; i <= this->split_cnt; i++) {
		context->PSSetShaderResources(i, 1,
			this->csms[i - 1]->getSRV().GetAddressOf());
	}
	context->DrawIndexed(
		this->ibuffer->getCount(),
		0, 0);
}

void ShadowRender::renderCSM(CamType type)
{
	ComPtr<ID3D11DeviceContext> context =
		d_graphic->getContext();


	// update structured buffer
	for (int i = 0; i < split_cnt; i++) {
		this->csms[i]->updateCBuffer(type);
		this->mvps[i] = this->csms[i]->getMVP();
	}
	this->structured_buffer->CopyToInput(
		static_cast<void*>(this->mvps.data()));

	// render chunk (light 시점)
	this->setCSMPipe();
	for (int k = 0; k < split_cnt; k++) {
		context->VSSetConstantBuffers(0, 1,
			this->csms[k]->getCBuffer()->getComPtr().GetAddressOf());
		d_graphic->renderBegin(this->csms[k]->getDBuffer().get(),
			this->csms[k]->getDSV());
		d_graphic->setViewPort(this->csms[k]->getViewPort());

		// TODO: 나중에 지울 부분
		/*for (int i = 0; i < this->m_info->size_h; i++) {
			for (int j = 0; j < this->m_info->size_w; j++) {
				if (this->m_info->chunks[i][j]->render_flag == false)
					continue;
				this->m_info->chunks[i][j]->setShadowRender(
					d_graphic->getContext()
				);
			}
		}*/
		for (auto chunkIndex : this->m_info->GetRenderableChunkListToRead())
		{
			if (this->m_info->chunks[chunkIndex.y][chunkIndex.x]->render_flag == false)
				continue;
			this->m_info->chunks[chunkIndex.y][chunkIndex.x]->setShadowRender(
				d_graphic->getContext()
			);
		}
	}
}

ComPtr<ID3D11ShaderResourceView> ShadowRender::getSRV()
{
	return this->d_buffer->getSRV(0);
}

