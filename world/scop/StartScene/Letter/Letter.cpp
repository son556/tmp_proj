#include "pch.h"
#include "Letter.h"
#include "Block.h"
#include "Buffer.h"
#include "DeferredGraphics.h"
#include "Texture.h"

shared_ptr<Texture> Letter::letter_tex;
vector<shared_ptr<Buffer<VertexDefer>>> Letter::v_buffers;
shared_ptr<Buffer<uint32>> Letter::i_buffer;

void Letter::init()
{
	ComPtr<ID3D11Device> const& device = d_graphic->getDevice();
	v_buffers.resize(96);
	vector<VertexDefer> vertices;
	vector<uint32> indices;
	float w = 16.0 / 256.0;
	Block::makeBox(1, vertices, indices);
	for (int i = 0; i < 6; i++) {
		for (int j = 0; j < 16; j++) {
			vertices[0].uv = vec2(w * j, w * (i + 3));
			vertices[1].uv = vec2(w * j, w * (i + 2));
			vertices[2].uv = vec2(w * (j + 1), w * (i + 2));
			vertices[3].uv = vec2(w * (j + 1), w * (i + 3));
			v_buffers[i * 16 + j] = make_shared<Buffer<VertexDefer>>(
				device,
				vertices.data(),
				vertices.size(),
				D3D11_BIND_VERTEX_BUFFER
			);
		}
	}
	i_buffer = make_shared<Buffer<uint32>>(
		device,
		indices.data(),
		indices.size(),
		D3D11_BIND_INDEX_BUFFER
	);

	letter_tex = make_shared<Texture>(
		device,
		L"./textures/start_scene/ascii.png"
	);
}

shared_ptr<Buffer<VertexDefer>> const& Letter::getLetterVertexBuffer(char c)
{
	int idx = static_cast<int>(c - 32);
	if (idx >= v_buffers.size() || idx < 0)
		assert(false);
	return v_buffers[idx];
}


shared_ptr<Buffer<uint32>> const& Letter::getLetterIndexBuffer()
{
	return i_buffer;
}

shared_ptr<Texture> const& Letter::getTexture()
{
	return letter_tex;
}
