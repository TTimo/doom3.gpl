/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company. 

This file is part of the Doom 3 GPL Source Code (?Doom 3 Source Code?).  

Doom 3 Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#include "../../idlib/precompiled.h"
#pragma hdrstop

#include "../../sys/win32/rc/Radiant_resource.h"
#include "DialogColorPicker.h"

#ifdef ID_DEBUG_MEMORY
#undef new
#undef DEBUG_NEW
#define DEBUG_NEW new
#endif

// Old color picker

class CMyColorDialog : public CColorDialog 
{
  DECLARE_DYNCREATE(CMyColorDialog);
     // Construction
public:
     CMyColorDialog( COLORREF clrInit = 0, DWORD dwFlags = 0, CWnd *pParentWnd = NULL );
     virtual int DoModal();

protected:
     enum { NCUSTCOLORS = 16 };
     static COLORREF	c_CustColors[NCUSTCOLORS];
     static COLORREF	c_LastCustColors[NCUSTCOLORS];
     static bool		c_NeedToInitCustColors;
     static void		InitCustColors();
     static void		SaveCustColors();

	 // Dialog Data
     //{{AFX_DATA(CMyColorDialog)
     //}}AFX_DATA

protected:
     // ClassWizard generate virtual function overrides
     //{{AFX_VIRTUAL(CMyColorDialog)
     virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
     //}}AFX_VIRTUAL

	 // Generated message map functions
     //{{AFX_MSG(CMyColorDialog)
     //}}AFX_MSG
     DECLARE_MESSAGE_MAP()
};

IMPLEMENT_DYNCREATE( CMyColorDialog, CColorDialog )

bool CMyColorDialog::c_NeedToInitCustColors = true;
COLORREF CMyColorDialog::c_CustColors[];
COLORREF CMyColorDialog::c_LastCustColors[];

#define SECTION _T("Custom Colors")

void CMyColorDialog::InitCustColors() {
	for ( int i = 0; i < NCUSTCOLORS; i++) {
		CString entry;
		entry.Format( "tool_color%d", i);
		idCVar *cvar = cvarSystem->Find( entry );
		if ( cvar ) {
			c_LastCustColors[i] = c_CustColors[i] = cvar->GetInteger();
		} else {
			c_LastCustColors[i] = c_CustColors[i] = RGB( 255, 255, 255 );
		}
	}
	c_NeedToInitCustColors= false;
}

void CMyColorDialog::SaveCustColors() {
	for (int i = 0; i < NCUSTCOLORS; i++) {
		if ( c_LastCustColors[i] != c_CustColors[i] ) {
			CString entry;
			entry.Format( "tool_color%d", i );
			if ( c_CustColors[i] == RGB( 255, 255, 255 ) ) {
				cvarSystem->SetCVarString( entry, "" );
			} else {
				cvarSystem->SetCVarString( entry, va( "%d", c_CustColors[i] ), CVAR_TOOL );
			}
			c_LastCustColors[i] = c_CustColors[i];
		}
	}
}

CMyColorDialog::CMyColorDialog( COLORREF clrInit, DWORD dwFlags, 
		CWnd* pParentWnd) : CColorDialog(clrInit,dwFlags,pParentWnd)
{
     //{{AFX_DATA_INIT(CMyColorDialog)
     //}}AFX_DATA_INIT
     if (c_NeedToInitCustColors) {
          InitCustColors();
     }
     m_cc.lpCustColors = c_CustColors;
}

int CMyColorDialog::DoModal() {
     int code = CColorDialog::DoModal();
     SaveCustColors();
     return code;
}

void CMyColorDialog::DoDataExchange(CDataExchange* pDX) {
     // overridden (calls this base class)
     CColorDialog::DoDataExchange(pDX);
     //{{AFX_DATA_MAP(CMyColorDialog)
     //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMyColorDialog, CColorDialog)
//{{AFX_MSG_MAP(CMyColorDialog)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

COLORREF DoOldColor(COLORREF cr) {
	CMyColorDialog dlg(cr, CC_FULLOPEN | CC_RGBINIT | CC_ANYCOLOR);
	if (dlg.DoModal() == IDOK) {
		return dlg.GetColor();
	}
	return cr;
}

// New color picker

// Original ColorPicker/DIB source by Rajiv Ramachandran <rrajivram@hotmail.com>
// included with Permission from the author

#define RADUIS	100

#define IN_NOTHING		0
#define IN_CIRCLE		1
#define IN_BRIGHT		2
#define IN_OVERBRIGHT	3

int Distance(CPoint pt1,CPoint pt2);


double Slope( CPoint pt1,CPoint pt2 ) {
	double x,y;

	y = pt2.y - pt1.y;
	x = pt2.x - pt1.x;
	if( x ) {
		return y/x;
	} else {
		return BAD_SLOPE;
	}
}

CPoint Intersection(LineDesc l1,LineDesc l2)
{
	CPoint pt;
	double x,y;

	if(l1.slope == l2.slope)
	{
		// Parallel lines, no intersection
		return CPoint(0,0);
	}
	else
	if(l1.slope == BAD_SLOPE )
	{
		// First Line is vertical, eqn is x=0
		// Put x = 0 in second line eqn to get y;
		x = l1.x;
		y = l2.slope * x + l2.c;
	}
	else
	if(l2.slope == BAD_SLOPE)
	{
		// second line is vertical Equation of line is x=0;
		// Put x = 0 in first line eqn to get y;
		x = l2.x;
		y = l1.slope * l2.x + l1.c;
	}
	else
	{
		y = ((l1.c * l2.slope) - (l2.c * l1.slope))/(l2.slope - l1.slope);
		x = (y - l1.c)/l1.slope;
	}

	return CPoint((int)x,(int)y);
}

double FindC(LineDesc& l)
{
	double c;

	if(l.slope == BAD_SLOPE)
	{
		c = l.y;
	}
	else
	{
		c = l.y - l.slope * l.x;
	}
	return c;
}

CPoint PointOnLine(CPoint pt1,CPoint pt2,int len,int maxlen )
{
	double x,y,m,a,c,C,A;
	double a2,c2,m2,B;	
	CPoint opt = pt1;
	CPoint pt;

	pt1.y *= -1;
	pt2.y *= -1;

	a = (double)len;

	if(pt2.x != pt1.x)
	{
		m = (double)(pt2.y - pt1.y)/(pt2.x - pt1.x);
		m2 = m*m;
		a2 = a*a;
		c = (double)pt1.y - m * (double)pt1.x;
		c2 = c*c;


		A = 1.0;
		
		x = pt1.x;

		B = 2.0 * pt1.x;

		x *= x;
		C = x - a2/(m2 + 1);
		
		x = (B + idMath::Sqrt(B*B - (4.0*A*C)))/(2.0*A);
		y = m*x + c;
		pt = CPoint((int)x,(int)y);
		if(Distance(pt,pt1) > maxlen || Distance(pt,pt2) > maxlen)
		{
			x = (B - idMath::Sqrt(B*B - (4.0*A*C)))/(2.0 * A);
			y = m*x + c;
			pt = CPoint((int)x,(int)y);
		}
	}
	else
	{
		a2 = a*a;
		y = idMath::Sqrt(a2);
		x = 0;
		pt = CPoint((int)x,(int)y);
		pt += pt1;
		if(Distance(pt,pt1) > maxlen || Distance(pt,pt2) > maxlen)
		{
			y = -1.0 *y;
			pt = CPoint((int)x,(int)y);
			pt+=pt1;
		}
	}
	pt.y *= -1;
	return pt;
}


int Distance(CPoint pt1,CPoint pt2)
{
	double a;
	int x,y;

	y = (pt1.y - pt2.y);
	y *= y;

	x = (pt1.x - pt2.x);
	x *= x;
	
	a = (double)x + (double)y ;
	a = idMath::Sqrt(a);
	return (int)a;
}

double AngleFromPoint(CPoint pt,CPoint center)
{
	double x,y;

	y = -1 * (pt.y - center.y);
	x = pt.x - center.x;
	if(x == 0 && y == 0)
	{
		return 0.0;
	}
	else
	{
		return atan2(y,x);
	}
}

CPoint PtFromAngle(double angle,double sat,CPoint center)
{
	angle = DEG2RAD(angle);
	sat = TOSCALE(sat);

	double x,y;

	x = sat * cos(angle);
	y = sat * sin(angle);

	CPoint pt;

	pt = CPoint((int)x,(int)y);
	pt.y *= -1;
	pt += center;
	return pt;
}

RGBType HSVType::toRGB()
{
	RGBType rgb;

	if(!h  && !s)
	{
		rgb.r = rgb.g = rgb.b = v;
	}

	double min,max,delta,hue;
	
	max = v;
	delta = (max * s)/255.0;
	min = max - delta;

	hue = h;
	if(h > 300 || h <= 60)
	{
		rgb.r = (int)max;
		if(h > 300)
		{
			rgb.g = (int)min;
			hue = (hue - 360.0)/60.0;
			rgb.b = (int)((hue * delta - min) * -1);
		}
		else
		{
			rgb.b = (int)min;
			hue = hue / 60.0;
			rgb.g = (int)(hue * delta + min);
		}
	}
	else if(h > 60 && h < 180)
	{
		rgb.g = (int)max;
		if(h < 120)
		{
			rgb.b = (int)min;
			hue = (hue/60.0 - 2.0 ) * delta;
			rgb.r = (int)(min - hue);
		}
		else
		{
			rgb.r = (int)min;
			hue = (hue/60 - 2.0) * delta;
			rgb.b = (int)(min + hue);
		}
	}
	else
	{
		rgb.b = (int)max;
		if(h < 240)
		{
			rgb.r = (int)min;
			hue = (hue/60.0 - 4.0 ) * delta;
			rgb.g = (int)(min - hue);
		}
		else
		{
			rgb.g = (int)min;
			hue = (hue/60 - 4.0) * delta;
			rgb.r = (int)(min + hue);
		}
	}
	return rgb;
}


HSVType RGBType::toHSV()
{
	HSVType hsv;

	double min,max,delta,temp;

	min = __min(r,__min(g,b));	
	max = __max(r,__max(g,b));
	delta = max - min;
	
	hsv.v = (int)max;
	if(!delta)
	{
		hsv.h = hsv.s = 0;
	}
	else
	{
		temp = delta/max;
		hsv.s = (int)(temp*255);

		if(r == (int)max)
		{
			temp = (double)(g-b)/delta;
		}
		else
		if(g == (int)max)
		{
			temp = 2.0 + ((double)(b-r)/delta);
		}
		else
		{
			temp = 4.0 + ((double)(r-g)/delta);
		}
		temp *= 60;
		if(temp < 0)
		{
			temp+=360;
		}
		if(temp == 360)
		{
			temp = 0;
		}
		hsv.h = (int)temp;
	}
	return hsv;

}
/////////////////////////////////////////////////////////////////////////////
// CDialogColorPicker dialog


CDialogColorPicker::CDialogColorPicker( COLORREF c, CWnd* pParent /*=NULL*/)
	: CDialog(CDialogColorPicker::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDialogColorPicker)
	m_overBright = 0.0f;
	//}}AFX_DATA_INIT

	Vertex = CPoint(102,108);
	Top = CPoint(102,9);
	Left = CPoint(23,147);
	Right = CPoint(181,147);

	color.r = GetRValue(c);
	color.g = GetGValue(c);
	color.b = GetBValue(c);

	m_OldColor = color;
	hsvColor = color.toHSV();
	m_bInMouse = FALSE;
	m_bInitOver = FALSE;
	m_bInDrawAll = FALSE;
	overBright = 1.0f;
	UpdateParent = NULL;
}

CDialogColorPicker::~CDialogColorPicker()
{
	if(m_RgbBitmap.GetSafeHandle())
	{
		m_RgbBitmap.DeleteObject();
	}
	if(m_HsbBitmap.GetSafeHandle())
	{
		m_HsbBitmap.DeleteObject();
	}
}


BEGIN_MESSAGE_MAP(CDialogColorPicker, CDialog)
	//{{AFX_MSG_MAP(CDialogColorPicker)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_PAINT()
	ON_EN_CHANGE(IDC_EDIT_BLUE, OnChangeEditBlue)
	ON_EN_CHANGE(IDC_EDIT_GREEN, OnChangeEditGreen)
	ON_EN_CHANGE(IDC_EDIT_HUE, OnChangeEditHue)
	ON_EN_CHANGE(IDC_EDIT_RED, OnChangeEditRed)
	ON_EN_CHANGE(IDC_EDIT_SAT, OnChangeEditSat)
	ON_EN_CHANGE(IDC_EDIT_VAL, OnChangeEditVal)
	ON_EN_CHANGE(IDC_EDIT_OVERBRIGHT, OnChangeEditOverbright)
	ON_BN_CLICKED(IDC_BTN_OLDCOLOR, OnBtnColor)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDialogColorPicker message handlers

void CDialogColorPicker::OnLButtonDown(UINT nFlags, CPoint point) {
	if(hsbRect.PtInRect(point)) {
		m_bInMouse = FALSE;
		if(InCircle(point)) {
			m_nMouseIn = IN_CIRCLE;
		} else if (InBright(point)) {
			m_nMouseIn = IN_BRIGHT;
		} else if (InOverBright(point)) {
			m_nMouseIn = IN_OVERBRIGHT;
		} else {
			m_nMouseIn = IN_NOTHING;
		}

		if(m_nMouseIn) {
			SetCapture();
			TrackPoint(point);
		}
	}
	else if (rgbRect.PtInRect(point)) {
		m_nMouseIn = IN_NOTHING;
		if(rects[RED].PtInRect(point)) {
			SetCapture();
			m_bInMouse = TRUE;
			nIndex = RED;
		} else if (rects[GREEN].PtInRect(point)) {
			SetCapture();
			m_bInMouse = TRUE;
			nIndex = GREEN;
		} else if (rects[BLUE].PtInRect(point)) {
			SetCapture();
			m_bInMouse = TRUE;
			nIndex = BLUE;
		}
	}
	
	CDialog::OnLButtonDown(nFlags, point);
}

void CDialogColorPicker::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if(GetCapture() == this)
	{
		ReleaseCapture();
		m_bInMouse = FALSE;
	}
	CDialog::OnLButtonUp(nFlags, point);
}

void CDialogColorPicker::OnMouseMove(UINT nFlags, CPoint point) 
{
	if(GetCapture() == this && m_nMouseIn)
	{
		TrackPoint(point);
	}
	else if(GetCapture() == this && m_bInMouse)
	{
		double val;
		BOOL bChange = FALSE;

		if(nIndex == RED)
		{
			if(point.y > Vertex.y)
			{
				point.y = Vertex.y;
			}
			point.x = Vertex.x;
			val = Distance(point,Vertex);
			if(val > RedLen)
			{
				val = RedLen;
			}
			CClientDC dc(this);
			DrawLines(&dc);
			val = (val/RedLen)*255;
			color.r = (int)val;
			CPoint pt;
			pt = PointOnLine(Vertex,Top,(color.r*RedLen)/255,RedLen);
			rects[RED] = CRect(pt.x - RECT_WIDTH ,pt.y-RECT_WIDTH ,pt.x+RECT_WIDTH ,pt.y+RECT_WIDTH );
			CalcCuboid();
			DrawLines(&dc);
			bChange = TRUE;
		}
		else if(nIndex == GREEN)
		{
			if(point.x > Vertex.x)
			{
				point.x = Vertex.x;
			}
			point.y = rects[GREEN].top + RECT_WIDTH;
			val = Distance(point,Vertex);
			if(val > GreenLen)
			{
				val = GreenLen;
			}
			CClientDC dc(this);
			DrawLines(&dc);
			val = (val/GreenLen)*255;
			color.g = (int)val;
			CPoint pt;
			pt = PointOnLine(Vertex,Left,(color.g*GreenLen)/255,GreenLen);
			rects[GREEN] = CRect(pt.x - RECT_WIDTH ,pt.y-RECT_WIDTH ,pt.x+RECT_WIDTH ,pt.y+RECT_WIDTH );
			CalcCuboid();
			DrawLines(&dc);
			bChange = TRUE;
		}
		else if(nIndex == BLUE)
		{
			if(point.x < Vertex.x)
			{
				point.x = Vertex.x;
			}
			point.y = rects[BLUE].top + RECT_WIDTH;
			val = Distance(point,Vertex);
			if(val > BlueLen)
			{
				val = BlueLen;
			}
			CClientDC dc(this);
			DrawLines(&dc);
			val = (val/BlueLen)*255;
			color.b = (int)val;
			CPoint pt;
			pt = PointOnLine(Vertex,Right,(color.b*GreenLen)/255,BlueLen);
			rects[BLUE] = CRect(pt.x - RECT_WIDTH ,pt.y-RECT_WIDTH ,pt.x+RECT_WIDTH ,pt.y+RECT_WIDTH );
			CalcCuboid();
			DrawLines(&dc);
			bChange = TRUE;
		}
		if(bChange)
		{
			hsvColor = color.toHSV();
			SetEditVals();
			CClientDC dc(this);
			DrawMarkers(&dc);
			CalcRects();
			SetDIBPalette();

			InvalidateRect(&brightRect,FALSE);
			DrawHSB(&dc);
		}
	}
	CDialog::OnMouseMove(nFlags, point);
}

void CDialogColorPicker::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	DrawHSB(&dc);
	DrawRGB(&dc);
}

BOOL CDialogColorPicker::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	GetDlgItem(IDC_STATIC_RGB_RECT)->GetWindowRect(&rgbRect);
	GetDlgItem(IDC_STATIC_HSB_RECT)->GetWindowRect(&hsbRect);
	ScreenToClient(&rgbRect);
	ScreenToClient(&hsbRect);

	GetDlgItem(IDC_STATIC_NEWCOLOR)->GetWindowRect(&NewColorRect);
	ScreenToClient(&NewColorRect);

	
	CWindowDC dc(NULL);
	CSize bmSize;

	//	Set Up HSB
	
	memDC.CreateCompatibleDC(&dc);

	LoadMappedBitmap(m_HsbBitmap,IDB_BITMAP_HSB,bmSize);
	hsbWidth = bmSize.cx;
	hsbHeight = bmSize.cy;

	hsbRect.InflateRect(-5,-5);
	hsbRect.top += 20;
	hsbRect.left += 10;

	m_Centre = CPoint(RADIUS,RADIUS);
	m_Centre += CPoint(hsbRect.left,hsbRect.top);

	brightRect = CRect(hsbRect.left+hsbWidth+20,hsbRect.top,hsbRect.left+hsbWidth+20+20,hsbRect.top + hsbHeight);
	overBrightRect = brightRect;
	overBrightRect.OffsetRect(brightRect.Width() + 5, 0);

	CreateBrightDIB();
	CalcRects();
	SetDIBPalette();


	//	Set Up RGB

	LoadMappedBitmap(m_RgbBitmap,IDB_BITMAP_RGB,bmSize);
	rgbWidth = bmSize.cx;
	rgbHeight = bmSize.cy;

	rgbRect.InflateRect(-5,-5);
	rgbRect.top+=10;
	rgbRect.left-=3;

	CPoint pt = CPoint(rgbRect.left,rgbRect.top);

	Top += pt;
	Left += pt;
	Right += pt;
	Vertex += pt;
	// TODO: Add your specialized code here and/or call the base class

	RedLen = Distance(Vertex,Top);
	GreenLen = Distance(Vertex,Left);
	BlueLen = Distance(Vertex,Right);

	CalcSlopes();
	CalcCuboid();

	SetSpinVals();
	SetEditVals();

	m_bInitOver = TRUE;

	SetTimer(0, 50, NULL);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDialogColorPicker::DrawMarkers(CDC *pDC)
{
	if(m_CurrentRect.Width())
	{
		CPen *oldPen;
		CBrush *oldBrush;
		int oldMode;
		CRect cr = m_CurrentRect;

		oldPen = (CPen *)pDC->SelectStockObject(WHITE_PEN);
		oldBrush = (CBrush *)pDC->SelectStockObject(NULL_BRUSH);

		oldMode = pDC->SetROP2(R2_XORPEN);

		pDC->Rectangle(&cr);
		CPen pen;
		pen.CreatePen(PS_SOLID,2,RGB(255,255,255));
		pDC->SelectObject(&pen);
		pDC->Rectangle(&brightMark);

		pDC->SelectObject(oldPen);
		pDC->SelectObject(oldBrush);
		pDC->SetROP2(oldMode);
		pen.DeleteObject();
	}
}

BOOL CDialogColorPicker::InCircle(CPoint pt)
{
	return Distance(pt,m_Centre) <= RADIUS;
}

BOOL CDialogColorPicker::InBright(CPoint pt)
{
	return brightRect.PtInRect(pt);
}

BOOL CDialogColorPicker::InOverBright(CPoint pt)
{
	return overBrightRect.PtInRect(pt);
}

void CDialogColorPicker::TrackPoint(CPoint pt)
{
	if(m_nMouseIn == IN_CIRCLE)
	{
		CClientDC dc(this);

		DrawMarkers(&dc);
		
		hsvColor.h = (int)RAD2DEG(AngleFromPoint(pt,m_Centre));
		if(hsvColor.h < 0)
		{
			hsvColor.h += 360;
		}
		hsvColor.s = (int)SCALETOMAX(Distance(pt,m_Centre));
		if(hsvColor.s > 255) hsvColor.s = 255;

		SetDIBPalette();
		CalcRects();


		InvalidateRect(&brightRect,FALSE);

		DrawMarkers(&dc);

		color = hsvColor.toRGB();
		SetEditVals();
		DrawLines(&dc);
		CalcCuboid();
		DrawRGB(&dc);

	}
	else if(m_nMouseIn == IN_BRIGHT)
	{
		double d;
		d = brightRect.bottom - pt.y;
		d *= 255;
		d /= brightRect.Height();
		if(d < 0 ) d = 0;
		if(d > 255) d = 255;
		CClientDC dc(this);
		DrawMarkers(&dc);
		hsvColor.v = (int)d;
		CalcRects();
		DrawMarkers(&dc);

		color = hsvColor.toRGB();
		SetEditVals();
		DrawLines(&dc);
		CalcCuboid();
		DrawRGB(&dc);
	}
}

void CDialogColorPicker::CreateBrightDIB()
{
	CDIB& d = m_BrightDIB;

	d.Create(brightRect.Width(),brightRect.Height(),8);
	for(int i=0; i < d.Height(); i++)
	{
		memset(d.GetLinePtr(i),i,d.Width());
	}
}

void CDialogColorPicker::SetDIBPalette()
{
	BYTE palette[768],*p;
	HSVType h = hsvColor;
	double d;

	d = 255.0/brightRect.Height();
	p = palette;
	for(int i=brightRect.Height()-1; i >= 0 ;i--,p+=3)
	{
		h.v = (int)((double)i * d);
		RGBType rgb = h.toRGB();
		p[0] = rgb.r;
		p[1] = rgb.g;
		p[2] = rgb.b;
	}
	m_BrightDIB.SetPalette(palette);
}

void CDialogColorPicker::CalcRects()
{
	CPoint pt;

	pt = PtFromAngle(hsvColor.h,hsvColor.s,m_Centre);
	m_CurrentRect = CRect(pt.x - RECT_WIDTH,pt.y - RECT_WIDTH,pt.x+RECT_WIDTH,pt.y + RECT_WIDTH);	
	
	int y;

	y = (int)(((double)hsvColor.v/255)*brightRect.Height()); 	
	y = brightRect.bottom - y;
	brightMark = CRect(brightRect.left - 2, y - 4, brightRect.right+2,y+4);
}


void CDialogColorPicker::DrawHSB(CDC *pDC)
{
	if(m_HsbBitmap.GetSafeHandle())
	{
	CBitmap *pOldBitmap ;
		pOldBitmap  = (CBitmap *)memDC.SelectObject(&m_HsbBitmap);
		pDC->BitBlt(hsbRect.left,hsbRect.top,hsbWidth,hsbHeight,&memDC,0,0,SRCCOPY);
		m_BrightDIB.BitBlt(pDC->m_hDC,brightRect.left,brightRect.top,brightRect.Width(),brightRect.Height(),0,0);
		DrawMarkers(pDC);
		memDC.SelectObject(pOldBitmap);
	}
}

void CDialogColorPicker::DrawRGB(CDC *pDC)
{
	if(m_RgbBitmap.GetSafeHandle())
	{
	CBitmap *pOldBitmap ;
		pOldBitmap  = (CBitmap *)memDC.SelectObject(&m_RgbBitmap);
		pDC->BitBlt(rgbRect.left,rgbRect.top,rgbWidth,rgbHeight,&memDC,0,0,SRCCOPY);
		DrawLines(pDC);
		memDC.SelectObject(pOldBitmap);
	}
}

void CDialogColorPicker::DrawLines(CDC *pDC)
{
	CPoint pt[3];

	pt[0] = PointOnLine(Vertex,Top,(color.r*RedLen)/255,RedLen);
	pt[1] = PointOnLine(Vertex,Left,(color.g*GreenLen)/255,GreenLen);
	pt[2] = PointOnLine(Vertex,Right,(color.b*BlueLen)/255,BlueLen);

	COLORREF col = RGB(255,255,255);
	CRect cr;

	for(int i = 0; i < 3; i++ ) {
		cr = CRect(pt[i].x - RECT_WIDTH ,pt[i].y-RECT_WIDTH ,pt[i].x+RECT_WIDTH ,pt[i].y+RECT_WIDTH );
		rects[i] = cr;
		DrawXorRect(pDC,cr);
	}

	CPen *oldPen;
	int oldMode;

	oldPen = (CPen *)pDC->SelectStockObject(WHITE_PEN);
	oldMode = pDC->SetROP2(R2_XORPEN);

	/*
	Draw the following lines :
		
		1 -2 
		2 -3
		3 - 4
		4- 5
		5 -2
		5 - 6
		6-7
		7-4
	*/
	pDC->MoveTo(m_Cuboid[1]);
	pDC->LineTo(m_Cuboid[2]);
	pDC->LineTo(m_Cuboid[3]);
	pDC->LineTo(m_Cuboid[4]);
	pDC->LineTo(m_Cuboid[5]);
	pDC->LineTo(m_Cuboid[2]);

	pDC->MoveTo(m_Cuboid[5]);
	pDC->LineTo(m_Cuboid[6]);
	pDC->LineTo(m_Cuboid[7]);
	pDC->LineTo(m_Cuboid[4]);
	
	pDC->MoveTo(m_Cuboid[1]);
	pDC->LineTo(m_Cuboid[6]);

	pDC->SelectObject(oldPen);
	pDC->SetROP2(oldMode);

	DrawFilledColor(pDC,NewColorRect,color.color());
}

void CDialogColorPicker::DrawXorRect(CDC *pDC,CRect& cr)
{
	CPen pen,*oldPen;
	CBrush *oldBrush;
	int oldMode;

	pen.CreatePen(PS_SOLID,1,RGB(255,255,255));
	oldPen = (CPen *)pDC->SelectObject(&pen);
	oldBrush = (CBrush *)pDC->SelectStockObject(NULL_BRUSH);
	oldMode =pDC->SetROP2(R2_XORPEN);
	pDC->Rectangle(&cr);
	pDC->SetROP2(oldMode);
	pDC->SelectObject(oldPen);
	pDC->SelectObject(oldBrush);
	pen.DeleteObject();

}

void CDialogColorPicker::CalcSlopes()
{
	lines[RED].slope = Slope(Top,Vertex);
	lines[GREEN].slope = Slope(Left,Vertex);
	lines[BLUE].slope = Slope(Right,Vertex);

	int i;

	for( i = 0; i < 3; i++ ) {
		lines[i].x = Vertex.x;
		lines[i].y = Vertex.y;
		lines[i].c = FindC(lines[i]);
	}
}

/*

  Cuboid points
   0 = vertex
   1 = Red Axis
   2 = Red Green Intersection
   3 = Green Axis
   4 = Blue Green Intersection
   5 = Blue Green Red Intersection
   6 = Red Blue Intersection
   7 = Blue Axis

  Draw the following lines :
	
	1 -2 
	2 -3
	3 - 4
	4- 5
	5 -2
	5 - 6
	6-7
	7-4
*/

void CDialogColorPicker::CalcCuboid()
{
	double rLen,gLen,bLen;

	rLen = (double)(color.r*RedLen)/255;
	gLen = (double)(color.g*GreenLen)/255;
	bLen = (double)(color.b*BlueLen)/255;

	LineDesc l[12];

	m_Cuboid[0] = Vertex;
	m_Cuboid[1] = PointOnLine(Vertex,Top,(int)rLen,RedLen);
	m_Cuboid[3] = PointOnLine(Vertex,Left,(int)gLen,GreenLen);
	m_Cuboid[7] = PointOnLine(Vertex,Right,(int)bLen,BlueLen);

	l[0] = lines[RED];
	l[1] = lines[GREEN];
	l[2] = lines[BLUE];

	l[3].slope = lines[GREEN].slope;
	l[3].x = m_Cuboid[1].x;
	l[3].y = m_Cuboid[1].y;
	l[3].c = FindC(l[3]);
	
	l[4].slope = lines[RED].slope;
	l[4].x = m_Cuboid[3].x;
	l[4].y = m_Cuboid[3].y;
	l[4].c = FindC(l[4]);

	l[5].slope = lines[BLUE].slope;
	l[5].x = m_Cuboid[3].x;
	l[5].y = m_Cuboid[3].y;
	l[5].c = FindC(l[5]);
	
	l[6].slope = lines[GREEN].slope;
	l[6].x = m_Cuboid[7].x;
	l[6].y = m_Cuboid[7].y;
	l[6].c = FindC(l[6]);

	l[10].slope = lines[BLUE].slope;
	l[10].x = m_Cuboid[1].x;
	l[10].y = m_Cuboid[1].y;
	l[10].c = FindC(l[10]);
	
	l[11].slope = lines[RED].slope;
	l[11].x = m_Cuboid[7].x;
	l[11].y = m_Cuboid[7].y;
	l[11].c = FindC(l[11]);

	m_Cuboid[2] = Intersection(l[3],l[4]);
	m_Cuboid[4] = Intersection(l[5],l[6]);
	m_Cuboid[6] = Intersection(l[10],l[11]);

	l[7].slope = lines[RED].slope;
	l[7].x = m_Cuboid[4].x;
	l[7].y = m_Cuboid[4].y;
	l[7].c = FindC(l[7]);
	
	l[8].slope = lines[BLUE].slope;
	l[8].x = m_Cuboid[2].x;
	l[8].y = m_Cuboid[2].y;
	l[8].c = FindC(l[8]);

	m_Cuboid[5] = Intersection(l[7],l[8]);
		
}

void CDialogColorPicker::SetSpinVals()
{
	((CSpinButtonCtrl *)GetDlgItem(IDC_SPIN_RED))->SetRange(0,255);
	((CSpinButtonCtrl *)GetDlgItem(IDC_SPIN_GREEN))->SetRange(0,255);
	((CSpinButtonCtrl *)GetDlgItem(IDC_SPIN_BLUE))->SetRange(0,255);

	((CSpinButtonCtrl *)GetDlgItem(IDC_SPIN_HUE))->SetRange(0,360);
	((CSpinButtonCtrl *)GetDlgItem(IDC_SPIN_SAT))->SetRange(0,255);
	((CSpinButtonCtrl *)GetDlgItem(IDC_SPIN_VAL))->SetRange(0,255);

	((CSpinButtonCtrl *)GetDlgItem(IDC_SPIN_OVERBRIGHT))->SetRange(0,1023);

}

void CDialogColorPicker::SetEditVals()
{
	((CSpinButtonCtrl *)GetDlgItem(IDC_SPIN_RED))->SetPos(color.r);
	((CSpinButtonCtrl *)GetDlgItem(IDC_SPIN_GREEN))->SetPos(color.g);
	((CSpinButtonCtrl *)GetDlgItem(IDC_SPIN_BLUE))->SetPos(color.b);

	((CSpinButtonCtrl *)GetDlgItem(IDC_SPIN_HUE))->SetPos(hsvColor.h);
	((CSpinButtonCtrl *)GetDlgItem(IDC_SPIN_SAT))->SetPos(hsvColor.s);
	((CSpinButtonCtrl *)GetDlgItem(IDC_SPIN_VAL))->SetPos(hsvColor.v);

}

void CDialogColorPicker::OnChangeEditBlue() 
{
	int b;

	b = GetDlgItemInt(IDC_EDIT_BLUE);
	if( b != color.b && m_bInitOver)
	{
		color.b = b;
		if(color.b < 0) color.b = 0;
		if(color.b > 255) color.b = 255;
		hsvColor = color.toHSV();
		DrawAll();
	}
}

void CDialogColorPicker::OnChangeEditGreen() 
{
	int g;
	
	g = GetDlgItemInt(IDC_EDIT_GREEN);
	if(g != color.g && m_bInitOver)
	{
		color.g = g;
		if(color.g < 0) color.g = 0;
		if(color.g > 255) color.g = 255;
		hsvColor = color.toHSV();
		DrawAll();
	}
}

void CDialogColorPicker::OnChangeEditRed() 
{
	int r;

	r = GetDlgItemInt(IDC_EDIT_RED);
	if(r != color.r && m_bInitOver)
	{
		color.r = r;
		if(color.r < 0) color.r = 0;
		if(color.r > 255) color.r = 255;
		hsvColor = color.toHSV();
		DrawAll();
	}
}

void CDialogColorPicker::OnChangeEditHue() 
{
	int h;

	h = GetDlgItemInt(IDC_EDIT_HUE);
	if(h != hsvColor.h && m_bInitOver)
	{
		hsvColor.h = h;
		if(hsvColor.h < 0) hsvColor.h = 0;
		if(hsvColor.h > 359) hsvColor.h = 359;
		color = hsvColor.toRGB();
		DrawAll();
	}
}

void CDialogColorPicker::OnChangeEditSat() 
{
	int s;

	s = GetDlgItemInt(IDC_EDIT_SAT);
	if(s != hsvColor.s && m_bInitOver)
	{
		hsvColor.s = s;
		if(hsvColor.s < 0) hsvColor.s = 0;
		if(hsvColor.s > 255) hsvColor.s = 255;
		color = hsvColor.toRGB();
		DrawAll();
	}
}

void CDialogColorPicker::OnChangeEditVal() 
{
	int v;

	v = GetDlgItemInt(IDC_EDIT_VAL);
	if(v != hsvColor.v && m_bInitOver)
	{
		hsvColor.v = v;
		if(hsvColor.v < 0) hsvColor.v = 0;
		if(hsvColor.v > 255) hsvColor.v = 255;
		color = hsvColor.toRGB();
		DrawAll();
	}
}

void CDialogColorPicker::OnChangeEditOverbright() {
	CString str;
	GetDlgItemText(IDC_EDIT_OVERBRIGHT, str);
	if(m_bInitOver) {
		overBright = atof(str);
	}
}

void CDialogColorPicker::DrawAll()
{
	if(m_bInitOver && !m_bInDrawAll)
	{
		CClientDC dc(this);

		DrawMarkers(&dc);
		DrawLines(&dc);
		m_bInDrawAll = TRUE;
		CalcCuboid();
		CalcRects();
		SetDIBPalette();
		DrawRGB(&dc);
		DrawHSB(&dc);
		SetEditVals();
		m_bInDrawAll = FALSE;
	}
}

void CDialogColorPicker::DrawFilledColor(CDC *pDC,CRect cr,COLORREF c)
{
	pDC->FillSolidRect(&cr,c);
	pDC->Draw3dRect(&cr,RGB(0,0,0),RGB(0,0,0));
	cr.InflateRect(-1,-1);
	pDC->Draw3dRect(&cr,RGB(192,192,192),RGB(128,128,128));
}

void CDialogColorPicker::LoadMappedBitmap(CBitmap& bitmap,UINT nIdResource,CSize& size)
{
	CBitmap *pOldBitmap;
	
	if(bitmap.GetSafeHandle()) bitmap.DeleteObject();
	
	if(bitmap.LoadBitmap(nIdResource))
	{

		int width,height;
		BITMAP bmInfo;

		::GetObject(bitmap.m_hObject,sizeof(bmInfo),&bmInfo);	
		width = bmInfo.bmWidth;
		height = bmInfo.bmHeight;

		COLORREF colorWindow = ::GetSysColor(COLOR_3DFACE);
		COLORREF sourceColor = RGB(192,192,192);

		pOldBitmap = (CBitmap *)memDC.SelectObject(&bitmap);
	
		int i,j;
		
		for(i=0; i < height; i++)
		{
			for(j=0; j < width; j++)
			{
				if(memDC.GetPixel(j,i) == sourceColor)
				{
					memDC.SetPixel(j,i,colorWindow);
				}
			}
		}

		memDC.SelectObject(&pOldBitmap);
		size = CSize(width,height);
	}
}

void CDialogColorPicker::OnSysColorChange()
{
	CSize size;
	LoadMappedBitmap(m_HsbBitmap,IDB_BITMAP_HSB,size);
	LoadMappedBitmap(m_RgbBitmap,IDB_BITMAP_RGB,size);
}

void CDialogColorPicker::OnTimer(UINT nIDEvent)  {
	if ( UpdateParent ) {
		UpdateParent( color.r, color.g, color.b, 1.0f );
	}
}

void CDialogColorPicker::OnBtnColor() {
	COLORREF cr = DoOldColor(GetColor());
	color.r = GetRValue(cr);
	color.g = GetGValue(cr);
	color.b = GetBValue(cr);
	hsvColor = color.toHSV();
	DrawAll();
}

bool DoNewColor( int* i1, int* i2, int* i3, float *overBright, void (*Update)( float, float, float, float ) ) {
	COLORREF cr = (*i1) + ((*i2) <<8) + ((*i3) <<16);
	CDialogColorPicker dlg( cr );
	//CMyColorDialog dlg(cr, CC_FULLOPEN | CC_RGBINIT | CC_ANYCOLOR);

	dlg.UpdateParent = Update;

	if ( dlg.DoModal() == IDOK ) {
		*i1 = (dlg.GetColor() & 255);
		*i2 = ((dlg.GetColor() >> 8) & 255);
		*i3 = ((dlg.GetColor() >> 16) & 255);
		*overBright = dlg.GetOverBright();
		return true;
	}
	return false;
}
