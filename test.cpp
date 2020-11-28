#include <iostream>
#include <map>
#include <vector>

using namespace std;

int main()
{
    map<int, vector<int>> turnaroundTimes;
    turnaroundTimes[1].push_back(1);
    turnaroundTimes[1].push_back(2000);

    cout << turnaroundTimes[1][0] << endl;
    cout << turnaroundTimes[1][1] << endl;
}