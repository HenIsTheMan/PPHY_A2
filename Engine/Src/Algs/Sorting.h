#pragma once
#include "Swapping.h"

enum struct SortingAlgType{
	Bubble = 0,
	Selection,
	Insertion,
	Quick,
	Merge,
	Amt
};

enum struct SortOrder{
	Ascending = 0,
	Descending
};

class SortingAlgs final{
private:
	template <class T>
	friend void SortArr(T* arr, const int& start, const int& end, const SortOrder& order, const SortingAlgType& algType);
	template <class T>
	static void BubbleSortArr(T* arr, const int& start, const int& end, const SortOrder& order);
	template <class T>
	static void SelectionSortArr(T* arr, const int& start, const int& end, const SortOrder& order);
	template <class T>
	static void InsertionSortArr(T* arr, const int& start, const int& end, const SortOrder& order);
	template <class T>
	static void QuickSortArr(T* arr, const int& start, const int& end, const SortOrder& order);
	template <class T>
	static void MergeSortArr(T* arr, const int& start, const int& end, const SortOrder& order);
};

template <class T>
void SortingAlgs::BubbleSortArr(T* arr, const int& start, const int& end, const SortOrder& order){
	for(int alrSorted = 1; alrSorted <= end; ++alrSorted){
		for(int i = start; i <= end - alrSorted; ++i){
			if(bool(order) ? arr[i] < arr[i + 1] : arr[i] > arr[i + 1]){
				SwapArrElements(arr, i, i + 1);
			}
		}
	}
}

template <class T>
void SortingAlgs::SelectionSortArr(T* arr, const int& start, const int& end, const SortOrder& order){
	for(int loc = start; loc <= end; ++loc){
		int currMinMaxLoc = loc;
		for(int i = loc; i <= end; ++i){
			if(bool(order) ? arr[i] > arr[currMinMaxLoc] : arr[i] < arr[currMinMaxLoc]){
				currMinMaxLoc = i;
			}
		}
		SwapArrElements(arr, loc, currMinMaxLoc);
	}
}

template <class T>
void SortingAlgs::InsertionSortArr(T* arr, const int& start, const int& end, const SortOrder& order){
	for(int unsorted1st = start + 1; unsorted1st <= end; ++unsorted1st){
		if(bool(order) ? arr[unsorted1st] > arr[unsorted1st - 1] : arr[unsorted1st] < arr[unsorted1st - 1]){
			const T temp = arr[unsorted1st];
			int loc = unsorted1st;
			while(loc > start && (bool(order) ? arr[loc - 1] < temp : arr[loc - 1] > temp)){
				arr[loc] = arr[loc - 1];
				--loc;
			}
			arr[loc] = temp;
		}
	}
}

template <class T>
void SortingAlgs::QuickSortArr(T* arr, const int& start, const int& end, const SortOrder& order){
	if(start < end){
		const auto&& Partition = [arr, start, end, order](){
			SwapArrElements(arr, start, int(.5f * (start + end))); //Swap start with middle
			const T& pivot = arr[start];
			int smallIndex = start;

			for(int i = start + 1; i <= end; ++i){
				if(bool(order) ? arr[i] > pivot : arr[i] < pivot){
					++smallIndex;
					SwapArrElements(arr, smallIndex, i);
				}
			}
			SwapArrElements(arr, start, smallIndex);
			return smallIndex;
		};
		const int&& pivotLoc = Partition();
		QuickSortArr(arr, start, pivotLoc - 1, order);
		QuickSortArr(arr, pivotLoc + 1, end, order);
	}
}

template <class T>
void SortingAlgs::MergeSortArr(T* arr, const int& start, const int& end, const SortOrder& order){
	if(start < end){
		const int&& middle = int(.5f * (start + end));
		MergeSortArr(arr, start, middle, order);
		MergeSortArr(arr, middle + 1, end, order);

		T* temp = new T[(size_t)middle - (size_t)start + (size_t)1];
		int i = 0; //Index for 1st half of arr
		int j = start; //Index for 2nd half of arr
		int k = start; //Index for whole arr
		for(; j <= middle; ++i, ++j){ //Duplicate 1st half of arr
			temp[i] = arr[j];
		}
		for(i = 0; k < j && j <= end; ){
			arr[k++] = (bool(order) ? temp[i] > arr[j] : temp[i] < arr[j]) ? temp[i++] : arr[j++];
		}
		while(k < j){ //Copy remaining elements (if any)
			arr[k++] = temp[i++];
		}
		delete[] temp;
	}
}

template <class T>
void SortArr(T* arr, const int& start, const int& end, const SortOrder& order, const SortingAlgType& algType){
	switch(algType){
		case SortingAlgType::Bubble:
			SortingAlgs::BubbleSortArr(arr, start, end, order);
			break;
		case SortingAlgType::Selection:
			SortingAlgs::SelectionSortArr(arr, start, end, order);
			break;
		case SortingAlgType::Insertion:
			SortingAlgs::InsertionSortArr(arr, start, end, order);
			break;
		case SortingAlgType::Quick:
			SortingAlgs::QuickSortArr(arr, start, end, order);
			break;
		case SortingAlgType::Merge:
			SortingAlgs::MergeSortArr(arr, start, end, order);
			break;
	}
}