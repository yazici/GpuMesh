#include "Mesh.h"

#include <map>
#include <iostream>

using namespace std;


void Mesh::initialize(
        const std::vector<glm::dvec3>& vertices,
        const std::vector<Tetrahedron>& tetrahedras)
{
    vert.resize(vertices.size());
    for(int i=0; i<vert.size(); ++i)
    {
        vert[i] = Vertex(vertices[i]);
        cMin = glm::min(cMin, vertices[i]);
        cMax = glm::max(cMax, vertices[i]);
    }

    tetra.clear();
    for(auto& t : tetrahedras)
    {
        std::shared_ptr<Tetrahedron> tet(new Tetrahedron(t));
        computeDataCache(*tet);
        tetra.push_back(tet);
    }
}

void Mesh::compileArrayBuffers(
        std::vector<unsigned int>& indices,
        std::vector<glm::dvec3>& vertices)
{
    int id = -1;
    indices.resize(elemCount());
    for(const auto& t : tetra)
    {
        indices[++id] = t->v[0];
        indices[++id] = t->v[1];
        indices[++id] = t->v[2];

        indices[++id] = t->v[0];
        indices[++id] = t->v[2];
        indices[++id] = t->v[3];

        indices[++id] = t->v[0];
        indices[++id] = t->v[3];
        indices[++id] = t->v[1];

        indices[++id] = t->v[1];
        indices[++id] = t->v[3];
        indices[++id] = t->v[2];
    }

    id = -1;
    vertices.resize(vertCount());
    for(const auto& v : vert)
    {
        vertices[++id] = v.p;
    }

    std::cout << "NV / NT: "
              << tetra.size() << "/" << vert.size() << " = "
              << tetra.size() / (double) vert.size() << endl;
}

void Mesh::insertVertices(const std::vector<glm::dvec3>& vertices)
{
    int idStart = vert.size();
    int idEnd = idStart + vertices.size();
    vert.resize(vert.size() + vertices.size());

    for(int i=idStart; i<idEnd; ++i)
    {
        vert[i] = Vertex(vertices[i-idStart]);
    }

    /* Flat implementation
    for(int i=idStart; i<idEnd; ++i)
    {
        insertVertex(i);
        if(((i-idStart) * 1000) % (idEnd-idStart) == 0)
            std::cout << (i-idStart)*100 / (double) (idEnd-idStart) << "% done" << endl;
    }
    //*/

    //* Regular grid implementation
    glm::ivec3 size(10, 10, 10);
    int cellCount = size.x*size.y*size.z;
    std::cout << "Initializing grid (size="
              << size.x << "x" << size.y << "x" << size.z << ")" << endl;
    std::cout << "Cell density: vert count/cell count = " <<
                 (idEnd-idStart) / (double) cellCount << endl;

    gridSize = size;
    initializeGrid(idStart, idEnd);


    std::cout << "Inserting vertices in the mesh" << endl;
    int cId = 0;
    for(int k=0; k<gridSize.z; ++k)
    {
        for(int j=0; j<gridSize.y; ++j)
        {
            for(int i=0; i<gridSize.x; ++i)
            {
                insertCell(glm::ivec3(i, j, k));

                double progress = (++cId) / (double) (cellCount);
                //std::cout <<  progress * 100 << "% done" << endl;
            }
        }
    }


    std::cout << "Collecting tetrahedrons" << endl;
    collectTetrahedronsGrid();
    //*/
}

void Mesh::insertVertex(int id)
{
    map<Triangle, int> triBuffer;
    const glm::dvec3& v = vert[id].p;

    auto tetraIt = tetra.begin();
    while(tetraIt != tetra.end())
    {
        Tetrahedron& t = *(*tetraIt);
        glm::dvec3 dist = v - t.circumCenter;
        double len2 = glm::dot(dist, dist);

        if(len2 < t.circumRadius2)
        {
            ++triBuffer[Triangle(t.v[0], t.v[1], t.v[2])];
            ++triBuffer[Triangle(t.v[0], t.v[2], t.v[3])];
            ++triBuffer[Triangle(t.v[0], t.v[3], t.v[1])];
            ++triBuffer[Triangle(t.v[1], t.v[3], t.v[2])];

            tetraIt = tetra.erase(tetraIt);
        }
        else
        {
            ++tetraIt;
        }
    }

    for(const auto& triCount : triBuffer)
    {
        if(triCount.second == 1)
        {
            const Triangle& tri = triCount.first;
            std::shared_ptr<Tetrahedron> t(
                new Tetrahedron(id, tri.v[0], tri.v[1], tri.v[2]));
            computeDataCache(*t);
            tetra.push_back(t);
        }
    }
}

void Mesh::computeDataCache(Tetrahedron& tet)
{
    // Compute tetrahedron circumcircle
    glm::dvec3 A = vert[tet.v[0]].p;
    glm::dvec3 B = vert[tet.v[1]].p;
    glm::dvec3 C = vert[tet.v[2]].p;
    glm::dvec3 D = vert[tet.v[3]].p;
    glm::dmat3 S = glm::transpose(
        glm::dmat3(A-B, B-C, C-D));
    glm::dvec3 R(
        (glm::dot(A, A) - glm::dot(B, B)) / 2.0f,
        (glm::dot(B, B) - glm::dot(C, C)) / 2.0f,
        (glm::dot(C, C) - glm::dot(D, D)) / 2.0f);

    double SdetInv = 1.0 / glm::determinant(S);
    double Sx = glm::determinant(glm::dmat3(R, S[1], S[2]));
    double Sy = glm::determinant(glm::dmat3(S[0], R, S[2]));
    double Sz = glm::determinant(glm::dmat3(S[0], S[1], R));

    tet.circumCenter = glm::dvec3(Sx, Sy, Sz) * SdetInv;
    glm::dvec3 dist = A - tet.circumCenter;
    tet.circumRadius2 = glm::dot(dist, dist);


    // Check tetrahedron volume positivity
    glm::dvec3 d10(A - B);
    glm::dvec3 d12(C - B);
    glm::dvec3 d23(D - C);
    glm::dmat3 triple(d10, d12, d23);
    if(glm::determinant(triple) < 0)
    {
        std::swap(tet.v[2], tet.v[3]);
    }

    // Reinitialise algo flag
    tet.flag = false;
}

void Mesh::initializeGrid(int idStart, int idEnd)
{
    // Construct grid
    grid.resize(gridSize.z);
    for(int k=0; k<gridSize.z; ++k)
    {
        grid[k].resize(gridSize.x);
        for(int j=0; j<gridSize.y; ++j)
        {
            grid[k][j].resize(gridSize.x);
        }
    }

    // Bin the vertices
    glm::dvec3 boxSize = (cMax - cMin);
    glm::dvec3 floatSize = glm::dvec3(gridSize);
    for(int vId=idStart; vId<idEnd; ++vId)
    {
        const glm::dvec3& v = vert[vId].p;
        glm::ivec3 bin = glm::ivec3((v - cMin) / (boxSize) * floatSize);
        grid[bin.z][bin.y][bin.x].vertId.push_back(vId);
    }

    // Put starting tetrahedrons in the first cell
    const glm::ivec3 FIRST_CELL(0, 0, 0);
    for(auto tet : tetra)
    {
        insertTetrahedronGrid(FIRST_CELL, tet);
    }

    //* Radially sort cell vertices
    for(int k=0; k<gridSize.z; ++k)
    {
        for(int j=0; j<gridSize.y; ++j)
        {
            for(int i=0; i<gridSize.x; ++i)
            {
                glm::dvec3 floatBin = glm::dvec3(i+1, j+1, k+1) / floatSize;
                glm::dvec3 cellCorner = floatBin * boxSize + cMin;
                std::sort(grid[k][j][i].vertId.begin(),
                          grid[k][j][i].vertId.end(),
                          [this, &cellCorner](int a, int b) {
                    glm::dvec3 distA = cellCorner - vert[a].p;
                    glm::dvec3 distB = cellCorner - vert[b].p;
                    return glm::dot(distA, distA) < glm::dot(distB, distB);
                });
            }
        }
    }
    //*/
}


void Mesh::insertCell(const glm::ivec3& cId)
{
    GridCell cell = grid[cId.z][cId.y][cId.x];
    int vertCount = cell.vertId.size();
    for(int vId=0; vId<vertCount; ++vId)
    {
        insertVertexGrid(cId, cell.vertId[vId]);
    }
}

void Mesh::insertVertexGrid(const glm::ivec3& cId, int vId)
{
    std::shared_ptr<Tetrahedron> base = findBaseTetrahedron(cId, vId);

    std::set<Triangle> triBuffer;
    findDelaunayBall(vId, base, triBuffer);


    // Build new tetrehedrons
    for(auto t : triBuffer)
    {
        std::shared_ptr<Tetrahedron> tet(
            new Tetrahedron(vId, t.v[0], t.v[1], t.v[2]));
        insertTetrahedronGrid(cId, tet);
        computeDataCache(*tet);
    }
}

std::shared_ptr<Tetrahedron> Mesh::findBaseTetrahedron(const glm::ivec3& cId, int vId)
{
    const glm::ivec3 DOWN(0, 0, -1);
    const glm::ivec3 BACK(-1, 0, 0);
    const glm::ivec3 BACK_RIGHT(-1, -1, 0);
    const glm::ivec3 RIGHT(0, -1, 0);
    const glm::ivec3 FRONT_RIGHT(1, -1, 0);

    const glm::ivec3 BACK_DOWN(-1, 0, -1);
    const glm::ivec3 BACK_RIGHT_DOWN(-1, -1, -1);
    const glm::ivec3 RIGHT_DOWN(0, -1, -1);
    const glm::ivec3 FRONT_RIGHT_DOWN(1, -1, -1);
    const glm::ivec3 FRONT_DOWN(1, 0, -1);
    const glm::ivec3 FRONT_LEFT_DOWN(1, 1, -1);
    const glm::ivec3 LEFT_DOWN(0, 1, -1);
    const glm::ivec3 BACK_LEFT_DOWN(-1, 1, -1);

    std::list<std::pair<glm::ivec3, glm::ivec3>> cells;
    cells.push_back(make_pair(cId, glm::ivec3()));
    cells.push_back(make_pair(cId + DOWN, DOWN));
    cells.push_back(make_pair(cId + BACK, BACK));
    cells.push_back(make_pair(cId + BACK_RIGHT, BACK_RIGHT));
    cells.push_back(make_pair(cId + RIGHT, RIGHT));
    cells.push_back(make_pair(cId + FRONT_RIGHT, FRONT_RIGHT));
    cells.push_back(make_pair(cId + BACK_DOWN, BACK_DOWN));
    cells.push_back(make_pair(cId + BACK_RIGHT_DOWN, BACK_RIGHT_DOWN));
    cells.push_back(make_pair(cId + RIGHT_DOWN, RIGHT_DOWN));
    cells.push_back(make_pair(cId + FRONT_RIGHT_DOWN, FRONT_RIGHT_DOWN));
    cells.push_back(make_pair(cId + FRONT_DOWN, FRONT_DOWN));
    cells.push_back(make_pair(cId + FRONT_LEFT_DOWN, FRONT_LEFT_DOWN));
    cells.push_back(make_pair(cId + LEFT_DOWN, LEFT_DOWN));
    cells.push_back(make_pair(cId + BACK_LEFT_DOWN, BACK_LEFT_DOWN));


    while(!cells.empty())
    {
        glm::ivec3 c = cells.front().first;

        if(glm::all(glm::greaterThanEqual(c, glm::ivec3())) &&
           glm::all(glm::lessThan(c, gridSize)))
        {

            GridCell& cell = grid[c.z][c.y][c.x];
            for(auto tet : cell.tetra)
            {
                if(isBase(vId, tet))
                {
                    return tet;
                }
            }

            // Push neighbors
            glm::ivec3 d = cells.front().second;
            if(d.x != 0)
            {
                glm::ivec3 x(d.x, 0, 0);
                cells.push_back(make_pair(c+x, x));

                if(d.y != 0)
                {
                    glm::ivec3 y(0, d.y, 0);
                    cells.push_back(make_pair(c+y, y));

                    glm::ivec3 xy(d.x, d.y, 0);
                    cells.push_back(make_pair(c+xy, xy));

                    if(d.z != 0)
                    {
                        glm::ivec3 z(0, 0, d.z);
                        cells.push_back(make_pair(c+z, z));

                        glm::ivec3 xz(d.x, 0, d.z);
                        cells.push_back(make_pair(c+xz, xz));

                        glm::ivec3 yz(0, d.y, d.z);
                        cells.push_back(make_pair(c+yz, yz));

                        cells.push_back(make_pair(c+d, d));
                    }
                }
                else
                {
                    if(d.z != 0)
                    {
                        glm::ivec3 z(0, 0, d.z);
                        cells.push_back(make_pair(c+z, z));

                        cells.push_back(make_pair(c+d, d));
                    }
                }
            }
            else if(d.y != 0)
            {
                glm::ivec3 y(0, d.y, 0);
                cells.push_back(make_pair(c+y, y));

                if(d.z != 0)
                {
                    glm::ivec3 z(0, 0, d.z);
                    cells.push_back(make_pair(c+z, z));

                    cells.push_back(make_pair(c+d, d));
                }
            }
            else if(d.z != 0)
            {
                // Only z is not null
                cells.push_back(make_pair(c+d, d));
            }
        }

        cells.pop_front();
    }
}


void Mesh::findDelaunayBall(int vId, const std::shared_ptr<Tetrahedron>& base,
                            std::set<Triangle>& ball)
{
    glm::dvec3 v = vert[vId].p;

    std::list<pair<std::shared_ptr<Tetrahedron>, bool>> queue;
    std::vector<std::shared_ptr<Tetrahedron>> preserved;

    removeTetrahedronGrid(base->cId, base);
    for(int i=0; i<4; ++i)
    {
        for(auto neighbor : vert[base->v[i]].tetra)
        {
            if(!neighbor->flag)
            {
                neighbor->flag = true;
                queue.push_back(make_pair(neighbor, true));
            }
        }
    }

    int delBaseCount = 0;
    int delAuxCount = 0;

    while(!queue.empty())
    {
        std::shared_ptr<Tetrahedron> tet = queue.front().first;

        glm::dvec3 dist = v - tet->circumCenter;
        if(glm::dot(dist, dist) < tet->circumRadius2)
        {
            Triangle tri[] {tet->t0(), tet->t1(), tet->t2(), tet->t3()};

            for(int i=0; i<4; ++i)
            {
                auto it = ball.insert(tri[i]);
                if(!it.second)
                {
                    ball.erase(it.first);
                }

                for(auto neighbor : vert[tet->v[i]].tetra)
                {
                    if(!neighbor->flag)
                    {
                        neighbor->flag = true;
                        queue.push_back(make_pair(neighbor, false));
                    }
                }
            }

            removeTetrahedronGrid(tet->cId, tet);
            if(queue.front().second)
                ++delBaseCount;
            else
                ++delAuxCount;
        }
        else
        {
            preserved.push_back(tet);
        }

        queue.pop_front();
    }


    // Reset algo flag on preserved tetrahedrons
    for(auto tet : preserved)
    {
        tet->flag = false;
    }

    cout << "(" << delBaseCount << ", " << delAuxCount << ", " << preserved.size() << ") ";
}

bool Mesh::isBase(int vId, const std::shared_ptr<Tetrahedron>& tet)
{
    glm::dmat4 D(
        glm::dvec4(vert[tet->v[0]].p, 1),
        glm::dvec4(vert[tet->v[1]].p, 1),
        glm::dvec4(vert[tet->v[2]].p, 1),
        glm::dvec4(vert[tet->v[3]].p, 1));

    glm::dvec4 v4(vert[vId].p, 1);
    glm::dmat4 Dv(v4, D[1], D[2], D[3]);

    if(glm::determinant(Dv) < 0)
        return false;
    Dv[0] = D[0];

    Dv[1] = v4;
    if(glm::determinant(Dv) < 0)
        return false;
    Dv[1] = D[1];

    Dv[2] = v4;
    if(glm::determinant(Dv) < 0)
        return false;
    Dv[2] = D[2];

    Dv[3] = v4;
    if(glm::determinant(Dv) < 0)
        return false;

    return true;
}

void Mesh::insertTetrahedronGrid(
        const glm::ivec3& cId,
        const std::shared_ptr<Tetrahedron>& tet)
{
    tet->cId = cId;
    grid[cId.z][cId.y][cId.x].tetra.insert(tet);
    vert[tet->v[0]].tetra.insert(tet);
    vert[tet->v[1]].tetra.insert(tet);
    vert[tet->v[2]].tetra.insert(tet);
    vert[tet->v[3]].tetra.insert(tet);
}

void Mesh::removeTetrahedronGrid(
        const glm::ivec3& cId,
        const std::shared_ptr<Tetrahedron>& tet)
{
    grid[cId.z][cId.y][cId.x].tetra.erase(tet);
}

void Mesh::collectTetrahedronsGrid()
{
    tetra.clear();
    for(int k=0; k<gridSize.z; ++k)
    {
        for(int j=0; j<gridSize.y; ++j)
        {
            for(int i=0; i<gridSize.x; ++i)
            {
                for(auto tet : grid[k][j][i].tetra)
                {
                    vert[tet->v[0]].tetra.erase(tet);
                    vert[tet->v[1]].tetra.erase(tet);
                    vert[tet->v[2]].tetra.erase(tet);
                    vert[tet->v[3]].tetra.erase(tet);
                    tetra.push_back(tet);
                }
                grid[k][j][i].tetra.clear();
            }
        }
    }
}
