vec3 riemannianSegment(in vec3 a, in vec3 b);


const mat3 Fr_TET_INV = mat3(
    vec3(1, 0, 0),
    vec3(-0.5773502691896257645091, 1.154700538379251529018, 0),
    vec3(-0.4082482904638630163662, -0.4082482904638630163662, 1.224744871391589049099)
);

const mat3 Fr_PRI_INV = mat3(
    vec3(1.0, 0.0, 0.0),
    vec3(-0.5773502691896257645091, 1.154700538379251529018, 0.0),
    vec3(0.0, 0.0, 1.0)
);

float cornerQuality(in mat3 Fk)
{
    float Fk_det = determinant(Fk);

    float Fk_frobenius2 =
        dot(Fk[0], Fk[0]) +
        dot(Fk[1], Fk[1]) +
        dot(Fk[2], Fk[2]);

    return sign(Fk_det) * 3.0 * pow(abs(Fk_det), 2.0/3.0) / Fk_frobenius2;
}

float tetQuality(in vec3 vp[4])
{
    vec3 e03 = riemannianSegment(vp[0], vp[3]);
    vec3 e13 = riemannianSegment(vp[1], vp[3]);
    vec3 e23 = riemannianSegment(vp[2], vp[3]);

    mat3 Fk0 = mat3(e03, e13, e23) * Fr_TET_INV;

    float qual0 = cornerQuality(Fk0);

    // Shape measure is independent of chosen corner
    return qual0;
}

float priQuality(in vec3 vp[6])
{
    vec3 e03 = riemannianSegment(vp[0], vp[3]);
    vec3 e14 = riemannianSegment(vp[1], vp[4]);
    vec3 e25 = riemannianSegment(vp[2], vp[5]);
    vec3 e01 = riemannianSegment(vp[0], vp[1]);
    vec3 e12 = riemannianSegment(vp[1], vp[2]);
    vec3 e20 = riemannianSegment(vp[2], vp[0]);
    vec3 e34 = riemannianSegment(vp[3], vp[4]);
    vec3 e45 = riemannianSegment(vp[4], vp[5]);
    vec3 e53 = riemannianSegment(vp[5], vp[3]);

    // Prism corner quality is not invariant under edge swap
    // Third edge is the expected to be colinear with the first two cross product
    mat3 Fk0 = mat3(-e01, e20, e03) * Fr_PRI_INV;
    mat3 Fk1 = mat3(-e12, e01, e14) * Fr_PRI_INV;
    mat3 Fk2 = mat3(-e20, e12, e25) * Fr_PRI_INV;
    mat3 Fk3 = mat3(-e34, e53, e03) * Fr_PRI_INV;
    mat3 Fk4 = mat3(-e45, e34, e14) * Fr_PRI_INV;
    mat3 Fk5 = mat3(-e53, e45, e25) * Fr_PRI_INV;

    float qual0 = cornerQuality(Fk0);
    float qual1 = cornerQuality(Fk1);
    float qual2 = cornerQuality(Fk2);
    float qual3 = cornerQuality(Fk3);
    float qual4 = cornerQuality(Fk4);
    float qual5 = cornerQuality(Fk5);

    return (qual0 + qual1 + qual2 + qual3 + qual4 + qual5) / 6.0;
}

float hexQuality(in vec3 vp[8])
{
    // Since hex's corner matrix is the identity matrix,
    // there's no need to define Fr_HEX_INV.
    vec3 e01 = riemannianSegment(vp[0], vp[1]);
    vec3 e03 = riemannianSegment(vp[0], vp[3]);
    vec3 e04 = riemannianSegment(vp[0], vp[4]);
    vec3 e12 = riemannianSegment(vp[1], vp[2]);
    vec3 e15 = riemannianSegment(vp[1], vp[5]);
    vec3 e23 = riemannianSegment(vp[2], vp[3]);
    vec3 e26 = riemannianSegment(vp[2], vp[6]);
    vec3 e37 = riemannianSegment(vp[3], vp[7]);
    vec3 e45 = riemannianSegment(vp[4], vp[5]);
    vec3 e47 = riemannianSegment(vp[4], vp[7]);
    vec3 e56 = riemannianSegment(vp[5], vp[6]);
    vec3 e67 = riemannianSegment(vp[6], vp[7]);

    mat3 Fk0 = mat3(e01,  e04, -e03);
    mat3 Fk1 = mat3(e01,  e12,  e15);
    mat3 Fk2 = mat3(e12,  e26, -e23);
    mat3 Fk3 = mat3(e03,  e23,  e37);
    mat3 Fk4 = mat3(e04,  e45,  e47);
    mat3 Fk5 = mat3(e15, -e56,  e45);
    mat3 Fk6 = mat3(e26,  e56,  e67);
    mat3 Fk7 = mat3(e37,  e67, -e47);

    float qual0 = cornerQuality(Fk0);
    float qual1 = cornerQuality(Fk1);
    float qual2 = cornerQuality(Fk2);
    float qual3 = cornerQuality(Fk3);
    float qual4 = cornerQuality(Fk4);
    float qual5 = cornerQuality(Fk5);
    float qual6 = cornerQuality(Fk6);
    float qual7 = cornerQuality(Fk7);

    return (qual0 + qual1 + qual2 + qual3 + qual4 + qual5 + qual6 + qual7) / 8.0;
}
