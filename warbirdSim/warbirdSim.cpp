# define __Windows__ 
#ifndef _INCLUDES465_
  #include "../../includes465/include465.hpp"
  #define _INCLUDES465_
#endif // !_INCLUDES465_

const int numModels = 7;  // number of models in this scene
char * modelFile[numModels] = { 
  "ruber.tri", 
  "unum.tri", 
  "duo.tri",
  "primus.tri",
  "secundus.tri",
  "warbird.tri",
  "missile.tri"
};
float modelBoundingRadius[numModels]; // Model's Bounding Radius (for use if not normalized) set in init()
const int numVertices[numModels] = { // Model Vertices Count (minimalist)
  8 * 3,
  8 * 3,
  8 * 3,
  8 * 3,
  8 * 3,
  4 * 3,
  4 * 3
};
//Shader Stuff
char * vertexShaderFile = "simpleVertex.glsl";
char * fragmentShaderFile = "simpleFragment.glsl";
GLuint shaderProgram;

//Vertex <variable> Objects
GLuint VAO[numModels];      // Vertex Array Objects
GLuint VBO[numModels];   // Vertex Buffer Objects


GLuint MVP;  // Model View Projection matrix's handle
GLuint vPosition[numModels], vColor[numModels], vNormal[numModels];// vPosition, vColor, vNormal handles for models

//Scale Value Variables
float modelSize[numModels] = { //Absolute Scales for models
  2000.0f, 
  200.0f, 
  400.0f,
  100.0f,
  150.0f,
  100.0f,
  25.0f
};
glm::vec3 scale[numModels];       // Relative Scales (updated in init() )

//Translation Value Variables
glm::vec3 localTranslate[numModels] = { // Distance from Orbital Centers
  glm::vec3(0,0,0),
  glm::vec3(4000.0f,0,0),
  glm::vec3(9000.0f,0,0),
  glm::vec3(2000.0f,0,0),
  glm::vec3(4000.0f,0,0),
  glm::vec3(15000.0f,0,0),
  glm::vec3(14500.0f,0,0)
};
glm::vec3 worldTranslate[numModels] = { // World Position Values ( updates in display() ) but here for camera's sake
  glm::vec3(0,0,0),
  glm::vec3(4000.0f,0,0),
  glm::vec3(9000.0f,0,0),
  glm::vec3(11000.0f,0,0),
  glm::vec3(13000.0f,0,0),
  glm::vec3(15000.0f,0,0),
  glm::vec3(14500.0f,0,0)
};

//Angle Value Variables
float orbitAngle[numModels] = { 0,0,0,0,0 }; // Current orbital angle
float orbitSpeed[numModels] = { // Angular speed of revolutions
  0,
  0.004f,
  0.002f,
  -0.002f, //These are negative for better visualization of orbits
  -0.004f, // "" above
  0,
  0
};
float rotateAngle[numModels] = { 0,0,0,0,0 }; // Current rotational angle
float rotateSpeed[numModels] = { // Angular speed of rotations 
  -.001f,
  0,
  0,
  0,
  0,
  0.01f,
  0.1f
};
glm::mat4 modelMatrix;          // For use in display()

//Camera Variables
const int numCameras = 5; // Number of Cameras for math
unsigned currentCamera = 0; // Current Camera for View Matrix
glm::vec3 eyeCameras[numCameras] = { //Relative Translations for Cameras
  glm::vec3(0, 10000, 20000),
  glm::vec3(0, 20000, 0),
  glm::vec3(0, 300, 0),
  glm::vec3(-4000.0f, 0.0f, -4000.0f),
  glm::vec3(-4000.0f, 0.0f, -4000.0f),
};
glm::mat4 viewMatrix;           // For used in display() set in update()

glm::mat4 projectionMatrix;     // For use in display() set in reshape()

glm::mat4 ModelViewProjectionMatrix; // For use in display();

char * missileTitle = "Warbird 1  Unum ?  Secundus ?  ";
char camTitle[20] = "Cam: Front";


//Timer Variables
float timerDelay;
float ace = 40;
float fast = 5;
double currentTime = 0;
double previousTime = 0;
int frameCount = 0;

void init() {
  // Shaders
  shaderProgram = loadShaders(vertexShaderFile, fragmentShaderFile);
  glUseProgram(shaderProgram);

  // Generate Vertex <variable> Objects
  glGenVertexArrays(numModels, VAO);
  glGenBuffers(numModels, VBO);
  // Load Models
  for (int i = 0; i < numModels; i++) {
    modelBoundingRadius[i] = loadModelBuffer(modelFile[i], numVertices[i], VAO[i], VBO[i], shaderProgram,
      vPosition[i], vColor[i], vNormal[i], "vPosition", "vColor", "vNormal");
    // Calculate scale from bounding radius and model scale
    scale[i] = glm::vec3(modelSize[i] * 1.0f / modelBoundingRadius[i]);
  }

  //Assign MVP handler
  MVP = glGetUniformLocation(shaderProgram, "ModelViewProjection");

  /*
  printf("Shader program variable locations:\n");
  printf("  vPosition = %d  vColor = %d  vNormal = %d MVP = %d\n",
  glGetAttribLocation( shaderProgram, "vPosition" ),
  glGetAttribLocation( shaderProgram, "vColor" ),
  glGetAttribLocation( shaderProgram, "vNormal" ), MVP);
  */

  // Initialize default View Matrix since I'm not sure if update will call before display
  viewMatrix = glm::lookAt(
    glm::vec3(0.0f, 10000.0f, 20000.0f),  // eye position
    glm::vec3(0),                   // look at position
    glm::vec3(0.0f, 1.0f, 0.0f)); // up vect0r

  timerDelay = ace; // Set default update rate

  // Set render state variables
  glEnable(GL_DEPTH_TEST);
  glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
}

void updateTitle() {
  char fullTitle[100]; // Full title to pass to glutSetWindowTitle
  char updTitle[50]; // String for UPS (no not that one) string
  char frameTitle[50]; // String for FPS string
  sprintf(updTitle, "Updates/sec %3d  ", (int)(1000.0f / timerDelay) );
  sprintf(frameTitle, "Frames/sec %4d  ", (int)(1000.0f/(currentTime - previousTime)));
  strcpy(fullTitle, missileTitle); //Static for now
  strcat(fullTitle, updTitle);
  strcat(fullTitle, frameTitle);
  strcat(fullTitle, camTitle); // Changed during update()
  glutSetWindowTitle(fullTitle);
}

void reshape(int width, int height) {
  float aspectRatio = (float)width / (float)height; // Determines AR from window size
  float FOV = glm::radians(60.0f); // Field of View Variable (note: this is vertical)
  glViewport(0, 0, width, height);
  projectionMatrix = glm::perspective(FOV, aspectRatio, 1.0f, 100000.0f); //Generate projection matrix from FOV and AR
}

void keyboard(unsigned char key, int x, int y) {
  switch (key) {
    case 033: case 'q':  case 'Q': exit(EXIT_SUCCESS); break; // Quit Program
    //Camera Logic
    case '1':
      currentCamera = 0; break;
    case '2':
      currentCamera = 1; break;
    case '3':
      currentCamera = 2; break;
    case '4':
      currentCamera = 3; break;
    case '5':
      currentCamera = 4; break;
    case 'v': case 'V': currentCamera = (currentCamera + 1) % numCameras; break;
    case 'f': case 'F': 
      if (currentCamera == 0) currentCamera = 4;
      else currentCamera = (currentCamera - 1) % numCameras;
      break;
    case 'o': case 'O':
      timerDelay = ace;
      updateTitle();
      break;
    case 'p': case 'P':
      timerDelay = fast;
      updateTitle();
      break;
  }
}

void update(int i) {
  glutTimerFunc(timerDelay, update, 1);
  // Increments rotational and revolution angles
  for (int i = 0; i < numModels; i++) {
    orbitAngle[i] = orbitAngle[i] + orbitSpeed[i];
    rotateAngle[i] = rotateAngle[i] + rotateSpeed[i];
  }
  
  // Generate View Matrix
  glm::vec3 eye = eyeCameras[currentCamera];
  glm::vec3 at = glm::vec3(0);
  switch (currentCamera) {
  case 0: // General Camera
    viewMatrix = glm::lookAt(eye, at, glm::vec3(0, 1, 0));
    strcpy(camTitle, "Cam: Front");
    break;
  case 1: //Topview Camera
    viewMatrix = glm::lookAt(eye, at, glm::vec3(0, 0, 1));
    strcpy(camTitle, "Cam: Top");
    break;
  case 2: // Warbird Camera
    eye = worldTranslate[5] + glm::vec3(0, 300.0f, 0);
    at = worldTranslate[5]; // Warbird's Position
    viewMatrix = glm::lookAt(eye, at, glm::vec3(0, 0, -1));
    strcpy(camTitle, "Cam: Warbird");
    break;
  case 3: // Unum Camera (Ruber eclipses a lot but wait you'll see it eventually)
    eye = (glm::rotate(glm::mat4(), orbitAngle[1], glm::vec3(0.0f, 1.0f, 0.0f))*glm::translate(glm::mat4(), eye))[3]; // Make translate matrix, apply rotation transform, then extract world position
    at = worldTranslate[1]; // Unums Position
    viewMatrix = glm::lookAt(eye, at, glm::vec3(0, 1, 0));
    strcpy(camTitle, "Cam: Unum");
    break;
  case 4: // Duo Camera
    eye = (glm::rotate(glm::mat4(), orbitAngle[2], glm::vec3(0.0f, 1.0f, 0.0f))*glm::translate(glm::mat4(), eye))[3]; // Make translate matrix, apply rotation transform, then extract world position
    at = worldTranslate[2]; // Duo's position
    viewMatrix = glm::lookAt(eye, at, glm::vec3(0, 1, 0));
    strcpy(camTitle, "Cam: Duo");
    break;
  }
  updateTitle();


}

void display() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear Buffer
  
  for (int m = 0; m < numModels; m++) {
    // Generate Model Matrix
    glm::mat4 translateMatrix = glm::translate(glm::mat4(), localTranslate[m]);
    glm::mat4 scaleMatrix = glm::scale(glm::mat4(), glm::vec3(scale[m]));
    glm::mat4 rotateMatrix = glm::rotate(glm::mat4(), rotateAngle[m], glm::vec3(0, 1.0f, 0));
    glm::mat4 orbitMatrix = glm::rotate(glm::mat4(), orbitAngle[m], glm::vec3(0, 1.0f, 0));

    if (m < 3) {
      modelMatrix = orbitMatrix * translateMatrix * rotateMatrix * scaleMatrix;
    }
    else if (m < 5) { // Moons need extra translate to orbit Duo (Orbit is handled as if orbitting Ruber than translated to Duo )
      modelMatrix = glm::translate( glm::mat4(), worldTranslate[2]) * orbitMatrix * translateMatrix * rotateMatrix * scaleMatrix;
    }
    else {  // Warbird and missile don't orbit they just spin about Z-axis
      glm::mat4 rotateMatrix = glm::rotate(glm::mat4(), rotateAngle[m], glm::vec3(0, 0, 1.0f));
      modelMatrix = translateMatrix * rotateMatrix * scaleMatrix;
    }

    worldTranslate[m] = glm::vec3(modelMatrix[3]); //Save world position for other bodies to see

    // Draw
    ModelViewProjectionMatrix = projectionMatrix * viewMatrix * modelMatrix;
    glUniformMatrix4fv(MVP, 1, GL_FALSE, glm::value_ptr(ModelViewProjectionMatrix));
    glBindVertexArray(VAO[m]);
    glDrawArrays(GL_TRIANGLES, 0, numVertices[m]);
  }
  // Get time elapsed between renders
  previousTime = currentTime;
  currentTime = glutGet(GLUT_ELAPSED_TIME);
  updateTitle();
  glutSwapBuffers();
}

int main(int argc, char* argv[]) {
  glutInit(&argc, argv); //Initialize GLUT
  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
  glutInitWindowSize(800, 600);
  glutInitContextVersion(3, 3);
  glutInitContextProfile(GLUT_CORE_PROFILE);
  glutCreateWindow("Project Phase 1 This should change");
  // Initialize Glew
  glewExperimental = GL_TRUE;  // needed my home system 
  GLenum err = glewInit();
  if (GLEW_OK != err)
    printf("GLEW Error: %s \n", glewGetErrorString(err));
  else {
    printf("Using GLEW %s \n", glewGetString(GLEW_VERSION));
    printf("OpenGL %s, GLSL %s\n",
      glGetString(GL_VERSION),
      glGetString(GL_SHADING_LANGUAGE_VERSION));
  }
  // Initialize Warbird Sim
  init();
  // Set Glut Functions
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(keyboard);
  glutTimerFunc(timerDelay, update, 1);
  glutIdleFunc(display);
  glutMainLoop();
  return 0;
}


