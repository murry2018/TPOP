#if !defined ( GUARD__CSV_H )
#define GUARD__CSV_H

#include <iostream>
#include <string>
#include <vector>

// 콤마 구분값(csv)을 읽고 파싱한다.
class Csv {
    // 입력 예시: "LU",86.25,"11/4/1998","2:19PM",+4.0625
public:
    Csv(std::istream& fin = std::cin, std::string sep = ",") :
        fin(fin), fieldsep(sep) {}

    int getline(std::string&);
    std::string getfield(int n);
    int getnfield() const { return nfield; }

private:
    std::istream& fin;              // 입력 파일 포인터
    std::string line;               // 입력 줄
    std::vector<std::string> field; // 필드 문자열
    int nfield;                     // 필드 개수
    std::string fieldsep;           // 구분자

    int split();
    int endofline(char);
    int advplain(const std::string& line, std::string& fld, int);
    int advquoted(const std::string& line, std::string& fld, int);
};

#endif
