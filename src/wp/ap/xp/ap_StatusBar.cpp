/* AbiWord
 * Copyright (C) 1998-2000 AbiSource, Inc.
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>

#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ap_StatusBar.h"
#include "ap_FrameData.h"
#include "gr_Graphics.h"
#include "xap_Frame.h"
#include "xav_View.h"
#include "ap_Strings.h"
#include "fl_DocLayout.h"
#include "fv_View.h"

#include "ut_timer.h"

// WL: ONLY ENABLE NEW STATUS BAR ON UNIX/GTK FOR NOW
#if !defined(WIN32) && !defined(__BEOS__) && !defined(__APPLE__)

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

#define AP_STATUSBARELEMENT_TEXT_ALIGN_LEFT 0
#define AP_STATUSBARELEMENT_TEXT_ALIGN_CENTER 1

#define AP_STATUSBAR_STATUSMESSAGE_REPRESENTATIVE_STRING "MMMMMMMMMMMMMMMMMMMMMMMMMMMM"
#define AP_STATUSBAR_INPUTMODE_REP_STRING "MMMMMMMM"
#define AP_STATUSBAR_INSERTMODE_REP_STRING "MMM"

#define AP_STATUSBAR_MAX_PAGES 999

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

AP_StatusBarField::AP_StatusBarField(AP_StatusBar * pSB)
{
	m_pSB = pSB;
	m_pStatusBarFieldListener = NULL;
	m_fillMethod = MAX_POSSIBLE;
}

AP_StatusBarField::~AP_StatusBarField(void)
{
	if(m_pStatusBarFieldListener)
		delete(m_pStatusBarFieldListener);
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

AP_StatusBarField_TextInfo::AP_StatusBarField_TextInfo(AP_StatusBar *pSB) 
	: AP_StatusBarField(pSB) 
{ 
	m_lenBufUCS = 0; 
	memset(m_bufUCS, 0, sizeof(m_bufUCS)); 
	strcpy(m_sRepresentativeString, "");
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
class AP_StatusBarField_PageInfo : public AP_StatusBarField_TextInfo
{
public:
	AP_StatusBarField_PageInfo(AP_StatusBar * pSB);

	virtual void		notify(AV_View * pView, const AV_ChangeMask mask);

private:
	UT_uint32			m_pageNr;
	UT_uint32			m_nrPages;

	const XML_Char *	m_szFormat;
};

AP_StatusBarField_PageInfo::AP_StatusBarField_PageInfo(AP_StatusBar * pSB)
	: AP_StatusBarField_TextInfo(pSB)
{
	m_pageNr = 0;
	m_nrPages = 0;

	m_szFormat = pSB->getFrame()->getApp()->getStringSet()->getValue(AP_STRING_ID_PageInfoField);
	m_fillMethod = REPRESENTATIVE_STRING;
	m_alignmentMethod = LEFT;

	sprintf(m_sRepresentativeString,m_szFormat,AP_STATUSBAR_MAX_PAGES,AP_STATUSBAR_MAX_PAGES);
}

void AP_StatusBarField_PageInfo::notify(AV_View * pavView, const AV_ChangeMask mask)
{
	FV_View * pView = (FV_View *)pavView;
	
	bool bNeedNewString = false;

	if (mask & (AV_CHG_PAGECOUNT))
	{
		UT_uint32 currentPage = pView->getCurrentPageNumForStatusBar(); // experiment: ripped out of area below
		UT_uint32 newPageCount = pView->getLayout()->countPages();

		if (newPageCount != m_nrPages)
		{
			bNeedNewString = true;
			m_nrPages = newPageCount;
			m_pageNr = currentPage; // experimented: ripped out of commented area below
		}
	}

// 	if (mask & (AV_CHG_MOTION | AV_CHG_PAGECOUNT))
// 	{
// 		UT_uint32 currentPage = pView->getCurrentPageNumForStatusBar();

// 		if (currentPage != m_pageNr)
// 		{
// 			bNeedNewString = true;
// 			m_pageNr = currentPage;
// 		}
// 	}

	if (bNeedNewString)
	{
		char buf[AP_MAX_MESSAGE_FIELD];
		sprintf(buf,m_szFormat,m_pageNr,m_nrPages);

		m_lenBufUCS = strlen(buf);
		UT_UCS4_strcpy_char(m_bufUCS,buf);		
		if (getListener())
			getListener()->notify();

	}
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

class AP_StatusBarField_StatusMessage : public AP_StatusBarField_TextInfo
{
 public:
	AP_StatusBarField_StatusMessage(AP_StatusBar * pSB);

	virtual void		notify(AV_View * pView, const AV_ChangeMask mask);
	void update(UT_UCS4Char *pMessage); // for receiving messages from the status bar itself
};

AP_StatusBarField_StatusMessage::AP_StatusBarField_StatusMessage(AP_StatusBar * pSB)
	: AP_StatusBarField_TextInfo(pSB)
{
	m_fillMethod = MAX_POSSIBLE;
	m_alignmentMethod = LEFT;
	strcpy(m_sRepresentativeString, AP_STATUSBAR_STATUSMESSAGE_REPRESENTATIVE_STRING);
}

void AP_StatusBarField_StatusMessage::notify(AV_View * /*pView*/, const AV_ChangeMask /*mask*/)
{
	UT_UCS4_strcpy(m_bufUCS, m_pSB->getStatusMessage());

	if (getListener())
		getListener()->notify();
}

void AP_StatusBarField_StatusMessage::update(UT_UCS4Char *pMessage)
{
	UT_UCS4_strcpy(m_bufUCS, pMessage);

	if (getListener())
		getListener()->notify();
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

class AP_StatusBarField_InputMode : public AP_StatusBarField_TextInfo
{
 public:
	AP_StatusBarField_InputMode(AP_StatusBar * pSB);
	
	virtual void		notify(AV_View * pView, const AV_ChangeMask mask);
};

AP_StatusBarField_InputMode::AP_StatusBarField_InputMode(AP_StatusBar * pSB)
	: AP_StatusBarField_TextInfo(pSB)
{
	const char * szInputMode = m_pSB->getFrame()->getInputMode();
	m_lenBufUCS = strlen(szInputMode);
	UT_UCS4_strcpy_char(m_bufUCS,szInputMode);

	m_fillMethod = REPRESENTATIVE_STRING;
	m_alignmentMethod = LEFT;
	strcpy(m_sRepresentativeString, AP_STATUSBAR_INPUTMODE_REP_STRING);
}

void AP_StatusBarField_InputMode::notify(AV_View * /*pavView*/, const AV_ChangeMask mask)
{
	if (mask & (AV_CHG_INPUTMODE))
	{
		const char * szInputMode = m_pSB->getFrame()->getInputMode();
		m_lenBufUCS = strlen(szInputMode);
		UT_UCS4_strcpy_char(m_bufUCS,szInputMode);

		if (getListener())
			getListener()->notify();
	}
}


//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

class AP_StatusBarField_InsertMode : public AP_StatusBarField_TextInfo
{
 public:
	AP_StatusBarField_InsertMode(AP_StatusBar * pSB);
	
	virtual void        notify(AV_View * pView, const AV_ChangeMask mask);

 private:
	UT_UCSChar m_InsertMode[2][AP_MAX_MESSAGE_FIELD];
	bool m_bInsertMode;
};

AP_StatusBarField_InsertMode::AP_StatusBarField_InsertMode(AP_StatusBar * pSB)
    : AP_StatusBarField_TextInfo(pSB)
{
    UT_UCS4_strcpy_char(m_InsertMode[(int)true],pSB->getFrame()->getApp()->getStringSet()->getValue(AP_STRING_ID_InsertModeFieldINS));
    UT_UCS4_strcpy_char(m_InsertMode[(int)false],pSB->getFrame()->getApp()->getStringSet()->getValue(AP_STRING_ID_InsertModeFieldOVR));

    m_bInsertMode = true;

    m_fillMethod = REPRESENTATIVE_STRING;
    m_alignmentMethod = CENTER;
    strcpy(m_sRepresentativeString, AP_STATUSBAR_INSERTMODE_REP_STRING);
}

void AP_StatusBarField_InsertMode::notify(AV_View * /*pavView*/, const AV_ChangeMask mask)
{
    if (mask & (AV_CHG_INSERTMODE))
    {
		AP_FrameData * pData = (AP_FrameData *) m_pSB->getFrame()->getFrameData();
		if (pData) {
			m_bInsertMode = pData->m_bInsertMode;
			UT_UCS4_strcpy(m_bufUCS, m_InsertMode[m_bInsertMode]);
		}

		if (getListener())
			getListener()->notify();
    }
}


//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

class AP_StatusBarField_Language : public AP_StatusBarField_TextInfo
{
 public:
	AP_StatusBarField_Language(AP_StatusBar * pSB);

	virtual void notify(AV_View * pView, const AV_ChangeMask mask);
};

AP_StatusBarField_Language::AP_StatusBarField_Language(AP_StatusBar * pSB)
	: AP_StatusBarField_TextInfo(pSB)
{
	m_fillMethod = REPRESENTATIVE_STRING;
	m_alignmentMethod = CENTER;
	strcpy(m_sRepresentativeString, "mm-MM"); // this should actually be longer.. see old code for info on why
}

void AP_StatusBarField_Language::notify(AV_View * pavView, const AV_ChangeMask mask)
{
	// TODO do we want our own bit for language change?
	//if (mask & (AV_CHG_INSERTMODE))
	{
		const char * szLang = NULL;

		const XML_Char ** props_in = NULL;
		if (pavView && static_cast<FV_View *>(pavView)->getCharFormat(&props_in))
		{
			szLang = UT_getAttribute("lang", props_in);
			FREEP(props_in);

			UT_UCS4_strcpy_char(m_bufUCS, szLang ? szLang : "");
		}

		if (getListener())
			getListener()->notify();
	}
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

// PROGRESSBAR. CURRENTLY UNUSED. MAY BE BROKEN. NEEDS TESTING.

AP_StatusBarField_ProgressBar::AP_StatusBarField_ProgressBar(AP_StatusBar * pSB)
	: AP_StatusBarField(pSB)
{
	m_ProgressStartPoint = 0;
	m_ProgressFlags = 0;
	m_ProgressTimer = NULL;
}

AP_StatusBarField_ProgressBar::~AP_StatusBarField_ProgressBar(void)
{
}

static void updateProgress(UT_Worker * pWorker)
{
    UT_ASSERT(pWorker);

    AP_StatusBarField_ProgressBar *pfspb;
    pfspb = (AP_StatusBarField_ProgressBar *)pWorker->getInstanceData();
    UT_ASSERT(pfspb);

    if(pfspb->getListener())
	    pfspb->getListener()->notify();
}

void AP_StatusBarField_ProgressBar::notify(AV_View * /*pView*/, const AV_ChangeMask /*mask*/)
{
	// do nothing, we get our information from the status bar	
}

void AP_StatusBarField_ProgressBar::setStatusProgressType(int start, int end, int flags)
{
	m_ProgressStart = 
		m_ProgressValue = start;
	m_ProgressEnd = end;
	m_ProgressFlags = flags;
	m_ProgressStartPoint = 0;

	if (m_ProgressTimer) {
		delete m_ProgressTimer;
		m_ProgressTimer = NULL;
	}

	if (m_ProgressStart == m_ProgressEnd &&
	    (m_ProgressFlags & PROGRESS_CMD_MASK) == PROGRESS_STARTBAR) {  
		m_ProgressTimer = UT_Timer::static_constructor(updateProgress, this);
		m_ProgressTimer->stop();
		m_ProgressTimer->set(50);	//Milliseconds
	}	
}

void AP_StatusBarField_ProgressBar::setStatusProgressValue(int value)
{
	m_ProgressValue = value;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

AP_StatusBar::AP_StatusBar(XAP_Frame * pFrame)
:       m_pFrame(pFrame),
        m_pView(0),
        m_bInitFields(false),
        m_pStatusMessageField(0)
{

#define DclField(type,var)								\
		type * var = new type(this);					\
		UT_ASSERT((var));								\
		m_vecFields.addItem((var));						\
		
		DclField(AP_StatusBarField_PageInfo, pf1);
		DclField(AP_StatusBarField_StatusMessage, pf2);

		m_pStatusMessageField = pf2;	// its in the vector, but we remember it explicitly
		                                // so that setStatusMessage() can do its thing.

		DclField(AP_StatusBarField_InsertMode, pf4);
		DclField(AP_StatusBarField_InputMode, pf3);
		
		DclField(AP_StatusBarField_Language, pf5);

		// TODO add other fields

#undef DclField
		

        m_bufUCS[0] = 0;
}

AP_StatusBar::~AP_StatusBar(void)
{
	UT_VECTOR_PURGEALL(AP_StatusBarField *, m_vecFields);
}

XAP_Frame * AP_StatusBar::getFrame(void) const
{
	return m_pFrame;
}

void AP_StatusBar::setView(AV_View * pView)
{
	m_pView = pView;

	// Register the StatusBar as a ViewListener on the View.
	// This lets us receive notify events as the user interacts
	// with the document (cmdCharMotion, etc).  This will let
	// us update the display as we move from block to block and
	// from column to column.

	AV_ListenerId lidTopRuler;
	m_pView->addListener(static_cast<AV_Listener *>(this),&lidTopRuler);

	if (!m_bInitFields)
	{
		
		m_bInitFields = true;
	}

	// force a full notify of all fields so that they all
	// completely update themselves.
	
	notify(pView,AV_CHG_ALL);
	
	return;
}

bool AP_StatusBar::notify(AV_View * pView, const AV_ChangeMask mask)
{
	// Handle AV_Listener events on the view.	

	// We choose to clear any status message we may have,
	// since it's a pain for the code which set the message
	// to hang around and clear it at some point in the future.
	// This way, message will get cleared any time the user does
	// something with the window.

	if (*m_bufUCS)
		setStatusMessage((UT_UCSChar *)NULL);
	
	// Let each field on the status bar update itself accordingly.
	
	UT_ASSERT(pView==m_pView);
	UT_uint32 kLimit = m_vecFields.getItemCount();
	UT_uint32 k;

	for (k=0; k<kLimit; k++)
	{
		AP_StatusBarField * pf = (AP_StatusBarField *)m_vecFields.getNthItem(k);
		if(pf)
		{
			pf->notify(pView,mask);
		}
	}

	return true;
}

void AP_StatusBar::setStatusMessage(UT_UCSChar * pBufUCS, int redraw)
{
	memset(m_bufUCS, 0, sizeof(m_bufUCS));

	if (pBufUCS && *pBufUCS) {
		UT_ASSERT(UT_UCS4_strlen(pBufUCS) < AP_MAX_MESSAGE_FIELD);
		UT_UCS4_strcpy(m_bufUCS, pBufUCS);
	}
	
 	AP_StatusBarField_StatusMessage * pf = (AP_StatusBarField_StatusMessage *)m_pStatusMessageField;
 	if(pf)
 		pf->update(pBufUCS);
}

void AP_StatusBar::setStatusMessage(const char * pBuf, int redraw)
{
	UT_uint32 len = ((pBuf && *pBuf) ? strlen(pBuf) : 0);
	UT_ASSERT(len*MB_LEN_MAX < AP_MAX_MESSAGE_FIELD);

	UT_UCSChar bufUCS[AP_MAX_MESSAGE_FIELD];

	if (len)
		UT_UCS4_strcpy_char(bufUCS,pBuf);
	else
		memset(bufUCS,0,sizeof(bufUCS));

 	AP_StatusBarField_StatusMessage * pf = (AP_StatusBarField_StatusMessage *)m_pStatusMessageField;
 	if(pf) {
		pf->update(bufUCS);
	}
}

const UT_UCSChar * AP_StatusBar::getStatusMessage(void) const
{
	return m_bufUCS;
}

void AP_StatusBar::setStatusProgressType(int start, int end, int flags) {
// 	AP_StatusBarField_StatusMessage * pf = (AP_StatusBarField_StatusMessage *)m_pStatusMessageField;
// 	if(pf)
// 	{
// 		pf->setStatusProgressType(start, end, flags);
// 		if (pf->getListener())
// 			pf->getListener()->notify();
// 	}
}

void AP_StatusBar::setStatusProgressValue(int value) {
// 	AP_StatusBarField_StatusMessage * pf = (AP_StatusBarField_StatusMessage *)m_pStatusMessageField;
// 	if(pf)
// 	{
// 		pf->setStatusProgressValue(value);

// 		if (pf->getListener())
// 			pf->getListener()->notify();
// 	}
}

// use old code otherwise
#else
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

class ap_sb_Field
{
public:
	ap_sb_Field(AP_StatusBar * pSB);
	virtual ~ap_sb_Field(void);
	
	void				setLeftOrigin(UT_uint32 left);

	virtual UT_uint32	getDesiredWidth(void) = 0;
	virtual void		draw(void) = 0;
	virtual void		notify(AV_View * pView, const AV_ChangeMask mask) = 0;
	
protected:
	void				_draw3D(void);
	
	AP_StatusBar *		m_pSB;
	UT_Rect				m_rect3d;
};

ap_sb_Field::ap_sb_Field(AP_StatusBar * pSB)
{
	m_pSB = pSB;
	memset(&m_rect3d,0,sizeof(m_rect3d));
}

ap_sb_Field::~ap_sb_Field(void)
{
}

void ap_sb_Field::setLeftOrigin(UT_uint32 left)
{
	m_rect3d.left	= left;
	m_rect3d.width	= getDesiredWidth();
	m_rect3d.top	= _UL(3);

	UT_uint32 barHeight = m_pSB->getHeight();
	m_rect3d.height	= barHeight - 2*m_rect3d.top;
}

void ap_sb_Field::_draw3D(void)
{
	GR_Graphics * pG = m_pSB->getGraphics();
	
	pG->fillRect(GR_Graphics::CLR3D_Background,
				 m_rect3d.left,m_rect3d.top,
				 m_rect3d.width,m_rect3d.height);

	UT_uint32 l = m_rect3d.left -_UL(1);
	UT_uint32 r = l + m_rect3d.width +_UL(2);
	UT_uint32 t = m_rect3d.top -_UL(1);
	UT_uint32 b = t + m_rect3d.height +_UL(2);
	
	pG->setColor3D(GR_Graphics::CLR3D_BevelDown);
	pG->drawLine(l,t, l,b);
	pG->drawLine(l,t, r,t);
	
	pG->setColor3D(GR_Graphics::CLR3D_BevelUp);
	pG->drawLine(l+_UL(1),b, r,b);
	pG->drawLine(r,b, r,t);
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

class ap_sb_Field_PageInfo : public ap_sb_Field
{
public:
	ap_sb_Field_PageInfo(AP_StatusBar * pSB);
	virtual ~ap_sb_Field_PageInfo(void);

	virtual UT_uint32	getDesiredWidth(void);
	virtual void		draw(void);
	virtual void		notify(AV_View * pView, const AV_ChangeMask mask);

private:
	UT_uint32			m_pageNr;
	UT_uint32			m_nrPages;
	UT_uint32			m_lenBufUCS;
	UT_UCSChar			m_bufUCS[AP_MAX_MESSAGE_FIELD];

	const XML_Char *	m_szFormat;
	UT_uint32			m_iDesiredWidth;
};

ap_sb_Field_PageInfo::ap_sb_Field_PageInfo(AP_StatusBar * pSB)
	: ap_sb_Field(pSB)
{
	m_pageNr = 0;
	m_nrPages = 0;
	m_lenBufUCS = 0;
	memset(m_bufUCS,0,sizeof(m_bufUCS));

	m_szFormat = pSB->getFrame()->getApp()->getStringSet()->getValue(AP_STRING_ID_PageInfoField);
	m_iDesiredWidth = 0;
}

ap_sb_Field_PageInfo::~ap_sb_Field_PageInfo(void)
{
}

UT_uint32 ap_sb_Field_PageInfo::getDesiredWidth(void)
{
	if (!m_iDesiredWidth)
	{
		char buf[AP_MAX_MESSAGE_FIELD];
		sprintf(buf,m_szFormat,999,999);
		UT_uint32 len = strlen(buf);
		UT_UCSChar bufUCS[AP_MAX_MESSAGE_FIELD];
		UT_GrowBufElement charWidths[AP_MAX_MESSAGE_FIELD];
		UT_UCS4_strcpy_char(bufUCS,buf);

		GR_Graphics * pG = m_pSB->getGraphics();
		UT_uint32 w = pG->measureString(bufUCS,0,len,charWidths);
		
		m_iDesiredWidth = w + _UL(6);
	}
	
	return m_iDesiredWidth;
}

void ap_sb_Field_PageInfo::draw(void)
{
	_draw3D();

	if (m_lenBufUCS)
	{
		GR_Graphics * pG = m_pSB->getGraphics();
		UT_uint32 iFontHeight = pG->getFontHeight();

		UT_uint32 x = m_rect3d.left + _UL(3);
		UT_uint32 y = m_rect3d.top + (m_rect3d.height-iFontHeight)/2;

		pG->setColor3D(GR_Graphics::CLR3D_Foreground);
		pG->setClipRect(&m_rect3d);
		pG->drawChars(m_bufUCS,0,m_lenBufUCS,x,y);
		pG->setClipRect(NULL);
	}
}

void ap_sb_Field_PageInfo::notify(AV_View * pavView, const AV_ChangeMask mask)
{
	FV_View * pView = (FV_View *)pavView;
	
	bool bNeedNewString = false;
	
	if (mask & (AV_CHG_PAGECOUNT))
	{
		UT_uint32 newPageCount = pView->getLayout()->countPages();

		if (newPageCount != m_nrPages)
		{
			bNeedNewString = true;
			m_nrPages = newPageCount;
		}
	}

	if (mask & (AV_CHG_MOTION | AV_CHG_PAGECOUNT))
	{
		UT_uint32 currentPage = pView->getCurrentPageNumForStatusBar();

		if (currentPage != m_pageNr)
		{
			bNeedNewString = true;
			m_pageNr = currentPage;
		}
	}

	if (bNeedNewString)
	{
		char buf[AP_MAX_MESSAGE_FIELD];
		sprintf(buf,m_szFormat,m_pageNr,m_nrPages);

		m_lenBufUCS = strlen(buf);
		UT_UCS4_strcpy_char(m_bufUCS,buf);

		draw();
	}
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

class ap_sb_Field_StatusMessage : public ap_sb_Field
{
public:
	ap_sb_Field_StatusMessage(AP_StatusBar * pSB);
	virtual ~ap_sb_Field_StatusMessage(void);

	virtual UT_uint32	getDesiredWidth(void);
	virtual void		draw(void);
	virtual void		notify(AV_View * pView, const AV_ChangeMask mask);

	void				setStatusProgressType(int start, int end, int flags);
	void 				setStatusProgressValue(int value);

	UT_sint32			m_ProgressStart;
	UT_sint32			m_ProgressEnd;
	UT_sint32			m_ProgressValue;
	UT_sint32			m_ProgressStartPoint;
	UT_uint32			m_ProgressFlags;
	UT_Timer			*m_ProgressTimer;

};

ap_sb_Field_StatusMessage::ap_sb_Field_StatusMessage(AP_StatusBar * pSB)
	: ap_sb_Field(pSB)
{
	m_ProgressStartPoint = 0;
	m_ProgressFlags = 0;
	m_ProgressTimer = NULL;
}

ap_sb_Field_StatusMessage::~ap_sb_Field_StatusMessage(void)
{
}

UT_uint32 ap_sb_Field_StatusMessage::getDesiredWidth(void)
{
	return _UL(450);							// TODO define this somewhere
}

void ap_sb_Field_StatusMessage::draw(void)
{
	int centertext = 0;

	if ((m_ProgressFlags & PROGRESS_CMD_MASK) == PROGRESS_STARTBAR) {
		GR_Graphics * pG = m_pSB->getGraphics();
		UT_RGBColor clr(255,255,0);			//Yellow
		UT_RGBColor anticlr(120,120,120);	//Dark grey
		UT_Rect newrect;
		UT_Rect greyrect;

		pG->setClipRect(&m_rect3d);
		
		//If we are doing a continuous update ...
		if (m_ProgressStart == m_ProgressEnd) {

			newrect.top = m_rect3d.top + m_rect3d.height - m_ProgressStartPoint; 
			newrect.left = m_rect3d.left; 
			newrect.height = m_rect3d.height;
			newrect.width = _UL(1);

			m_ProgressStartPoint--;
			if (m_ProgressStartPoint < 0) {
				m_ProgressStartPoint = (2 * m_rect3d.height) -_UL(1);
			}

			while (newrect.left < m_rect3d.width) { 
				
				pG->fillRect(clr, newrect);

				greyrect = newrect;
				greyrect.top += (newrect.top < m_rect3d.top) ? greyrect.height : (-1 * greyrect.height);
				pG->fillRect(anticlr, greyrect);

				newrect.left++;
				newrect.top--;
				if (newrect.top < m_rect3d.top - m_rect3d.height) {
					newrect.top = m_rect3d.top + m_rect3d.height;
				}
			}
				
		}
		//We are doing a percentage update ...
		else {
			char buffer[AP_MAX_MESSAGE_FIELD];

			buffer[0] = '\0';
			newrect = 
			greyrect = m_rect3d;

			//TODO: Get rid of the double here ...
			double percent = (double)m_ProgressValue / (double)(m_ProgressEnd - m_ProgressStart); 
			newrect.width = (UT_sint32)((double)newrect.width * percent); 

			greyrect.left += newrect.width;
			greyrect.width -= newrect.width;

			pG->fillRect(clr, newrect);
			pG->fillRect(anticlr, greyrect);

			/* If we were asked to, we would need to print the percent value */
			switch (m_ProgressFlags & (PROGRESS_SHOW_RAW | PROGRESS_SHOW_PERCENT)) {
				case PROGRESS_SHOW_RAW:
					sprintf(buffer, "%d", m_ProgressValue); 
					break;
				case PROGRESS_SHOW_PERCENT:
					sprintf(buffer, "%.1f%%", 100 * percent); 
					break;
				case (PROGRESS_SHOW_RAW | PROGRESS_SHOW_PERCENT):
					sprintf(buffer, "%d (%.1f%%)", m_ProgressValue, 100 * percent); 
					break;
				case 0:
				default:
					break;
			}

			if (buffer[0]) {
				m_pSB->setStatusMessage((const char *)buffer, false);
				centertext = 1;
			}
		}
		
		pG->setClipRect(NULL);

		if (!(m_ProgressFlags & (PROGRESS_SHOW_MSG | PROGRESS_SHOW_RAW | PROGRESS_SHOW_PERCENT))) {
			return;
		}
	}
	else {
		_draw3D();
	}

	const UT_UCSChar * szMsg = m_pSB->getStatusMessage();
	UT_uint32 len = UT_UCS4_strlen(szMsg);

	if (len)
	{
		GR_Graphics * pG = m_pSB->getGraphics();
		UT_uint32 iFontHeight = pG->getFontHeight();
		UT_uint32 iStringWidth = pG->measureString(szMsg, 0, len, NULL);

		UT_uint32 x = m_rect3d.left + _UL(3);
		UT_uint32 y = m_rect3d.top + (m_rect3d.height-iFontHeight)/2;

		if (centertext) {
			x = m_rect3d.left + ((m_rect3d.width - iStringWidth) / 2);
		}

		pG->setColor3D(GR_Graphics::CLR3D_Foreground);
	
		pG->setClipRect(&m_rect3d);
		pG->drawChars(szMsg,0,len,x,y);
		pG->setClipRect(NULL);
	}
}

void ap_sb_Field_StatusMessage::notify(AV_View * /*pView*/, const AV_ChangeMask /*mask*/)
{
	return;
}

static void updateProgress(UT_Worker * pWorker)
{
    UT_ASSERT(pWorker);

    ap_sb_Field_StatusMessage *pfsm;
    pfsm = (ap_sb_Field_StatusMessage *)pWorker->getInstanceData();
	UT_ASSERT(pfsm);

	pfsm->draw();
}

/*
 This will set up the status message with a status/percentage updater.
 Flags are 0 for now.
 If start == end then the progress bar is a continuous update.
 If start != end then the progress bar will indicate an amount
	between start-end

 FLAGS: 0 
 TODO: Allow status message to xor on top,
       or update the message with the value (as a percent or
       raw via the setProgressType. Start/Stop the progress bar.
*/ 
void ap_sb_Field_StatusMessage::setStatusProgressType(int start, int end, int flags) {
	m_ProgressStart = 
	m_ProgressValue = start;
	m_ProgressEnd = end;
	m_ProgressFlags = flags;
	m_ProgressStartPoint = 0;

	if (m_ProgressTimer) {
		delete m_ProgressTimer;
		m_ProgressTimer = NULL;
	}

	if (m_ProgressStart == m_ProgressEnd &&
       (m_ProgressFlags & PROGRESS_CMD_MASK) == PROGRESS_STARTBAR) {  
		m_ProgressTimer = UT_Timer::static_constructor(updateProgress, this);
		m_ProgressTimer->stop();
		m_ProgressTimer->set(50);	//Milliseconds
	}
}

void ap_sb_Field_StatusMessage::setStatusProgressValue(int value) {
	m_ProgressValue = value;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

class ap_sb_Field_InputMode : public ap_sb_Field
{
public:
	ap_sb_Field_InputMode(AP_StatusBar * pSB);
	virtual ~ap_sb_Field_InputMode(void);

	virtual UT_uint32	getDesiredWidth(void);
	virtual void		draw(void);
	virtual void		notify(AV_View * pView, const AV_ChangeMask mask);

private:
	UT_uint32			m_lenBufUCS;
	UT_UCSChar			m_bufUCS[AP_MAX_MESSAGE_FIELD];

	UT_uint32			m_iDesiredWidth;
};

ap_sb_Field_InputMode::ap_sb_Field_InputMode(AP_StatusBar * pSB)
	: ap_sb_Field(pSB)
{
	const char * szInputMode = m_pSB->getFrame()->getInputMode();
	m_lenBufUCS = strlen(szInputMode);
	UT_UCS4_strcpy_char(m_bufUCS,szInputMode);

	m_iDesiredWidth = 0;
}

ap_sb_Field_InputMode::~ap_sb_Field_InputMode(void)
{
}

UT_uint32 ap_sb_Field_InputMode::getDesiredWidth(void)
{
	if (!m_iDesiredWidth)
	{
		char * szBuf = "MMMMMMMM";
		UT_uint32 len = strlen(szBuf);
		UT_UCSChar bufUCS[AP_MAX_MESSAGE_FIELD];
		UT_GrowBufElement charWidths[AP_MAX_MESSAGE_FIELD];
		UT_UCS4_strcpy_char(bufUCS,szBuf);

		GR_Graphics * pG = m_pSB->getGraphics();
		UT_uint32 w = pG->measureString(bufUCS,0,len,charWidths);
		
		m_iDesiredWidth = w + _UL(6);
	}
	
	return m_iDesiredWidth;
}

void ap_sb_Field_InputMode::draw(void)
{
	_draw3D();

	if (m_lenBufUCS)
	{
		GR_Graphics * pG = m_pSB->getGraphics();
		UT_uint32 iFontHeight = pG->getFontHeight();

		UT_uint32 x = m_rect3d.left + _UL(3);
		UT_uint32 y = m_rect3d.top + (m_rect3d.height-iFontHeight)/2;

		pG->setColor3D(GR_Graphics::CLR3D_Foreground);

		pG->setClipRect(&m_rect3d);
		pG->drawChars(m_bufUCS,0,m_lenBufUCS,x,y);
		pG->setClipRect(NULL);
	}
}

void ap_sb_Field_InputMode::notify(AV_View * /*pavView*/, const AV_ChangeMask mask)
{
	if (mask & (AV_CHG_INPUTMODE))
	{
		const char * szInputMode = m_pSB->getFrame()->getInputMode();
		m_lenBufUCS = strlen(szInputMode);
		UT_UCS4_strcpy_char(m_bufUCS,szInputMode);

		draw();
	}
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

class ap_sb_Field_InsertMode : public ap_sb_Field
{
public:
    ap_sb_Field_InsertMode(AP_StatusBar * pSB);
    virtual ~ap_sb_Field_InsertMode(void);

    virtual UT_uint32   getDesiredWidth(void);
    virtual void        draw(void);
    virtual void        notify(AV_View * pView, const AV_ChangeMask mask);

private:
    UT_UCSChar          m_InsertMode[2][AP_MAX_MESSAGE_FIELD];
    bool             m_bInsertMode;
    UT_uint32           m_iDesiredWidth;
};

ap_sb_Field_InsertMode::ap_sb_Field_InsertMode(AP_StatusBar * pSB)
    : ap_sb_Field(pSB)
{
    UT_UCS4_strcpy_char(m_InsertMode[(int)true],pSB->getFrame()->getApp()->getStringSet()->getValue(AP_STRING_ID_InsertModeFieldINS));
    UT_UCS4_strcpy_char(m_InsertMode[(int)false],pSB->getFrame()->getApp()->getStringSet()->getValue(AP_STRING_ID_InsertModeFieldOVR));

    m_iDesiredWidth = 0;
    m_bInsertMode = true;
}

ap_sb_Field_InsertMode::~ap_sb_Field_InsertMode(void)
{
}

UT_uint32 ap_sb_Field_InsertMode::getDesiredWidth(void)
{
    if (!m_iDesiredWidth)
    {
        UT_uint32 i;
        UT_GrowBufElement charWidths[AP_MAX_MESSAGE_FIELD];
        UT_uint32 len;
        for(i = 0;i < 2;i++){
            len = UT_UCS4_strlen(m_InsertMode[i]);
            m_iDesiredWidth = MyMax(m_iDesiredWidth,m_pSB->getGraphics()->measureString(m_InsertMode[i],0,len,charWidths));
        }
        UT_ASSERT(m_iDesiredWidth);
        m_iDesiredWidth = MyMin(m_iDesiredWidth,_UL(300)) + _UL(6);
    }
    return m_iDesiredWidth;
}

void ap_sb_Field_InsertMode::draw(void)
{
    _draw3D();
    int len;

    UT_UCSChar *bufInsertMode = m_InsertMode[m_bInsertMode];

    len = UT_UCS4_strlen(bufInsertMode);

    if (len)
    {
        GR_Graphics * pG = m_pSB->getGraphics();
        UT_uint32 iFontHeight = pG->getFontHeight();

        UT_uint32 x = m_rect3d.left + _UL(3);
        UT_uint32 y = m_rect3d.top + (m_rect3d.height-iFontHeight)/2;

        pG->setColor3D(GR_Graphics::CLR3D_Foreground);

        pG->setClipRect(&m_rect3d);
        pG->drawChars(bufInsertMode,0,len,x,y);
        pG->setClipRect(NULL);
    }
}

void ap_sb_Field_InsertMode::notify(AV_View * /*pavView*/, const AV_ChangeMask mask)
{
    if (mask & (AV_CHG_INSERTMODE))
    {
		AP_FrameData * pData = (AP_FrameData *) m_pSB->getFrame()->getFrameData();
		if (pData)
	        m_bInsertMode = pData->m_bInsertMode;

        draw();
    }
}


//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

class ap_sb_Field_Language : public ap_sb_Field
{
public:
	ap_sb_Field_Language(AP_StatusBar * pSB);
	virtual ~ap_sb_Field_Language(void);

	virtual UT_uint32 getDesiredWidth(void);
	virtual void draw(void);
	virtual void notify(AV_View * pView, const AV_ChangeMask mask);

private:
	UT_UCSChar m_Language[AP_MAX_MESSAGE_FIELD];
	UT_uint32 m_iDesiredWidth;
};

ap_sb_Field_Language::ap_sb_Field_Language(AP_StatusBar * pSB)
	: ap_sb_Field(pSB)
{
	m_Language[0] = 0;
	m_iDesiredWidth = 0;
}

ap_sb_Field_Language::~ap_sb_Field_Language(void)
{
}

UT_uint32 ap_sb_Field_Language::getDesiredWidth(void)
{
	if (!m_iDesiredWidth)
	{
		UT_GrowBufElement charWidths[AP_MAX_MESSAGE_FIELD];

		// TODO language tags can be longer, eg "art-lojban"
		UT_UCSChar language[5+1];
		// Use m/M to measure string since it's a fat letter
		UT_UCS4_strcpy_char(language,"mm-MM");

		// TODO language tags can be longer, eg "art-lojban"
		m_iDesiredWidth = m_pSB->getGraphics()->measureString(language,0,5,charWidths);
		UT_ASSERT(m_iDesiredWidth);
		m_iDesiredWidth = MyMin(m_iDesiredWidth,_UL(300)) + _UL(6);
	}
	return m_iDesiredWidth;
}

void ap_sb_Field_Language::draw(void)
{
	_draw3D();
	int len;

	UT_UCSChar *bufLanguage = m_Language;

	len = UT_UCS4_strlen(bufLanguage);

	if (len)
	{
		GR_Graphics * pG = m_pSB->getGraphics();
		UT_uint32 iFontHeight = pG->getFontHeight();

		UT_uint32 x = m_rect3d.left + _UL(3);
		UT_uint32 y = m_rect3d.top + (m_rect3d.height-iFontHeight)/2;

		pG->setColor3D(GR_Graphics::CLR3D_Foreground);

		pG->setClipRect(&m_rect3d);
		pG->drawChars(bufLanguage,0,len,x,y);
		pG->setClipRect(NULL);
	}
}

void ap_sb_Field_Language::notify(AV_View * pavView, const AV_ChangeMask mask)
{
	// TODO do we want our own bit for language change?
	//if (mask & (AV_CHG_INSERTMODE))
	{
		const char * szLang = NULL;

		const XML_Char ** props_in = NULL;
		if (pavView && static_cast<FV_View *>(pavView)->getCharFormat(&props_in))
		{
			szLang = UT_getAttribute("lang", props_in);
			FREEP(props_in);

			UT_UCS4_strcpy_char(m_Language,szLang ? szLang : "");
		}

		draw();
	}
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

AP_StatusBar::AP_StatusBar(XAP_Frame * pFrame)
:       m_pFrame(pFrame),
        m_pView(0),
        m_pG(0),
        m_iHeight(0),
        m_iWidth(0),
        s_iFixedHeight(_UL(20)),
        m_bInitFields(false),
        m_pStatusMessageField(0)
{
        const XML_Char * szRulerUnits;
        if (pFrame->getApp()->getPrefsValue(AP_PREF_KEY_RulerUnits,&szRulerUnits))
                m_dim = UT_determineDimension(szRulerUnits);
        else
                m_dim = DIM_IN;

        // really this should be "static const x = 20;" in the
        // class declaration, but MSVC5 can't handle it....
        // (GCC can :-)

        s_iFixedHeight = _UL(20);

        m_bufUCS[0] = 0;
}

AP_StatusBar::~AP_StatusBar(void)
{
	UT_VECTOR_PURGEALL(ap_sb_Field *, m_vecFields);
}

XAP_Frame * AP_StatusBar::getFrame(void) const
{
	return m_pFrame;
}

GR_Graphics * AP_StatusBar::getGraphics(void) const
{
	return m_pG;
}

void AP_StatusBar::setView(AV_View * pView)
{
	m_pView = pView;

	// Register the StatusBar as a ViewListener on the View.
	// This lets us receive notify events as the user interacts
	// with the document (cmdCharMotion, etc).  This will let
	// us update the display as we move from block to block and
	// from column to column.

	AV_ListenerId lidTopRuler;
	m_pView->addListener(static_cast<AV_Listener *>(this),&lidTopRuler);

	if (!m_bInitFields)
	{
		UT_uint32 xOrigin = _UL(3);
		UT_uint32 xGap = _UL(6);
		
#define DclField(type,var)								\
		type * var = new type(this);					\
		UT_ASSERT((var));								\
		m_vecFields.addItem((var));						\
		(var)->setLeftOrigin(xOrigin);					\
		xOrigin += (var)->getDesiredWidth() + xGap
		
		DclField(ap_sb_Field_PageInfo, pf1);
		DclField(ap_sb_Field_StatusMessage, pf2);

		m_pStatusMessageField = pf2;	// its in the vector, but we remember it explicitly
										// so that setStatusMessage() can do its thing.

		DclField(ap_sb_Field_InsertMode, pf4);
		DclField(ap_sb_Field_InputMode, pf3);
		
		DclField(ap_sb_Field_Language, pf5);

		// TODO add other fields

#undef DclField
		m_bInitFields = true;
	}

	// force a full notify of all fields so that they all
	// completely update themselves.
	
	notify(pView,AV_CHG_ALL);
	
	return;
}

void AP_StatusBar::setHeight(UT_uint32 iHeight)
{
	m_iHeight = MyMax(iHeight,s_iFixedHeight);
}

UT_uint32 AP_StatusBar::getHeight(void) const
{
	return m_iHeight;
}

void AP_StatusBar::setWidth(UT_uint32 iWidth)
{
	// TODO change status message control width
	// TODO change x position of all controls to right of status message
	m_iWidth = iWidth;
}

UT_uint32 AP_StatusBar::getWidth(void) const
{
	return m_iWidth;
}

bool AP_StatusBar::notify(AV_View * pView, const AV_ChangeMask mask)
{
	// Handle AV_Listener events on the view.	

	// We choose to clear any status message we may have,
	// since it's a pain for the code which set the message
	// to hang around and clear it at some point in the future.
	// This way, message will get cleared any time the user does
	// something with the window.

	if (*m_bufUCS)
		setStatusMessage((UT_UCSChar *)NULL);
	
	// Let each field on the status bar update itself accordingly.
	
	UT_ASSERT(pView==m_pView);
	UT_uint32 kLimit = m_vecFields.getItemCount();
	UT_uint32 k;

	for (k=0; k<kLimit; k++)
	{
		ap_sb_Field * pf = (ap_sb_Field *)m_vecFields.getNthItem(k);
		if(pf)
		{
			pf->notify(pView,mask);
		}
	}

	return true;
}

void AP_StatusBar::draw(void)
{
	if (!m_pG)
		return;
	
	// draw the background

	m_pG->fillRect(GR_Graphics::CLR3D_Background,0,0,m_iWidth,m_iHeight);

	// draw the foreground
	
	_draw();
}

void AP_StatusBar::_draw(void)
{
	UT_uint32 kLimit = m_vecFields.getItemCount();
	UT_uint32 k;

	for (k=0; k<kLimit; k++)
	{
		ap_sb_Field * pf = (ap_sb_Field *)m_vecFields.getNthItem(k);
		if(pf)
		{
			pf->draw();
		}
	}
}

void AP_StatusBar::setStatusMessage(UT_UCSChar * pBufUCS, int redraw)
{
	memset(m_bufUCS,0,sizeof(m_bufUCS));

	if (pBufUCS && *pBufUCS)
	{
		UT_ASSERT(UT_UCS4_strlen(pBufUCS) < AP_MAX_MESSAGE_FIELD);
		UT_UCS4_strcpy(m_bufUCS,pBufUCS);
	}
	
	ap_sb_Field_StatusMessage * pf = (ap_sb_Field_StatusMessage *)m_pStatusMessageField;
	if (pf && redraw != 0) {
		pf->draw();
	}
}

void AP_StatusBar::setStatusMessage(const char * pBuf, int redraw)
{
	UT_uint32 len = ((pBuf && *pBuf) ? strlen(pBuf) : 0);
	UT_ASSERT(len*MB_LEN_MAX < AP_MAX_MESSAGE_FIELD);

	UT_UCSChar bufUCS[AP_MAX_MESSAGE_FIELD];

	if (len)
		UT_UCS4_strcpy_char(bufUCS,pBuf);
	else
		memset(bufUCS,0,sizeof(bufUCS));

	setStatusMessage(bufUCS, redraw);
}

const UT_UCSChar * AP_StatusBar::getStatusMessage(void) const
{
	return m_bufUCS;
}

void AP_StatusBar::setStatusProgressType(int start, int end, int flags) {
	ap_sb_Field_StatusMessage * pf = (ap_sb_Field_StatusMessage *)m_pStatusMessageField;
	if(pf)
	{
		pf->setStatusProgressType(start, end, flags);
		pf->draw();
	}
}

void AP_StatusBar::setStatusProgressValue(int value) {
	ap_sb_Field_StatusMessage * pf = (ap_sb_Field_StatusMessage *)m_pStatusMessageField;
	if(pf)
	{
		pf->setStatusProgressValue(value);
		pf->draw();
	}
}

#endif
