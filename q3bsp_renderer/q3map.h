#pragma once
#ifndef Q3MAP_H
#define Q3MAP_H
//File format/structure referenced from http://www.misofruit.co.kr/seojewoo/programming/opengl/Quake3Format.htm

#include <string>
#include <vector>
#include <set>
#include "d3dx9math.h"

class Camera;

using namespace std;

class q3map
{
public:
	typedef unsigned char UBYTE;

	q3map(std::string mapName);
	~q3map();

	//bsp file lumps in order of appearance
	enum eLumps
	{
		Entities = 0,     // Stores player/object positions, etc...
		Textures,         // Stores texture information
		Planes,           // Stores the splitting planes
		Nodes,            // Stores the BSP nodes
		Leaves,            // Stores the leafs of the nodes
		LeafFaces,        // Stores the leaf's indices into the faces
		LeafBrushes,      // Stores the leaf's indices into the brushes
		Models,           // Stores the info of world models
		Brushes,          // Stores the brushes info (for collision)
		BrushSides,       // Stores the brush surfaces info
		Vertices,         // Stores the level vertices
		MeshVerts,        // Stores the model vertices offsets
		Shaders,          // Stores the shader files (blending, anims..)
		Faces,            // Stores the faces for the level
		LightMaps,        // Stores the lightmaps for the level
		LightVolumes,     // Stores extra world lighting information
		VisData,          // Stores PVS and cluster info (visibility)
		MaxLumps          // A constant to store the number of lumps
	};

	struct BSPLump
	{
		int offset; //location in file
		int length; //length of lump
	};

	struct BSPHeader
	{
		char strID[4];     // This should always be 'IBSP'
		int version;       // This should be 0x2e for Quake 3 files
	}; 

	struct BSPVertex
	{
		float vPosition[3];      // (x, y, z) position. 
		float vTextureCoord[2];  // (u, v) texture coordinate
		float vLightmapCoord[2]; // (u, v) lightmap coordinate
		float vNormal[3];        // (x, y, z) normal vector
		UBYTE color[4];           // RGBA color for the vertex 
	};

	struct BSPD3DVertex
	{
		float vPosition[3];      // (x, y, z) position. 
		float vNormal[3];        // (x, y, z) normal vector
		UBYTE color[4];           // RGBA color for the vertex 
		float vTextureCoord[2];  // (u, v) texture coordinate
		float vLightmapCoord[2]; // (u, v) lightmap coordinate
	};

	struct BSPFace
	{
		int textureID;        // The index into the texture array 
		int effect;           // The index for the effects (or -1 = n/a) 
		int type;             // 1=polygon, 2=patch, 3=mesh, 4=billboard 
		int vertexIndex;      // The index into this face's first vertex 
		int numOfVerts;       // The number of vertices for this face 
		int meshVertIndex;    // The index into the first meshvertex 
		int numMeshVerts;     // The number of mesh vertices 
		int lightmapID;       // The texture index for the lightmap 
		int lMapCorner[2];    // The face's lightmap corner in the image 
		int lMapSize[2];      // The size of the lightmap section 
		float lMapPos[3];     // The 3D origin of lightmap. 
		float lMapBitsets[2][3]; // The 3D space for s and t unit vectors. 
		float vNormal[3];     // The face normal. 
		int size[2];          // The bezier patch dimensions. 
	};

	struct BSPTexture
	{
		char strName[64];   // The name of the texture w/o the extension 
		int flags;          // The surface flags (unknown) 
		int contents;       // The content flags (unknown)
	};

	struct BSPLightMap
	{
		UBYTE imageBits[128][128][3];   // The RGB data in a 128x128 image
	};

	struct BSPNode
	{
		int plane;      // The index into the planes array 
		int front;      // The child index for the front node 
		int back;       // The child index for the back node 
		int mins[3];    // The bounding box min position. 
		int maxs[3];    // The bounding box max position. 
	};

	struct BSPLeaf
	{
		int cluster;           // The visibility cluster 
		int area;              // The area portal 
		int mins[3];           // The bounding box min position 
		int maxs[3];           // The bounding box max position 
		int leafFirstFace;     // The first index into the face array 
		int leafFaceCount;    // The number of faces for this leaf 
		int leafBrush;         // The first index for into the brushes 
		int numOfLeafBrushes;  // The number of brushes for this leaf 
	}; 

	struct BSPPlane
	{
		float vNormal[3];     // Plane normal. 
		float d;              // The plane distance from origin 
	};

	struct BSPVisData
	{
		int numOfClusters;   // The number of clusters
		int bytesPerCluster; // Bytes (8 bits) in the cluster's bitset
		UBYTE *pBitsets;      // Array of bytes holding the cluster vis.
	};

	struct BSPBrush 
	{
		int brushSide;           // The starting brush side for the brush 
		int numOfBrushSides;     // Number of brush sides for the brush
		int textureID;           // The texture index for the brush
	};

	struct BSPBrushSide 
	{
		int plane;              // The plane index
		int textureID;          // The texture index
	};

	struct BSPModel 
	{
		float min[3];           // The min position for the bounding box
		float max[3];           // The max position for the bounding box. 
		int faceIndex;          // The first face index in the model 
		int numOfFaces;         // The number of faces in the model 
		int brushIndex;         // The first brush index in the model 
		int numOfBrushes;       // The number brushes for the model
	};

	struct BSPShader
	{
		char strName[64];     // The name of the shader file 
		int brushIndex;       // The brush index for this shader 
		int unknown;          // This is 99% of the time 5
	};

	struct BSPLight
	{
		UBYTE ambient[3];     // This is the ambient color in RGB
		UBYTE directional[3]; // This is the directional color in RGB
		UBYTE direction[2];   // The direction of the light: [phi,theta] 
	};

public: 

	bool LoadMap(LPDIRECT3DDEVICE9 device, LPDIRECT3DVERTEXBUFFER9 &vertBuffer);
	void BuildVis(D3DXVECTOR3 cameraPos);
	void DrawMap(LPDIRECT3DDEVICE9 device, LPDIRECT3DVERTEXBUFFER9 vertBuffer, D3DXVECTOR3 cameraPos);
	int GetNumVertices() const { return _vertices.size(); }
	void RayTest(const Camera *cam, LPDIRECT3DDEVICE9 device, LPDIRECT3DVERTEXBUFFER9 vertBuffer);

private:

	std::string _mapName;
	BSPHeader _header;
	BSPLump _lumpDir[MaxLumps]; //lump directory, each lumps location and size
	std::string _entities;
	std::vector<BSPTexture> _textures;
	std::vector<BSPPlane> _planes; //BSP planes to check if in front or back of node space
	std::vector<BSPNode> _nodes; //BSP nodes
	std::vector<BSPLeaf> _leaves;
	int *pLeafFaces;           // The index into the face array
	int *pLeafBrushes;         // The index into the brush array
	std::vector<BSPModel> _models;
	std::vector<BSPBrush> _brushes;
	std::vector<BSPBrushSide> _brushSides;
	std::vector<BSPD3DVertex> _vertices;
	int *pMeshVerts;           // The offsets for index buffer for a complete mesh
	std::vector<BSPShader> _shaders;
	std::vector<BSPFace> _faces; 
	std::vector<BSPLightMap> _lightMaps;
	std::vector<BSPLight> _lights; 
	BSPVisData _visData; //visibility data between clusters
	std::vector<int> _visibleLeaves; //bsp leaves in view
	std::set<int> _alreadyVisibleFaces; //keep track so we dont double up on faces to render or check
	std::vector<int> _visibleFaces; //faces to render or ray check
	std::vector<LPDIRECT3DINDEXBUFFER9> _faceIndexBuffers; //index buffer for a mesh
	std::vector< std::vector<int> > _faceIndexes; //all faces for a mesh.  This is after the offset calculations from meshVerts

	//Find the leaf of the tree our camera is in.
	int findLeaf(D3DXVECTOR3 cameraPos);
	void findVisibleLeaves(int cameraLeaf);
	bool isClusterVisible(int currentCluster, int testCluster);
	void findVisibleFaces();
	bool createIndexBuffers(LPDIRECT3DDEVICE9 device);

	void drawMapIP(LPDIRECT3DDEVICE9 device);

	//Convert q3 coordinate system to play nice with d3d
	void swizzle(float *v);
	void swizzle(int *v);
	void convertCoordForD3D();

	void releaseIndexBuffers();
};



#endif 