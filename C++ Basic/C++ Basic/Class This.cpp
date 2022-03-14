//#include <iostream>
//
//using namespace std;
//
//class A {
//public:
//	A(int age) {
//		this->age = age;
//	}
//	int age;
//
//	A& ageAdd(A& a) {
//		this->age += a.age;
//
//		return *this;
//	}
//};
//
//int main() {
//
//	A b(10);
//	A c(20);
//	cout << b.age << '\t' << c.age << endl;
//
//	A d = b.ageAdd(c).ageAdd(c);
//	cout << b.age << '\t' << d.age << endl;
//}