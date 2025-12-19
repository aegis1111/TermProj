#include "pch.h"
#include "framework.h"
#include "TermProj.h"
#include "TermProjDlg.h"
#include "afxdialogex.h"
#include <atlimage.h>

#include <vtkAutoInit.h>
VTK_MODULE_INIT(vtkRenderingOpenGL2);
VTK_MODULE_INIT(vtkInteractionStyle);
VTK_MODULE_INIT(vtkRenderingFreeType);

#include <vtkSTLReader.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkCamera.h>
#include <vtkActor.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkOutputWindow.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CTermProjDlg::CTermProjDlg(CWnd* pParent)
    : CDialogEx(IDD_TERMPROJ_DIALOG, pParent)
    , m_nCurSheet(0)
    , m_nMaxSheet(0)
    , m_nCurSheetPos(-1)
    , m_nSheet(0)
    , m_nSelectedChar(-1)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CTermProjDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT_BOOKNAME, m_editBookName);
    DDX_Control(pDX, IDC_SPIN_SHEET, m_spinSheet);
    DDX_Control(pDX, IDC_SPIN_TYPE, m_spinType);
    DDX_Control(pDX, IDC_STATIC_PAGE, m_staticPage);
    DDX_Control(pDX, IDC_STATIC_CHARINFO, m_staticCharInfo);
    DDX_Control(pDX, IDC_STATIC_CHARIMG, m_staticCharImage);
    DDX_Control(pDX, IDC_STATIC_SELCHAR, m_staticSelChar);
    DDX_Control(pDX, IDC_STATIC_MODEL, m_staticModel);
    DDX_Control(pDX, IDC_LIST_CHARS, m_listChars);
    DDX_Control(pDX, IDC_BUTTON_OPEN, m_btnBrowseBook);
    DDX_Control(pDX, IDC_STATIC_NUM, m_staticTypeNum);
    DDX_Control(pDX, IDC_STATIC_TYPES, m_static1);
}

BEGIN_MESSAGE_MAP(CTermProjDlg, CDialogEx)
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_WM_LBUTTONDOWN()
    ON_BN_CLICKED(IDC_BUTTON_OPEN, &CTermProjDlg::OnBnClickedButtonOpen)
    ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_SHEET, &CTermProjDlg::OnDeltaposSpinSheet)
    ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_TYPE, &CTermProjDlg::OnDeltaposSpinType)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_CHARS, &CTermProjDlg::OnItemChangedListChars)
END_MESSAGE_MAP()

static bool ParseTypePngName(const CString& fileName, int& sheet, int& sx, int& sy)
{
    sheet = sx = sy = -1;
    CString name = fileName;
    int dot = name.ReverseFind(_T('.'));
    if (dot < 0) return false;
    CString base = name.Left(dot);
    int cur = 0;
    CString t1 = base.Tokenize(_T("_"), cur);
    CString t2 = base.Tokenize(_T("_"), cur);
    CString t3 = base.Tokenize(_T("_"), cur);
    if (t1.IsEmpty() || t2.IsEmpty() || t3.IsEmpty()) return false;
    sheet = _ttoi(t1);
    sx = _ttoi(t2);
    sy = _ttoi(t3);
    return (sheet >= 1 && sheet <= 3 && sx >= 0 && sy >= 0);
}

BOOL CTermProjDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();
    SetIcon(m_hIcon, TRUE);
    SetIcon(m_hIcon, FALSE);
    m_btnBrowseBook.ModifyStyle(0, BS_ICON);
    HICON hIcon = AfxGetApp()->LoadIcon(IDI_ICON_SEARCH);
    m_btnBrowseBook.SetIcon(hIcon);
    InitControls();
    ClearAll();
    InitVtkWindow();
    return TRUE;
}

void CTermProjDlg::InitVtkWindow()
{
    CWnd* pPC = GetDlgItem(IDC_STATIC_MODEL);
    if (pPC == nullptr) return;
    m_vtkRenderer = vtkSmartPointer<vtkRenderer>::New();
    m_vtkWindow = vtkSmartPointer<vtkRenderWindow>::New();
    m_vtkInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    m_vtkWindow->SetParentId(pPC->GetSafeHwnd());
    m_vtkWindow->AddRenderer(m_vtkRenderer);
    m_vtkInteractor->SetRenderWindow(m_vtkWindow);
    m_vtkRenderer->SetBackground(0.2, 0.3, 0.4);
    m_vtkWindow->Render();
    m_vtkInteractor->Initialize();
}

void CTermProjDlg::Update3DView(const CString& stlFilePath)
{
    if (m_vtkRenderer == nullptr) return;
    m_vtkRenderer->RemoveAllViewProps();
    m_vtkActor = nullptr;
    CWnd* pPC = GetDlgItem(IDC_STATIC_MODEL);
    if (pPC)
    {
        CRect rect;
        pPC->GetClientRect(&rect);
        if (m_vtkWindow) m_vtkWindow->SetSize(rect.Width(), rect.Height());
    }
    if (stlFilePath.IsEmpty()) {
        m_vtkWindow->Render();
        return;
    }
    CFileFind finder;
    if (!finder.FindFile(stlFilePath)) return;
    auto reader = vtkSmartPointer<vtkSTLReader>::New();
    CW2A utf8Path(stlFilePath, CP_UTF8);
    reader->SetFileName(utf8Path);
    reader->Update();
    vtkPolyData* data = reader->GetOutput();
    if (data == nullptr || data->GetNumberOfPoints() == 0) return;
    auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(reader->GetOutputPort());
    m_vtkActor = vtkSmartPointer<vtkActor>::New();
    m_vtkActor->SetMapper(mapper);
    m_vtkActor->GetProperty()->SetColor(0.8, 0.8, 0.8);
    data->ComputeBounds();
    double bounds[6];
    data->GetBounds(bounds);
    double cx = (bounds[0] + bounds[1]) / 2.0;
    double cy = (bounds[2] + bounds[3]) / 2.0;
    double cz = (bounds[4] + bounds[5]) / 2.0;
    m_vtkActor->SetOrigin(cx, cy, cz);
    m_vtkRenderer->AddActor(m_vtkActor);
    DWORD dwColor = GetSysColor(COLOR_BTNFACE);
    m_vtkRenderer->SetBackground(GetRValue(dwColor) / 255.0, GetGValue(dwColor) / 255.0, GetBValue(dwColor) / 255.0);
    auto camera = m_vtkRenderer->GetActiveCamera();
    camera->SetFocalPoint(cx, cy, cz);
    camera->SetViewUp(0, 1, 0);
    camera->SetPosition(cx, cy, cz + 100);
    m_vtkRenderer->ResetCamera();
    m_vtkRenderer->ResetCameraClippingRange();
    camera->Zoom(1.2);
    m_vtkWindow->Render();
}

BOOL CTermProjDlg::LoadCharCSV(const CString& path)
{
    m_chars.clear();
    CStdioFile file;
    CFileException ex;
    if (!file.Open(path, CFile::modeRead | CFile::typeText, &ex)) return FALSE;
    CString line;
    bool firstLine = true;
    while (file.ReadString(line))
    {
        line.Trim();
        if (line.IsEmpty()) continue;
        if (firstLine && !line.IsEmpty() && line[0] == 0xFEFF) line = line.Mid(1);
        if (firstLine) { firstLine = false; continue; }
        SCharInfo info{};
        int cur = 0;
        CString token;
        token = line.Tokenize(_T(","), cur); info.m_char = token;
        token = line.Tokenize(_T(","), cur); info.m_type = _ttoi(token);
        token = line.Tokenize(_T(","), cur); info.m_sheet = _ttoi(token);
        token = line.Tokenize(_T(","), cur); info.m_sx = _ttoi(token);
        token = line.Tokenize(_T(","), cur); info.m_sy = _ttoi(token);
        token = line.Tokenize(_T(","), cur); info.m_line = _ttoi(token);
        token = line.Tokenize(_T(","), cur); info.m_order = _ttoi(token);
        token = line.Tokenize(_T(","), cur); info.m_width = _ttoi(token);
        token = line.Tokenize(_T(","), cur); info.m_height = _ttoi(token);
        m_chars.push_back(info);
    }
    file.Close();
    SelectFirstCharOfCurrentSheet();
    return TRUE;
}

void CTermProjDlg::InitControls()
{
    m_spinSheet.SetRange(1, 1);
    m_spinSheet.SetPos(1);
    m_nSheet = 0;
    m_nSelectedChar = -1;
    CRect rc;
    m_staticPage.GetClientRect(&rc);
    m_staticPage.ClientToScreen(&rc);
    ScreenToClient(&rc);
    m_rcPageArea = rc;
    m_listChars.ModifyStyle(0, LVS_REPORT);
    m_listChars.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
    m_listChars.DeleteAllItems();
    while (m_listChars.DeleteColumn(0)) {}
    m_listChars.InsertColumn(0, _T("장"), LVCFMT_CENTER, 45);
    m_listChars.InsertColumn(1, _T("행"), LVCFMT_CENTER, 45);
    m_listChars.InsertColumn(2, _T("번"), LVCFMT_CENTER, 45);
    if (!m_imgPage.IsNull()) m_imgPage.Destroy();
}

void CTermProjDlg::UpdateBookInfo()
{
    int totalChars = (int)m_chars.size();
    std::set<CString, CStringLess> charSet;
    std::set<CString, CStringLess> fontSet;
    for (const auto& ch : m_chars) {
        charSet.insert(ch.m_char);
        CString key;
        key.Format(_T("%s#%d"), ch.m_char.GetString(), ch.m_type);
        fontSet.insert(key);
    }
    int charKinds = (int)charSet.size();
    int fontCount = (int)fontSet.size();
    CString s;
    s.Format(_T("한글 글자수 %d 개"), totalChars);
    SetDlgItemText(IDC_STATIC_BOOK_CHARCOUNT, s);
    s.Format(_T("한글 글자 종류 %d 종"), charKinds);
    SetDlgItemText(IDC_STATIC_BOOK_CHARKINDS, s);
    s.Format(_T("한글 활자수 %d 개"), fontCount);
    SetDlgItemText(IDC_STATIC_BOOK_TYPECOUNT, s);
}

void CTermProjDlg::UpdateSheetInfo()
{
    int totalChars = 0;
    std::set<CString, CStringLess> charSet;
    std::set<CString, CStringLess> fontSet;
    for (const auto& ch : m_chars) {
        if (ch.m_sheet != m_nSheet) continue;
        ++totalChars;
        charSet.insert(ch.m_char);
        CString key;
        key.Format(_T("%s#%d"), ch.m_char.GetString(), ch.m_type);
        fontSet.insert(key);
    }
    int charKinds = (int)charSet.size();
    int fontCount = (int)fontSet.size();
    CString s;
    s.Format(_T("한글 글자수 %d 개"), totalChars);
    SetDlgItemText(IDC_STATIC_SHEET_CHARCOUNT, s);
    s.Format(_T("한글 글자 종류 %d 종"), charKinds);
    SetDlgItemText(IDC_STATIC_SHEET_CHARKINDS, s);
    s.Format(_T("한글 활자수 %d 개"), fontCount);
    SetDlgItemText(IDC_STATIC_SHEET_TYPECOUNT, s);
}

void CTermProjDlg::ClearAll()
{
    m_chars.clear();
    m_vecBmpFiles.clear();
    m_nSheet = 0;
    m_nSelectedChar = -1;
    m_spinSheet.SetRange(0, 0);
    m_spinSheet.SetPos(0);
    m_spinSheet.EnableWindow(FALSE);
    SetDlgItemText(IDC_STATIC_SHEETS, _T("/ 0장"));
    SetDlgItemText(IDC_STATIC_CUR_SHEET, _T("0"));
    if (!m_imgPage.IsNull()) m_imgPage.Destroy();
    Update3DView(_T(""));
    Invalidate(FALSE);
}

void CTermProjDlg::LoadSheetBitmap(int sheetIndex)
{
    if (sheetIndex < 1 || sheetIndex >(int)m_vecBmpFiles.size()) return;
    CString imgPath = m_vecBmpFiles[sheetIndex - 1];
    if (!m_imgPage.IsNull()) m_imgPage.Destroy();
    if (FAILED(m_imgPage.Load(imgPath))) return;
    InvalidateRect(&m_rcPageArea, FALSE);
}

void CTermProjDlg::OnBnClickedButtonOpen()
{
    CFolderPickerDialog dlg(nullptr, 0, this);
    if (dlg.DoModal() != IDOK) return;
    CWaitCursor wait;
    m_strBmpFolder = dlg.GetPathName();
    CString bookFolder = m_strBmpFolder;
    int pos = bookFolder.ReverseFind(_T('\\'));
    if (pos != -1) bookFolder = bookFolder.Left(pos);
    CString bookName = bookFolder;
    pos = bookName.ReverseFind(_T('\\'));
    if (pos != -1) bookName = bookName.Mid(pos + 1);
    SetDlgItemText(IDC_EDIT_BOOKNAME, bookName);
    m_vecBmpFiles.clear();
    CString pattern;
    pattern.Format(_T("%s\\*.jpg"), m_strBmpFolder.GetString());
    CFileFind finder;
    BOOL bWorking = finder.FindFile(pattern);
    while (bWorking) {
        bWorking = finder.FindNextFile();
        if (!finder.IsDots()) m_vecBmpFiles.push_back(finder.GetFilePath());
    }
    finder.Close();
    if (m_vecBmpFiles.empty()) return;
    std::sort(m_vecBmpFiles.begin(), m_vecBmpFiles.end(), [](const CString& a, const CString& b) {
        return a.CompareNoCase(b) < 0;
        });
    int count = (int)m_vecBmpFiles.size();
    m_spinSheet.EnableWindow(TRUE);
    m_spinSheet.SetRange(1, (short)count);
    m_nSheet = 1;
    m_spinSheet.SetPos(m_nSheet);
    SetDlgItemText(IDC_STATIC_CUR_SHEET, _T("1"));
    CString buf; buf.Format(_T("/ %d"), count);
    SetDlgItemText(IDC_STATIC_SHEETS, buf);
    LoadSheetBitmap(m_nSheet);
    CString csvPath;
    pattern.Format(_T("%s\\*.csv"), bookFolder.GetString());
    bWorking = finder.FindFile(pattern);
    while (bWorking) {
        bWorking = finder.FindNextFile();
        if (!finder.IsDots() && finder.GetFileName().Left(6).CompareNoCase(_T("typeDB")) == 0) {
            csvPath = finder.GetFilePath();
            break;
        }
    }
    finder.Close();
    if (csvPath.IsEmpty() || !LoadCharCSV(csvPath)) return;
    UpdateBookInfo();
    UpdateSheetInfo();
    SelectFirstCharOfCurrentSheet();
    UpdateSelectedCharInfo();
    UpdateTypeInfoFrom03Type();
    Invalidate(FALSE);
}

void CTermProjDlg::GotoSheet(int nSheet)
{
    if (m_vecBmpFiles.empty()) return;
    int maxSheet = static_cast<int>(m_vecBmpFiles.size());
    if (nSheet < 1) nSheet = 1; else if (nSheet > maxSheet) nSheet = maxSheet;
    if (m_nSheet == nSheet) return;
    m_nSheet = nSheet;
    if (m_spinSheet.GetSafeHwnd()) m_spinSheet.SetPos(m_nSheet);
    CString cur; cur.Format(_T("%d"), m_nSheet);
    SetDlgItemText(IDC_STATIC_CUR_SHEET, cur);
    LoadSheetBitmap(m_nSheet);
    UpdateSheetInfo();
    SelectFirstCharOfCurrentSheet();
    UpdateTypeInfoFrom03Type();
    Invalidate(FALSE);
}

void CTermProjDlg::ChangeSheet(int nDelta) { if (!m_vecBmpFiles.empty()) GotoSheet(m_nSheet + nDelta); }

void CTermProjDlg::OnDeltaposSpinSheet(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMUPDOWN p = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
    int minPos, maxPos;
    m_spinSheet.GetRange(minPos, maxPos);
    m_nSheet -= p->iDelta;
    if (m_nSheet < minPos) m_nSheet = minPos; else if (m_nSheet > maxPos) m_nSheet = maxPos;
    m_spinSheet.SetPos(m_nSheet);
    CString cur; cur.Format(_T("%d"), m_nSheet);
    SetDlgItemText(IDC_STATIC_CUR_SHEET, cur);
    LoadSheetBitmap(m_nSheet);
    UpdateSheetInfo();
    SelectFirstCharOfCurrentSheet();
    UpdateSelectedCharInfo();
    UpdateTypeInfoFrom03Type();
    Invalidate(FALSE);
    *pResult = 0;
}

void CTermProjDlg::OnDeltaposSpinType(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMUPDOWN p = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
    int cur = m_spinType.GetPos32() - p->iDelta;
    if (cur < 1) cur = 1; else if (cur > m_nTypeTotalCount) cur = m_nTypeTotalCount;
    m_curTypePos = cur;
    m_spinType.SetPos(cur);
    CString s; s.Format(_T("%d"), m_curTypePos);
    m_staticTypeNum.SetWindowText(s);
    int index = cur - 1;
    if (index >= 0 && index < m_listChars.GetItemCount()) {
        m_listChars.SetItemState(index, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
        m_listChars.EnsureVisible(index, FALSE);
        ShowSelectedTypePngByListIndex(index);
    }
    *pResult = 0;
}

void CTermProjDlg::DrawPage(CDC& dc)
{
    dc.FillSolidRect(m_rcPageArea, RGB(240, 240, 240));
    if (!m_imgPage.IsNull()) m_imgPage.Draw(dc.m_hDC, m_rcPageArea);
    DrawCharBoxes(dc);
}

void CTermProjDlg::ShowSTLModel(int) {}

void CTermProjDlg::DrawCharBoxes(CDC& dc)
{
    if (m_chars.empty() || m_imgPage.IsNull()) return;
    double sxScale = (double)m_rcPageArea.Width() / m_imgPage.GetWidth();
    double syScale = (double)m_rcPageArea.Height() / m_imgPage.GetHeight();
    CPen greenPen(PS_SOLID, 2, RGB(0, 200, 0)), redPen(PS_SOLID, 3, RGB(255, 0, 0));
    CBrush* pNullBrush = CBrush::FromHandle((HBRUSH)GetStockObject(NULL_BRUSH));
    CPen* oldPen = dc.SelectObject(&greenPen);
    CBrush* pOldBrush = dc.SelectObject(pNullBrush);
    for (int i = 0; i < (int)m_chars.size(); ++i) {
        const auto& c = m_chars[i];
        if (c.m_sheet != m_nSheet) continue;
        int l = m_rcPageArea.left + (int)(c.m_sx * sxScale), t = m_rcPageArea.top + (int)(c.m_sy * syScale);
        int r = l + (int)(c.m_width * sxScale), b = t + (int)(c.m_height * syScale);
        if (i == m_nSelectedChar) { dc.SelectObject(&redPen); dc.Rectangle(l, t, r, b); dc.SelectObject(&greenPen); }
        else dc.Rectangle(l, t, r, b);
    }
    dc.SelectObject(oldPen); dc.SelectObject(pOldBrush);
}

void CTermProjDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
    if (m_imgPage.IsNull() || m_rcPageArea.Width() <= 0) { CDialogEx::OnLButtonDown(nFlags, point); return; }
    double sxScale = (double)m_rcPageArea.Width() / m_imgPage.GetWidth();
    double syScale = (double)m_rcPageArea.Height() / m_imgPage.GetHeight();
    int clickedIndex = -1;
    for (int i = (int)m_chars.size() - 1; i >= 0; --i) {
        const auto& c = m_chars[i];
        if (c.m_sheet != m_nSheet) continue;
        CRect rc(m_rcPageArea.left + (int)(c.m_sx * sxScale), m_rcPageArea.top + (int)(c.m_sy * syScale),
            m_rcPageArea.left + (int)((c.m_sx + c.m_width) * sxScale), m_rcPageArea.top + (int)((c.m_sy + c.m_height) * syScale));
        if (rc.PtInRect(point)) { clickedIndex = i; break; }
    }
    if (clickedIndex != -1 && clickedIndex != m_nSelectedChar) {
        m_nSelectedChar = clickedIndex;
        UpdateSelectedCharInfo();
        UpdateTypeInfoFrom03Type();
        Invalidate(FALSE);
    }
    CDialogEx::OnLButtonDown(nFlags, point);
}

void CTermProjDlg::SelectFirstCharOfCurrentSheet()
{
    m_nSelectedChar = -1;
    for (int i = 0; i < (int)m_chars.size(); ++i) {
        if (m_chars[i].m_sheet == m_nSheet && m_chars[i].m_line == 1 && m_chars[i].m_order == 1) {
            m_nSelectedChar = i; break;
        }
    }
    UpdateSelectedCharInfo();
}

void CTermProjDlg::OnPaint()
{
    if (IsIconic()) {
        CPaintDC dc(this);
        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);
        CRect rect; GetClientRect(&rect);
        dc.DrawIcon((rect.Width() - GetSystemMetrics(SM_CXICON) + 1) / 2, (rect.Height() - GetSystemMetrics(SM_CYICON) + 1) / 2, m_hIcon);
    }
    else {
        CPaintDC dc(this);
        CDialogEx::OnPaint();
        DrawPage(dc);
    }
}

void CTermProjDlg::ShowSelectedTypePngByListIndex(int listIndex) {
    if (listIndex < 0 || listIndex >= m_listChars.GetItemCount()) return;
    int pathIndex = (int)m_listChars.GetItemData(listIndex);
    if (pathIndex < 0 || pathIndex >= (int)m_typePngPaths.size()) return;
    if (!m_imgSelChar.IsNull()) m_imgSelChar.Destroy();
    if (FAILED(m_imgSelChar.Load(m_typePngPaths[pathIndex]))) return;
    CRect rc; m_staticSelChar.GetClientRect(&rc);
    if (rc.Width() > 0) { CClientDC dc(&m_staticSelChar); dc.FillSolidRect(rc, RGB(255, 255, 255)); m_imgSelChar.Draw(dc.m_hDC, rc); }
}

void CTermProjDlg::OnItemChangedListChars(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
    if (!(pNMLV->uChanged & LVIF_STATE) || !(pNMLV->uNewState & LVIS_SELECTED)) return;
    m_curTypePos = pNMLV->iItem + 1;
    m_spinType.SetPos(m_curTypePos);
    CString s; s.Format(_T("%d"), m_curTypePos);
    m_staticTypeNum.SetWindowText(s);
    ShowSelectedTypePngByListIndex(pNMLV->iItem);
    *pResult = 0;
}

HCURSOR CTermProjDlg::OnQueryDragIcon() { return static_cast<HCURSOR>(m_hIcon); }

void CTermProjDlg::ClearCharInfoImage() { if (!m_imgCharInfo.IsNull()) m_imgCharInfo.Destroy(); if (m_staticCharImage.GetSafeHwnd()) m_staticCharImage.Invalidate(); }

void CTermProjDlg::LoadCharInfoImage(const SCharInfo& rec)
{
    ClearCharInfoImage();
    CString bookRoot = m_strBmpFolder; int pos = bookRoot.ReverseFind(_T('\\'));
    if (pos != -1) bookRoot = bookRoot.Left(pos);
    CString dir; dir.Format(_T("%s\\03_type\\%s\\%d"), bookRoot.GetString(), rec.m_char.GetString(), rec.m_type);
    CFileFind finder;
    if (!finder.FindFile(dir + _T("\\*.png"))) return;
    finder.FindNextFile();
    if (FAILED(m_imgCharInfo.Load(finder.GetFilePath()))) return;
    CRect rc; m_staticCharImage.GetClientRect(&rc);
    if (rc.Width() > 0) { CClientDC dc(&m_staticCharImage); dc.FillSolidRect(rc, RGB(255, 255, 255)); m_imgCharInfo.Draw(dc.m_hDC, rc); }
}

void CTermProjDlg::UpdateSelectedCharInfo()
{
    if (!m_staticCharInfo.GetSafeHwnd()) return;
    if (m_nSelectedChar < 0) { m_staticCharInfo.SetWindowText(_T("")); ClearCharInfoImage(); Update3DView(_T("")); return; }
    const SCharInfo& c = m_chars[m_nSelectedChar];
    CString info; info.Format(_T("%s\r\n%d장 %d행 %d번"), c.m_char.GetString(), c.m_sheet, c.m_line, c.m_order);
    m_staticCharInfo.SetWindowText(info);
    LoadCharInfoImage(c);
    CString strFullPath; strFullPath.Format(_T("C:\\TermProj\\04_3d\\%s_1.stl"), c.m_char.GetString());
    CFileFind finder; Update3DView(finder.FindFile(strFullPath) ? strFullPath : _T(""));
}

void CTermProjDlg::UpdateTypeInfoFrom03Type()
{
    m_listChars.DeleteAllItems(); m_typePngPaths.clear();
    if (m_nSelectedChar < 0) { m_spinType.SetRange(1, 1); m_staticTypeNum.SetWindowText(_T("0")); LoadSelCharImage(-1); return; }
    BuildTypeListForSelectedChar();
    FillOccurrenceListByType(m_typeList[m_curTypePos - 1]);
    if (m_listChars.GetItemCount() > 0) {
        m_listChars.SetItemState(0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
        ShowSelectedTypePngByListIndex(0);
    }
    else LoadSelCharImage(-1);
}

void CTermProjDlg::BuildTypeListForSelectedChar()
{
    const SCharInfo& sel = m_chars[m_nSelectedChar];
    std::set<int> typeSet;
    for (const auto& r : m_chars) if (r.m_char == sel.m_char) typeSet.insert(r.m_type);
    m_typeList.assign(typeSet.begin(), typeSet.end());
    m_curTypePos = 1;
    for (int i = 0; i < (int)m_typeList.size(); ++i) if (m_typeList[i] == sel.m_type) { m_curTypePos = i + 1; break; }
    m_spinType.SetRange(1, (short)m_typeList.size());
    m_spinType.SetPos(m_curTypePos);
    CString s; s.Format(_T("%d"), m_curTypePos); m_staticTypeNum.SetWindowText(s);
}

void CTermProjDlg::FillOccurrenceListByType(int typeValue)
{
    m_listChars.DeleteAllItems(); m_typePngPaths.clear();
    const SCharInfo& sel = m_chars[m_nSelectedChar];
    CString bookRoot = m_strBmpFolder; int pos = bookRoot.ReverseFind(_T('\\'));
    if (pos != -1) bookRoot = bookRoot.Left(pos);
    CString dir; dir.Format(_T("%s\\03_type\\%s\\%d"), bookRoot.GetString(), sel.m_char.GetString(), typeValue);
    CFileFind ff; BOOL b = ff.FindFile(dir + _T("\\*.png"));
    while (b) {
        b = ff.FindNextFile();
        if (ff.IsDots()) continue;
        int sh, sx, sy; if (!ParseTypePngName(ff.GetFileName(), sh, sx, sy)) continue;
        for (const auto& r : m_chars) {
            if (r.m_sheet == sh && r.m_sx == sx && r.m_sy == sy && r.m_char == sel.m_char && r.m_type == typeValue) {
                int row = m_listChars.InsertItem(m_listChars.GetItemCount(), _T(""));
                CString s; s.Format(_T("%d"), sh); m_listChars.SetItemText(row, 0, s);
                s.Format(_T("%d"), r.m_line); m_listChars.SetItemText(row, 1, s);
                s.Format(_T("%d"), r.m_order); m_listChars.SetItemText(row, 2, s);
                m_typePngPaths.push_back(ff.GetFilePath());
                m_listChars.SetItemData(row, m_typePngPaths.size() - 1);
                break;
            }
        }
    }
    int cnt = m_listChars.GetItemCount();
    m_nTypeTotalCount = cnt;
    if (cnt > 0) {
        m_spinType.SetRange(1, (short)cnt);
        CString s; s.Format(_T("1 / %d"), cnt); m_staticTypeNum.SetWindowText(s);
        ShowSelectedTypePngByListIndex(0);
    }
    else { m_staticTypeNum.SetWindowText(_T("0 / 0")); LoadSelCharImage(-1); }
    CString sTotal; sTotal.Format(_T("/ %d개"), cnt); m_static1.SetWindowText(sTotal);
}

void CTermProjDlg::LoadSelCharImage(int listIndex)
{
    if (!m_staticSelChar.GetSafeHwnd()) return;
    if (!m_imgSelChar.IsNull()) m_imgSelChar.Destroy();
    if (listIndex >= 0 && listIndex < (int)m_typePngPaths.size()) {
        if (SUCCEEDED(m_imgSelChar.Load(m_typePngPaths[listIndex]))) {
            CRect rc; m_staticSelChar.GetClientRect(&rc);
            CClientDC dc(&m_staticSelChar); dc.FillSolidRect(rc, RGB(255, 255, 255)); m_imgSelChar.Draw(dc.m_hDC, rc);
        }
    }
}

BOOL CTermProjDlg::PreTranslateMessage(MSG* pMsg)
{
    if (pMsg->message == WM_KEYDOWN) {
        bool bCtrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
        switch (pMsg->wParam) {
        case VK_NEXT: ChangeSheet(+1); return TRUE;
        case VK_PRIOR: ChangeSheet(-1); return TRUE;
        case VK_RIGHT: if (bCtrl) { ChangeSheet(+1); return TRUE; } break;
        case VK_LEFT: if (bCtrl) { ChangeSheet(-1); return TRUE; } break;
        case VK_HOME: if (bCtrl) { GotoSheet(1); return TRUE; } break;
        case VK_END: if (bCtrl && !m_vecBmpFiles.empty()) { GotoSheet(static_cast<int>(m_vecBmpFiles.size())); return TRUE; } break;
        }
    }
    return CDialogEx::PreTranslateMessage(pMsg);
}