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

#ifndef __TYPEINFOGEN_H__
#define __TYPEINFOGEN_H__

/*
===================================================================================

	Type Info Generator

	- template classes are commented out (different instantiations are not identified)
	- bit fields are commented out (cannot get the address of bit fields)
	- multiple inheritance is not supported (only tracks a single super type)

===================================================================================
*/

class idConstantInfo {
public:
	idStr						name;
	idStr						type;
	idStr						value;
};

class idEnumValueInfo {
public:
	idStr						name;
	int							value;
};

class idEnumTypeInfo {
public:
	idStr						typeName;
	idStr						scope;
	bool						unnamed;
	bool						isTemplate;
	idList<idEnumValueInfo>		values;
};

class idClassVariableInfo {
public:
	idStr						name;
	idStr						type;
	int							bits;
};

class idClassTypeInfo {
public:
	idStr						typeName;
	idStr						superType;
	idStr						scope;
	bool						unnamed;
	bool						isTemplate;
	idList<idClassVariableInfo>	variables;
};

class idTypeInfoGen {
public:
								idTypeInfoGen( void );
								~idTypeInfoGen( void );

	void						AddDefine( const char *define );
	void						CreateTypeInfo( const char *path );
	void						WriteTypeInfo( const char *fileName ) const;

private:
	idStrList					defines;

	idList<idConstantInfo *>	constants;
	idList<idEnumTypeInfo *>	enums;
	idList<idClassTypeInfo *>	classes;

	int							numTemplates;
	int							maxInheritance;
	idStr						maxInheritanceClass;

	int							GetInheritance( const char *typeName ) const;
	int							EvaluateIntegerString( const idStr &string );
	float						EvaluateFloatString( const idStr &string );
	idConstantInfo *			FindConstant( const char *name );
	int							GetIntegerConstant( const char *scope, const char *name, idParser &src );
	float						GetFloatConstant( const char *scope, const char *name, idParser &src );
	int							ParseArraySize( const char *scope, idParser &src );
	void						ParseConstantValue( const char *scope, idParser &src, idStr &value );
	idEnumTypeInfo *			ParseEnumType( const char *scope, bool isTemplate, bool typeDef, idParser &src );
	idClassTypeInfo *			ParseClassType( const char *scope, const char *templateArgs, bool isTemplate, bool typeDef, idParser &src );
	void						ParseScope( const char *scope, bool isTemplate, idParser &src, idClassTypeInfo *typeInfo );
};

#endif /* !__TYPEINFOGEN_H__ */
