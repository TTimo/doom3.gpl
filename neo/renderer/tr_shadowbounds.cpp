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
#include "../idlib/precompiled.h"
#pragma hdrstop

#include "tr_local.h"



// Compute conservative shadow bounds as the intersection
// of the object's bounds' shadow volume and the light's bounds.
// 
// --cass


template <class T, int N>
struct MyArray
{
	MyArray() : s(0) {}

	MyArray( const MyArray<T,N> & cpy ) : s(cpy.s)
	{
		for(int i=0; i < s; i++)
			v[i] = cpy.v[i];
	}

	void push_back(const T & i) {
		v[s] = i;
		s++;
		//if(s > max_size)
		//	max_size = int(s);
	}

	T & operator[](int i) {
		return v[i];
	}

	const T & operator[](int i) const {
		return v[i];
	}

	unsigned int size() const {
		return s;
	}

	void empty() {
		s = 0;
	}

	T v[N];
	int s;
//	static int max_size;
};

typedef MyArray<int, 4> MyArrayInt;
//int MyArrayInt::max_size = 0;
typedef MyArray<idVec4, 16> MyArrayVec4;
//int MyArrayVec4::max_size = 0;

struct poly
{
    MyArrayInt vi;
    MyArrayInt ni;
    idVec4 plane;
};

typedef MyArray<poly, 9> MyArrayPoly;
//int MyArrayPoly::max_size = 0;

struct edge
{
    int vi[2];
    int pi[2];
};

typedef MyArray<edge, 15> MyArrayEdge;
//int MyArrayEdge::max_size = 0;

MyArrayInt four_ints(int a, int b, int c, int d)
{
    MyArrayInt vi;
    vi.push_back(a);
    vi.push_back(b);
    vi.push_back(c);
    vi.push_back(d);
    return vi;
}

idVec3 homogeneous_difference(idVec4 a, idVec4 b)
{
    idVec3 v;
	v.x = b.x * a.w - a.x * b.w;
	v.y = b.y * a.w - a.y * b.w;
	v.z = b.z * a.w - a.z * b.w;
    return v;
}

// handles positive w only
idVec4 compute_homogeneous_plane(idVec4 a, idVec4 b, idVec4 c)
{
    idVec4 v, t;

    if(a[3] == 0)
    { t = a; a = b; b = c; c = t; }
    if(a[3] == 0)
    { t = a; a = b; b = c; c = t; }

    // can't handle 3 infinite points
    if( a[3] == 0 )
        return v;

    idVec3 vb = homogeneous_difference(a, b);
    idVec3 vc = homogeneous_difference(a, c);
    
    idVec3 n = vb.Cross(vc);
    n.Normalize();
    
	v.x = n.x;
	v.y = n.y;
	v.z = n.z;

	v.w = - (n * idVec3(a.x, a.y, a.z)) / a.w ;

    return v;
}

struct polyhedron
{
    MyArrayVec4 v;
    MyArrayPoly  p;
    MyArrayEdge  e;

    void add_quad( int va, int vb, int vc, int vd )
    {
        poly pg;
        pg.vi = four_ints(va, vb, vc, vd);
        pg.ni = four_ints(-1, -1, -1, -1);
        pg.plane = compute_homogeneous_plane(v[va], v[vb], v[vc]);
        p.push_back(pg);
    }

    void discard_neighbor_info()
    {
        for(unsigned int i = 0; i < p.size(); i++ )
        {
            MyArrayInt & ni = p[i].ni;
            for(unsigned int j = 0; j < ni.size(); j++)
                ni[j] = -1;
        }
    }

    void compute_neighbors()
    {
		e.empty();

        discard_neighbor_info();

        bool found;
        int P = p.size();
        // for each polygon
        for(int i = 0; i < P-1; i++ )
        {
            const MyArrayInt & vi = p[i].vi;
            MyArrayInt & ni = p[i].ni;
            int Si = vi.size();

            // for each edge of that polygon
            for(int ii=0; ii < Si; ii++)
            {
                int ii0 = ii;
                int ii1 = (ii+1) % Si;

                // continue if we've already found this neighbor
                if(ni[ii] != -1)
                    continue;
                found = false;
                // check all remaining polygons
                for(int j = i+1; j < P; j++ )
                {
                    const MyArrayInt & vj = p[j].vi;
                    MyArrayInt & nj = p[j].ni;
                    int Sj = vj.size();

                    for( int jj = 0; jj < Sj; jj++ )
                    {
                        int jj0 = jj;
                        int jj1 = (jj+1) % Sj;
                        if(vi[ii0] == vj[jj1] && vi[ii1] == vj[jj0])
                        {
                            edge ed;
                            ed.vi[0] = vi[ii0];
                            ed.vi[1] = vi[ii1];
                            ed.pi[0] = i;
                            ed.pi[1] = j;
                            e.push_back(ed);
                            ni[ii] = j;
                            nj[jj] = i;
                            found = true;
                            break;
                        }
                        else if ( vi[ii0] == vj[jj0] && vi[ii1] == vj[jj1] )
                        {
                            fprintf(stderr,"why am I here?\n");
                        }
                    }
                    if( found ) 
                        break;
                }
            }
        }
    }

    void recompute_planes()
    {
        // for each polygon
        for(unsigned int i = 0; i < p.size(); i++ )
        {
            p[i].plane = compute_homogeneous_plane(v[p[i].vi[0]], v[p[i].vi[1]], v[p[i].vi[2]]);
        }
    }

    void transform(const idMat4 & m)
    {
        for(unsigned int i=0; i < v.size(); i++ )
            v[i] = m * v[i];
        recompute_planes();
    }

};

// make a unit cube
polyhedron PolyhedronFromBounds( const idBounds & b )
{

//       3----------2
//       |\        /|
//       | \      / |
//       |   7--6   |
//       |   |  |   |
//       |   4--5   |
//       |  /    \  |
//       | /      \ |
//       0----------1
//

	static polyhedron p;

	if( p.e.size() == 0 ) {

		p.v.push_back(idVec4( -1, -1,  1, 1));
		p.v.push_back(idVec4(  1, -1,  1, 1));
		p.v.push_back(idVec4(  1,  1,  1, 1));
		p.v.push_back(idVec4( -1,  1,  1, 1));
		p.v.push_back(idVec4( -1, -1, -1, 1));
		p.v.push_back(idVec4(  1, -1, -1, 1));
		p.v.push_back(idVec4(  1,  1, -1, 1));
		p.v.push_back(idVec4( -1,  1, -1, 1));

		p.add_quad( 0, 1, 2, 3 );
		p.add_quad( 7, 6, 5, 4 );
		p.add_quad( 1, 0, 4, 5 );
		p.add_quad( 2, 1, 5, 6 );
		p.add_quad( 3, 2, 6, 7 );
		p.add_quad( 0, 3, 7, 4 );

		p.compute_neighbors();
		p.recompute_planes();
		p.v.empty(); // no need to copy this data since it'll be replaced
	}

	polyhedron p2(p);

	const idVec3 & min = b[0];
	const idVec3 & max = b[1];

	p2.v.empty();
	p2.v.push_back(idVec4( min.x, min.y, max.z, 1));
	p2.v.push_back(idVec4( max.x, min.y, max.z, 1));
	p2.v.push_back(idVec4( max.x, max.y, max.z, 1));
	p2.v.push_back(idVec4( min.x, max.y, max.z, 1));
	p2.v.push_back(idVec4( min.x, min.y, min.z, 1));
	p2.v.push_back(idVec4( max.x, min.y, min.z, 1));
	p2.v.push_back(idVec4( max.x, max.y, min.z, 1));
	p2.v.push_back(idVec4( min.x, max.y, min.z, 1));

	p2.recompute_planes();
    return p2;
}


polyhedron make_sv(const polyhedron & oc, idVec4 light)
{
	static polyhedron lut[64];
	int index = 0;

	for(unsigned int i = 0; i < 6; i++) {
		if( ( oc.p[i].plane * light ) > 0 )
			index |= 1<<i;
	}

	if( lut[index].e.size() == 0 )
	{
		polyhedron & ph = lut[index];
		ph = oc;

		int V = ph.v.size();
		for( int j = 0; j < V; j++ ) 
		{
			idVec3 proj = homogeneous_difference( light, ph.v[j] );
			ph.v.push_back( idVec4(proj.x, proj.y, proj.z, 0) );
		}

		ph.p.empty(); 

		for(unsigned int i=0; i < oc.p.size(); i++)
		{
			if( (oc.p[i].plane * light) > 0)
			{
				ph.p.push_back(oc.p[i]);
			}
		}

		if(ph.p.size() == 0)
			return ph = polyhedron();

		ph.compute_neighbors();

		MyArrayPoly vpg;
		int I = ph.p.size();

		for(int i=0; i < I; i++)
		{
			MyArrayInt & vi = ph.p[i].vi;
			MyArrayInt & ni = ph.p[i].ni;
			int S = vi.size();

			for(int j = 0; j < S; j++)
			{
				if( ni[j] == -1 )
				{
					poly pg;
					int a = vi[(j+1)%S];
					int b = vi[j];
					pg.vi = four_ints( a, b, b+V, a+V);
					pg.ni = four_ints(-1, -1, -1, -1);
					vpg.push_back(pg);
				}
			}
		}
		for(unsigned int i = 0; i < vpg.size(); i++)
			ph.p.push_back(vpg[i]);

		ph.compute_neighbors();
		ph.v.empty(); // no need to copy this data since it'll be replaced
	}

	polyhedron ph2 = lut[index];

	// initalize vertices
	ph2.v = oc.v;
	int V = ph2.v.size();
	for( int j = 0; j < V; j++ ) 
	{
		idVec3 proj = homogeneous_difference( light, ph2.v[j] );
		ph2.v.push_back( idVec4(proj.x, proj.y, proj.z, 0) );
	}

    // need to compute planes for the shadow volume (sv)
    ph2.recompute_planes();

    return ph2;
}

typedef MyArray<idVec4, 36> MySegments;
//int MySegments::max_size = 0;

void polyhedron_edges(polyhedron & a, MySegments & e)
{
	e.empty();
    if(a.e.size() == 0 && a.p.size() != 0)
        a.compute_neighbors();

    for(unsigned int i = 0; i < a.e.size(); i++)
    {
        e.push_back(a.v[a.e[i].vi[0]]);
        e.push_back(a.v[a.e[i].vi[1]]);
    }

}

// clip the segments of e by the planes of polyhedron a.
void clip_segments(const polyhedron & ph, MySegments & is, MySegments & os)
{
    const MyArrayPoly & p = ph.p;

    for(unsigned int i = 0; i < is.size(); i+=2 )
    {
        idVec4 a = is[i  ];
        idVec4 b = is[i+1];
        idVec4 c;

        bool discard = false;

        for(unsigned int j = 0; j < p.size(); j++ )
        {
            float da = a * p[j].plane;
            float db = b * p[j].plane;
            float rdw = 1/(da - db);

            int code = 0;
            if( da > 0 )
                code = 2;
            if( db > 0 )
                code |= 1;


            switch ( code ) 
            {
            case 3:
                discard = true;
                break;

            case 2:
                c = -db * rdw * a + da * rdw * b;
                a = c;
                break;

            case 1:
                c = -db * rdw * a + da * rdw * b;
                b = c;
                break;

            case 0:
                break;

            default:
                common->Printf("bad clip code!\n");
                break;
            }

            if( discard )
                break;
        }

        if( ! discard )
        {
            os.push_back(a);
            os.push_back(b);
        }
    }

}

idMat4 make_idMat4(const float * m)
{
	return idMat4( m[ 0], m[ 4], m[ 8], m[12],
				   m[ 1], m[ 5], m[ 9], m[13],
				   m[ 2], m[ 6], m[10], m[14],
				   m[ 3], m[ 7], m[11], m[15] );
}

idVec3 v4to3(const idVec4 & v)
{
	return idVec3(v.x/v.w, v.y/v.w, v.z/v.w);
}

void draw_polyhedron( const viewDef_t *viewDef, const polyhedron & p, idVec4 color )
{
	for(unsigned int i = 0; i < p.e.size(); i++)
	{
		viewDef->renderWorld->DebugLine( color, v4to3(p.v[p.e[i].vi[0]]), v4to3(p.v[p.e[i].vi[1]]));
	}
}

void draw_segments( const viewDef_t *viewDef, const MySegments & s, idVec4 color )
{
	for(unsigned int i = 0; i < s.size(); i+=2)
	{
		viewDef->renderWorld->DebugLine( color, v4to3(s[i]), v4to3(s[i+1]));
	}
}

void world_to_hclip( const viewDef_t *viewDef, const idVec4 &global, idVec4 &clip ) {
	int		i;
	idVec4	view;

	for ( i = 0 ; i < 4 ; i ++ ) {
		view[i] = 
			global[0] * viewDef->worldSpace.modelViewMatrix[ i + 0 * 4 ] +
			global[1] * viewDef->worldSpace.modelViewMatrix[ i + 1 * 4 ] +
			global[2] * viewDef->worldSpace.modelViewMatrix[ i + 2 * 4 ] +
			global[3] *	viewDef->worldSpace.modelViewMatrix[ i + 3 * 4 ];
	}


	for ( i = 0 ; i < 4 ; i ++ ) {
		clip[i] = 
			view[0] * viewDef->projectionMatrix[ i + 0 * 4 ] +
			view[1] * viewDef->projectionMatrix[ i + 1 * 4 ] +
			view[2] * viewDef->projectionMatrix[ i + 2 * 4 ] +
			view[3] * viewDef->projectionMatrix[ i + 3 * 4 ];
	}
}

idScreenRect R_CalcIntersectionScissor( const idRenderLightLocal * lightDef,
									    const idRenderEntityLocal * entityDef,
										const viewDef_t * viewDef ) {

	idMat4 omodel = make_idMat4( entityDef->modelMatrix );
	idMat4 lmodel = make_idMat4( lightDef->modelMatrix );

	// compute light polyhedron
	polyhedron lvol = PolyhedronFromBounds( lightDef->frustumTris->bounds );
	// transform it into world space
	//lvol.transform( lmodel );

	// debug //
	if ( r_useInteractionScissors.GetInteger() == -2 ) {
		draw_polyhedron( viewDef, lvol, colorRed );
	}

	// compute object polyhedron
	polyhedron vol = PolyhedronFromBounds( entityDef->referenceBounds );

	//viewDef->renderWorld->DebugBounds( colorRed, lightDef->frustumTris->bounds );
	//viewDef->renderWorld->DebugBox( colorBlue, idBox( model->Bounds(), entityDef->parms.origin, entityDef->parms.axis ) );

	// transform it into world space
    vol.transform( omodel );

	// debug //
	if ( r_useInteractionScissors.GetInteger() == -2 ) {
		draw_polyhedron( viewDef, vol, colorBlue );
	}

	// transform light position into world space
	idVec4 lightpos = idVec4(lightDef->globalLightOrigin.x,
							 lightDef->globalLightOrigin.y,
							 lightDef->globalLightOrigin.z,
							 1.0f );

	// generate shadow volume "polyhedron"
    polyhedron sv = make_sv(vol, lightpos);

    MySegments in_segs, out_segs;

	// get shadow volume edges
    polyhedron_edges(sv, in_segs);
	// clip them against light bounds planes
    clip_segments(lvol, in_segs, out_segs);

	// get light bounds edges
	polyhedron_edges(lvol, in_segs);
	// clip them by the shadow volume
    clip_segments(sv, in_segs, out_segs);

	// debug // 
	if ( r_useInteractionScissors.GetInteger() == -2 ) {
		draw_segments( viewDef, out_segs, colorGreen );
	}

	idBounds outbounds;
	outbounds.Clear();
	for( unsigned int i = 0; i < out_segs.size(); i++ ) {

		idVec4 v;
		world_to_hclip( viewDef, out_segs[i], v );

		if( v.w <= 0.0f ) {
			return lightDef->viewLight->scissorRect;
		}

		idVec3 rv(v.x, v.y, v.z);
		rv /= v.w;

		outbounds.AddPoint( rv );
	}

	// limit the bounds to avoid an inside out scissor rectangle due to floating point to short conversion
	if ( outbounds[0].x < -1.0f ) {
		outbounds[0].x = -1.0f;
	}
	if ( outbounds[1].x > 1.0f ) {
		outbounds[1].x = 1.0f;
	}
	if ( outbounds[0].y < -1.0f ) {
		outbounds[0].y = -1.0f;
	}
	if ( outbounds[1].y > 1.0f ) {
		outbounds[1].y = 1.0f;
	}

	float w2 = ( viewDef->viewport.x2 - viewDef->viewport.x1 + 1 ) / 2.0f;
	float x = viewDef->viewport.x1;
	float h2 = ( viewDef->viewport.y2 - viewDef->viewport.y1 + 1 ) / 2.0f;
	float y = viewDef->viewport.y1;

	idScreenRect rect;
	rect.x1 = outbounds[0].x * w2 + w2 + x;
	rect.x2 = outbounds[1].x * w2 + w2 + x;
	rect.y1 = outbounds[0].y * h2 + h2 + y;
	rect.y2 = outbounds[1].y * h2 + h2 + y;
	rect.Expand();

	rect.Intersect( lightDef->viewLight->scissorRect );

	// debug //
	if ( r_useInteractionScissors.GetInteger() == -2 && !rect.IsEmpty() ) {
		viewDef->renderWorld->DebugScreenRect( colorYellow, rect, viewDef );
	}

	return rect;
}
