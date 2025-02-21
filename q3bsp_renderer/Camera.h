#pragma once
#ifndef CAMERA_H
#define CAMERA_H

#define CAM_MOVE_SPEED 10.0f
#define CAM_ROT_SPEED_MOUSE 0.005f
#define CAM_ROT_SPEED_KB 0.05f

class Camera
{
public:
	Camera(D3DXVECTOR3 startPos);
	~Camera();

	float GetYaw() const {return _yaw;}
	float GetPitch() const {return _pitch;}
	float GetRoll() const {return _roll;}
	D3DXVECTOR3 GetPosition() const {return _position;}	
	D3DXVECTOR3 GetLook() const {return _look;}	
	D3DXVECTOR3 GetUp() const {return _up;}	
	D3DXVECTOR3 GetRight() const {return _right;}	

	void MoveForward(float amount) { _position += _look * amount; }
	void MoveRight(float amount) { _position += _right * amount; }
	void MoveUp(float amount) { _position += _up * amount; }

	void Yaw(float amount); // rotate around x axis
	void Pitch(float amount); // rotate around x axis
	void Roll(float amount); // rotate around z axis

	float RestrictAngleTo360(float angle) const;

	void CalculateViewMatrix(D3DXMATRIX *viewMatrix);

private:

	D3DXVECTOR3 _position;      // camera position
	float _yaw;                 // rotation around the y axis
	float _pitch;               // rotation around the x axis
	float _roll;                // rotation around the z axis
	D3DXVECTOR3 _up;			// camera axis
	D3DXVECTOR3 _look;
	D3DXVECTOR3 _right; 
};

#endif