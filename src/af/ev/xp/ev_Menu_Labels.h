/* AbiSource Program Utilities
 * Copyright (C) 1998 AbiSource, Inc.
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

#ifndef EV_MENU_LABELS_H
#define EV_MENU_LABELS_H

#include "xap_Types.h"


/*****************************************************************
******************************************************************
** This file defines a framework for the string labels which
** are used with menus.  This binding allows us to localize
** menus in a platform-independent manner.
**
** We create one EV_Menu_Label per menu-item (such as "File|Open")
** per language per application (not per window or actual menu).
**
** We create one EV_Menu_LabelSet per language per application.
**
******************************************************************
*****************************************************************/

// TODO decide if we should make all labels Unicode

class EV_Menu_Label
{
public:
	EV_Menu_Label(XAP_Menu_Id id,
				  const char * szMenuLabel,		/* label on the actual menu itself */
				  const char * szStatusMsg);	/* status bar message */
	~EV_Menu_Label(void);

	XAP_Menu_Id						getMenuId(void) const;
	const char *					getMenuLabel(void) const;
	const char *					getMenuStatusMessage(void) const;

protected:
	XAP_Menu_Id						m_id;
	char *							m_szMenuLabel;
	char *							m_szStatusMsg;
};

/*****************************************************************/

class EV_Menu_LabelSet					/* a glorified array with bounds checking */
{
public:
	EV_Menu_LabelSet(const char * szLanguage,
					 XAP_Menu_Id first, XAP_Menu_Id last);
	~EV_Menu_LabelSet(void);

	UT_Bool				setLabel(XAP_Menu_Id id,
								 const char * szMenuLabel,
								 const char * szStatusMsg);
#ifdef __MRC__
	EV_Menu_Label *		getLabel(XAP_Menu_Id id);
#else
	EV_Menu_Label *		getLabel(XAP_Menu_Id id) const;
#endif
	const char *		getLanguage(void) const;

protected:
	EV_Menu_Label **	m_labelTable;
	XAP_Menu_Id			m_first;
	XAP_Menu_Id			m_last;
	char *				m_szLanguage;	/* for the convenience of the app only  */
};

#endif /* EV_MENU_LABELS_H */
