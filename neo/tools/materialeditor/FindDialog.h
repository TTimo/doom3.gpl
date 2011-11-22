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
#pragma once

#include "MaterialEditor.h"
#include "../common/registryoptions.h"

class MEMainFrame;

/**
* Dialog that provides an input box and several checkboxes to define
* the parameters of a search. These parameters include: text string, search
* scope and search only name flag.
*/
class FindDialog : public CDialog
{

public:
	enum { IDD = IDD_FIND };
	
public:
	FindDialog(CWnd* pParent = NULL);
	virtual ~FindDialog();

	BOOL					Create();

protected:
	DECLARE_DYNAMIC(FindDialog)

	//Overrides
	virtual void			DoDataExchange(CDataExchange* pDX);
	virtual BOOL			OnInitDialog();

	//Messages
	afx_msg void			OnBnClickedFindNext();
	virtual void			OnCancel();
	DECLARE_MESSAGE_MAP()

	//Protected Operations
	void					LoadFindSettings();
	void					SaveFindSettings();

protected:
	MEMainFrame*			parent;
	MaterialSearchData_t	searchData;
	rvRegistryOptions		registry;
};
