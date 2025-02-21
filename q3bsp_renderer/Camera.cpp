#include <d3dx9.h>
#include "Camera.h"

Camera::Camera(D3DXVECTOR3 startPos) : _position(startPos), _yaw(0), _pitch(0), _roll(0)
{
	_up = D3DXVECTOR3(0.0f,1.0f,0.0f);
	_look = D3DXVECTOR3(0.0f,0.0f,1.0f);
	_right = D3DXVECTOR3(1.0f,0.0f,0.0f);
}

Camera::~Camera()
{

}

void Camera::CalculateViewMatrix(D3DXMATRIX *viewMatrix)
{
	_up = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	_look = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
	_right = D3DXVECTOR3(1.0f, 0.0f, 0.0f);

	D3DXMATRIX yawMatrix;
	D3DXMatrixRotationAxis(&yawMatrix, &_up, _yaw);

	D3DXVec3TransformCoord(&_look, &_look, &yawMatrix); 
	D3DXVec3TransformCoord(&_right, &_right, &yawMatrix); 

	D3DXMATRIX pitchMatrix;
	D3DXMatrixRotationAxis(&pitchMatrix, &_right, _pitch);

	D3DXVec3TransformCoord(&_look, &_look, &pitchMatrix); 
	D3DXVec3TransformCoord(&_up, &_up, &pitchMatrix); 
		
	D3DXMATRIX rollMatrix;
	D3DXMatrixRotationAxis(&rollMatrix, &_look, _roll);

	D3DXVec3TransformCoord(&_right, &_right, &rollMatrix); 
	D3DXVec3TransformCoord(&_up, &_up, &rollMatrix); 
	
	D3DXMatrixIdentity(viewMatrix);

	viewMatrix->_11 = _right.x; viewMatrix->_12 = _up.x; viewMatrix->_13 = _look.x;
	viewMatrix->_21 = _right.y; viewMatrix->_22 = _up.y; viewMatrix->_23 = _look.y;
	viewMatrix->_31 = _right.z; viewMatrix->_32 = _up.z; viewMatrix->_33 = _look.z;
	
	viewMatrix->_41 = - D3DXVec3Dot( &_position, &_right ); 
	viewMatrix->_42 = - D3DXVec3Dot( &_position, &_up );
	viewMatrix->_43 = - D3DXVec3Dot( &_position, &_look );
}

// Yaw - rotation around y axis
void Camera::Yaw(float amount) 
{
	_yaw += amount;
	_yaw=RestrictAngleTo360(_yaw);
}	

// Pitch - rotation around x axis
void Camera::Pitch(float amount)
{
	_pitch += amount;
	_pitch = RestrictAngleTo360(_pitch);
}

// Roll - rotation around z axis
void Camera::Roll(float amount) 
{
	_roll += amount;
	_roll = RestrictAngleTo360(_roll);
}

// Keep the angle in the range 0 to 360 (2*PI)
float Camera::RestrictAngleTo360(float angle) const
{
	while(angle > 2 * D3DX_PI)
		angle -= 2 * D3DX_PI;

	while(angle < 0)
		angle += 2 * D3DX_PI;

	return angle;
}