
#include "stdafx.h"
#include "CubeRenderer0.h"
#include "DisplayList.h"
#include "RenderContext.h"
#include <D3dx9math.h>
#include "Debug.h"
#include <assert.h>

using namespace Math;

#ifdef OPENGL
extern PFNGLGENBUFFERSARBPROC pglGenBuffersARB;
extern PFNGLBINDBUFFERARBPROC pglBindBufferARB;                     
extern PFNGLBUFFERDATAARBPROC pglBufferDataARB;                               
extern PFNGLDELETEBUFFERSARBPROC pglDeleteBuffersARB;               
extern PFNGLGETBUFFERPARAMETERIVARBPROC pglGetBufferParameterivARB; 
extern PFNGLMAPBUFFERARBPROC pglMapBufferARB;                       
extern PFNGLUNMAPBUFFERARBPROC pglUnmapBufferARB;  
#endif

CubeRenderer0::VertexData CubeRenderer0::CubeData[] =
{
	// back
	{-1.0f,  1.0f, -1.0f,		0.0f, 0.0f, -1.0f,		0.0f, 0.0f },
	{ 1.0f,  1.0f, -1.0f,		0.0f, 0.0f, -1.0f,		1.0f, 0.0f },
	{-1.0f, -1.0f, -1.0f,		0.0f, 0.0f, -1.0f,		0.0f, 1.0f },
	{ 1.0f, -1.0f, -1.0f,		0.0f, 0.0f, -1.0f,		1.0f, 1.0f },
	
	// front
	{ -1.0f,  1.0f, 1.0f,		0.0f, 0.0f, 1.0f,		1.0f, 0.0f },
	{ -1.0f, -1.0f, 1.0f,		0.0f, 0.0f, 1.0f,		1.0f, 1.0f },
	{  1.0f,  1.0f, 1.0f,		0.0f, 0.0f, 1.0f,		0.0f, 0.0f },
	{  1.0f, -1.0f, 1.0f,		0.0f, 0.0f, 1.0f,		0.0f, 1.0f },
	
	// top
	{ -1.0f, 1.0f,  1.0f,		0.0f, 1.0f, 0.0f,		0.0f,0.0f },
	{  1.0f, 1.0f,  1.0f,		0.0f, 1.0f, 0.0f,		1.0f,0.0f },
	{ -1.0f, 1.0f, -1.0f,		0.0f, 1.0f, 0.0f,		0.0f,1.0f },
	{  1.0f, 1.0f, -1.0f,		0.0f, 1.0f, 0.0f,		1.0f,1.0f },
	
	// bottom
	{ -1.0f, -1.0f,  1.0f,		0.0f, -1.0f, 0.0f,		0.0f,0.0f },
	{ -1.0f, -1.0f, -1.0f,		0.0f, -1.0f, 0.0f,		1.0f,0.0f },
	{  1.0f, -1.0f,  1.0f,		0.0f, -1.0f, 0.0f,		0.0f,1.0f },
	{  1.0f, -1.0f, -1.0f,		0.0f, -1.0f, 0.0f,		1.0f,1.0f },

	//right 
	{ 1.0f,  1.0f, -1.0f,		1.0f, 0.0f, 0.0f,		0.0f,0.0f },
	{ 1.0f,  1.0f,  1.0f,		1.0f, 0.0f, 0.0f,		1.0f,0.0f },
	{ 1.0f, -1.0f, -1.0f,		1.0f, 0.0f, 0.0f,		0.0f,1.0f },
	{ 1.0f, -1.0f,  1.0f,		1.0f, 0.0f, 0.0f,		1.0f,1.0f },
	
	// left
	{-1.0f,  1.0f,-1.0f,		-1.0f, 0.0f, 0.0f,		 1.0f,0.0f },
	{-1.0f, -1.0f,-1.0f,		-1.0f, 0.0f, 0.0f,		 1.0f,1.0f },
	{-1.0f,  1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,		 0.0f,0.0f },
	{-1.0f, -1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,		 0.0f,1.0f }
};

WORD CubeRenderer0::CubeIndices[] = 
{
	// back
	0, 1, 2,
	1, 3, 2,

	// front
	5, 6, 4,
	5, 7, 6,

	// top
	11, 10, 8,
	8, 9, 11,

	// bottom
	12, 13, 15,
	15, 14, 12,

	// right
	19, 18, 16,
	17, 19, 16,

	// left
	23, 22, 21,
	21, 22, 20,

};


CubeRenderer0::CubeRenderer0(  RenderContext* context )
{
	m_Rotation = 0.0f;
	m_Scale = 1.0f;
	m_NumCubes = 0;
	m_NumTransparentCubes = 0;
	GridTopLeft = Vector( -50, 45, 0 );
	GridBottomRight = Vector( 50, -55, 50 );

	m_CubeRotationSpeed = 0.1f; 
	m_LightRotationSpeed = 0.25f;

	m_DynamicUpdate = true;

	SetLightPosition( GridTopLeft + (GridTopLeft - GridBottomRight).Normal().Scale( 50 ) );
}

CubeRenderer0::~CubeRenderer0()
{
}

void CubeRenderer0::ReleaseResources( RenderContext* )
{
#ifndef OPENGL
	if( m_pVertexDeclaration ) 
	{
		int c = m_pVertexDeclaration->Release();
		m_pVertexDeclaration = 0;
	}

	if( m_pVertexBuffer ) 
	{
		int c = m_pVertexBuffer->Release();
		m_pVertexBuffer = 0;
	}

	if( m_pIndexBuffer ) 
	{
		int c = m_pIndexBuffer->Release();
		m_pIndexBuffer = 0;
	}

	if( m_Texture ) 
	{
		int c = m_Texture->Release();
		m_Texture = 0;
	}

	if( m_pEffect ) 
	{
		int c = m_pEffect->Release();
		m_pEffect = 0;

		m_effWorldViewProjection = 0; 
		m_effTexture = 0;
		m_effTechnique = 0;
		m_effTransparentTechnique = 0;
		m_effPosition = 0;
		m_effRotationTexture = 0;
		m_effRotationAngleScale = 0;
		m_effLightDirection = 0;
	}
#else
#endif
}

void CubeRenderer0::AcquireResources( RenderContext* context )
{
	const int numVerticesPerCube = sizeof(CubeData) /sizeof(CubeData[0]);
	const int numIndicesPerCube = sizeof(CubeIndices) / sizeof(CubeIndices[0]);

	// adjust texture coordinates for atlas
	const int numTextures = 8;
	for( int i=0; i<numVerticesPerCube; ++i )
		CubeData[i].u /= numTextures;

#ifndef OPENGL
	// create vertex declaration
	{
		D3DVERTEXELEMENT9 vertexElements[] = 
		{
			{0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
			{0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},
			{0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
			D3DDECL_END()
		};

		context->Device()->CreateVertexDeclaration( vertexElements, &m_pVertexDeclaration );
	}

	// create and fill vertex and index buffers
	{
		context->Device()->CreateVertexBuffer( numVerticesPerCube * VERTEX_SIZE, D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, &m_pVertexBuffer, NULL );
		context->Device()->CreateIndexBuffer( numIndicesPerCube * sizeof( WORD ), 0,  D3DFMT_INDEX16, D3DPOOL_MANAGED, &m_pIndexBuffer, NULL );

		// fill vertex buffer
		VertexData* vertices;
		WORD* indices;

		m_pVertexBuffer->Lock( 0, 0, (void**)&vertices, 0 );
		m_pIndexBuffer->Lock( 0, 0, (void**)&indices, 0 );

		memcpy( vertices, CubeData, sizeof( CubeData ) );
		memcpy( indices, CubeIndices, sizeof( CubeIndices ) );

		m_pVertexBuffer->Unlock();
		m_pIndexBuffer->Unlock();
	}

	// load shader
	{
		TCHAR szDirectory[MAX_PATH] = "";

		GetCurrentDirectory(sizeof(szDirectory) - 1, szDirectory);

		LPD3DXBUFFER buf;
		HRESULT hr = D3DXCreateEffectFromFile( context->Device(), "../Shaders/Cube0.fx", NULL, NULL, NULL, NULL, &m_pEffect, &buf );

		if( FAILED(hr) && buf)
		{
			const char* text = reinterpret_cast<char*>( buf->GetBufferPointer() );
			DXTRACE_ERR( text, hr );
			buf->Release();
		}

		m_effTechnique = m_pEffect->GetTechniqueByName( "Cube" );
		m_effWorldViewProjection = m_pEffect->GetParameterBySemantic( NULL, "WORLDVIEWPROJECTION" );
		m_effTexture = m_pEffect->GetParameterBySemantic( NULL, "TEXTURE" );
		
		m_effTransparentTechnique = m_pEffect->GetTechniqueByName( "TransparentCube" );
        m_effPosition = m_pEffect->GetParameterBySemantic( NULL, "INSTANCE_POSITION" );
        m_effRotationTexture = m_pEffect->GetParameterBySemantic( NULL, "INSTANCE_ROTATION" );
        m_effRotationAngleScale = m_pEffect->GetParameterBySemantic( NULL, "ROTATIONANGLESCALE" );
        m_effLightDirection = m_pEffect->GetParameterBySemantic( NULL, "LIGHTDIRECTION" );	
	}

	// load texture
	{
		HRESULT hr = D3DXCreateTextureFromFile( context->Device(), "Textures/smiley.dds", &m_Texture );
		if( FAILED( hr ) )
		{
			DXTRACE_ERR( "Error loading texture", hr );
		}
	}
#else
	

	m_pEffect = cgCreateEffectFromFile(context->CgContext(), "Cube.fx", NULL);

	m_effTechnique  = cgGetFirstTechnique(m_pEffect);
	m_TransparentTechnique = cgGetNextTechnique(m_effTechnique);
	
	m_effWorldViewProjection = cgGetEffectParameterBySemantic( m_pEffect, "WORLDVIEWPROJECTION" );
	m_effTexture = cgGetEffectParameterBySemantic( m_pEffect, "TEXTURE" );
		
    m_effInstancePosition = cgGetEffectParameterBySemantic( m_pEffect, "INSTANCE_POSITION" );
    m_effInstanceRotation = cgGetEffectParameterBySemantic( m_pEffect, "INSTANCE_ROTATION" );
    m_effRotationAngleScale = cgGetEffectParameterBySemantic( m_pEffect, "ROTATIONANGLESCALE" );
    m_effLightDirection = cgGetEffectParameterBySemantic( m_pEffect, "LIGHTDIRECTION" );	
	
	// fill buffers 
	VertexData vertices[numVerticesPerCube * CUBES_PER_BATCH];
	WORD indices[numIndicesPerCube * CUBES_PER_BATCH];

	FillBuffers( vertices, indices, numVerticesPerCube, numIndicesPerCube, CUBES_PER_BATCH );

	glGenBuffersARB(1, &m_VerticesVBO);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_VerticesVBO);
    glBufferDataARB(GL_ARRAY_BUFFER_ARB, VERTEX_SIZE * CUBES_PER_BATCH * numVerticesPerCube, vertices , GL_STATIC_DRAW_ARB);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

	glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, m_IndicesVBO);
	glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, sizeof(WORD) * CUBES_PER_BATCH * numIndicesPerCube, indices, GL_STATIC_DRAW_ARB);
	glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
#endif
}

void CubeRenderer0::AddCubes( int count )
{
	const int numTextures = 8;

	for( int i=0; i<count; ++i )
	{
		Math::Vector rotationAxis = Math::Vector( 
			Math::Random::Next<float>(-1,1),
			Math::Random::Next<float>(-1,1), 
			Math::Random::Next<float>(-1,1) 
		);
		
		InstanceRotationTexture.push_back( rotationAxis.Normal() );
		InstanceRotationTexture.back().w = Math::Random::Next<int>( 0, numTextures+1 ) / (float)numTextures;


		++m_NumCubes;
	}
	
	UpdateGrid();
}

void CubeRenderer0::RemoveCubes( int count )
{
	if( m_NumCubes > 0 )
	{ 
		m_NumCubes = max(0, m_NumCubes - count );
		UpdateGrid();
	}
}


void CubeRenderer0::SetLightPosition( Math::Vector lightPosition )
{
	Vector gridCenter = (GridTopLeft + GridBottomRight).Scale( 0.5f );
	m_LightDirection = (lightPosition - gridCenter).Normal();
	m_LightPosition = lightPosition;
}

void CubeRenderer0::UpdateGrid()
{
	const int layerCount = (int)ceil( pow( (float)m_NumCubes, 1.0f/3.0f ) );
	const int cubesPerLayer = layerCount * layerCount;

	InstancePosition.resize( m_NumCubes );
	InstanceRotationTexture.resize( m_NumCubes );

	std::vector<D3DXVECTOR4> positions;
	positions.resize (m_NumCubes );

	std::vector<bool> transparent;
	transparent.resize( m_NumCubes );

	m_NumTransparentCubes = 0;

	// figure out position and transparency setting for each cube
	for( int z=0; z<layerCount; ++z )
	{
		for( int y=0; y<layerCount; ++y )
		{
			for( int x=0; x<layerCount; ++x )
			{
				const int cubeIndex = z * cubesPerLayer + layerCount * y + x;
				if( cubeIndex < m_NumCubes )
				{
					positions[cubeIndex].x = GridTopLeft.GetX() + (float)x/layerCount * (GridBottomRight.GetX() - GridTopLeft.GetX());
					positions[cubeIndex].y = GridTopLeft.GetY() + (float)y/layerCount * (GridBottomRight.GetY()- GridTopLeft.GetY());
					positions[cubeIndex].z = GridTopLeft.GetZ() + (float)z/layerCount * (GridBottomRight.GetZ() - GridTopLeft.GetZ());
					positions[cubeIndex].w = 1.0f;	

					transparent[cubeIndex] = Math::Random::Next() < 0.33;
				}
			}
		}
	}

	// rearrange cubes: partition instance position array into two parts: opaque cubes first and then transparent cubes
	// at the same time establish ordering: opaque cubes: front to back, transparent cubes: back to front.
	int j=0, k = InstancePosition.size()-1;
	for( unsigned int i=0; i<positions.size(); ++i )
	{
		if( !transparent[i] )
		{
			InstancePosition[j] = positions[i];
			++j;
		}
		else
		{
			InstancePosition[k] = positions[i];
			--k;
			++m_NumTransparentCubes;
		}
	}

	// just a sanity check
	assert( abs(j - k) == 1 );
}

void CubeRenderer0:: Update( float dt )
{
	if( m_DynamicUpdate )
	{
		m_Rotation += m_CubeRotationSpeed * dt;
	
		Matrix lightRotation = Matrix::RotationYawPitchRoll( Vector(0.0f,  0.0f, m_LightRotationSpeed).Scale( dt ) );
		Vector newLightPosition = lightRotation.Transform( m_LightPosition );
		SetLightPosition( newLightPosition );
	}
}


void CubeRenderer0::Render( RenderContext* context )
{
	return;
	context->Device()->SetVertexDeclaration( m_pVertexDeclaration );
	context->Device()->SetStreamSource( 0, m_pVertexBuffer, 0, VERTEX_SIZE );
	context->Device()->SetIndices( m_pIndexBuffer );
 
	m_pEffect->SetMatrix( m_effWorldViewProjection, &(context->GetViewMatrix() * context->GetProjectionMatrix()).data );
	m_pEffect->SetTexture( m_effTexture, m_Texture );

	D3DXVECTOR4 rotationScale;
	rotationScale.x = cos( m_Rotation );
	rotationScale.y = sin( m_Rotation );
	rotationScale.z = GetScale();
	 m_pEffect->SetVector( m_effRotationAngleScale, &rotationScale );

	D3DXVECTOR4 lightDirection = m_LightDirection;
	m_pEffect->SetVector( m_effLightDirection, &lightDirection );

	// render opaque cubes first
	{
		m_pEffect->SetTechnique( m_effTechnique );
	
		UINT cPasses;
		m_pEffect->Begin( &cPasses, 0 );

		for( unsigned int iPass = 0; iPass < cPasses; iPass++ )
		{
			m_pEffect->BeginPass( iPass );
			RenderInstances( context, 0, m_NumCubes - m_NumTransparentCubes );
			m_pEffect->EndPass();
		}

		m_pEffect->End();
	}

	// now the transparent cubes. renderstates were initially set in the shader but cg didn't agree. So I moved them here.
	{
		context->Device()->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
		context->Device()->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
		context->Device()->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
		context->Device()->SetRenderState( D3DRS_ZWRITEENABLE, D3DZB_FALSE);

		HRESULT hr = m_pEffect->SetTechnique( m_effTransparentTechnique );
	
		UINT cPasses;
		m_pEffect->Begin( &cPasses, 0 );

		for( unsigned int iPass = 0; iPass < cPasses; iPass++ )
		{
			m_pEffect->BeginPass( iPass );
			RenderInstances( context, m_NumCubes - m_NumTransparentCubes, m_NumTransparentCubes );
			m_pEffect->EndPass();
		}

		m_pEffect->End();

		context->Device()->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
		context->Device()->SetRenderState( D3DRS_ZWRITEENABLE, D3DZB_TRUE);

	}

	m_pEffect->SetTexture( m_effTexture, NULL );
	context->Device()->SetStreamSource( 0, NULL, 0, 0 );
	context->Device()->SetIndices( NULL );
}

HRESULT CubeRenderer0::RenderInstances( RenderContext* context, int start, int count )
{
	HRESULT hr =  S_OK;

	for( int c=0; c<count; ++c )
	{
		hr = m_pEffect->SetVector( m_effPosition, &InstancePosition[start + c] );
		hr = m_pEffect->SetVector( m_effRotationTexture, &InstanceRotationTexture[start + c] );

		hr = m_pEffect->CommitChanges();
		context->Device()->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, 0, 4 * 6, 0, 6 * 2 );
	}

	return hr;
}
