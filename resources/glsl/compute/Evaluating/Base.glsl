///////////////////////////////
//   Function declarations   //
///////////////////////////////

// Externally defined
float tetQuality(in vec3 vp[PARAM_VERTEX_COUNT], inout Tet tet);
float priQuality(in vec3 vp[PARAM_VERTEX_COUNT], inout Pri pri);
float hexQuality(in vec3 vp[PARAM_VERTEX_COUNT], inout Hex hex);

// Internally defined
float tetQuality(inout Tet tet);
float priQuality(inout Pri pri);
float hexQuality(inout Hex hex);

float patchQuality(in uint vId);


//////////////////////////////
//   Function definitions   //
//////////////////////////////

// Element Quality
float tetQuality(inout Tet tet)
{
    const vec3 vp[] = vec3[](
        verts[tet.v[0]].p,
        verts[tet.v[1]].p,
        verts[tet.v[2]].p,
        verts[tet.v[3]].p,
        vec3(0),
        vec3(0),
        vec3(0),
        vec3(0)
    );

    return tetQuality(vp, tet);
}

float priQuality(inout Pri pri)
{
    const vec3 vp[] = vec3[](
        verts[pri.v[0]].p,
        verts[pri.v[1]].p,
        verts[pri.v[2]].p,
        verts[pri.v[3]].p,
        verts[pri.v[4]].p,
        verts[pri.v[5]].p,
        vec3(0),
        vec3(0)
    );

    return priQuality(vp, pri);
}

float hexQuality(inout Hex hex)
{
    const vec3 vp[] = vec3[](
        verts[hex.v[0]].p,
        verts[hex.v[1]].p,
        verts[hex.v[2]].p,
        verts[hex.v[3]].p,
        verts[hex.v[4]].p,
        verts[hex.v[5]].p,
        verts[hex.v[6]].p,
        verts[hex.v[7]].p
    );

    return hexQuality(vp, hex);
}

void accumulatePatchQuality(
        inout double patchQuality,
        inout double patchWeight,
        in double elemQuality)
{
    if(patchWeight != -1.0 &&  elemQuality > 0.0)
    {
        patchWeight += 1.0;
        patchQuality += 1/elemQuality;
    }
    else
    {
        patchWeight = -1.0;
        patchQuality = min(patchQuality, elemQuality);
    }
}

float finalizePatchQuality(in double patchQuality, in double patchWeight)
{
    if(patchWeight > 0.0)
        return float(patchWeight/patchQuality);
    else
        return float(patchQuality);
}


subroutine float patchQualitySub(in uint vId);
layout(location=PATCH_QUALITY_SUBROUTINE_LOC)
subroutine uniform patchQualitySub patchQualityUni;

float patchQuality(in uint vId)
{
    return patchQualityUni(vId);
}


layout(index=PATCH_QUALITY_SUBROUTINE_IDX) subroutine(patchQualitySub)
float patchQualityImpl(in uint vId)
{
    Topo topo = topos[vId];

    double patchWeight = 0.0;
    double patchQuality = 0.0;
    uint neigElemCount = topo.neigElemCount;
    for(uint i=0, n = topo.neigElemBase; i < neigElemCount; ++i, ++n)
    {
        NeigElem neigElem = neigElems[n];

        switch(neigElem.type)
        {
        case TET_ELEMENT_TYPE:
            accumulatePatchQuality(
                patchQuality, patchWeight,
                double(tetQuality(tets[neigElem.id])));
            break;

        case PRI_ELEMENT_TYPE:
            accumulatePatchQuality(
                patchQuality, patchWeight,
                double(priQuality(pris[neigElem.id])));
            break;

        case HEX_ELEMENT_TYPE:
            accumulatePatchQuality(
                patchQuality, patchWeight,
                double(hexQuality(hexs[neigElem.id])));
            break;
        }
    }

    return finalizePatchQuality(patchQuality, patchWeight);
}
