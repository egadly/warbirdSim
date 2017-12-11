# version 330 core
        
in  vec4 vsColor;
out vec4 color;
        
in vec3 vs_worldpos;
in vec3 vs_normal;
in vec3 vs_lightpos;
        
// define light propoerties
uniform vec4 color_ambient = vec4(0.1, 0.1, 1.0, 1.0); //Ambient Color

uniform sampler2D Texture; //Texture input
uniform bool IfRuber; //Ruber Check for brightening
uniform bool UseTexture; // Texture variable
        
uniform vec3 light_direction = vec3(0.0f, 0.0f, 1.0f); //Camera Light Direction

void main(void) {
   float ambient = 0.15f;   // Ambient Light Weight
   vec3 vs_light_direction = normalize(vs_lightpos - vs_worldpos); // Determine Light Direction
   vec3 normal = normalize(vs_normal);
   float diffuse_cam = 0.25f*max(0.0, 0.1f+dot(normal, light_direction)); //Camera Directional Light/ Normal Diffuse (Shifted to get some more light out of orthognal normals)
   float diffuse_point;
   if (  dot( normal,vs_light_direction) > -0.333f ) diffuse_point = 1.0f; //Ruber Point Light / Flat Shading
   else diffuse_point = 0.0f;
   float diffuse_ruber = 0.1f; //If not Ruber use this
   if ( IfRuber ) diffuse_ruber = 0.5f; //If Ruber use this
   if ( UseTexture ) { //If textured
    color = texture(Texture,vec2( gl_FragCoord.x/800.0f, gl_FragCoord.y/600.0f) ); //Texture coordinate is based on Camera Position
   }
   else { //If not textured calculate light as normal
     color = ambient * (color_ambient) + (1.0f - ambient) * (diffuse_cam + diffuse_point + diffuse_ruber) * vsColor; //Ambient weight + diffuse weight
   }
}