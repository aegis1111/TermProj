#include <vtkSmartPointer.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSTLReader.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkCamera.h>

#pragma once

#include "TypeDB.h"

#include <vector>
#include <set>
#include <algorithm>

const int PAGE_ORG_WIDTH = 9921;
const int PAGE_ORG_HEIGHT = 7015;

struct CStringLess
{
    bool operator()(const CString& a, const CString& b) const
    {
        return a.Compare(b) < 0;
    }
};

class CTermProjDlg : public CDialogEx
{
public:
    CTermProjDlg(CWnd* pParent = nullptr);
    enum { IDD = IDD_TERMPROJ_DIALOG };
    virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:
    CString m_strBmpFolder;
    std::vector<CString> m_vecBmpFiles;
    int m_nSheet;
    std::vector<SCharInfo> m_chars;
    CButton m_btnBrowseBook;

    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    DECLARE_MESSAGE_MAP()

    BOOL LoadCharCSV(const CString& pathChar);
    void UpdateBookInfo();
    void UpdateSheetInfo();
    void GotoSheet(int nSheet);
    void ChangeSheet(int nDelta);

    afx_msg void OnBnClickedButtonOpen();
    afx_msg void OnDeltaposSpinSheet(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnDeltaposSpinType(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnItemChangedListChars(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnBnClickedButtonLoadbook();

private:
    CEdit m_editBookName;
    CSpinButtonCtrl m_spinSheet;
    CSpinButtonCtrl m_spinType;
    CStatic m_staticPage;
    CStatic m_staticCharInfo;
    CStatic m_staticCharImage;
    CStatic m_staticSelChar;
    CStatic m_staticModel;
    CListCtrl m_listChars;
    CStatic m_staticTypeNum;

    int m_nSelectedChar = -1;
    std::vector<int> m_typeList;
    int m_curTypePos = 1;

    std::vector<CString> m_typePngPaths;
    void BuildTypeListForSelectedChar();
    void FillOccurrenceListByType(int typeValue);
    void UpdateTypeInfoFrom03Type();
    void ShowSelectedTypePngByListIndex(int listIndex);

    CImage m_imgCharInfo;

    void UpdateSelectedCharInfo();
    void LoadCharInfoImage(const SCharInfo& rec);
    void ClearCharInfoImage();

    HICON m_hIcon;

    CString m_strBookRoot;
    CTypeDB m_typeDB;

    int m_nCurSheet;
    int m_nMaxSheet;
    int m_nCurSheetPos;
    int m_nTypeTotalCount;
    std::vector<int> m_vecSheetIdx;

    CImage m_imgPage;
    CImage m_imgChar;
    CImage m_imgSelChar;

    CRect m_rcPageArea;

    void InitControls();
    void ClearAll();
    void ClearSheet();
    void LoadBook(const CString& bookRoot);
    void LoadSheet(int nSheet);
    void SelectSheetPos(int pos);
    void LoadSheetBitmap(int sheetIndex);
    void UpdateBookStats();
    void UpdateSheetStats();
    void UpdateCharInfo(int globalIndex);
    void UpdateTypeInfo(int globalIndex);
    void LoadPageImage(int nSheet);
    void LoadCharImage(int globalIndex);
    void LoadSelCharImage(int pathIndex);
    void LoadTypeSampleList(int globalIndex);
    void DrawPage(CDC& dc);
    CString MakeGlyphString(const CString& code12);
    void ShowSTLModel(int globalIndex);
    void DrawCharBoxes(CDC& dc);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    void SelectFirstCharOfCurrentSheet();

public:
    CStatic m_static1;

    void InitVtkWindow();

    vtkSmartPointer<vtkRenderWindow> m_vtkWindow;
    vtkSmartPointer<vtkRenderer> m_vtkRenderer;
    vtkSmartPointer<vtkRenderWindowInteractor> m_vtkInteractor;
    vtkSmartPointer<vtkActor> m_vtkActor;

    void Update3DView(const CString& stlFilePath);
};