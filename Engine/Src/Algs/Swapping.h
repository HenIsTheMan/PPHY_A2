#pragma once

template <class T>
void SwapArrElements(T* const& arr, const int& i1, const int& i2){
	const T temp = arr[i1];
	arr[i1] = arr[i2];
	arr[i2] = temp;
}