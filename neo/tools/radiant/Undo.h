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

//start operation
void Undo_Start(char *operation);
//end operation
void Undo_End(void);
//add brush to the undo
void Undo_AddBrush(brush_t *pBrush);
//add a list with brushes to the undo
void Undo_AddBrushList(brush_t *brushlist);
//end a brush after the operation is performed
void Undo_EndBrush(brush_t *pBrush);
//end a list with brushes after the operation is performed
void Undo_EndBrushList(brush_t *brushlist);
//add entity to undo
void Undo_AddEntity(entity_t *entity);
//end an entity after the operation is performed
void Undo_EndEntity(entity_t *entity);
//undo last operation
void Undo_Undo(void);
//redo last undone operation
void Undo_Redo(void);
//returns true if there is something to be undone available
int  Undo_UndoAvailable(void);
//returns true if there is something to redo available
int  Undo_RedoAvailable(void);
//clear the undo buffer
void Undo_Clear(void);
//set maximum undo size (default 64)
void Undo_SetMaxSize(int size);
//get maximum undo size
int  Undo_GetMaxSize(void);
//set maximum undo memory in bytes (default 2 MB)
void Undo_SetMaxMemorySize(int size);
//get maximum undo memory in bytes
int  Undo_GetMaxMemorySize(void);
//returns the amount of memory used by undo
int  Undo_MemorySize(void);

