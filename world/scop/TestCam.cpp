#include "pch.h"
#include "TestCam.h"
#include <algorithm>
#include "ConstantBuffer.h"
#include "DeferredGraphics.h"

TestCam::TestCam(
	float width, 
	float height, 
	float fov,
	float cam_near, 
	float cam_far
)
{
	this->w_width = width;
	this->w_height = height;
	this->fov = fov;
	this->setDir(vec3(0, 0, 1));
	this->mvp.view = Mat::Identity;
	this->mvp.proj = XMMatrixPerspectiveFovLH(
		XMConvertToRadians(fov),
		width / height,
		cam_near,
		cam_far
	);
	this->cam_near = cam_near;
	this->cam_far = cam_far;
	this->constant_buffer = make_shared<ConstantBuffer>(
		d_graphic->getDevice(),
		d_graphic->getContext(),
		this->mvp
	);
	this->constant_reflection_buffer = make_shared<ConstantBuffer>(
		d_graphic->getDevice(),
		d_graphic->getContext(),
		this->mvp
	);
	this->constant_tmp_buffer = make_shared<ConstantBuffer>(
		d_graphic->getDevice(),
		d_graphic->getContext(),
		this->mvp
	);

	// 원래 반사보다 조금더 위쪽을 반사하여 지형을 수면에서 자를 때 여유분이 생김
	float h = WATER_HEIGHT + 0.1;
	SimpleMath::Plane plane = SimpleMath::Plane(vec3(0, h, 0),
		vec3(0, 1, 0));
	this->reflection_mat = Mat::CreateReflection(plane);
	
	plane = SimpleMath::Plane(vec3(0, WATER_HEIGHT, 0),
		vec3(0, -1, 0));
	this->reflection_cmat = Mat::CreateReflection(plane);
}

void TestCam::movePos(float x, float y, float z)
{
	this->pos = vec3(x, y, z);
}

void TestCam::setDir(vec3 dir)
{
	this->dir = XMVector3Normalize(dir);
}

double TestCam::ndcToDegree(double ndc)
{
	return this->fov / 2 * ndc;
}


void TestCam::onMouseMove(HWND hWnd, int mouse_x, int mouse_y)
{
	float x = mouse_x * 2.0 / this->w_width - 1.0;
	float y = -mouse_y * 2.0 / this->w_height + 1.0;
	if (x < -1.f)
		x = -1.f;
	if (x > 1.f)
		x = 1.f;
	if (y < -1.f)
		y = -1.f;
	if (y > 1.f)
		y = 1.f;
	// 각도로 
	/*{
		double y_degree = ndcToDegree(x);
		double x_degree = ndcToDegree(-y);
		this->dir = vec3::Transform(this->dir,
			Mat::CreateRotationY(XMConvertToRadians(y_degree)) *
			Mat::CreateRotationX(XMConvertToRadians(x_degree)));
	}*/
	
	// 역행렬으로 
	/*{
		vec4 pos = vec4(x, y, 0, 1) * this->cam_near;
		vec4 ppos = vec4(x, y, 1, 1) * this->cam_far;
		Mat r_mat = (this->mvp.view * this->mvp.proj).Invert();
		pos = vec4::Transform(pos, r_mat);
		ppos = vec4::Transform(ppos, r_mat);
		vec4 ddir = ppos - pos;
		this->dir = vec3(ddir.x, ddir.y, ddir.z);
		this->dir.Normalize();
	}*/

	// vec3
	{
		vec3 npos = vec3(x, y, 0);
		vec3 fpos = vec3(x, y, 1);
		Mat invViewProj = (this->mvp.view * this->mvp.proj).Invert();
		npos = vec3::Transform(npos, invViewProj);
		fpos = vec3::Transform(fpos, invViewProj);
		this->dir = fpos - npos;
		this->dir.Normalize();
	}
	float t = this->dir.Cross(vec3(0, 1, 0)).Length();
	if (t < 0.0000001 && t > -0.0000001) {
		if (this->dir.y > 0)
			this->dir = XMVector3Normalize(vec3(0, 1, 0.001));
		else
			this->dir = XMVector3Normalize(vec3(0, -1, 0.001));
	}
	this->mvp.view = XMMatrixLookToLH(this->pos, this->dir, vec3(0, 1, 0));
}

void TestCam::setCursorInClient(HWND hwnd)
{
	RECT clientRect;
	GetClientRect(hwnd, &clientRect);

	// 클라이언트 영역의 중앙 좌표 계산
	int c_x = (clientRect.right - clientRect.left) / 2;
	int c_y = (clientRect.bottom - clientRect.top) / 2;

	POINT pt;
	pt.x = c_x;
	pt.y = c_y;
	ClientToScreen(hwnd, &pt);

	SetCursorPos(pt.x, pt.y);
}

void TestCam::update()
{
	vec3 right_dir = vec3(0, 1, 0).Cross(this->dir);
	vec3 up_dir = this->dir.Cross(right_dir);
	vec3 move_dir = vec3(0, 0, 0);
	if (GetAsyncKeyState('A') & 0x8000)
		move_dir -= vec3(right_dir.x, 0, right_dir.z);
	if (GetAsyncKeyState('D') & 0x8000)
		move_dir += vec3(right_dir.x, 0, right_dir.z);
	if (GetAsyncKeyState('W') & 0x8000)
		move_dir += vec3(this->dir.x, 0, this->dir.z);
	if (GetAsyncKeyState('S') & 0x8000)
		move_dir -= vec3(this->dir.x, 0, this->dir.z);
	if (GetAsyncKeyState('Q') & 0x8000)
		move_dir += vec3(0, 1, 0);
	if (GetAsyncKeyState('E') & 0x8000)
		move_dir -= vec3(0, 1, 0);
	move_dir = XMVector3Normalize(move_dir) * 0.03f;
	this->pos += move_dir;
	this->mvp.view = XMMatrixLookToLH(this->pos, this->dir, vec3(0, 1, 0));
}

void TestCam::update(vec3 const& character_pos, vec3 const& character_dir)
{
	static bool pre_shift = false;
	bool current_shift = GetAsyncKeyState(VK_SHIFT) & 0x8000;
	if (current_shift && !pre_shift)
		this->free_cam ^= 1;
	pre_shift = current_shift;

	if (this->free_cam)
		this->update();
	else {
		vec3 reset_pos = -2 * character_dir + character_pos;
		this->pos.x = reset_pos.x;
		this->pos.z = reset_pos.z;
		this->pos.y = reset_pos.y + 2.5;
		this->mvp.view = XMMatrixLookToLH(this->pos, this->dir, vec3(0, 1, 0));
	}
	MVP tmvp;
	tmvp.view = this->mvp.view.Transpose();
	tmvp.proj = this->mvp.proj.Transpose();
	this->constant_buffer->update(tmvp);

	this->reflection_mvp = this->mvp;

	if (this->pos.y >= WATER_HEIGHT) {
		this->reflection_mvp.view = this->reflection_mat * this->mvp.view;
		under_water = false;
	}
	else {
		this->reflection_mvp.view = this->reflection_cmat * this->mvp.view;
		under_water = true;
	}
	MVP m;
	m.model = this->reflection_mvp.model.Transpose();
	m.view = this->reflection_mvp.view.Transpose();
	m.proj = this->reflection_mvp.proj.Transpose();
	this->constant_reflection_buffer->update(m);
}

void TestCam::setNear(float cam_near)
{
	this->cam_near = cam_near;
	this->mvp.proj = XMMatrixPerspectiveFovLH(
		XMConvertToDegrees(this->fov),
		this->w_width / this->w_height,
		this->cam_near,
		this->cam_far
	);
}

void TestCam::setFar(float cam_far)
{
	this->cam_far = cam_far;
	this->mvp.proj = XMMatrixPerspectiveFovLH(
		XMConvertToDegrees(this->fov),
		this->w_width / this->w_height,
		this->cam_near,
		this->cam_far
	);
}

void TestCam::setFOV(float fov)
{
	this->fov = fov;
	this->mvp.proj = XMMatrixPerspectiveFovLH(
		XMConvertToDegrees(this->fov),
		this->w_width / this->w_height,
		this->cam_near,
		this->cam_far
	);
}

float TestCam::getFOV()
{
	return this->fov;
}

float TestCam::getNear()
{
	return this->cam_near;
}

float TestCam::getFar()
{
	return this->cam_far;
}

void TestCam::setWidth(float w)
{
	this->w_width = w;
}

void TestCam::setHeight(float h)
{
	this->w_height = h;
}

void TestCam::setTmpMVP(Mat const& model, Mat const& view, Mat const& proj)
{
	this->tmp_mvp.model = model;
	this->tmp_mvp.view = view;
	this->tmp_mvp.proj = proj;
}

void TestCam::tmpBufferUpdate(MVP const& mvp)
{
	this->constant_tmp_buffer->update(mvp);
}

MVP TestCam::getMVP(CamType type)
{
	if (type == CamType::NORMAL)
		return this->mvp;
	else if (type == CamType::TEMP)
		return this->tmp_mvp;
	return this->reflection_mvp;
}


shared_ptr<ConstantBuffer>& TestCam::getConstantBuffer(CamType type)
{
	if (type == CamType::NORMAL)
		return this->constant_buffer;
	else if (type == CamType::TEMP)
		return this->constant_tmp_buffer;
	return this->constant_reflection_buffer;
}

Mat TestCam::getReflection()
{
	if (this->pos.y > WATER_HEIGHT)
		return this->reflection_mat;
	return this->reflection_cmat;
}

bool TestCam::getFreeCamFlag()
{
	return this->free_cam;
}

void TestCam::set3rdView()
{
	this->free_cam ^= 1;
}

vec3 TestCam::getPos()
{
	return this->pos;
}

vec3 TestCam::getDir()
{
	return this->dir;
}
