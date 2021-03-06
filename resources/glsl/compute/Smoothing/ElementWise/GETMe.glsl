uniform float Lambda;


// Quality Interface
float tetQuality(in vec3 vp[PARAM_VERTEX_COUNT], inout Tet tet);
float priQuality(in vec3 vp[PARAM_VERTEX_COUNT], inout Pri pri);
float hexQuality(in vec3 vp[PARAM_VERTEX_COUNT], inout Hex hex);
float tetVolume(in vec3 vp[PARAM_VERTEX_COUNT], inout Tet tet);
float priVolume(in vec3 vp[PARAM_VERTEX_COUNT], inout Pri pri);
float hexVolume(in vec3 vp[PARAM_VERTEX_COUNT], inout Hex hex);

// Vertex Accum
void addPosition(in uint vId, in vec3 pos, in float weight);


// ENTRY POINTS //
void smoothTet(uint eId)
{
    Tet tet = tets[eId];

    uint vi[] = uint[](
        tet.v[0],
        tet.v[1],
        tet.v[2],
        tet.v[3]
    );

    vec3 vp[] = vec3[](
        verts[vi[0]].p,
        verts[vi[1]].p,
        verts[vi[2]].p,
        verts[vi[3]].p
    );

    vec3 n[] = vec3[](
        cross(vp[3]-vp[1], vp[2]-vp[1]),
        cross(vp[3]-vp[2], vp[0]-vp[2]),
        cross(vp[1]-vp[3], vp[0]-vp[3]),
        cross(vp[1]-vp[0], vp[2]-vp[0])
    );

    vec3 vpp[] = vec3[](
        vp[0] + n[0] * (Lambda / sqrt(length(n[0]))),
        vp[1] + n[1] * (Lambda / sqrt(length(n[1]))),
        vp[2] + n[2] * (Lambda / sqrt(length(n[2]))),
        vp[3] + n[3] * (Lambda / sqrt(length(n[3])))
    );


    float volume = tetVolume(vp, tet);
    float volumePrime = tetVolume(vpp, tet);
    float absVolumeRatio = abs(volume / volumePrime);
    float volumeVar = pow(absVolumeRatio, 1.0/3.0);

    vec3 center = (1.0/4.0) * (
        vp[0] + vp[1] + vp[2] + vp[3]);

    vpp[0] = center + volumeVar * (vpp[0] - center);
    vpp[1] = center + volumeVar * (vpp[1] - center);
    vpp[2] = center + volumeVar * (vpp[2] - center);
    vpp[3] = center + volumeVar * (vpp[3] - center);

    float quality = tetQuality(vp, tet);
    float qualityPrime = tetQuality(vpp, tet);

    float weight = qualityPrime / (1.0 + quality);
    addPosition(vi[0], vpp[0], weight);
    addPosition(vi[1], vpp[1], weight);
    addPosition(vi[2], vpp[2], weight);
    addPosition(vi[3], vpp[3], weight);
}

void smoothPri(uint eId)
{
    Pri pri = pris[eId];

    uint vi[] = uint[](
        pri.v[0],
        pri.v[1],
        pri.v[2],
        pri.v[3],
        pri.v[4],
        pri.v[5]
    );

    vec3 vp[] = vec3[](
        verts[vi[0]].p,
        verts[vi[1]].p,
        verts[vi[2]].p,
        verts[vi[3]].p,
        verts[vi[4]].p,
        verts[vi[5]].p
    );

    vec3 aux[] = vec3[](
        (vp[0] + vp[1] + vp[2]) / 3.0,
        (vp[0] + vp[1] + vp[4] + vp[3]) / 4.0,
        (vp[1] + vp[2] + vp[5] + vp[4]) / 4.0,
        (vp[2] + vp[0] + vp[3] + vp[5]) / 4.0,
        (vp[3] + vp[4] + vp[5]) / 3.0
    );

    vec3 n[] = vec3[](
        cross(aux[1] - aux[0], aux[3] - aux[0]),
        cross(aux[2] - aux[0], aux[1] - aux[0]),
        cross(aux[3] - aux[0], aux[2] - aux[0]),
        cross(aux[3] - aux[4], aux[1] - aux[4]),
        cross(aux[1] - aux[4], aux[2] - aux[4]),
        cross(aux[2] - aux[4], aux[3] - aux[4])
    );

    float t = (4.0/5.0) * (1.0 - pow(4.0/39.0, 0.25) * Lambda);
    float it = 1.0 - t;
    vec3 bases[] = vec3[](
        it * aux[0] + t * (aux[3] + aux[1]) / 2.0,
        it * aux[0] + t * (aux[1] + aux[2]) / 2.0,
        it * aux[0] + t * (aux[2] + aux[3]) / 2.0,
        it * aux[4] + t * (aux[3] + aux[1]) / 2.0,
        it * aux[4] + t * (aux[1] + aux[2]) / 2.0,
        it * aux[4] + t * (aux[2] + aux[3]) / 2.0
    );


    // New positions
    vec3 vpp[] = vec3[](
        bases[0] + n[0] * (Lambda / sqrt(length(n[0]))),
        bases[1] + n[1] * (Lambda / sqrt(length(n[1]))),
        bases[2] + n[2] * (Lambda / sqrt(length(n[2]))),
        bases[3] + n[3] * (Lambda / sqrt(length(n[3]))),
        bases[4] + n[4] * (Lambda / sqrt(length(n[4]))),
        bases[5] + n[5] * (Lambda / sqrt(length(n[5])))
    );


    float volume = priVolume(vp, pri);
    float volumePrime = priVolume(vpp, pri);
    float absVolumeRatio = abs(volume / volumePrime);
    float volumeVar = pow(absVolumeRatio, 1.0/3.0);

    vec3 center = (1.0/6.0) * (
        vp[0] + vp[1] + vp[2] + vp[3] + vp[4] + vp[5]);

    vpp[0] = center + volumeVar * (vpp[0] - center);
    vpp[1] = center + volumeVar * (vpp[1] - center);
    vpp[2] = center + volumeVar * (vpp[2] - center);
    vpp[3] = center + volumeVar * (vpp[3] - center);
    vpp[4] = center + volumeVar * (vpp[4] - center);
    vpp[5] = center + volumeVar * (vpp[5] - center);


    float quality = priQuality(vp, pri);
    float qualityPrime = priQuality(vpp, pri);

    float weight = qualityPrime / (1.0 + quality);
    addPosition(vi[0], vpp[0], weight);
    addPosition(vi[1], vpp[1], weight);
    addPosition(vi[2], vpp[2], weight);
    addPosition(vi[3], vpp[3], weight);
    addPosition(vi[4], vpp[4], weight);
    addPosition(vi[5], vpp[5], weight);
}

void smoothHex(uint eId)
{
    Hex hex = hexs[eId];

    uint vi[] = uint[](
        hex.v[0],
        hex.v[1],
        hex.v[2],
        hex.v[3],
        hex.v[4],
        hex.v[5],
        hex.v[6],
        hex.v[7]
    );

    vec3 vp[] = vec3[](
        verts[vi[0]].p,
        verts[vi[1]].p,
        verts[vi[2]].p,
        verts[vi[3]].p,
        verts[vi[4]].p,
        verts[vi[5]].p,
        verts[vi[6]].p,
        verts[vi[7]].p
    );

    vec3 aux[] = vec3[](
        (vp[0] + vp[1] + vp[2] + vp[3]) / 4.0,
        (vp[0] + vp[4] + vp[5] + vp[1]) / 4.0,
        (vp[1] + vp[5] + vp[6] + vp[2]) / 4.0,
        (vp[2] + vp[6] + vp[7] + vp[3]) / 4.0,
        (vp[0] + vp[3] + vp[7] + vp[4]) / 4.0,
        (vp[4] + vp[7] + vp[6] + vp[5]) / 4.0
    );

    vec3 n[] = vec3[](
        cross(aux[1] - aux[0], aux[4] - aux[0]),
        cross(aux[2] - aux[0], aux[1] - aux[0]),
        cross(aux[3] - aux[0], aux[2] - aux[0]),
        cross(aux[4] - aux[0], aux[3] - aux[0]),
        cross(aux[4] - aux[5], aux[1] - aux[5]),
        cross(aux[1] - aux[5], aux[2] - aux[5]),
        cross(aux[2] - aux[5], aux[3] - aux[5]),
        cross(aux[3] - aux[5], aux[4] - aux[5])
    );

    vec3 bases[] = vec3[](
        (aux[0] + aux[1] + aux[4]) / 3.0,
        (aux[0] + aux[2] + aux[1]) / 3.0,
        (aux[0] + aux[3] + aux[2]) / 3.0,
        (aux[0] + aux[4] + aux[3]) / 3.0,
        (aux[5] + aux[4] + aux[1]) / 3.0,
        (aux[5] + aux[1] + aux[2]) / 3.0,
        (aux[5] + aux[2] + aux[3]) / 3.0,
        (aux[5] + aux[3] + aux[4]) / 3.0
    );


    // New positions
    vec3 vpp[] = vec3[](
        bases[0] + n[0] * (Lambda / sqrt(length(n[0]))),
        bases[1] + n[1] * (Lambda / sqrt(length(n[1]))),
        bases[2] + n[2] * (Lambda / sqrt(length(n[2]))),
        bases[3] + n[3] * (Lambda / sqrt(length(n[3]))),
        bases[4] + n[4] * (Lambda / sqrt(length(n[4]))),
        bases[5] + n[5] * (Lambda / sqrt(length(n[5]))),
        bases[6] + n[6] * (Lambda / sqrt(length(n[6]))),
        bases[7] + n[7] * (Lambda / sqrt(length(n[7])))
    );


    float volume = hexVolume(vp, hex);
    float volumePrime = hexVolume(vpp, hex);
    float absVolumeRatio = abs(volume / volumePrime);
    float volumeVar = pow(absVolumeRatio, 1.0/3.0);

    vec3 center = (1.0/8.0) * (
        vp[0] + vp[1] + vp[2] + vp[3] + vp[4] + vp[5] + vp[6] + vp[7]);

    vpp[0] = center + volumeVar * (vpp[0] - center);
    vpp[1] = center + volumeVar * (vpp[1] - center);
    vpp[2] = center + volumeVar * (vpp[2] - center);
    vpp[3] = center + volumeVar * (vpp[3] - center);
    vpp[4] = center + volumeVar * (vpp[4] - center);
    vpp[5] = center + volumeVar * (vpp[5] - center);
    vpp[6] = center + volumeVar * (vpp[6] - center);
    vpp[7] = center + volumeVar * (vpp[7] - center);


    float quality = hexQuality(vp, hex);
    float qualityPrime = hexQuality(vpp, hex);

    float weight = qualityPrime / (1.0 + quality);
    addPosition(vi[0], vpp[0], weight);
    addPosition(vi[1], vpp[1], weight);
    addPosition(vi[2], vpp[2], weight);
    addPosition(vi[3], vpp[3], weight);
    addPosition(vi[4], vpp[4], weight);
    addPosition(vi[5], vpp[5], weight);
    addPosition(vi[6], vpp[6], weight);
    addPosition(vi[7], vpp[7], weight);
}
