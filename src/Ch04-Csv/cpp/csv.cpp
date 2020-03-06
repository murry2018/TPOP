#include "csv.hpp"

using namespace std;

// getline: 한 줄을 입력받고 필요에 따라 늘린다.
int Csv::getline(string& str)
{
    char c;
    for (line = ""; fin.get(c) && !endofline(c); )
        line += c;
    split();
    str = line;
    return !fin.eof();
}

// endofline: \r, \n, \r\n, EOF를 확인하고 제거함
int Csv::endofline(char c)
{
    int eol;

    eol = (c=='\r' || c=='\n');
    if (c == '\r') {
        fin.get(c);
        if (!fin.eof() && c != '\n')
            fin.putback(c);     // 너무 많이 읽었음
    }
    return eol;
}

// split: line을 필드로 나눔
int Csv::split()
{
    string fld;
    int i, j;

    nfield = 0;
    if (line.length() == 0)
        return 0;
    i = 0;

    do {
        if (i < line.length() && line[i] == '"')
            j = advquoted(line, fld, ++i); // 따옴표를 지나침
        else
            j = advplain(line, fld, i);
        if (nfield >= field.size()) // ?
            field.push_back(fld);
        nfield++;
        i = j + 1;              // i를 컴마의 다음 오프셋으로
    } while (j < line.length());
    return nfield;
}

// advquoted: 따옴표를 축약하거나 생략하고, 다음 구분자의 위치를 리턴함.
int Csv::advquoted(const string& s, string& fld, int i)
{
    int j;

    fld = "";
    for (j = i; j < s.length(); j++) {
        if (s[j] == '"' && s[++j] != '"') {
            int k = s.find_first_of(fieldsep, j);
            if (k > s.length()) // 구분자를 찾지 못함
                k = s.length();
            for (k -= j; k-- > 0; )
                fld += s[j++];
            break;
        }
        fld += s[j];
    }
    return j;
}

// advplain: 다음 구분자의 위치를 리턴
int Csv::advplain(const string& s, string& fld, int i)
{
    int j;
    j = s.find_first_of(fieldsep, i); // 구분자를 찾음
    if (j > s.length())               // 아무것도 찾지 못함
        j = s.length();
    fld = string(s, i, j-i);
    return j;
}

// getfield: n번째 필드를 리턴함
string Csv::getfield(int n)
{
    if (n < 0 || n >= nfield)
        return "";
    else
        return field[n];
}
