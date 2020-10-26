#pragma once
#include <Core.h>

class Cam final{
public:
	Cam();
	Cam(const glm::vec3& pos, const glm::vec3& target, const glm::vec3& up, const float& aspectRatio, const float& spd);
	~Cam() = default;
	glm::vec3 CalcFront(const bool& normalised = true) const;
	glm::vec3 CalcRight() const;
	glm::vec3 CalcUp() const;
	glm::mat4 LookAt() const;
	void Update(const int& up, const int& down, const int& left, const int& right, const int& front, const int& back);

	void Reset();
	void ResetAspectRatio();
	void ResetSpd();
	void ResetPos();
	void ResetTarget();
	void ResetUp();

	///Getters
	const float& GetAspectRatio() const;
	const float& GetSpd() const;
	const glm::vec3& GetPos() const;
	const glm::vec3& GetTarget() const;
	const glm::vec3& GetUp() const;
	const float& GetDefaultAspectRatio() const;
	const float& GetDefaultSpd() const;
	const glm::vec3& GetDefaultPos() const;
	const glm::vec3& GetDefaultTarget() const;
	const glm::vec3& GetDefaultUp() const;

	///Setters
	void SetAspectRatio(const float& aspectRatio);
	void SetSpd(const float& spd);
	void SetPos(const glm::vec3 & pos);
	void SetTarget(const glm::vec3 & target);
	void SetUp(const glm::vec3 & up);
	void SetDefaultAspectRatio(const float& defaultAspectRatio);
	void SetDefaultSpd(const float& defaultSpd);
	void SetDefaultPos(const glm::vec3 & defaultPos);
	void SetDefaultTarget(const glm::vec3 & defaultTarget);
	void SetDefaultUp(const glm::vec3 & defaultUp);
private:
	float aspectRatio;
	float spd;
	glm::vec3 pos;
	glm::vec3 target;
	glm::vec3 up;

	float defaultAspectRatio;
	float defaultSpd;
	glm::vec3 defaultPos;
	glm::vec3 defaultTarget;
	glm::vec3 defaultUp;
};