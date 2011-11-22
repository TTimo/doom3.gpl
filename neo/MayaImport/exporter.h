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
#define MAYA_DEFAULT_CAMERA		"camera1"

#define	ANIM_TX			BIT( 0 )
#define	ANIM_TY			BIT( 1 )
#define	ANIM_TZ			BIT( 2 )
#define	ANIM_QX			BIT( 3 )
#define	ANIM_QY			BIT( 4 )
#define	ANIM_QZ			BIT( 5 )

typedef enum {
	WRITE_MESH,
	WRITE_ANIM,
	WRITE_CAMERA
} exportType_t;

typedef struct {
	idCQuat				q;
	idVec3				t;
} jointFrame_t;

typedef struct {
	idCQuat				q;
	idVec3				t;
	float				fov;
} cameraFrame_t;

/*
==============================================================================================

	idTokenizer

==============================================================================================
*/

class idTokenizer {
private:
	int					currentToken;
	idStrList			tokens;

public:
						idTokenizer()			{ Clear(); };
	void				Clear( void )			{ currentToken = 0;	tokens.Clear(); };

	int					SetTokens( const char *buffer );
	const char			*NextToken( const char *errorstring = NULL );

	bool				TokenAvailable( void )	{ return currentToken < tokens.Num(); };
	int					Num( void ) 			{ return tokens.Num(); };
	void				UnGetToken( void )		{ if ( currentToken > 0 ) { currentToken--; } };
	const char			*GetToken( int index )	{ if ( ( index >= 0 ) && ( index < tokens.Num() ) ) { return tokens[ index ]; } else { return NULL; } };
	const char			*CurrentToken( void )	{ return GetToken( currentToken ); };
};

/*
==============================================================================================

	idExportOptions

==============================================================================================
*/

class idNamePair {
public:
	idStr	from;
	idStr	to;
};

class idAnimGroup {
public:
	idStr		name;
	idStrList	joints;
};

class idExportOptions {
private:
	idTokenizer				tokens;
							
	void					Reset( const char *commandline );
							
public:						
	idStr					commandLine;
	idStr					src;
	idStr					dest;
	idStr					game;
	idStr					prefix;
	float					scale;
	exportType_t			type;
	bool					ignoreMeshes;
	bool					clearOrigin;
	bool					clearOriginAxis;
	bool					ignoreScale;
	int						startframe;
	int						endframe;
	int						framerate;
	float					xyzPrecision;
	float					quatPrecision;
	idStr					align;
	idList<idNamePair>		renamejoints;
	idList<idNamePair>		remapjoints;
	idStrList				keepjoints;
	idStrList				skipmeshes;
	idStrList				keepmeshes;
	idList<idAnimGroup *>	exportgroups;
	idList<idAnimGroup>		groups;
	float					rotate;
	float					jointThreshold;
	int						cycleStart;
							
							idExportOptions( const char *commandline, const char *ospath );
							
	bool					jointInExportGroup( const char *jointname );
};

/*
==============================================================================

idExportJoint

==============================================================================
*/

class idExportJoint {
public:
	idStr						name;
	idStr						realname;
	idStr						longname;
	int							index;
	int							exportNum;
	bool						keep;
								
	float						scale;
	float						invscale;
								
	MFnDagNode					*dagnode;

	idHierarchy<idExportJoint>	mayaNode;
	idHierarchy<idExportJoint>	exportNode;
							
	idVec3						t;
	idMat3						wm;

	idVec3						idt;
	idMat3						idwm;

	idVec3						bindpos;
	idMat3						bindmat;

	int							animBits;
	int							firstComponent;
	jointFrame_t				baseFrame;
	int							depth;
								
								idExportJoint();
	idExportJoint				&operator=( const idExportJoint &other );
};

/*
==============================================================================

misc structures

==============================================================================
*/

typedef struct {
	idExportJoint			*joint;
	float					jointWeight;
	idVec3					offset;
} exportWeight_t;

typedef struct {
	idVec3					pos;
	idVec2					texCoords;
	int						startweight;
	int						numWeights;
} exportVertex_t;

typedef struct {
	int						indexes[ 3 ];
} exportTriangle_t;

typedef struct {
	idVec2					uv[ 3 ];
} exportUV_t;

ID_INLINE int operator==( exportVertex_t a, exportVertex_t b ) {
	if ( a.pos != b.pos ) {
		return false;
	}

	if ( ( a.texCoords[ 0 ] != b.texCoords[ 0 ] ) || ( a.texCoords[ 1 ] != b.texCoords[ 1 ] ) ) {
		return false;
	}

	if ( ( a.startweight != b.startweight ) || ( a.numWeights != b.numWeights ) ) {
		return false;
	}

	return true;
}

/*
========================================================================

.MD3 triangle model file format

========================================================================
*/

#define MD3_IDENT			(('3'<<24)+('P'<<16)+('D'<<8)+'I')
#define MD3_VERSION			15

// limits
#define MD3_MAX_LODS		4
#define	MD3_MAX_TRIANGLES	8192	// per surface
#define MD3_MAX_VERTS		4096	// per surface
#define MD3_MAX_SHADERS		256		// per surface
#define MD3_MAX_FRAMES		1024	// per model
#define	MD3_MAX_SURFACES	32		// per model
#define MD3_MAX_TAGS		16		// per frame

// vertex scales
#define	MD3_XYZ_SCALE		(1.0/64)

// surface geometry should not exceed these limits
#define	SHADER_MAX_VERTEXES	1000
#define	SHADER_MAX_INDEXES	(6*SHADER_MAX_VERTEXES)


// the maximum size of game reletive pathnames
#define	MAX_Q3PATH		64

typedef struct md3Frame_s {
	idVec3		bounds[2];
	idVec3		localOrigin;
	float		radius;
	char		name[16];
} md3Frame_t;

typedef struct md3Tag_s {
	char		name[MAX_Q3PATH];	// tag name
	idVec3		origin;
	idVec3		axis[3];
} md3Tag_t;

/*
** md3Surface_t
**
** CHUNK			SIZE
** header			sizeof( md3Surface_t )
** shaders			sizeof( md3Shader_t ) * numShaders
** triangles[0]		sizeof( md3Triangle_t ) * numTriangles
** st				sizeof( md3St_t ) * numVerts
** XyzNormals		sizeof( md3XyzNormal_t ) * numVerts * numFrames
*/
typedef struct {
	int			ident;				// 
				
	char		name[MAX_Q3PATH];	// polyset name
				
	int			flags;
	int			numFrames;			// all surfaces in a model should have the same
				
	int			numShaders;			// all surfaces in a model should have the same
	int			numVerts;
				
	int			numTriangles;
	int			ofsTriangles;
				
	int			ofsShaders;			// offset from start of md3Surface_t
	int			ofsSt;				// texture coords are common for all frames
	int			ofsXyzNormals;		// numVerts * numFrames
				
	int			ofsEnd;				// next surface follows
} md3Surface_t;

typedef struct {
	char		name[MAX_Q3PATH];
	int			shaderIndex;	// for in-game use
} md3Shader_t;

typedef struct {
	int			indexes[3];
} md3Triangle_t;

typedef struct {
	float		st[2];
} md3St_t;

typedef struct {
	short		xyz[3];
	short		normal;
} md3XyzNormal_t;

typedef struct {
	int			ident;
	int			version;

	char		name[MAX_Q3PATH];	// model name

	int			flags;

	int			numFrames;
	int			numTags;			
	int			numSurfaces;

	int			numSkins;

	int			ofsFrames;			// offset for first frame
	int			ofsTags;			// numFrames * numTags
	int			ofsSurfaces;		// first surface, others follow

	int			ofsEnd;				// end of file
} md3Header_t;

/*
==============================================================================

idExportMesh

==============================================================================
*/

class idExportMesh {
public:

	idStr						name;
	idStr						shader;

	bool						keep;

	idList<exportVertex_t>		verts;
	idList<exportTriangle_t>	tris;
	idList<exportWeight_t>		weights;
	idList<exportUV_t>			uv;

								idExportMesh() { keep = true; };
	void						ShareVerts( void );
	void						GetBounds( idBounds &bounds ) const;
	void						Merge( idExportMesh *mesh );
};

/*
==============================================================================

idExportModel

==============================================================================
*/

class idExportModel {
public:
	idExportJoint				*exportOrigin;
	idList<idExportJoint>		joints;
	idHierarchy<idExportJoint>	mayaHead;
	idHierarchy<idExportJoint>	exportHead;
	idList<int>					cameraCuts;
	idList<cameraFrame_t>		camera;
	idList<idBounds>			bounds;
	idList<jointFrame_t>		jointFrames;
	idList<jointFrame_t	*>		frames;
	int							frameRate;
	int							numFrames;
	int							skipjoints;
	int							export_joints;								
	idList<idExportMesh *>		meshes;

								idExportModel();
								~idExportModel();
	idExportJoint				*FindJointReal( const char *name );
	idExportJoint				*FindJoint( const char *name );
	bool						WriteMesh( const char *filename, idExportOptions &options );
	bool						WriteAnim( const char *filename, idExportOptions &options );
	bool						WriteCamera( const char *filename, idExportOptions &options );
};

/*
==============================================================================

Maya

==============================================================================
*/

class idMayaExport {
private:
	idExportModel			model;
	idExportOptions			&options;

	void					FreeDagNodes( void );

	float					TimeForFrame( int num ) const;
	int						GetMayaFrameNum( int num ) const;
	void					SetFrame( int num );


	void					GetBindPose( MObject &jointNode, idExportJoint *joint, float scale );
	void					GetLocalTransform( idExportJoint *joint, idVec3 &pos, idMat3 &mat );
	void					GetWorldTransform( idExportJoint *joint, idVec3 &pos, idMat3 &mat, float scale );
	
	void					CreateJoints( float scale );
	void					PruneJoints( idStrList &keepjoints, idStr &prefix );
	void					RenameJoints( idList<idNamePair> &renamejoints, idStr &prefix );
	bool					RemapParents( idList<idNamePair> &remapjoints );

	MObject					FindShader( MObject& setNode );
	void					GetTextureForMesh( idExportMesh *mesh, MFnDagNode &dagNode );

	idExportMesh			*CopyMesh( MFnSkinCluster &skinCluster, float scale );
	void					CreateMesh( float scale );
	void					CombineMeshes( void );

	void					GetAlignment( idStr &alignName, idMat3 &align, float rotate, int startframe );

	const char				*GetObjectType( MObject object );

	float					GetCameraFov( idExportJoint *joint );
	void					GetCameraFrame( idExportJoint *camera, idMat3 &align, cameraFrame_t *cam );
	void					CreateCameraAnim( idMat3 &align );

	void					GetDefaultPose( idMat3 &align );
	void					CreateAnimation( idMat3 &align );

public:
							idMayaExport( idExportOptions &exportOptions ) : options( exportOptions ) { };
							~idMayaExport();

	void					ConvertModel( void );
	void					ConvertToMD3( void );
};
