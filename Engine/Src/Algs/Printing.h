#pragma once
#include <iostream>

enum struct PrintFormat{
	ListHoriz = 0,
	ListVert,
	CSListHoriz,
	CSListVert,
	Amt
};

template <class T>
void PrintArrElements(T* arr, const int& start, const int& end, const PrintFormat& format){
	for(int i = start; i <= end; ++i){
		switch(format){
			case PrintFormat::ListHoriz:
				std::cout << arr[i] << (i == end ? "\n" : " ");
				break;
			case PrintFormat::ListVert:
				std::cout << arr[i] << "\n";
				break;
			case PrintFormat::CSListHoriz:
				std::cout << arr[i] << (i == end ? "\n" : ", ");
				break;
			case PrintFormat::CSListVert:
				std::cout << arr[i] << (i == end ? "" : ",") << std::endl;
				break;
		}
	}
}