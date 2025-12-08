#pragma once

class Texture;
template <typename T> class Buffer;
struct VertexDefer;

class Letter
{
public:
	static void init();
	static shared_ptr<Buffer<VertexDefer>> const& getLetterVertexBuffer(char c);
	static shared_ptr<Buffer<uint32>> const& getLetterIndexBuffer();
	static shared_ptr<Texture> const& getTexture();

private:
	static shared_ptr<Texture> letter_tex;
	static vector<shared_ptr<Buffer<VertexDefer>>> v_buffers;
	static shared_ptr<Buffer<uint32>> i_buffer;
};

