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

#ifndef __HIERARCHY_H__
#define __HIERARCHY_H__

/*
==============================================================================

	idHierarchy

==============================================================================
*/

template< class type >
class idHierarchy {
public:

						idHierarchy();
						~idHierarchy();
	
	void				SetOwner( type *object );
	type *				Owner( void ) const;
	void				ParentTo( idHierarchy &node );
	void				MakeSiblingAfter( idHierarchy &node );
	bool				ParentedBy( const idHierarchy &node ) const;
	void				RemoveFromParent( void );
	void				RemoveFromHierarchy( void );

	type *				GetParent( void ) const;		// parent of this node
	type *				GetChild( void ) const;			// first child of this node
	type *				GetSibling( void ) const;		// next node with the same parent
	type *				GetPriorSibling( void ) const;	// previous node with the same parent
	type *				GetNext( void ) const;			// goes through all nodes of the hierarchy
	type *				GetNextLeaf( void ) const;		// goes through all leaf nodes of the hierarchy

private:
	idHierarchy *		parent;
	idHierarchy *		sibling;
	idHierarchy *		child;
	type *				owner;

	idHierarchy<type>	*GetPriorSiblingNode( void ) const;	// previous node with the same parent
};

/*
================
idHierarchy<type>::idHierarchy
================
*/
template< class type >
idHierarchy<type>::idHierarchy() {
	owner	= NULL;
	parent	= NULL;	
	sibling	= NULL;
	child	= NULL;
}

/*
================
idHierarchy<type>::~idHierarchy
================
*/
template< class type >
idHierarchy<type>::~idHierarchy() {
	RemoveFromHierarchy();
}

/*
================
idHierarchy<type>::Owner

Gets the object that is associated with this node.
================
*/
template< class type >
type *idHierarchy<type>::Owner( void ) const {
	return owner;
}

/*
================
idHierarchy<type>::SetOwner

Sets the object that this node is associated with.
================
*/
template< class type >
void idHierarchy<type>::SetOwner( type *object ) {
	owner = object;
}

/*
================
idHierarchy<type>::ParentedBy
================
*/
template< class type >
bool idHierarchy<type>::ParentedBy( const idHierarchy &node ) const {
	if ( parent == &node ) {
		return true;
	} else if ( parent ) {
		return parent->ParentedBy( node );
	}
	return false;
}

/*
================
idHierarchy<type>::ParentTo

Makes the given node the parent.
================
*/
template< class type >
void idHierarchy<type>::ParentTo( idHierarchy &node ) {
	RemoveFromParent();

	parent		= &node;
	sibling		= node.child;
	node.child	= this;
}

/*
================
idHierarchy<type>::MakeSiblingAfter

Makes the given node a sibling after the passed in node.
================
*/
template< class type >
void idHierarchy<type>::MakeSiblingAfter( idHierarchy &node ) {
	RemoveFromParent();
	parent	= node.parent;
	sibling = node.sibling;
	node.sibling = this;
}

/*
================
idHierarchy<type>::RemoveFromParent
================
*/
template< class type >
void idHierarchy<type>::RemoveFromParent( void ) {
	idHierarchy<type> *prev;

	if ( parent ) {
		prev = GetPriorSiblingNode();
		if ( prev ) {
			prev->sibling = sibling;
		} else {
			parent->child = sibling;
		}
	}

	parent = NULL;
	sibling = NULL;
}

/*
================
idHierarchy<type>::RemoveFromHierarchy

Removes the node from the hierarchy and adds it's children to the parent.
================
*/
template< class type >
void idHierarchy<type>::RemoveFromHierarchy( void ) {
	idHierarchy<type> *parentNode;
	idHierarchy<type> *node;

	parentNode = parent;
	RemoveFromParent();

	if ( parentNode ) {
		while( child ) {
			node = child;
			node->RemoveFromParent();
			node->ParentTo( *parentNode );
		}
	} else {
		while( child ) {
			child->RemoveFromParent();
		}
	}
}

/*
================
idHierarchy<type>::GetParent
================
*/
template< class type >
type *idHierarchy<type>::GetParent( void ) const {
	if ( parent ) {
		return parent->owner;
	}
	return NULL;
}

/*
================
idHierarchy<type>::GetChild
================
*/
template< class type >
type *idHierarchy<type>::GetChild( void ) const {
	if ( child ) {
		return child->owner;
	}
	return NULL;
}

/*
================
idHierarchy<type>::GetSibling
================
*/
template< class type >
type *idHierarchy<type>::GetSibling( void ) const {
	if ( sibling ) {
		return sibling->owner;
	}
	return NULL;
}

/*
================
idHierarchy<type>::GetPriorSiblingNode

Returns NULL if no parent, or if it is the first child.
================
*/
template< class type >
idHierarchy<type> *idHierarchy<type>::GetPriorSiblingNode( void ) const {
	if ( !parent || ( parent->child == this ) ) {
		return NULL;
	}

	idHierarchy<type> *prev;
	idHierarchy<type> *node;

	node = parent->child;
	prev = NULL;
	while( ( node != this ) && ( node != NULL ) ) {
		prev = node;
		node = node->sibling;
	}

	if ( node != this ) {
		idLib::Error( "idHierarchy::GetPriorSibling: could not find node in parent's list of children" );
	}

	return prev;
}

/*
================
idHierarchy<type>::GetPriorSibling

Returns NULL if no parent, or if it is the first child.
================
*/
template< class type >
type *idHierarchy<type>::GetPriorSibling( void ) const {
	idHierarchy<type> *prior;

	prior = GetPriorSiblingNode();
	if ( prior ) {
		return prior->owner;
	}

	return NULL;
}

/*
================
idHierarchy<type>::GetNext

Goes through all nodes of the hierarchy.
================
*/
template< class type >
type *idHierarchy<type>::GetNext( void ) const {
	const idHierarchy<type> *node;

	if ( child ) {
		return child->owner;
	} else {
		node = this;
		while( node && node->sibling == NULL ) {
			node = node->parent;
		}
		if ( node ) {
			return node->sibling->owner;
		} else {
			return NULL;
		}
	}
}

/*
================
idHierarchy<type>::GetNextLeaf

Goes through all leaf nodes of the hierarchy.
================
*/
template< class type >
type *idHierarchy<type>::GetNextLeaf( void ) const {
	const idHierarchy<type> *node;

	if ( child ) {
		node = child;
		while ( node->child ) {
			node = node->child;
		}
		return node->owner;
	} else {
		node = this;
		while( node && node->sibling == NULL ) {
			node = node->parent;
		}
		if ( node ) {
			node = node->sibling;
			while ( node->child ) {
				node = node->child;
			}
			return node->owner;
		} else {
			return NULL;
		}
	}
}

#endif /* !__HIERARCHY_H__ */
