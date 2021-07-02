#version 330 core

in vec2 texCoord;
in vec3 fragPos;
in vec3 fragColor;
in vec3 Normal;

uniform sampler2D texture1;
uniform sampler2D texture2;

uniform vec3 xyz;      // light position
uniform vec3 xyzColor; // light color
uniform vec3 xyzView;  // camera position

out vec4 color;

void main() {
// ambient
	// ambient
	float strength = 0.5;
	vec3 ambient   = strength * xyzColor;

	// diffuse
	vec3  norm     = normalize(Normal);
	vec3  lightDir = normalize(xyz - fragPos);
	float diff     = max(dot(norm, lightDir), 0.0);
	vec3  diffuse  = diff * xyzColor;

	// specular
	float specularStrength = 0.5;
	vec3  viewDir          = normalize(xyzView - fragPos);
	vec3  reflectDir       = reflect(-lightDir, norm);
	float spec             = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	vec3  specular         = specularStrength * spec * xyzColor;

	vec3 texColor =  vec3(texture(texture1, texCoord));
	vec3 result    = (ambient + diffuse + specular) * texColor ;
	color          = vec4(result, 1.0);

    //color = texture(texture1, texCoord) * xyzColor * (ambient + diffuse + specular) ;
}


