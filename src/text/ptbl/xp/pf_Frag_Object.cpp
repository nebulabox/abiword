/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (c) 2001,2002 Tomas Frydrych
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

#define pf_FRAG_OBJECT_LENGTH 1

#include "pf_Frag_Object.h"
#include "px_ChangeRecord.h"
#include "px_CR_Object.h"
#include "pt_Types.h"
#include "ut_string.h"
#include "pt_PieceTable.h"

pf_Frag_Object::pf_Frag_Object(pt_PieceTable * pPT,
                               PTObjectType objectType,
                               PT_AttrPropIndex indexAP)
    : pf_Frag(pPT, pf_Frag::PFT_Object, pf_FRAG_OBJECT_LENGTH)
{
	m_pObjectSubclass = NULL;
    m_objectType = objectType;
    m_indexAP = indexAP;
    const PP_AttrProp * pAP = NULL;
    m_pPieceTable->getAttrProp(m_indexAP,&pAP);
    UT_ASSERT(pAP);
    const XML_Char* pszType = NULL;
    const XML_Char* pszName = NULL;
	const XML_Char* pszParam = NULL;

    pAP->getAttribute(static_cast<const XML_Char *>("type"), pszType);
    pAP->getAttribute(static_cast<const XML_Char *>("name"), pszName);
    pAP->getAttribute(static_cast<const XML_Char *>("param"), pszParam);

    fd_Field::FieldType fieldType;

    if (objectType==PTO_Field) 
    {
    	switch(*pszType)
    	{
    		case 'a':
				if (0 == UT_strcmp(pszType, "app_ver"))
				{
					fieldType = fd_Field::FD_App_Version;
				}
				else if (0 == UT_strcmp(pszType, "app_id"))
				{
					fieldType = fd_Field::FD_App_ID;
				}
				else if (0 == UT_strcmp(pszType, "app_options"))
				{
					fieldType = fd_Field::FD_App_Options;
				}
				else if (0 == UT_strcmp(pszType, "app_target"))
				{
					fieldType = fd_Field::FD_App_Target;
				}
				else if (0 == UT_strcmp(pszType, "app_compiledate"))
				{
					fieldType = fd_Field::FD_App_CompileDate;
				}
				else if (0 == UT_strcmp(pszType, "app_compiletime"))
				{
					fieldType = fd_Field::FD_App_CompileTime;
				}
		        else
        		{
		            UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
        		    //Better than segfaulting I figure
		            fieldType = fd_Field::FD_Test;
        		}
				break;
    		case 'c':
				if (0 == UT_strcmp(pszType, "char_count"))
				{
					fieldType = fd_Field::FD_Doc_CharCount;
				}
		        else
        		{
		            UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
        		    //Better than segfaulting I figure
		            fieldType = fd_Field::FD_Test;
        		}
    			break;
    		case 'd':
				if (0 == UT_strcmp(pszType, "date"))
				{
					fieldType = fd_Field::FD_Date;
				}
				else if (0 == UT_strcmp(pszType, "date_mmddyy"))
				{
					fieldType = fd_Field::FD_Date_MMDDYY;
				}
				else if (0 == UT_strcmp(pszType, "date_ddmmyy"))
				{
					fieldType = fd_Field::FD_Date_DDMMYY;
				}
				else if (0 == UT_strcmp(pszType, "date_mdy"))
				{
					fieldType = fd_Field::FD_Date_MDY;
				}
				else if (0 == UT_strcmp(pszType, "date_mthdy"))
				{
					fieldType = fd_Field::FD_Date_MthDY;
				}
				else if (0 == UT_strcmp(pszType, "date_dfl"))
				{
					fieldType = fd_Field::FD_Date_DFL;
				}
				else if (0 == UT_strcmp(pszType, "date_ntdfl"))
				{
					fieldType = fd_Field::FD_Date_NTDFL;
				}
				else if (0 == UT_strcmp(pszType, "date_wkday"))
				{
					fieldType = fd_Field::FD_Date_Wkday;
				}
				else if (0 == UT_strcmp(pszType, "date_doy"))
				{
					fieldType = fd_Field::FD_Date_DOY;
				}
				else if (0 == UT_strcmp(pszType, "datetime_custom"))
				{
					fieldType = fd_Field::FD_DateTime_Custom;
				}
		        else
        		{
		            UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
        		    //Better than segfaulting I figure
		            fieldType = fd_Field::FD_Test;
        		}
				break;
    		case 'e':
				if (0 == UT_strcmp(pszType, "endnote_ref"))
				{
					fieldType = fd_Field::FD_Endnote_Ref;
				}
				else if (0 == UT_strcmp(pszType, "endnote_anchor"))
				{
					fieldType = fd_Field::FD_Endnote_Anchor;
				}
		        else
        		{
		            UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
        		    //Better than segfaulting I figure
		            fieldType = fd_Field::FD_Test;
        		}
				break;
    		case 'f':
				if (0 == UT_strcmp(pszType, "file_name"))
				{
					fieldType = fd_Field::FD_FileName;
				}
				else if (0 == UT_strcmp(pszType, "footnote_ref"))
				{
					fieldType = fd_Field::FD_Footnote_Ref;
				}
				else if (0 == UT_strcmp(pszType, "footnote_anchor"))
				{
					fieldType = fd_Field::FD_Footnote_Anchor;
				}
		        else
        		{
		            UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
        		    //Better than segfaulting I figure
		            fieldType = fd_Field::FD_Test;
        		}
				break;
    		case 'l':
				if (0 == UT_strcmp(pszType, "list_label"))
		        {
        		    fieldType = fd_Field::FD_ListLabel;
		        }
				else if (0 == UT_strcmp(pszType, "line_count"))
				{
					fieldType = fd_Field::FD_Doc_LineCount;
				}
		        else
        		{
		            UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
        		    //Better than segfaulting I figure
		            fieldType = fd_Field::FD_Test;
        		}
    			break;
    		case 'm':
		        if (0 == UT_strcmp(pszType, "mail_merge"))
		        {
        		    fieldType = fd_Field::FD_MailMerge;
		        }
			else if(0 == UT_strcmp(pszType, "meta_title"))
			  {
			    fieldType = fd_Field::FD_Meta_Title;
			  }
			else if(0 == UT_strcmp(pszType, "meta_creator"))
			  {
			    fieldType = fd_Field::FD_Meta_Creator;
			  }
			else if(0 == UT_strcmp(pszType, "meta_subject"))
			  {
			    fieldType = fd_Field::FD_Meta_Subject;
			  }
			else if(0 == UT_strcmp(pszType, "meta_publisher"))
			  {
			    fieldType = fd_Field::FD_Meta_Publisher;
			  }
			else if(0 == UT_strcmp(pszType, "meta_date"))
			  {
			    fieldType = fd_Field::FD_Meta_Date;
			  }
			else if(0 == UT_strcmp(pszType, "meta_type"))
			  {
			    fieldType = fd_Field::FD_Meta_Type;
			  }
			else if(0 == UT_strcmp(pszType, "meta_language"))
			  {
			    fieldType = fd_Field::FD_Meta_Language;
			  }
			else if(0 == UT_strcmp(pszType, "meta_rights"))
			  {
			    fieldType = fd_Field::FD_Meta_Rights;
			  }
			else if(0 == UT_strcmp(pszType, "meta_keywords"))
			  {
			    fieldType = fd_Field::FD_Meta_Keywords;
			  }
			else if(0 == UT_strcmp(pszType, "meta_contributor"))
			  {
			    fieldType = fd_Field::FD_Meta_Contributor;
			  }
			else if(0 == UT_strcmp(pszType, "meta_coverage"))
			  {
			    fieldType = fd_Field::FD_Meta_Coverage;
			  }
			else if(0 == UT_strcmp(pszType, "meta_description"))
			  {
			    fieldType = fd_Field::FD_Meta_Description;
			  }
		        else if (0 == UT_strcmp(pszType, "martin_test"))
		        {
        		    fieldType = fd_Field::FD_MartinTest;
		        }
		        else
        		{
		            UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
        		    //Better than segfaulting I figure
		            fieldType = fd_Field::FD_Test;
        		}
		        break;
    		case 'n':
				if (0 == UT_strcmp(pszType, "nbsp_count"))
				{
					fieldType = fd_Field::FD_Doc_NbspCount;
				}
		        else
        		{
		            UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
        		    //Better than segfaulting I figure
		            fieldType = fd_Field::FD_Test;
        		}
				break;
    		case 'p':
				if (0 == UT_strcmp(pszType, "page_number"))
		        {
        		    fieldType = fd_Field::FD_PageNumber;
		        }
		        else if (0 == UT_strcmp(pszType, "page_count"))
        		{
		            fieldType = fd_Field::FD_PageCount;
        		}
				else if (0 == UT_strcmp(pszType, "para_count"))
				{
					fieldType = fd_Field::FD_Doc_ParaCount;
				}
				else if (0 == UT_strcmp(pszType, "page_ref"))
				{
					fieldType = fd_Field::FD_PageReference;
				}
		        else
        		{
		            UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
        		    //Better than segfaulting I figure
		            fieldType = fd_Field::FD_Test;
        		}
        		break;
    		case 't':
		        if (0 == UT_strcmp(pszType, "test"))
        		{
		            fieldType = fd_Field::FD_Test;
        		}
		        else if (0 == UT_strcmp(pszType, "time"))
        		{
		            fieldType = fd_Field::FD_Time;
        		}
				else if (0 == UT_strcmp(pszType, "time_miltime"))
				{
					fieldType = fd_Field::FD_Time_MilTime;
				}
				else if (0 == UT_strcmp(pszType, "time_ampm"))
				{
					fieldType = fd_Field::FD_Time_AMPM;
				}
				else if (0 == UT_strcmp(pszType, "time_zone"))
				{
					fieldType = fd_Field::FD_Time_Zone;
				}
				else if (0 == UT_strcmp(pszType, "time_epoch"))
				{
					fieldType = fd_Field::FD_Time_Epoch;
				}
		        else
        		{
		            UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
        		    //Better than segfaulting I figure
		            fieldType = fd_Field::FD_Test;
        		}
        		break;
    		case 'w':
				if (0 == UT_strcmp(pszType, "word_count"))
				{
					fieldType = fd_Field::FD_Doc_WordCount;
				}
		        else
        		{
		            UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
        		    //Better than segfaulting I figure
		            fieldType = fd_Field::FD_Test;
        		}
				break;
#if 0
// When adding new fields under any of these characters, please move
// the label up where it belongs
    		case 'b':
    		case 'g':
    		case 'h':
    		case 'i':
    		case 'j':
    		case 'k':
    		case 'o':
    		case 'q':
    		case 'r':
    		case 's':
    		case 'u':
    		case 'v':
    		case 'x':
    		case 'y':
    		case 'z':
#endif
    		default:
    			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
       		    //Better than segfaulting I figure
	            fieldType = fd_Field::FD_Test;
    	}
        m_pField = new fd_Field(*this, pPT,fieldType, pszParam);
    }
    else if (objectType==PTO_Bookmark)
    {
    	po_Bookmark::BookmarkType BT;

		if(!pszType) {
			// see bug 6489...
			UT_ASSERT_NOT_REACHED();
			BT = po_Bookmark::POBOOKMARK_END;
		} else if(0 == UT_strcmp(pszType, "end"))
			BT = po_Bookmark::POBOOKMARK_END;
		else
			BT = po_Bookmark::POBOOKMARK_START;
			
		UT_ASSERT(pszName && *pszName);
		m_pObjectSubclass = static_cast<void *>(new po_Bookmark(*this,pPT,BT, pszName));
    }

}

pf_Frag_Object::~pf_Frag_Object()
{
	
    if (m_pObjectSubclass)
	{
		// make sure that we delete what we should ...
    	switch(m_objectType)
    	{
    		case PTO_Field:
    		break;
    		case PTO_Bookmark:
    		{
    			po_Bookmark *bm = static_cast<po_Bookmark*>(m_pObjectSubclass);
    			delete bm;
    		}
    		break;
    		default:
	    		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
    	}
	    m_pObjectSubclass = NULL;
	}
	delete m_pField;
	m_pField = 0;
}

PTObjectType pf_Frag_Object::getObjectType(void) const
{
    return m_objectType;
}

PT_AttrPropIndex pf_Frag_Object::getIndexAP(void) const
{
    return m_indexAP;
}

void pf_Frag_Object::setIndexAP(PT_AttrPropIndex indexNewAP)
{
    m_indexAP = indexNewAP;
}

bool pf_Frag_Object::createSpecialChangeRecord(PX_ChangeRecord ** ppcr,
                                                  PT_DocPosition dpos,
                                                  PT_BlockOffset blockOffset) const
{
    UT_ASSERT(ppcr);
	
    PX_ChangeRecord_Object * pcr
    	 = new PX_ChangeRecord_Object(PX_ChangeRecord::PXT_InsertObject,
                                     dpos, m_indexAP, m_objectType,
                                     blockOffset, m_pField);

    if (!pcr)
        return false;

    *ppcr = pcr;
    return true;
}


po_Bookmark * pf_Frag_Object::getBookmark() const
{
	if(m_objectType == PTO_Bookmark)
		return static_cast<po_Bookmark*>(m_pObjectSubclass);
	else
		return 0;
}
