//#include <iostream>
//#include<vector>
//
//using namespace std;
//
//
//
//int main() {
//
//	vector<int> vec;
//	for (int i = 0; i < 10; i++) {
//		vec.push_back(i);
//		cout << i << " ";
//	}
//	cout << endl;
//	vector<int>::iterator it = find(vec.begin(), vec.end(), 3);
//	int size = it - vec.begin();
//	int rightSize = vec.end() - it -1;
//	cout << size << endl;
//	cout << rightSize << endl;
//	//cout << vec.begin() << endl;
//	//cout << vec.end() << endl;
//	cout << *(vec.begin() + size) << endl;
//	vector<int> tmp(vec.begin()+size+1, vec.end()-1);
//	for (int i = 0; i < tmp.size(); i++) {
//		cout << tmp[i] << " ";
//	}
//	return 0;
//}