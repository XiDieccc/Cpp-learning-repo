//#include <iostream>
//
//using namespace std;
//
////void swap(int arr[], int i, int j) {
////	arr[i] = arr[i] ^ arr[j];
////	arr[j] = arr[i] ^ arr[j];
////	arr[i] = arr[i] ^ arr[j];
////}
//
//void eorSolution(int arr[], int size) {
//	int eor = 0,eor1 = 0;
//	for (int i = 0; i < size; i++) {
//		eor ^= arr[i];
//	}
//	int rightOne = eor & (~eor + 1);
//	for (int i = 0; i < size; i++) {
//		if ((arr[i] & rightOne) == 0) {
//			eor1 ^= arr[i];
//		}
//	}
//	cout << eor1 << " " << (eor ^ eor1) << endl;
//}
//
//int main() {
//	//int arr[2]{ 1,2 };
//	//swap(arr, 0, 0);
//	//cout << arr[0] << " " << arr[1] << endl;
//
//	///*char x = 'x';
//	//char y = 'y';*/
//	//wchar_t x = 1;
//	//wchar_t y = 0;
//	//x = x ^ y;
//	//y = x ^ y;
//	//x = x ^ y;
//	//cout << x << " " << y << endl;
//	int arr[] = { 1,1,1,2,3,3,4,4,5,5 };
//	eorSolution(arr,sizeof(arr)/sizeof(int));
//	
//
//		return 0;
//}