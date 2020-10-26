#pragma once

template <class T>
class Singleton{
	explicit Singleton(const Singleton&) = delete;
	explicit Singleton(Singleton&&) noexcept = delete;
	Singleton& operator=(const Singleton&) = delete;
	Singleton& operator=(Singleton&&) noexcept = delete;
public:
	static T* const& GetObjPtr(){
		if(!objPtr){
			objPtr = new T;
		}
		return objPtr;
	}
	static void Destroy(){
		if(objPtr){
			delete objPtr;
			objPtr = nullptr;
		}
	}
protected:
	Singleton() = default;
private:
	static T* objPtr;
};

template <class T>
T* Singleton<T>::objPtr = nullptr;