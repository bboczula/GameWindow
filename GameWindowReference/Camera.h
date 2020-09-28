#pragma once

#include <iostream>
#include <DirectXMath.h>
using namespace DirectX;

class Camera
{
public:
	Camera(float screenAspectRatio);
	DirectX::XMFLOAT4X4 getDxViewProjectionMatrix();
	void update();
	void strafe(float delta);
	void walk(float delta);
	void setYaw(float angle);
	void setPitch(float angle);
	void setPosition(float x, float y, float z);
	DirectX::XMFLOAT3 getPosition();
protected:
	float aspectRatio;
	float fieldOfView;
	float nearPlane;
	float farPlane;
	float width;
	float heigt;
	float yaw;
	float pitch;
protected:
	DirectX::XMFLOAT3 dxPosition;
	DirectX::XMFLOAT3 dxTarget;
	DirectX::XMFLOAT3 dxUp;
	DirectX::XMFLOAT4 dxRight;
	DirectX::XMFLOAT3 dxForward;
	DirectX::XMFLOAT4 dxDefaultRight;
	DirectX::XMFLOAT3 dxDefaultForward;
	DirectX::XMFLOAT4X4 dxView;
	DirectX::XMFLOAT4X4 dxProjection;
	DirectX::XMFLOAT4X4 viewProjection;
	DirectX::XMMATRIX camRotationMatrix;
};