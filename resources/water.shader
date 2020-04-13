[VERTEX]
attribute vec3 a_vertex;
attribute vec3 a_normal;
attribute vec2 a_uv;

uniform mat4 projection_matrix;
uniform mat4 camera_matrix;
uniform mat4 object_matrix;
uniform vec3 object_scale;

varying vec2 v_uv;

void main()
{
    //gl_Position = projection_matrix * camera_matrix * object_matrix * vec4(a_vertex.xyz * object_scale, 1.0);
    gl_Position = vec4(a_vertex, 1.0);
    gl_Position.x *= projection_matrix[0][0] / projection_matrix[1][1] * 2.0;
    v_uv = a_uv.xy;
}

[FRAGMENT]
uniform sampler2D texture_map;

varying vec2 v_uv;

void main()
{
    float a = texture2D(texture_map, v_uv).a;

    gl_FragColor.rgb = texture2D(texture_map, v_uv).rgb;
    if (a > 0.7)
        gl_FragColor.a = 0.6;
    else if (a > 0.3)
        gl_FragColor.a = mix(0.0, 0.8, (a - 0.3) / 0.2);
    else
        gl_FragColor.a = 0.0;
    if (gl_FragColor.a == 0.0)
        discard;
}
