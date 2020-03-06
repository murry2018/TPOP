#include <iostream>
#include "csv.hpp"
using namespace std;
// Csvtest main: Csv 클래스를 테스트함
int main()
{
    string line;
    Csv csv;
    while(csv.getline(line) != 0) {
        cout << "line = '" << line << "'\n";
        for (int i = 0; i < csv.getnfield(); i++)
            cout << "field [" << i << "] = '"
                 << csv.getfield(i) << "'\n";
    }
    return 0;
}
