#include "q3map.h"
#include "VertexFormats.h"
#include "Camera.h"
#include "assert.h"

q3map::q3map(std::string name) : _mapName(name)
{
	
}

q3map::~q3map()
{
	releaseIndexBuffers();
}

//Build all the visual data from the current camera position
void q3map::BuildVis(D3DXVECTOR3 cameraPos)
{
	int cameraLeaf = findLeaf(cameraPos);

	_visibleLeaves.clear();
	_visibleFaces.clear();
	_alreadyVisibleFaces.clear();
	findVisibleLeaves(cameraLeaf);
	findVisibleFaces();

	return;
}

//Where is the camera in the BSP tree right now
int q3map::findLeaf(D3DXVECTOR3 cameraPos)
{
	int index = 0;

	while(index >= 0)
	{
		const BSPNode &node = _nodes[index];
		const BSPPlane &plane = _planes[node.plane];
		D3DXVECTOR3 planeNormal = D3DXVECTOR3(plane.vNormal);
		float distance = D3DXVec3Dot(&planeNormal, &cameraPos) - plane.d;

		if(distance >= 0)
		{
			index = node.front;
		}
		else
		{
			index = node.back;
		}
	}

	return -index - 1;
}

//What leaves can we see from this leaf
void q3map::findVisibleLeaves(int cameraLeaf)
{
	BSPLeaf *camLeaf = &_leaves[cameraLeaf];
	for(size_t i = 0; i < _leaves.size(); i++)
	{
		if(isClusterVisible(camLeaf->cluster, _leaves[i].cluster))
		{
			_visibleLeaves.push_back(i);
		}
	}
}

//Out of all our visible leaves what faces do they contain
void q3map::findVisibleFaces()
{
	for(UINT i = 0; i < _visibleLeaves.size(); i++)
	{
		BSPLeaf bspl = _leaves[_visibleLeaves[i]];
		for(int f = 0; f < bspl.leafFaceCount; f++)
		{
			int faceIndex = bspl.leafFirstFace + f;
			int leafFaceIndex = pLeafFaces[faceIndex];
			if( _alreadyVisibleFaces.find(leafFaceIndex) == _alreadyVisibleFaces.end() )
			{
				_visibleFaces.push_back(leafFaceIndex);
				_alreadyVisibleFaces.insert(leafFaceIndex);
			}
		}
	}
}

//See what else is visible from where we are
bool q3map::isClusterVisible(int currentCluster, int testCluster)
{
    if(!_visData.pBitsets || currentCluster < 0) 
		return true;

	int i = (currentCluster * _visData.bytesPerCluster) + (testCluster >> 3);
    UBYTE visSet = _visData.pBitsets[i];
    int result = visSet & (1 << (testCluster & 7));

    return result != 0;
}

void q3map::DrawMap(LPDIRECT3DDEVICE9 device, LPDIRECT3DVERTEXBUFFER9 vertBuffer, D3DXVECTOR3 cameraPos)
{
	HRESULT res;
	res = device->SetTexture(0,NULL);
	res = device->SetStreamSource(0, vertBuffer, 0, sizeof(BSPD3DVertex));
	res = device->SetFVF(D3D_Q3_VERT_FVF);
	
	drawMapIP(device);
}

void q3map::drawMapIP(LPDIRECT3DDEVICE9 device)
{
	HRESULT res;

	for(std::vector<int>::iterator i = _visibleFaces.begin(); i != _visibleFaces.end(); i++)
	{
		const BSPFace &curFace = _faces[*i];

		res = device->SetIndices(_faceIndexBuffers[*i]);

		res = device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, curFace.numMeshVerts, 0, curFace.numMeshVerts / 3 );

		if( res != D3D_OK )
		{
			assert(0);
		}
	}
}

void q3map::swizzle(float *v) 
{
    float temp = v[1];
    v[1] = v[2];
    v[2] = -temp;
}

void q3map::swizzle(int *v) 
{
    int temp = v[1];
    v[1] = v[2];
    v[2] = -temp;
}

void q3map::convertCoordForD3D()
{
	for(UINT i = 0; i < _vertices.size(); i++)
	{
		swizzle(_vertices[i].vPosition);
		swizzle(_vertices[i].vNormal);
	}

	for(UINT i = 0; i < _planes.size(); i++)
	{
		swizzle(_planes[i].vNormal);
	}

	for(UINT i = 0; i < _faces.size(); i++)
	{
		swizzle(_faces[i].vNormal);
	}

	for(UINT i = 0; i < _nodes.size(); i++)
	{
		swizzle(_nodes[i].mins);
		swizzle(_nodes[i].maxs);
	}

	for(UINT i = 0; i < _leaves.size(); i++)
	{
		swizzle(_leaves[i].mins);
		swizzle(_leaves[i].maxs);
	}

	for(UINT i = 0; i < _models.size(); i++)
	{
		swizzle(_models[i].min);
		swizzle(_models[i].max);
	}
}

bool q3map::LoadMap(LPDIRECT3DDEVICE9 device, LPDIRECT3DVERTEXBUFFER9 &vertBuffer)
{
	FILE *mapFile = NULL;
	errno_t error;
	long fSize;
	char *buffer;
	int hdr_len;

	error = fopen_s(&mapFile, _mapName.c_str(), "r");
	if( error != 0 )
	{
		assert(0);
		return false;
	}

	try
	{
		fseek(mapFile, 0, SEEK_END);
		fSize = ftell(mapFile);
		rewind(mapFile);
		buffer = (char*)malloc(sizeof(char)*fSize);
		memset(buffer, 0, sizeof(char)*fSize);

		fread(buffer, 1, fSize, mapFile);
		
		fclose(mapFile);

		memcpy(&_header, buffer, sizeof(BSPHeader));

		memcpy(&_lumpDir, buffer + sizeof(BSPHeader), sizeof(BSPLump)*MaxLumps); 

		hdr_len = sizeof(BSPHeader) + (sizeof(BSPLump)*MaxLumps);

		//lump sections

		//Entities//
		int entitiesLen = _lumpDir[Entities].length;
		int strSize = sizeof(char)*entitiesLen;
		void *ents = malloc(strSize);
		memcpy(ents, buffer + _lumpDir[Entities].offset, strSize); 
		_entities = (char*)ents;
		//Entities//

		FILE *entFile;
		error = fopen_s(&entFile, "q3Ents.txt", "w+");
		if( error != 0 )
		{
			assert(0);
		}
		else
		{
			fwrite(_entities.c_str(), strSize, 1, entFile);
			fclose(entFile);
		}

		//Textures//
		int count = _lumpDir[Textures].length / sizeof(BSPTexture);
		for(int i = 0; i < count; i++)
		{
			BSPTexture bspt;
			int curTex = sizeof(BSPTexture) * i;
			memcpy(&bspt, buffer + _lumpDir[Textures].offset + curTex, sizeof(BSPTexture));
			_textures.push_back(bspt);
		}
		//Textures//

		//Planes//
		count = _lumpDir[Planes].length / sizeof(BSPPlane);
		for(int i = 0; i < count; i++)
		{
			BSPPlane bspp;
			int curPlane = i * sizeof(BSPPlane);
			memcpy(&bspp, buffer + _lumpDir[Planes].offset + curPlane, sizeof(BSPPlane));
			_planes.push_back(bspp);
		}
		//Planes//

		//Nodes//
		count = _lumpDir[Nodes].length / sizeof(BSPNode);
		for(int i = 0; i < count; i++)
		{
			BSPNode bspn;
			int curNode = i * sizeof(BSPNode);
			memcpy(&bspn, buffer + _lumpDir[Nodes].offset + curNode, sizeof(BSPNode));
			_nodes.push_back(bspn);
		}
		//Nodes

		//Leaves//
		count = _lumpDir[Leaves].length / sizeof(BSPLeaf);
		for(int i = 0; i < count; i++)
		{
			BSPLeaf bspl;
			int curLeaf = i * sizeof(BSPLeaf);
			memcpy(&bspl, buffer + _lumpDir[Leaves].offset + curLeaf, sizeof(BSPLeaf));
			_leaves.push_back(bspl);
		}
		//Leaves//

		//Leaf Faces//
		count = _lumpDir[LeafFaces].length / sizeof(int);
		pLeafFaces = (int *)malloc(sizeof(int) * count);
		memcpy(pLeafFaces, buffer + _lumpDir[LeafFaces].offset, _lumpDir[LeafFaces].length);
		//Leaf Faces//

		//Leaf Brushes//
		count = _lumpDir[LeafBrushes].length / sizeof(int);
		pLeafBrushes = (int *)malloc(sizeof(int) * count);
		memcpy(pLeafBrushes, buffer + _lumpDir[LeafBrushes].offset, _lumpDir[LeafBrushes].length);
		//Leaf Brushes//

		//Models//
		count = _lumpDir[Models].length / sizeof(BSPModel);
		for(int i = 0; i < count; i++)
		{
			BSPModel bspm;
			int curModel = i * sizeof(BSPModel);
			memcpy(&bspm, buffer + _lumpDir[Models].offset + curModel, sizeof(BSPModel));
			_models.push_back(bspm);
		}
		//Models//

		//Brushes//
		count = _lumpDir[Brushes].length / sizeof(BSPBrush);
		for(int i = 0; i < count; i++)
		{
			BSPBrush bspb;
			int curBrush = i * sizeof(BSPBrush);
			memcpy(&bspb, buffer + _lumpDir[Brushes].offset + curBrush, sizeof(BSPBrush));
			_brushes.push_back(bspb);
		}
		//Brushes//

		//Brush Sides//
		count = _lumpDir[BrushSides].length / sizeof(BSPBrushSide);
		for(int i = 0; i < count; i++)
		{
			BSPBrushSide bspbs;
			int curBSide = i * sizeof(BSPBrushSide);
			memcpy(&bspbs, buffer + _lumpDir[BrushSides].offset + curBSide, sizeof(BSPBrushSide));
			_brushSides.push_back(bspbs);
		}
		//Brush Sides//

		//Vertices//
		count = _lumpDir[Vertices].length / sizeof(BSPVertex);
		for(int i = 0; i < count; i++)
		{
			BSPVertex bspv;
			BSPD3DVertex bspd3dv;
			int curVert = i * sizeof(BSPVertex);
			memcpy(&bspv, buffer + _lumpDir[Vertices].offset + curVert, sizeof(BSPVertex));
			//Convert the q3 vertex struct to one the d3d FVF will be happy with.
			memcpy(bspd3dv.vPosition, bspv.vPosition, sizeof(bspd3dv.vPosition));
			memcpy(bspd3dv.vNormal, bspv.vNormal, sizeof(bspd3dv.vNormal));
			memcpy(bspd3dv.color, bspv.color, sizeof(bspd3dv.color));
			memcpy(bspd3dv.vTextureCoord, bspv.vTextureCoord, sizeof(bspd3dv.vPosition));
			memcpy(bspd3dv.vLightmapCoord, bspv.vLightmapCoord, sizeof(bspd3dv.vLightmapCoord));
			_vertices.push_back(bspd3dv);
		}
		//Vertices//

		//Mesh Vertices//
		count = _lumpDir[MeshVerts].length / sizeof(int);
		pMeshVerts = (int *)malloc(_lumpDir[MeshVerts].length);
		memcpy(pMeshVerts, buffer + _lumpDir[MeshVerts].offset, _lumpDir[MeshVerts].length);
		//Mesh Vertices//

		//Shaders//
		count = _lumpDir[Shaders].length / sizeof(BSPShader);
		for(int i = 0; i < count; i++)
		{
			BSPShader bsps;
			int curShader = i * sizeof(BSPShader);
			memcpy(&bsps, buffer + _lumpDir[Shaders].offset + curShader, sizeof(BSPShader));
			_shaders.push_back(bsps);
		}
		//Shaders//

		//Faces//
		count = _lumpDir[Faces].length / sizeof(BSPFace);
		for(int i = 0; i < count; i++)
		{
			BSPFace bspf;
			int curFace = i * sizeof(BSPFace);
			memcpy(&bspf, buffer + _lumpDir[Faces].offset + curFace, sizeof(BSPFace));
			_faces.push_back(bspf);
		}
		//Faces//

		//Light Map//
		count = _lumpDir[LightMaps].length / sizeof(BSPLightMap);
		for(int i = 0; i < count; i++)
		{
			BSPLightMap bsplm;
			int curLM = i * sizeof(BSPLightMap);
			memcpy(&bsplm, buffer + _lumpDir[LightMaps].offset + curLM, sizeof(BSPLightMap));
			_lightMaps.push_back(bsplm);
		}
		//Light Map//

		//Light Volumes//
		count = _lumpDir[LightVolumes].length / sizeof(BSPLight);
		for(int i = 0; i < count; i++)
		{
			BSPLight bspl;
			int curLight = i * sizeof(BSPLight);
			memcpy(&bspl, buffer + _lumpDir[LightVolumes].offset + curLight, sizeof(BSPLight));
			_lights.push_back(bspl);
		}
		//Light Volumes//

		//Vis Data//
		memcpy(&_visData.numOfClusters, buffer + _lumpDir[VisData].offset, sizeof(int));
		memcpy(&_visData.bytesPerCluster, buffer + _lumpDir[VisData].offset + sizeof(int), sizeof(int));
		int clusterSize = _visData.numOfClusters * _visData.bytesPerCluster;
		_visData.pBitsets = (UBYTE*)malloc(clusterSize);
		memcpy(_visData.pBitsets, buffer + _lumpDir[VisData].offset + (2*sizeof(int)), clusterSize);
		//Vis Data//

		//Convert coordinate system
		convertCoordForD3D();

		UINT buffer_size = _vertices.size() * sizeof(BSPD3DVertex);
		HRESULT res = device->CreateVertexBuffer( buffer_size, D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3D_Q3_VERT_FVF, D3DPOOL_DEFAULT, &vertBuffer, NULL );
		if (res != D3D_OK)
		{
			assert(0);
			return false;
		}

		//Fill vertex buffer
		BSPD3DVertex* pVMem;

		res = vertBuffer->Lock( 0, 0, (void**)&pVMem, 0 );
		if( res != D3D_OK )
		{
			assert(0);
			return false;
		}

		memcpy(pVMem, _vertices.data(), sizeof(BSPD3DVertex) * _vertices.size());

		vertBuffer->Unlock();

		if(!createIndexBuffers(device))
		{
			assert(0);
			return false;
		}
	}
	catch(...)
	{
		fclose(mapFile);
		assert(0);
		return false;
	}

	return true;
}

bool q3map::createIndexBuffers(LPDIRECT3DDEVICE9 device)
{
	HRESULT result;
	int *pIndices;

	//Create index buffers for faces (surfaces)
	for(UINT i = 0; i < _faces.size(); i++)
	{
		LPDIRECT3DINDEXBUFFER9 ibuf;
		std::vector<int> faceIndexes;

		std::vector<int> new_ibuf; //index buffer with calculated meshVert offets

		if(_faces[i].numMeshVerts == 0)
		{
			ibuf = NULL;
			_faceIndexBuffers.push_back(ibuf);
			_faceIndexes.push_back(faceIndexes);
			continue;
		}

		//all proper vertex indices for mesh after offest calc
		for(int j = 0; j < _faces[i].numMeshVerts; j++)
		{
			int index = _faces[i].meshVertIndex + j;
			int offset = _faces[i].vertexIndex + pMeshVerts[index];
			new_ibuf.push_back(offset);
			faceIndexes.push_back(offset);
		}

		UINT buffer_size = _faces[i].numMeshVerts * sizeof(int);
		result = device->CreateIndexBuffer( buffer_size, D3DUSAGE_WRITEONLY, D3DFMT_INDEX32, D3DPOOL_DEFAULT, &ibuf, NULL );
		if (result != D3D_OK)
		{
			assert(0);
			return false;
		}

		result = ibuf->Lock( 0, 0, (void**)&pIndices, 0 );  
		if(result != D3D_OK)
		{
			assert(0);
			return false;
		}

		memcpy(pIndices, new_ibuf.data(), sizeof(int) * new_ibuf.size());
		
		ibuf->Unlock();

		_faceIndexBuffers.push_back(ibuf);
		_faceIndexes.push_back(faceIndexes);
	}

	return true;
}

//Moller/Trumbore intersection test
void q3map::RayTest(const Camera *cam, LPDIRECT3DDEVICE9 device, LPDIRECT3DVERTEXBUFFER9 vertBuffer)
{
	int hitcount = 0;
	float closest = 0;
	int _targetFace[3];

	for(UINT i = 0; i < _visibleFaces.size(); i++)
	{
		int fi = _visibleFaces[i];
		std::vector<int> indexes = _faceIndexes[fi];
		for(UINT j = 0; j < indexes.size(); j+=3)
		{
			int vertOffset1 = indexes[j];
			int vertOffset2 = indexes[j+1];
			int vertOffset3 = indexes[j+2];

			//get verts for the vertex array
			BSPD3DVertex *bsp_v0 = &_vertices[vertOffset1];
			BSPD3DVertex *bsp_v1 = &_vertices[vertOffset2];
			BSPD3DVertex *bsp_v2 = &_vertices[vertOffset3];

			D3DXVECTOR3 v0 = D3DXVECTOR3(bsp_v0->vPosition[0], bsp_v0->vPosition[1], bsp_v0->vPosition[2]);
			D3DXVECTOR3 v1 = D3DXVECTOR3(bsp_v1->vPosition[0], bsp_v1->vPosition[1], bsp_v1->vPosition[2]);
			D3DXVECTOR3 v2 = D3DXVECTOR3(bsp_v2->vPosition[0], bsp_v2->vPosition[1], bsp_v2->vPosition[2]);

			//Edge vecotrs
			D3DXVECTOR3 e1 = v1 - v0;
			D3DXVECTOR3 e2 = v2 - v0;

			//Test ray direction against triangle
			D3DXVECTOR3 p;
			D3DXVec3Cross(&p, &(cam->GetLook()), &e2);
			float a = D3DXVec3Dot(&e1, &p);

			if(a == 0)
				continue;

			// compute denom
			float f = 1.0f / a;

			//barycentric coordinates
			//Calculate if plane intersect is in poly
			D3DXVECTOR3 s = cam->GetPosition() - v0;
			float u = f * D3DXVec3Dot(&s, &p);
			if(u < 0.0f || u > 1.0f)
				continue;

			D3DXVECTOR3 q;
			D3DXVec3Cross(&q, &s, &e1);
			float v = f * D3DXVec3Dot(&(cam->GetLook()), &q);
			if(v < 0.0f || ((u + v) > 1.0f))
				continue;

			//line/ray parameter
			float t = f * D3DXVec3Dot(&e2, &q);

			if(t >= 0)
			{
				if(closest == 0 || t < closest)
				{
					_targetFace[0] = vertOffset1;
					_targetFace[1] = vertOffset2;
					_targetFace[2] = vertOffset3;
					hitcount++;
					closest = t;
				}
			}
		}
	}


	if(hitcount > 0)
	{
		BSPD3DVertex* pVMem;

		HRESULT res = vertBuffer->Lock( 0, 0, (void**)&pVMem, D3DLOCK_NOOVERWRITE);
		if( res != D3D_OK )
		{
			assert(0);
		}
		
		//Paint green highlight when hit
		pVMem[_targetFace[0]].color[1] = 255;
		pVMem[_targetFace[0]].color[2] = 0;
		pVMem[_targetFace[0]].color[3] = 0;
		pVMem[_targetFace[1]].color[1] = 255;		
		pVMem[_targetFace[1]].color[2] = 0;		
		pVMem[_targetFace[1]].color[3] = 0;		
		pVMem[_targetFace[2]].color[1] = 255;
		pVMem[_targetFace[2]].color[2] = 0;
		pVMem[_targetFace[2]].color[3] = 0;
	
		vertBuffer->Unlock();
		
		return;
	}
}

void q3map::releaseIndexBuffers()
{
	for(UINT i = 0; i < _faceIndexBuffers.size(); i++)
	{
		//only drawing polygonal surfaces (no bezier) so 
		//there will be null refs in faces.
		if(_faceIndexBuffers[i])
			_faceIndexBuffers[i]->Release();
	}
}