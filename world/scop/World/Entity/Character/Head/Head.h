#pragma once

template<typename T> class Buffer;
class ConstantBuffer;
struct VertexPTN;

class Head
{
public:
	Head(Mat const& o_pos, Mat const& o_rot);
	~Head() = default;
	shared_ptr<Buffer<VertexPTN>>& getVertexBuffer();
	Mat getWorld();
	void update(Mat const& o_pos, Mat const& o_rot);

private:
	Head() = delete;
	Head(Head const&) = delete;
	Head& operator=(Head const&) = delete;

private:
	Mat world;
	Mat basic_mat;

private:
	shared_ptr<Buffer<VertexPTN>> v_buffer;
};

