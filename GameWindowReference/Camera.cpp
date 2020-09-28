#include "Camera.h"

Camera::Camera(float screenAspectRatio)
	: aspectRatio(screenAspectRatio), fieldOfView(39.6), nearPlane(0.01f), farPlane(1000.0f), width(1.0f), heigt(1.0f), dxPosition(0.0f, 1.6f, 0.00f), dxTarget(0.0f, 0.0f, 0.0f), dxUp(0.0f, 1.0f, 0.0f), dxRight(1.0f, 0.0f, 0.0f, 0.0f), dxForward(0.0f, 0.0f, 1.0f), dxDefaultRight(1.0f, 0.0f, 0.0f, 0.0f), dxDefaultForward(0.0f, 0.0f, 1.0f), yaw(0.0f), pitch(0.0f)
{
	// Update View Matrix
	//update();

#ifdef DEBUG
	std::cout << view << std::endl;
	std::cout << "( " << dxView._11 << " " << dxView._12 << " " << dxView._13 << " " << dxView._14 << " )" << std::endl;
	std::cout << "( " << dxView._21 << " " << dxView._22 << " " << dxView._23 << " " << dxView._24 << " )" << std::endl;
	std::cout << "( " << dxView._31 << " " << dxView._32 << " " << dxView._33 << " " << dxView._34 << " )" << std::endl;
	std::cout << "( " << dxView._41 << " " << dxView._42 << " " << dxView._43 << " " << dxView._44 << " )" << std::endl;
#endif // DEBUG

	// Update Projection Matrix
	DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(fieldOfView), aspectRatio, nearPlane, farPlane);
	DirectX::XMStoreFloat4x4(&dxProjection, projectionMatrix);

#ifdef DEBUG
	std::cout << projection << std::endl;
	std::cout << "( " << dxProjection._11 << " " << dxProjection._12 << " " << dxProjection._13 << " " << dxProjection._14 << " )" << std::endl;
	std::cout << "( " << dxProjection._21 << " " << dxProjection._22 << " " << dxProjection._23 << " " << dxProjection._24 << " )" << std::endl;
	std::cout << "( " << dxProjection._31 << " " << dxProjection._32 << " " << dxProjection._33 << " " << dxProjection._34 << " )" << std::endl;
	std::cout << "( " << dxProjection._41 << " " << dxProjection._42 << " " << dxProjection._43 << " " << dxProjection._44 << " )" << std::endl;
#endif // DEBUG
}

DirectX::XMFLOAT4X4 Camera::getDxViewProjectionMatrix()
{
	DirectX::XMMATRIX viewMatrix = DirectX::XMLoadFloat4x4(&dxView);
	DirectX::XMMATRIX projectionMatrix = DirectX::XMLoadFloat4x4(&dxProjection);
	DirectX::XMMATRIX temp = viewMatrix * projectionMatrix;
	DirectX::XMStoreFloat4x4(&viewProjection, temp);

	return viewProjection;
}

void Camera::update()
{
	camRotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(pitch, yaw, 0.0f);

	DirectX::XMVECTOR targetVector = DirectX::XMLoadFloat3(&dxTarget);
	DirectX::XMVECTOR tempDefaultForward = DirectX::XMLoadFloat3(&dxDefaultForward);
	targetVector = XMVector3TransformCoord(tempDefaultForward, camRotationMatrix);
	targetVector = DirectX::XMVector3Normalize(targetVector);

	XMMATRIX RotateYTempMatrix;
	RotateYTempMatrix = XMMatrixRotationY(yaw);

	DirectX::XMVECTOR tempDefaultRight = DirectX::XMLoadFloat4(&dxDefaultRight);
	DirectX::XMVECTOR tempRight = DirectX::XMVector3TransformCoord(tempDefaultRight, RotateYTempMatrix);

	DirectX::XMVECTOR tempUp = DirectX::XMLoadFloat3(&dxUp);
	tempUp = XMVector3TransformCoord(tempUp, RotateYTempMatrix);


	DirectX::XMVECTOR tempForward = XMVector3TransformCoord(tempDefaultForward, RotateYTempMatrix);

	// Store Modified Vectors
	DirectX::XMStoreFloat3(&dxUp, tempUp);
	DirectX::XMStoreFloat4(&dxRight, tempRight);
	DirectX::XMStoreFloat3(&dxForward, tempForward);

	DirectX::XMVECTOR positionVector = DirectX::XMLoadFloat3(&dxPosition);
	targetVector = positionVector + targetVector;
	DirectX::XMStoreFloat3(&dxTarget, targetVector);

	// Update Target


	DirectX::XMVECTOR upVector = DirectX::XMLoadFloat3(&dxUp);
	DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixLookAtLH(positionVector, targetVector, upVector);
	DirectX::XMStoreFloat4x4(&dxView, viewMatrix);
}

void Camera::strafe(float delta)
{
	DirectX::XMVECTOR tempPosition = DirectX::XMLoadFloat3(&dxPosition);
	DirectX::XMVECTOR tempRight = DirectX::XMLoadFloat4(&dxRight);
	tempPosition += tempRight * delta;
	DirectX::XMStoreFloat3(&dxPosition, tempPosition);
}

void Camera::walk(float delta)
{
	DirectX::XMVECTOR tempPosition = DirectX::XMLoadFloat3(&dxPosition);
	DirectX::XMVECTOR tempForward = DirectX::XMLoadFloat3(&dxForward);
	tempPosition += tempForward * delta;
	DirectX::XMStoreFloat3(&dxPosition, tempPosition);
}

void Camera::setYaw(float angle)
{
	yaw += angle;
}

void Camera::setPitch(float angle)
{
	pitch += angle;
}

void Camera::setPosition(float x, float y, float z)
{
	dxPosition = DirectX::XMFLOAT3(x, y, z);
}

DirectX::XMFLOAT3 Camera::getPosition()
{
	return dxPosition;
}
