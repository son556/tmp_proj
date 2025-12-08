#pragma once

template<typename T> class Buffer;
class ConstantBuffer;
struct VertexPTN;

class LeftArm
{
public:
	LeftArm(Mat const& o_pos, Mat const& o_rot);
	~LeftArm() = default;
	shared_ptr<Buffer<VertexPTN>>& getVertexBuffer();
	Mat getWorld();
	void setAnimationFlag();
	void setAnimationFlagWalk();
	void update(Mat const& o_pos, Mat const& o_rot);
	int getAnimationFlag();

private:
	void animation();
	void animationFirstView();
	void animationWalk();
	LeftArm() = delete;
	LeftArm(LeftArm const&) = delete;
	LeftArm& operator=(LeftArm const&) = delete;

private:
	Mat world;
	Mat basic_mat;
	int animation_flag = false;
	bool animation_flag_walk = false;

private:
	vector<VertexPTN> vertices;
	vector<VertexPTN> ani_vertices;
	shared_ptr<Buffer<VertexPTN>> v_buffer;
};

