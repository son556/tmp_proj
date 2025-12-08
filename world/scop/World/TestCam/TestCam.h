#pragma once

class ConstantBuffer;

class TestCam
{
public:
	TestCam(
		float width, 
		float height,
		float fov,
		float cam_near, 
		float cam_far
	);
	~TestCam() = default;
	void movePos(float x, float y, float z);
	void setDir(vec3 dir);
	void onMouseMove(HWND hWnd);
	void update(vec3 const& character_pos, vec3 const& character_dir);
	vec3 getPos();
	vec3 getDir();
	void setCursorInClient(HWND hwnd);
	void setNear(float cam_near);
	void setFar(float cam_far);
	void setFOV(float fov);
	float getFOV();
	float getNear();
	float getFar();
	void setWidth(float w);
	void setHeight(float h);
	void setTmpMVP(
		Mat const& model,
		Mat const& view,
		Mat const& proj
	);
	void tmpBufferUpdate(MVP const& mvp);
	MVP getMVP(CamType type);
	shared_ptr<ConstantBuffer>& getConstantBuffer(CamType type);
	Mat getReflection();

	pair<float, float> getCursorNDCPos(HWND hwnd);

private:
	TestCam() = delete;
	TestCam(TestCam const&) = delete;
	TestCam& operator=(TestCam const) = delete;

private:
	double ndcToDegree(double ndc);

private:
	float w_width;
	float w_height;
	float cam_near;
	float cam_far;
	float fov;
	vec3 pos;
	vec3 dir;
	vec3 up = vec3(0, 1, 0);
	MVP mvp;
	MVP reflection_mvp;
	Mat reflection_mat;
	Mat reflection_cmat;
	MVP tmp_mvp;
	shared_ptr<ConstantBuffer> constant_buffer;
	shared_ptr<ConstantBuffer> constant_reflection_buffer;
	shared_ptr<ConstantBuffer> constant_tmp_buffer;
};

