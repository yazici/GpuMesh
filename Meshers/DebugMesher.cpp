#include "DebugMesher.h"

#include <GLM/gtc/random.hpp>
#include <GLM/gtc/matrix_transform.hpp>

#include "Boundaries/BoxBoundary.h"
#include "Boundaries/TetBoundary.h"

#include <DataStructures/Tetrahedralizer.h>

using namespace std;


DebugMesher::DebugMesher() :
    _boxBoundary(new BoxBoundary()),
    _tetBoundary(new TetBoundary())
{
    using namespace std::placeholders;
    _modelFuncs.setDefault("Tet");
    _modelFuncs.setContent({
        {string("Singles"), ModelFunc(bind(&DebugMesher::genSingles, this, _1, _2))},
        {string("Squish"),  ModelFunc(bind(&DebugMesher::genSquish,  this, _1, _2))},
        {string("HexGrid"), ModelFunc(bind(&DebugMesher::genHexGrid, this, _1, _2))},
        {string("TetGrid"), ModelFunc(bind(&DebugMesher::genTetGrid, this, _1, _2))},
        {string("Cube"),    ModelFunc(bind(&DebugMesher::genCube,    this, _1, _2))},
        {string("Tet"),     ModelFunc(bind(&DebugMesher::genTet,     this, _1, _2))},
    });
}

DebugMesher::~DebugMesher()
{

}

void DebugMesher::genSingles(Mesh& mesh, size_t vertexCount)
{
    size_t eBase = 0;
    glm::dvec3 offset;
    glm::dvec3 jitter = glm::dvec3(0.4);
    double scale = 0.4 / glm::pow(double(vertexCount), 1.0/3.0);
    for(size_t vId=0; vId < vertexCount; ++vId)
    {
        double rotXY = glm::linearRand(0.0, glm::pi<double>() * 2.0);
        double rotXZ = glm::linearRand(0.0, glm::pi<double>() * 2.0);
        glm::dmat4 matRotXY = glm::rotate(glm::dmat4(), rotXY, glm::dvec3(0, 0, 1));
        glm::dmat3 matRot = glm::dmat3(glm::rotate(matRotXY, rotXZ, glm::dvec3(0, 1, 0)));

        //*
        // Tetrahedron
        mesh.verts.push_back(glm::dvec3(0, 0, 0));
        mesh.verts.push_back(glm::dvec3(1, 0, 0));
        mesh.verts.push_back(glm::dvec3(0.5, sqrt(3.0)/2, 0));
        mesh.verts.push_back(glm::dvec3(0.5, sqrt(3.0)/6, sqrt(2.0/3)));
        mesh.tets.push_back(MeshTet(eBase + 0, eBase + 1,
                                    eBase + 2, eBase + 3));
        offset = glm::linearRand(glm::dvec3(-1.0), glm::dvec3(1.0));
        for(int i=eBase; i<eBase+4; ++i)
        {
            mesh.verts[i].p += 0.75 * glm::linearRand(-jitter, jitter);
            mesh.verts[i].p = matRot * mesh.verts[i].p;
            mesh.verts[i].p *= scale;
            mesh.verts[i].p += offset;
        }
        eBase += MeshTet::VERTEX_COUNT;
        //*/

        //*
        // Prism
        mesh.verts.push_back(glm::dvec3(0, 0, 0));
        mesh.verts.push_back(glm::dvec3(0, 1, 0));
        mesh.verts.push_back(glm::dvec3(0, 0.5, sqrt(3.0)/2));
        mesh.verts.push_back(glm::dvec3(1, 0, 0));
        mesh.verts.push_back(glm::dvec3(1, 1, 0));
        mesh.verts.push_back(glm::dvec3(1, 0.5, sqrt(3.0)/2));
        mesh.pris.push_back(MeshPri(eBase + 0, eBase + 1, eBase + 2,
                                    eBase + 3, eBase + 4, eBase + 5));
        offset = glm::linearRand(glm::dvec3(-1.0), glm::dvec3(1.0));
        for(int i=eBase; i<eBase+6; ++i)
        {
            mesh.verts[i].p += glm::linearRand(-jitter, jitter);
            mesh.verts[i].p = matRot * mesh.verts[i].p;
            mesh.verts[i].p *= scale;
            mesh.verts[i].p += offset;
        }
        eBase += MeshPri::VERTEX_COUNT;
        //*/

        //*
        // Hexahedron
        mesh.verts.push_back(glm::dvec3(0, 0, 0));
        mesh.verts.push_back(glm::dvec3(1, 0, 0));
        mesh.verts.push_back(glm::dvec3(1, 1, 0));
        mesh.verts.push_back(glm::dvec3(0, 1, 0));
        mesh.verts.push_back(glm::dvec3(0, 0, 1));
        mesh.verts.push_back(glm::dvec3(1, 0, 1));
        mesh.verts.push_back(glm::dvec3(1, 1, 1));
        mesh.verts.push_back(glm::dvec3(0, 1, 1));
        mesh.hexs.push_back(MeshHex(eBase + 0, eBase + 1, eBase + 2, eBase + 3,
                                    eBase + 4, eBase + 5, eBase + 6, eBase + 7));
        offset = glm::linearRand(glm::dvec3(-1.0), glm::dvec3(1.0));
        for(int i=eBase; i<eBase+8; ++i)
        {
            mesh.verts[i].p += glm::linearRand(-jitter, jitter);
            mesh.verts[i].p = matRot * mesh.verts[i].p;
            mesh.verts[i].p *= scale;
            mesh.verts[i].p += offset;
        }
        eBase += MeshHex::VERTEX_COUNT;
        //*/
    }
}

void DebugMesher::genSquish(Mesh& mesh, size_t vertexCount)
{
    double squishRadius = 0.5;
    double squishHeight = 1.0;

    const int pow1_3 =  glm::pow((double)vertexCount, 1.0/3.0);
    const int pow1_3_pair = ((pow1_3 + 1) / 2) * 2;
    const int X_COUNT = pow1_3_pair;
    const int Y_COUNT = pow1_3_pair;
    const int Z_COUNT = pow1_3_pair;
    for(int z=-Z_COUNT/2; z <= Z_COUNT/2; ++z)
    {
        for(int y=-Y_COUNT/2; y <= Y_COUNT/2; ++y)
        {
            for(int x=-X_COUNT/2; x <= X_COUNT/2; ++x)
            {
                glm::dvec2 arm;
                double radius = glm::max(glm::abs(x), glm::abs(y)) * (2.0 * squishRadius / Y_COUNT);
                if(radius != 0.0) arm = glm::normalize(glm::dvec2(x, y)) * radius;

                mesh.verts.push_back(glm::dvec3(
                    arm, z * squishHeight / Z_COUNT));
            }
        }
    }

    const int X_WIDTH = 1;
    const int Y_WIDTH = X_COUNT + 1;
    const int Z_WIDTH = (Y_COUNT+1) * Y_WIDTH;
    for(int z=0; z < Z_COUNT; ++z)
    {
        for(int y=0; y< Y_COUNT; ++y)
        {
            for(int x=0; x< X_COUNT; ++x)
            {
                MeshHex hex(
                    (x+0) * X_WIDTH + (y+0) * Y_WIDTH + (z+0) * Z_WIDTH,
                    (x+1) * X_WIDTH + (y+0) * Y_WIDTH + (z+0) * Z_WIDTH,
                    (x+1) * X_WIDTH + (y+1) * Y_WIDTH + (z+0) * Z_WIDTH,
                    (x+0) * X_WIDTH + (y+1) * Y_WIDTH + (z+0) * Z_WIDTH,
                    (x+0) * X_WIDTH + (y+0) * Y_WIDTH + (z+1) * Z_WIDTH,
                    (x+1) * X_WIDTH + (y+0) * Y_WIDTH + (z+1) * Z_WIDTH,
                    (x+1) * X_WIDTH + (y+1) * Y_WIDTH + (z+1) * Z_WIDTH,
                    (x+0) * X_WIDTH + (y+1) * Y_WIDTH + (z+1) * Z_WIDTH);
                mesh.hexs.push_back(hex);
            }
        }
    }
}

void DebugMesher::genHexGrid(Mesh& mesh, size_t vertexCount)
{
    glm::dvec3 gridMin(-0.5);
    glm::dvec3 gridMax( 0.5);

    const int SECTION_COUNT =  glm::pow((double)vertexCount, 1.0/3.0);
    for(int z=0; z <= SECTION_COUNT; ++z)
    {
        for(int y=0; y <= SECTION_COUNT; ++y)
        {
            for(int x=0; x <= SECTION_COUNT; ++x)
            {
                glm::dvec3 a = glm::dvec3(x, y, z) / glm::dvec3(SECTION_COUNT);
                mesh.verts.push_back(glm::mix(gridMin, gridMax, a));
            }
        }
    }

    const int X_WIDTH = 1;
    const int Y_WIDTH = SECTION_COUNT + 1;
    const int Z_WIDTH = (SECTION_COUNT+1) * Y_WIDTH;
    for(int z=0; z < SECTION_COUNT; ++z)
    {
        for(int y=0; y< SECTION_COUNT; ++y)
        {
            for(int x=0; x< SECTION_COUNT; ++x)
            {
                MeshHex hex(
                    (x+0) * X_WIDTH + (y+0) * Y_WIDTH + (z+0) * Z_WIDTH,
                    (x+1) * X_WIDTH + (y+0) * Y_WIDTH + (z+0) * Z_WIDTH,
                    (x+1) * X_WIDTH + (y+1) * Y_WIDTH + (z+0) * Z_WIDTH,
                    (x+0) * X_WIDTH + (y+1) * Y_WIDTH + (z+0) * Z_WIDTH,
                    (x+0) * X_WIDTH + (y+0) * Y_WIDTH + (z+1) * Z_WIDTH,
                    (x+1) * X_WIDTH + (y+0) * Y_WIDTH + (z+1) * Z_WIDTH,
                    (x+1) * X_WIDTH + (y+1) * Y_WIDTH + (z+1) * Z_WIDTH,
                    (x+0) * X_WIDTH + (y+1) * Y_WIDTH + (z+1) * Z_WIDTH);
                mesh.hexs.push_back(hex);
            }
        }
    }
}

void DebugMesher::genTetGrid(Mesh& mesh, size_t vertexCount)
{
    genHexGrid(mesh, vertexCount);

    tetrahedrize(mesh.tets, mesh);
    mesh.hexs.clear();
}

void DebugMesher::genCube(Mesh& mesh, size_t vertexCount)
{
    glm::dvec3 cubeMin(-1.0);
    glm::dvec3 cubeMax( 1.0);

    mesh.verts.push_back(glm::dvec3(cubeMin.x, cubeMin.y, cubeMin.z));
    mesh.verts.push_back(glm::dvec3(cubeMax.x, cubeMin.y, cubeMin.z));
    mesh.verts.push_back(glm::dvec3(cubeMax.x, cubeMax.y, cubeMin.z));
    mesh.verts.push_back(glm::dvec3(cubeMin.x, cubeMax.y, cubeMin.z));
    mesh.verts.push_back(glm::dvec3(cubeMin.x, cubeMin.y, cubeMax.z));
    mesh.verts.push_back(glm::dvec3(cubeMax.x, cubeMin.y, cubeMax.z));
    mesh.verts.push_back(glm::dvec3(cubeMax.x, cubeMax.y, cubeMax.z));
    mesh.verts.push_back(glm::dvec3(cubeMin.x, cubeMax.y, cubeMax.z));

    mesh.topos.push_back(MeshTopo(_boxBoundary->v0()));
    mesh.topos.push_back(MeshTopo(_boxBoundary->v1()));
    mesh.topos.push_back(MeshTopo(_boxBoundary->v2()));
    mesh.topos.push_back(MeshTopo(_boxBoundary->v3()));
    mesh.topos.push_back(MeshTopo(_boxBoundary->v4()));
    mesh.topos.push_back(MeshTopo(_boxBoundary->v5()));
    mesh.topos.push_back(MeshTopo(_boxBoundary->v6()));
    mesh.topos.push_back(MeshTopo(_boxBoundary->v7()));

    for(uint t=0; t < MeshHex::TET_COUNT; ++t)
        mesh.tets.push_back(MeshHex::tets[t]);

    mesh.setBoundary(_boxBoundary);
}

void DebugMesher::genTet(Mesh& mesh, size_t vertexCount)
{
    mesh.tets.push_back(MeshTet(0, 1, 2, 3));
    mesh.verts.push_back(glm::dvec3(0, 0, 0));
    mesh.verts.push_back(glm::dvec3(1, 0, 0));
    mesh.verts.push_back(glm::dvec3(0.5, sqrt(3.0)/2, 0));
    mesh.verts.push_back(glm::dvec3(0.5, sqrt(3.0)/6, sqrt(2.0/3)));

    mesh.topos.push_back(MeshTopo(_tetBoundary->v0()));
    mesh.topos.push_back(MeshTopo(_tetBoundary->v1()));
    mesh.topos.push_back(MeshTopo(_tetBoundary->v2()));
    mesh.topos.push_back(MeshTopo(_tetBoundary->v3()));

    glm::dvec4 centerw;
    for(const MeshVert& v : mesh.verts)
        centerw += glm::dvec4(v.p, 1.0);
    glm::dvec3 center = glm::dvec3(centerw) / centerw.w;
    for(MeshVert& v : mesh.verts)
        v.p -= center;

    mesh.setBoundary(_tetBoundary);
}
