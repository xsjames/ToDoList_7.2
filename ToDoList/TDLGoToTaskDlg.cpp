// TDLGoToTaskDlg.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "TDLGoToTaskDlg.h"

#include "..\shared\Misc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

const LPCTSTR ID_MASK = _T("0123456789");

/////////////////////////////////////////////////////////////////////////////
// CTDLGoToTaskDlg dialog


CTDLGoToTaskDlg::CTDLGoToTaskDlg(const CToDoCtrl& tdc, CWnd* pParent /*=NULL*/)
	: 
	CDialog(IDD_GOTOTASK_DIALOG, pParent), 
	m_tdc(tdc), 
	m_eTaskID(ID_MASK),
	m_dwTaskID(0)
{
	//{{AFX_DATA_INIT(CTDLGoToTaskDlg)
	//}}AFX_DATA_INIT
}


void CTDLGoToTaskDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTDLGoToTaskDlg)
	DDX_Control(pDX, IDC_TASKID, m_eTaskID);
	DDX_Control(pDX, IDC_TASKTITLE, m_eTaskTitle);
	DDX_Text(pDX, IDC_TASKID, m_sTaskID);
	DDX_Text(pDX, IDC_TASKTITLE, m_sTaskTitle);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTDLGoToTaskDlg, CDialog)
	//{{AFX_MSG_MAP(CTDLGoToTaskDlg)
	ON_EN_SETFOCUS(IDC_TASKID, OnEditSetFocusTaskID)
	ON_EN_SETFOCUS(IDC_TASKTITLE, OnEditSetFocusTaskTitle)
	ON_EN_KILLFOCUS(IDC_TASKID, OnEditKillFocusTaskID)
	ON_EN_KILLFOCUS(IDC_TASKTITLE, OnEditKillFocusTaskTitle)
	ON_EN_CHANGE(IDC_TASKTITLE, OnChangeTaskTitle)
	ON_EN_CHANGE(IDC_TASKID, OnChangeTaskID)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTDLGoToTaskDlg message handlers

BOOL CTDLGoToTaskDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	UpdateEditPrompts();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CTDLGoToTaskDlg::UpdateEditPrompts()
{
	// One time initialisation
	static CString sPrompt;

	if (sPrompt.IsEmpty())
		sPrompt.Format(_T("<%s>"), CEnString(IDS_GOTOTASK_NOSUCHTASK));

	// Hide the prompts when the 'other's' text is empty
	m_wndPrompts.SetEditPrompt(m_eTaskID, (m_sTaskTitle.IsEmpty() ? _T("") : sPrompt), TRUE);
	m_wndPrompts.SetEditPrompt(m_eTaskTitle, (m_sTaskID.IsEmpty() ? _T("") : sPrompt), TRUE);

	m_eTaskTitle.SetReadOnly(TRUE);
}

void CTDLGoToTaskDlg::OnEditSetFocusTaskID()
{
	m_eTaskID.SetReadOnly(FALSE);
	m_eTaskID.SetMask(ID_MASK); 

	ReformatTaskID();
}

void CTDLGoToTaskDlg::OnEditSetFocusTaskTitle()
{
	m_eTaskTitle.SetReadOnly(FALSE);
}

void CTDLGoToTaskDlg::OnEditKillFocusTaskID()
{
	m_eTaskID.SetReadOnly(TRUE);
	m_eTaskID.SetMask(_T(""), ME_EXCLUDE);

	ReformatTaskID();
}

void CTDLGoToTaskDlg::OnEditKillFocusTaskTitle()
{
	m_eTaskTitle.SetReadOnly(TRUE);
}

void CTDLGoToTaskDlg::OnChangeTaskTitle() 
{
	if (GetFocus() == &m_eTaskTitle)
	{
		UpdateTaskID();
		GetDlgItem(IDOK)->EnableWindow(!m_sTaskID.IsEmpty());
	}
}

void CTDLGoToTaskDlg::OnChangeTaskID() 
{
	if (GetFocus() == &m_eTaskID)
	{
		UpdateData(TRUE);
		m_dwTaskID = _ttol(m_sTaskID);

		UpdateTaskTitle();
		GetDlgItem(IDOK)->EnableWindow(!m_sTaskTitle.IsEmpty());
	}
}

DWORD CTDLGoToTaskDlg::FindTask(const CString& sText, CString& sTitle) const
{
	SEARCHPARAMS params;
	params.aRules.Add(SEARCHPARAM(TDCA_TASKNAME, FOP_INCLUDES, Misc::GetQuoted(sText)));

	CResultArray aResults;

	if (m_tdc.FindTasks(params, aResults))
	{
		sTitle = aResults[0].aMatched[0];
		return aResults[0].dwTaskID;
	}

	// else
	return 0;
}

void CTDLGoToTaskDlg::ReformatTaskID()
{
	if (m_eTaskID.GetStyle() & ES_READONLY)
	{
		// Restore trailing text
		if (m_dwTaskID)
		{
			CString sTitle = m_tdc.GetTaskTitle(m_dwTaskID);
			m_sTaskID.Format(_T("%ld (%s)"), m_dwTaskID, sTitle);
		}
		else
		{
			m_sTaskID.Empty();
		}
	}
	else if (m_dwTaskID)
	{
		m_sTaskID.Format(_T("%ld"), m_dwTaskID);
	}

	UpdateData(FALSE);
	UpdateEditPrompts();
}

void CTDLGoToTaskDlg::UpdateTaskID()
{
	UpdateData(TRUE);

	CString sTitle;
	m_dwTaskID = FindTask(m_sTaskTitle, sTitle);

	ReformatTaskID();
}

void CTDLGoToTaskDlg::UpdateTaskTitle()
{
	UpdateData(TRUE);

	if (m_tdc.HasTask(m_dwTaskID))
		m_sTaskTitle = m_tdc.GetTaskTitle(m_dwTaskID);
	else
		m_sTaskTitle.Empty();

	UpdateData(FALSE);
	UpdateEditPrompts();
}
