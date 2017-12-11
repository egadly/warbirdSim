# version 330 core
        
in  vec4 vsColor;
out vec4 color;
        
in vec3 vs_worldpos;
in vec3 vs_normal;
in vec3 vs_lightpos;
        
// define light propoerties
uniform vec4 color_ambient = vec4(0.1, 0.1, 0.50, 1.0); //Ambient Color

uniform sampler2D Texture; //Texture input
uniform bool IfRuber; //Ruber Check for brightening
uniform bool UseTexture; // Texture variable
        
uniform vec3 light_direction = vec3(0.0f, 0.0f, 1.0f); //Camera Light Direction

void main(void) {
   float ambient = 0.15f;   // Ambient Light Weight
   vec3 vs_light_direction = normalize(vs_lightpos - vs_worldpos); // Determine Light Direction
   vec3 normal = normalize(vs_normal);
   float diffuse_cam = 0.334f*max(0.0, dot(normal, light_direction)); //Camera Directional Light
   float diffuse_point = min( 5.0f, max( 0.0, 1000.0f*dot(normal,vs_light_direction)) ); //Ruber Point Light
   float diffuse_ruber = 0.0f; //If not Ruber use this
   if ( IfRuber ) diffuse_ruber = 0.15f; //If Ruber use this
   if ( UseTexture ) {
    color = texture(Texture,vec2( gl_FragCoord.x/800.0f, gl_FragCoord.y/600.0f) ); //Texture coordinate is based on Camera Position
   }
   else {
     color = ambient * (color_ambient + vsColor)/2.0f + (1.0f - ambient) * (diffuse_cam + diffuse_point + diffuse_ruber) * vsColor; //Ambient weight + diffuse weight
   }
}