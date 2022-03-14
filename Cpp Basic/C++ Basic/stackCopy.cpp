//#include <iostream>
//#include<vector>
//#include<stack>
//
//using namespace std;
//
//
//
//int main() {
//    stack<int> st;
//    vector<int> tmp;
//    int sum = 16;
//    int ret = 0;
//    for (int i = 0; i < 10; i++) {
//        st.push(i);
//        sum = 15;
//        tmp.clear();
//        while (!st.empty()) {
//            int root = st.top();
//            tmp.push_back(root);
//            sum -= root;
//            if (sum == 0)
//                ret++;
//            st.pop();
//        }
//        for (auto it = tmp.rbegin(); it != tmp.rend(); it++) {
//            cout << *it << " ";
//            st.push(*it);
//        }
//        cout <<"以上为第"<<i<<"次"<< endl;
//    }
//    
//    cout <<endl<< "ret " << ret << endl;
//
//	return 0;
//}