#include <iostream>
#include<deque>
#include<queue>
#include<vector>
#include<list>
#include<map>
#include<string>

using namespace std;



int main() {
    deque<int> dque;
    queue<int>que;
    list<int> lst;
    int sum = 16;
    int ret = 0;
    for (int i = 0; i < 10; i++) {
        dque.push_back(i);
        dque.push_front(i);
        que.push(i);
        lst.push_back(i);
    }
    int size = dque.size();
    for (int i = 0; i < size; i++) {
        cout << dque[i] << " " ;
    }
    cout << endl;
    
    /*auto i = dque.begin();
    auto j = dque.rbegin();
    for(; i != dque.end(), j != dque.rend(); i++, j++) {
        
        cout << *i <<"; "<<*j << endl;
        
    }*/

    for (int i = 0, j = size - 1; i < size, j >= 0; i++, j--) {
        cout << dque[i] << "; " << dque[j] << endl;
    }
    cout << endl << endl;
    vector<int> vec(dque.begin(), dque.end());
    for (int i = 0, j = size - 1; i < size, j >= 0; i++, j--) {
        cout << vec[i] << "; " << vec[j] << endl;
    }

    cout << "map--------------------------" << endl;
    map<int, int> mp;
    mp.insert(pair<int,int>(1,2));
    for (auto it = mp.begin(); it != mp.end(); it++) {
        cout << it->first << " " << it->second;
        //cout << it->first;
    }
    int min = INT_MAX;

    cout <<endl<<endl<< "string---------------------------" << endl;

    string str = "abcdefg";
    string subStr = str.substr(1, 3);
    cout << "subStr = " << subStr << endl;

    string email = "nowcoder. a am I";
    if (email.find_last_not_of(" "))
        email.append(" ");
    while (!email.empty()) {
        int pos = email.find(" ");
        string username = email.substr(0, pos);
        cout << "username: " << username << endl;
        cout << "email: " << email << endl;
        email.erase(0, pos + 1);
        cout << "email: " << email << endl;
    }
    


	return 0;
}