#include "Cam.h"
#include <Global/GlobalFuncs.h>
#include <Global/GlobalVars.h>

Cam::Cam():
	aspectRatio(0.f),
	spd(0.f),
	pos(glm::vec3(0.f)),
	target(glm::vec3(0.f)),
	up(glm::vec3(0.f)),
	defaultAspectRatio(0.f),
	defaultSpd(0.f),
	defaultPos(glm::vec3(0.f)),
	defaultTarget(glm::vec3(0.f)),
	defaultUp(glm::vec3(0.f))
{
}

Cam::Cam(const glm::vec3& pos, const glm::vec3& target, const glm::vec3& up, const float& aspectRatio, const float& spd):
	aspectRatio(aspectRatio),
	spd(spd),
	pos(pos),
	target(target),
	up(up),
	defaultAspectRatio(aspectRatio),
	defaultSpd(spd),
	defaultPos(pos),
	defaultTarget(target),
	defaultUp(up)
{
}

glm::vec3 Cam::CalcFront(const bool& normalised) const{
	return normalised ? glm::normalize(target - pos) : target - pos;
}

glm::vec3 Cam::CalcRight() const{
	return glm::normalize(glm::cross(CalcFront(), up));
}

glm::vec3 Cam::CalcUp() const{
	return glm::normalize(glm::cross(CalcRight(), CalcFront()));
}

glm::mat4 Cam::LookAt() const{
	glm::vec3 vecArr[]{CalcRight(), CalcUp(), -CalcFront()};
	glm::mat4 translation = glm::mat4(1.f), rotation = glm::mat4(1.f);
	for(short i = 0; i < 3; ++i){ //Access elements as mat[col][row] due to column-major order
		translation[3][i] = -pos[i];
		for(short j = 0; j < 3; ++j){
			rotation[i][j] = (vecArr[j])[i];
		}
	}
	return rotation * translation;
}

void Cam::Update(const int& up, const int& down, const int& left, const int& right, const int& front, const int& back){
	const float camSpd = spd * dt;
	float upDown = float(Key(up) - Key(down));
	float leftRight = float(Key(left) - Key(right));
	float frontBack = float(Key(front) - Key(back));

	const glm::vec3&& camFront = CalcFront();
	const glm::vec3&& xzCamFront = glm::vec3(camFront.x, 0.f, camFront.z);
	glm::vec3&& frontBackDir = glm::normalize(glm::dot(camFront, glm::normalize(xzCamFront)) * glm::normalize(xzCamFront));
	frontBackDir.y = 1.f;

	glm::vec3&& change = glm::vec3(frontBack, upDown, frontBack) * frontBackDir + leftRight * -CalcRight() + leftRightMB * camFront;
	if(change != glm::vec3(0.f)){
		change = normalize(change);
	}
	pos += camSpd * change;
	//pos.y = std::max(-100.f + 100.f * Mesh::ReadHeightMap(Mesh::heightMap, pos.x / 500.f, pos.z / 500.f) + 1.f, pos.y);
	target += camSpd * change;

	glm::mat4 yawPitch = glm::rotate(glm::rotate(glm::mat4(1.f), glm::radians(yaw), {0.f, 1.f, 0.f}), glm::radians(pitch), CalcRight());
	target = pos + glm::vec3(yawPitch * glm::vec4(camFront, 0.f));
	this->up = glm::vec3(yawPitch * glm::vec4(this->up, 0.f));
	yaw = pitch = 0.f;
}

void Cam::Reset(){
	aspectRatio = defaultAspectRatio;
	spd = defaultSpd;
	pos = defaultPos;
	target = defaultTarget;
	up = defaultUp;
}

void Cam::ResetAspectRatio(){
	aspectRatio = defaultAspectRatio;
}

void Cam::ResetSpd(){
	spd = defaultSpd;
}

void Cam::ResetPos(){
	pos = defaultPos;
}

void Cam::ResetTarget(){
	target = defaultTarget;
}

void Cam::ResetUp(){
	up = defaultUp;
}

const float& Cam::GetAspectRatio() const{
	return aspectRatio;
}

const float& Cam::GetSpd() const{
	return spd;
}

const glm::vec3& Cam::GetPos() const{
	return pos;
}

const glm::vec3& Cam::GetTarget() const{
	return target;
}

const glm::vec3& Cam::GetUp() const{
	return up;
}

const float& Cam::GetDefaultAspectRatio() const{
	return defaultAspectRatio;
}

const float& Cam::GetDefaultSpd() const{
	return defaultSpd;
}

const glm::vec3& Cam::GetDefaultPos() const{
	return defaultPos;
}

const glm::vec3& Cam::GetDefaultTarget() const{
	return defaultTarget;
}

const glm::vec3& Cam::GetDefaultUp() const{
	return defaultUp;
}

void Cam::SetAspectRatio(const float& aspectRatio){
	this->aspectRatio = aspectRatio;
}

void Cam::SetSpd(const float& spd){
	this->spd = spd;
}

void Cam::SetPos(const glm::vec3& pos){
	this->pos = pos;
}

void Cam::SetTarget(const glm::vec3& target){
	this->target = target;
}

void Cam::SetUp(const glm::vec3& up){
	this->up = up;
}

void Cam::SetDefaultAspectRatio(const float& defaultAspectRatio){
	this->defaultAspectRatio = defaultAspectRatio;
}

void Cam::SetDefaultSpd(const float& defaultSpd){
	this->defaultSpd = defaultSpd;
}

void Cam::SetDefaultPos(const glm::vec3& defaultPos){
	this->defaultPos = defaultPos;
}

void Cam::SetDefaultTarget(const glm::vec3& defaultTarget){
	this->defaultTarget = defaultTarget;
}

void Cam::SetDefaultUp(const glm::vec3& defaultUp){
	this->defaultUp = defaultUp;
}