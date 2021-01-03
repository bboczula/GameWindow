#pragma once

#include <iostream>
#include <DirectXMath.h>
using namespace DirectX;

class Camera
{
public:
	Camera(float screenAspectRatio);
	DirectX::XMFLOAT4X4 getDxViewProjectionMatrix();
	void setPosition(float x, float y, float z);
protected:
	float aspectRatio;
	float fieldOfView;
	float nearPlane;
	float farPlane;
protected:
	DirectX::XMFLOAT3 dxPosition;
	DirectX::XMFLOAT3 dxTarget;
	DirectX::XMFLOAT3 dxUp;
	DirectX::XMFLOAT4X4 dxView;
	DirectX::XMFLOAT4X4 dxProjection;
	DirectX::XMFLOAT4X4 viewProjection;
};