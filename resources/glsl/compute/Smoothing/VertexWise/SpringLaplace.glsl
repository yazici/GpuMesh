uniform float MoveCoeff;


// Boundaries
vec3 snapToBoundary(int boundaryID, vec3 pos);

// Smoothing Helper
vec3 computeVertexEquilibrium(in uint vId);


// ENTRY POINT //
void smoothVert(uint vId)
{
    vec3 patchCenter = computeVertexEquilibrium(vId);

    vec3 pos = verts[vId].p;
    pos = mix(pos, patchCenter, MoveCoeff);


    Topo topo = topos[vId];
    if(topo.type > 0)
    {
        pos = snapToBoundary(topo.type, pos);
    }


    // Write
    verts[vId].p = pos;
}
