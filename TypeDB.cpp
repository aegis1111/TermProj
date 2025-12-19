#include "pch.h"
#include "TypeDB.h"

CTypeDB::CTypeDB()
    : m_nSheet(0)
    , m_nChar(0)
{
}

// 간단한 CSV 파서
static void SplitCSVLine(const CString& line, CStringArray& tokens)
{
    tokens.RemoveAll();

    int cur = 0;
    while (cur >= 0 && cur < line.GetLength())
    {
        CString token = line.Tokenize(_T(","), cur);
        token.Trim();
        tokens.Add(token);
    }
}

// CSV 포맷 가정 (열 순서는 교수님 파일과 조금 달라도 상관없게 느슨하게 처리)
// 예시: sheet,line,order,char12,type,sx,sy,width,height,...
bool CTypeDB::ReadCSVFile(const CString& path)
{
    m_nSheet = 0;
    m_nChar = 0;
    m_Chars.clear();

    CStdioFile file;
    if (!file.Open(path, CFile::modeRead | CFile::typeText))
        return false;

    CString line;
    bool bFirst = true;

    while (file.ReadString(line))
    {
        line.Trim();
        if (line.IsEmpty())
            continue;

        // 주석 줄(#...) 은 무시
        if (line[0] == _T('#'))
            continue;

        // 만약 첫 줄이 헤더라면(문자 포함) 대충 넘겨버리기
        if (bFirst)
        {
            bFirst = false;
            bool hasAlpha = false;
            for (int i = 0; i < line.GetLength(); ++i)
            {
                if ((line[i] >= 'A' && line[i] <= 'Z') ||
                    (line[i] >= 'a' && line[i] <= 'z'))
                {
                    hasAlpha = true;
                    break;
                }
            }
            if (hasAlpha)
                continue;   // 헤더라고 보고 넘김
        }

        CStringArray tokens;
        SplitCSVLine(line, tokens);

        // 최소한 sheet,line,order,char,type,sx,sy,width,height 까지는 있어야 함
        if (tokens.GetSize() < 9)
            continue;

        SCharInfo info;

        info.m_sheet = _ttoi(tokens[0]);
        info.m_line = _ttoi(tokens[1]);
        info.m_order = _ttoi(tokens[2]);
        info.m_char = tokens[3];      // 유니코드 12자리 문자열
        info.m_type = _ttoi(tokens[4]);
        info.m_sx = _ttoi(tokens[5]);
        info.m_sy = _ttoi(tokens[6]);
        info.m_width = _ttoi(tokens[7]);
        info.m_height = _ttoi(tokens[8]);

        m_Chars.push_back(info);
        ++m_nChar;

        if (info.m_sheet > m_nSheet)
            m_nSheet = info.m_sheet;
    }

    file.Close();
    return (m_nChar > 0);
}
