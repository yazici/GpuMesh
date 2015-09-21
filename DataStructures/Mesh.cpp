#include "Mesh.h"

#include <algorithm>
#include <iostream>
#include <cstdint>

#include <CellarWorkbench/Misc/Log.h>

#include "Evaluators/AbstractEvaluator.h"

#include "OptimizationPlot.h"

using namespace std;
using namespace cellar;


const MeshEdge MeshTet::edges[MeshTet::EDGE_COUNT] = {
    MeshEdge(0, 1),
    MeshEdge(0, 2),
    MeshEdge(0, 3),
    MeshEdge(1, 2),
    MeshEdge(2, 3),
    MeshEdge(3, 1)
};

const MeshTri MeshTet::tris[MeshTet::TRI_COUNT] = {
    MeshTri(0, 1, 2),
    MeshTri(0, 2, 3),
    MeshTri(0, 3, 1),
    MeshTri(1, 3, 2)
};

const MeshTet MeshTet::tets[MeshTet::TET_COUNT] = {
    MeshTet(0, 1, 2, 3)
};


const MeshEdge MeshPri::edges[MeshPri::EDGE_COUNT] = {
    MeshEdge(0, 1),
    MeshEdge(0, 2),
    MeshEdge(1, 3),
    MeshEdge(2, 3),
    MeshEdge(0, 4),
    MeshEdge(1, 5),
    MeshEdge(2, 4),
    MeshEdge(3, 5),
    MeshEdge(4, 5)
};

const MeshTri MeshPri::tris[MeshPri::TRI_COUNT] = {
    MeshTri(2, 1, 0), // Z neg face 0
    MeshTri(1, 2, 3), // Z neg face 1
    MeshTri(1, 4, 0), // Y neg face 0
    MeshTri(4, 1, 5), // Y neg face 1
    MeshTri(4, 3, 2), // Y pos face 0
    MeshTri(3, 4, 5), // Y pos face 1
    MeshTri(0, 4, 2), // X neg face
    MeshTri(1, 3, 5)  // X pos face
};

const MeshTet MeshPri::tets[MeshPri::TET_COUNT] = {
    MeshTet(4, 0, 1, 2),
    MeshTet(5, 1, 3, 2),
    MeshTet(4, 1, 5, 2)
};


const MeshEdge MeshHex::edges[MeshHex::EDGE_COUNT] = {
    MeshEdge(0, 1),
    MeshEdge(0, 2),
    MeshEdge(1, 3),
    MeshEdge(2, 3),
    MeshEdge(0, 4),
    MeshEdge(1, 5),
    MeshEdge(2, 6),
    MeshEdge(3, 7),
    MeshEdge(4, 5),
    MeshEdge(4, 6),
    MeshEdge(5, 7),
    MeshEdge(6, 7)
};

const MeshTri MeshHex::tris[MeshHex::TRI_COUNT] = {
    MeshTri(2, 1, 0), // Z neg face 0
    MeshTri(1, 2, 3), // Z pos face 1
    MeshTri(5, 6, 4), // Z pos face 0
    MeshTri(6, 5, 7), // Z pos face 1
    MeshTri(1, 4, 0), // Y neg face 0
    MeshTri(4, 1, 5), // Y neg face 1
    MeshTri(2, 7, 3), // Y pos face 0
    MeshTri(7, 2, 6), // Y pos face 1
    MeshTri(4, 2, 0), // X neg face 0
    MeshTri(2, 4, 6), // X neg face 1
    MeshTri(7, 1, 3), // X pos face 0
    MeshTri(1, 7, 5), // X pos face 1
};

const MeshTet MeshHex::tets[MeshHex::TET_COUNT] = {
    MeshTet(0, 1, 4, 2),
    MeshTet(3, 2, 7, 1),
    MeshTet(5, 1, 7, 4),
    MeshTet(6, 2, 4, 7),
    MeshTet(1, 7, 4, 2)
};


MeshBound::MeshBound(int id) :
    _id(id)
{

}

MeshBound::~MeshBound()
{

}

glm::dvec3 MeshBound::operator()(const glm::dvec3& pos) const
{
    return pos;
}


const MeshBound MeshTopo::NO_BOUNDARY = MeshBound(0);

MeshTopo::MeshTopo() :
    isFixed(false),
    isBoundary(false),
    snapToBoundary(&NO_BOUNDARY)
{
}

MeshTopo::MeshTopo(
        bool isFixed) :
    isFixed(isFixed),
    isBoundary(false),
    snapToBoundary(&NO_BOUNDARY)
{
}

MeshTopo::MeshTopo(const MeshBound* boundaryCallback) :
    isFixed(false),
    isBoundary(boundaryCallback != &NO_BOUNDARY),
    snapToBoundary(boundaryCallback)
{
}


const int Mesh::NO_GROUP = 0;
const int Mesh::UNSET_GROUP = -1;
const int Mesh::FIRST_GROUP =  1;

Mesh::Mesh() :
    _modelBoundsShaderName(":/shaders/compute/Boundary/None.glsl")
{

}

Mesh::~Mesh()
{

}

void Mesh::clear()
{
    verts.clear();
    verts.shrink_to_fit();
    tets.clear();
    tets.shrink_to_fit();
    pris.clear();
    pris.shrink_to_fit();
    hexs.clear();
    hexs.shrink_to_fit();
    topos.clear();
    topos.shrink_to_fit();
    independentGroups.clear();
    independentGroups.shrink_to_fit();
}

void Mesh::compileTopoly()
{
    getLog().postMessage(new Message('I', false,
        "Compiling mesh topology...", "Mesh"));

    // Compact verts and elems data structures
    verts.shrink_to_fit();
    tets.shrink_to_fit();
    pris.shrink_to_fit();
    hexs.shrink_to_fit();

    size_t vertCount = verts.size();
    topos.resize(vertCount);
    topos.shrink_to_fit();

    auto neigBegin = chrono::high_resolution_clock::now();
    compileNeighborhoods();

    auto indeBegin = chrono::high_resolution_clock::now();
    compileIndependentGroups();

    auto compileEnd = chrono::high_resolution_clock::now();


    getLog().postMessage(new Message('I', false,
        "Vertice count: " + to_string(vertCount), "Mesh"));

    size_t elemCount = tets.size() + pris.size() + hexs.size();
    getLog().postMessage(new Message('I', false,
        "Element count: " + to_string(elemCount), "Mesh"));

    double elemVertRatio = elemCount  / (double) vertCount;
    getLog().postMessage(new Message('I', false,
        "Element count / Vertice count: " + to_string(elemVertRatio), "Mesh"));

    getLog().postMessage(new Message('I', false,
        "Independent set count: " + to_string(independentGroups.size()), "Mesh"));

    size_t neigVertCount = 0;
    size_t neigElemCount = 0;
    for(int i=0; i < vertCount; ++i)
    {
        neigVertCount += topos[i].neighborVerts.size();
        neigElemCount += topos[i].neighborElems.size();
    }

    int64_t meshMemorySize =
            int64_t(verts.size() * sizeof(decltype(verts.front()))) +
            int64_t(tets.size() * sizeof(decltype(tets.front()))) +
            int64_t(pris.size() * sizeof(decltype(pris.front()))) +
            int64_t(hexs.size() * sizeof(decltype(hexs.front()))) +
            int64_t(topos.size() * sizeof(decltype(topos.front()))) +
            int64_t(neigVertCount * sizeof(MeshNeigVert)) +
            int64_t(neigElemCount * sizeof(MeshNeigElem));
    getLog().postMessage(new Message('I', false,
        "Approx mesh size in memory: " + to_string(meshMemorySize) + " Bytes", "Mesh"));

    int neigTime = chrono::duration_cast<chrono::milliseconds>(indeBegin - neigBegin).count();
    getLog().postMessage(new Message('I', false,
        "Neighborhood compilation time: " + to_string(neigTime) + "ms", "Mesh"));

    int indeTime = chrono::duration_cast<chrono::milliseconds>(compileEnd - indeBegin).count();
    getLog().postMessage(new Message('I', false,
        "Independent groups compilation time: " + to_string(indeTime) + "ms", "Mesh"));
}

void Mesh::updateGpuTopoly()
{

}

void Mesh::updateGpuVertices()
{

}

void Mesh::updateCpuVertices()
{

}

std::string Mesh::meshGeometryShaderName() const
{
    return std::string();
}

void Mesh::uploadGeometry(cellar::GlProgram& program) const
{

}

unsigned int Mesh::glBuffer(const EMeshBuffer&) const
{
    return 0;
}

void Mesh::bindShaderStorageBuffers() const
{

}

size_t Mesh::firstFreeBufferBinding() const
{
    return 0;
}

std::string Mesh::modelBoundsShaderName() const
{
    return _modelBoundsShaderName;
}

void Mesh::setmodelBoundariesShaderName(const std::string& name)
{
    _modelBoundsShaderName = name;
}

void Mesh::printPropperties(OptimizationPlot& plot) const
{
    plot.addMeshProperty("Model Name",        modelName);
    plot.addMeshProperty("Vertex Count",      to_string(verts.size()));
    plot.addMeshProperty("Tet Count",         to_string(tets.size()));
    plot.addMeshProperty("Prism Count",       to_string(pris.size()));
    plot.addMeshProperty("Hex Count",         to_string(hexs.size()));
    plot.addMeshProperty("Patch Group Count", to_string(independentGroups.size()));
}

void Mesh::compileNeighborhoods()
{
    size_t tetCount = tets.size();
    for(size_t i=0; i < tetCount; ++i)
    {
        for(int v=0; v < MeshTet::VERTEX_COUNT; ++v)
        {
            topos[tets[i].v[v]].neighborElems.push_back(
                MeshNeigElem(MeshTet::ELEMENT_TYPE, i));
        }

        for(int e=0; e < MeshTet::EDGE_COUNT; ++e)
        {
            addEdge(tets[i].v[MeshTet::edges[e][0]],
                    tets[i].v[MeshTet::edges[e][1]]);
        }
    }

    size_t prismCount = pris.size();
    for(size_t i=0; i < prismCount; ++i)
    {
        for(int v=0; v < MeshPri::VERTEX_COUNT; ++v)
        {
            topos[pris[i].v[v]].neighborElems.push_back(
                MeshNeigElem(MeshPri::ELEMENT_TYPE, i));
        }

        for(int e=0; e < MeshPri::EDGE_COUNT; ++e)
        {
            addEdge(pris[i].v[MeshPri::edges[e][0]],
                    pris[i].v[MeshPri::edges[e][1]]);
        }
    }

    size_t hexCount = hexs.size();
    for(size_t i=0; i < hexCount; ++i)
    {
        for(int v=0; v < MeshHex::VERTEX_COUNT; ++v)
        {
            topos[hexs[i].v[v]].neighborElems.push_back(
                MeshNeigElem(MeshHex::ELEMENT_TYPE, i));
        }

        for(int e=0; e < MeshHex::EDGE_COUNT; ++e)
        {
            addEdge(hexs[i].v[MeshHex::edges[e][0]],
                    hexs[i].v[MeshHex::edges[e][1]]);
        }
    }


    // Compact topology neighborhoods
    size_t vertCount = verts.size();
    for(int i=0; i < vertCount; ++i)
    {
        topos[i].neighborVerts.shrink_to_fit();
        topos[i].neighborElems.shrink_to_fit();
    }
}

void Mesh::addEdge(int firstVert, int secondVert)
{
    vector<MeshNeigVert>& neighbors = topos[firstVert].neighborVerts;
    int neighborCount = neighbors.size();
    for(int n=0; n < neighborCount; ++n)
    {
        if(secondVert == neighbors[n])
            return;
    }

    // This really is a new edge
    topos[firstVert].neighborVerts.push_back(secondVert);
    topos[secondVert].neighborVerts.push_back(firstVert);
}

void Mesh::compileIndependentGroups()
{
    size_t vertCount = verts.size();

    size_t seekStart = 0;
    std::set<uint> existingGroups;
    std::vector<size_t> nextNodes;
    std::vector<int> vertGroup(vertCount, NO_GROUP);
    while(nextNodes.size() < vertCount)
    {
        size_t firstNode = nextNodes.size();
        for(size_t v=seekStart; v < vertCount; ++v)
        {
            ++seekStart;
            if(vertGroup[v] == NO_GROUP)
            {
                vertGroup[v] = UNSET_GROUP;
                nextNodes.push_back(v);
                break;
            }
        }

        for(int v=firstNode; v < nextNodes.size(); ++v)
        {
            MeshTopo& topo = topos[v];
            std::set<uint> availableGroups = existingGroups;

            for(size_t e=0; e < topo.neighborElems.size(); ++e)
            {
                MeshNeigElem& neigElem = topo.neighborElems[e];
                if(neigElem.type == MeshTet::ELEMENT_TYPE)
                {
                    MeshTet& elem = tets[neigElem.id];
                    for(size_t n=0; n < MeshTet::VERTEX_COUNT; ++n)
                    {
                        int& group = vertGroup[elem.v[n]];
                        if(group == NO_GROUP)
                        {
                            group = UNSET_GROUP;
                            nextNodes.push_back(elem.v[n]);
                        }
                        else if(group != UNSET_GROUP)
                        {
                            availableGroups.erase(group);
                        }
                    }
                }
                else if(neigElem.type == MeshPri::ELEMENT_TYPE)
                {
                    MeshPri& elem = pris[neigElem.id];
                    for(size_t n=0; n < MeshPri::VERTEX_COUNT; ++n)
                    {
                        int& group = vertGroup[elem.v[n]];
                        if(group == NO_GROUP)
                        {
                            group = UNSET_GROUP;
                            nextNodes.push_back(elem.v[n]);
                        }
                        else if(group != UNSET_GROUP)
                        {
                            availableGroups.erase(group);
                        }
                    }
                }
                else if(neigElem.type == MeshHex::ELEMENT_TYPE)
                {
                    MeshHex& elem = hexs[neigElem.id];
                    for(size_t n=0; n < MeshHex::VERTEX_COUNT; ++n)
                    {
                        int& group = vertGroup[elem.v[n]];
                        if(group == NO_GROUP)
                        {
                            group = UNSET_GROUP;
                            nextNodes.push_back(elem.v[n]);
                        }
                        else if(group != UNSET_GROUP)
                        {
                            availableGroups.erase(group);
                        }
                    }
                }
            }

            int group;
            if(availableGroups.empty())
            {
                group = existingGroups.size() + 1;
                existingGroups.insert(group);
                independentGroups.push_back(std::vector<uint>());
            }
            else
            {
                group = *availableGroups.begin();
            }

            vertGroup[v] = group;
            independentGroups[group-1].push_back(v);
        }
    }


    // Make independent groups as compact as possible
    independentGroups.shrink_to_fit();
    for(auto& group : independentGroups)
        group.shrink_to_fit();
}
