#pragma once

#include <vector>

// 개별 글자 정보
struct SCharInfo
{
    int     m_sheet;    // 장 번호
    int     m_line;     // 행 번호
    int     m_order;    // 행 내 순서
    CString m_char;     // 유니코드 12자리 문자열 (예: "110011121130")
    int     m_type;     // 활자 type 번호

    int     m_sx;       // 원본 이미지에서의 x 좌표
    int     m_sy;       // 원본 이미지에서의 y 좌표
    int     m_width;    // 폭
    int     m_height;   // 높이

    SCharInfo()
        : m_sheet(0), m_line(0), m_order(0),
        m_type(1), m_sx(0), m_sy(0), m_width(0), m_height(0)
    {
    }
};

// 전체 DB
class CTypeDB
{
public:
    CTypeDB();

    // typeDB.csv 읽기 (성공하면 true)
    bool ReadCSVFile(const CString& path);

    // 통계용 멤버 (의도적으로 public)
    int m_nSheet;                 // 전체 장 수
    int m_nChar;                  // 전체 글자 수
    std::vector<SCharInfo> m_Chars;
};
