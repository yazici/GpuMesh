#version 440

uniform vec3 CameraPosition;
uniform vec3 LightDirection;
uniform float PointRadius;
uniform int LightMode;

layout(location = 0) out vec4 FragColor;

in vec3 pos;
noperspective in vec2 pix;
in float qual;
in float size;
in float dept;
in float dist;


const float MAT_SHINE = 40.0;
const vec3 LIGHT_AMBIANT = vec3(0.3);
const vec3 LIGHT_DIFFUSE = vec3(0.5);
const vec3 LIGHT_SPECULAR = vec3(1.0, 0.9, 0.7);


vec3 qualityLut(in float q);

float sphericalDiffuse(in vec3 n);
float lambertDiffuse(in vec3 n);
float phongSpecular(in vec3 n, in vec3 p, in float shine);


void main()
{
    // Cut plane distance
    if(dist > 0.0)
        discard;

    vec2 rad = (gl_FragCoord.xy - pix) / size;
    float radDist2 = min(dot(rad, rad), 1.0);
    vec3 sphere = vec3(rad, sqrt(1.0 - radDist2));

    vec3 up = vec3(0, 0, 1);
    vec3 cam = normalize(CameraPosition - pos);
    vec3 right = normalize(cross(up, cam));
    up = normalize(cross(cam, right));

    vec3 normal = normalize(
            sphere.x * right +
            sphere.y * up +
            sphere.z * cam);

    vec3 color;
    if(LightMode == 0)
    {
        vec3 diff = LIGHT_DIFFUSE * sphericalDiffuse(normal);
        color = qualityLut(qual) * (LIGHT_AMBIANT + diff * 1.5);
    }
    else
    {
        vec3 diff = LIGHT_DIFFUSE * lambertDiffuse(normal);
        vec3 spec = LIGHT_SPECULAR * phongSpecular(normal, pos, MAT_SHINE);
        color = qualityLut(qual) * (LIGHT_AMBIANT + diff) + spec;
    }

    gl_FragDepth = gl_FragCoord.z + dept * sphere.z;

    FragColor = vec4(color, 1.0);
}
