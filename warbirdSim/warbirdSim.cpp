// Ernest Gaddi
// Project Ruber Phase 3
// COMP465 GRAPHIC SYST/DSGN
// December 10 2017

# define __Windows__ 
  #ifndef _INCLUDES465_
  #include "../../includes465/include465.hpp"
  #include "../../includes465/texture465.hpp"
  #define _INCLUDES465_
#endif

const int numModels = 9;  // number of models in this scene
char * modelFile[numModels] = { // model filenames
  "ruber.tri",
  "unum.tri",
  "duo.tri",
  "primus.tri",
  "secundus.tri",
  "warbird.tri",
  "missile.tri",
  "missilesites.tri",
  "starfield.tri"
};

float modelBoundingRadius[numModels]; // model bound radius (for use if not normalized) set in init()
const int numVertices[numModels] = { // model vertices count (minimalist)
  117 * 3,
  21 * 3,
  21 * 3,
  21 * 3,
  21 * 3,
  69 * 3,
  93 * 3,
  61 * 3,
  2 * 3
};

//Shader Stuff
char * vertexShaderFile = "simpleVertex.glsl";
char * fragmentShaderFile = "simpleFragment.glsl";
GLuint shaderProgram;

//Vertex <variable> Objects
GLuint VAO[numModels];      // Vertex Array Objects
GLuint VBO[numModels];   // Vertex Buffer Objects

GLuint UseTexture, Texture, ViewMatrix, NormalMatrix, MVP, IfRuber;  // Handles for Shader Variables
GLuint vPosition[numModels], vColor[numModels], vNormal[numModels];// vPosition, vColor, vNormal handles for models

struct objectMVP { // general structure to produce model matrix
  glm::mat4 translateMatrix;
  glm::mat4 rotationMatrix;
  glm::mat4 orbitalMatrix;
  glm::mat4 scaleMatrix;
  glm::vec3 translation;
  float size;
  float orbitSpeed;
  float rotationSpeed;
  int modelIndex;
  objectMVP * parent;
};

struct ship { // warbird specific data
  objectMVP transform; // model matrix data
  bool active = true;
  int currentSpeed = 0; // speed setting of ship
  float speedSettings[3] = {
    10, 20, 200
  };
  int warpPoint = 0; // warp point location
  unsigned missiles = 9; // number of missiles
};

struct site { // missile site specific data
  objectMVP transform; //model matrix data
  bool active = true;
  unsigned missiles = 5; // number of missiles
};

struct projectile { // missile specific data
  objectMVP transform; // model matrix data
  objectMVP * target = NULL; // target to follow's model matrix data
  unsigned life = 0; // timer for lifetime
  bool active = false;
};

struct inputStructure { // structure to handle smooth inputs
  bool forwards = false;
  bool backwards = false;
  bool yawLeft = false;
  bool yawRight = false;
  bool pitchUp = false;
  bool pitchDown = false;
  bool rollLeft = false;
  bool rollRight = false;
};

inputStructure inputs;

objectMVP astralObjects[5]; // ruber, unum, duo, primus, secundus data
ship warbird;
projectile missiles[9]; // warbird missiles
site missileSites[2]; // unum, duo missile sites
projectile uMissiles[5]; // unum's missiles
projectile dMissiles[5]; // duo's missiles

bool gravityOn = false; // gravity setting and value
float gravity = 90000000.0f;



const int numCameras = 5; // Number of Cameras for math
unsigned currentCamera = 0; // Current Camera for View Matrix
glm::vec3 eyeCameras[numCameras] = { //Relative Translations for Cameras
  glm::vec3(0, 10000, 20000),
  glm::vec3(0, 20000, 0),
  glm::vec3(0, 300, 1000),
  glm::vec3(-4000.0f, 0.0f, -4000.0f),
  glm::vec3(-4000.0f, 0.0f, -4000.0f),
};

glm::mat4 modelMatrix;
glm::mat4 viewMatrix;
glm::mat4 projectionMatrix;
glm::mat3 normalMatrix;
glm::mat4 ModelViewProjectionMatrix; // For use in display();

char camTitle[20] = "Cam: Front"; // default camera title

//Timer Variables
float timerDelay; // timer quanta setting
float updateSpeed[4] = { // the actual speeds
  5,
  40,
  100,
  500
};
unsigned currentUpdateSpeed = 0; // index to current time quanta

// variables for update rate
double currentTime = 0;
double previousTime = 0;
int frameCount = 0;

glm::mat4 rotateToward(glm::vec3 facing, glm::vec3 towards) { // function to orient object with 'facing' at vector towards a point 'towards'
  facing = glm::normalize(facing);
  towards = glm::normalize(towards);
  float cosine = glm::dot(facing, towards);
  cosine = glm::clamp(cosine, -1.0f, 1.0f);
  glm::vec3 rotationAxis = glm::cross(facing, towards);
  float rotationAngle = glm::acos(cosine);
  if (rotationAxis == glm::vec3()) return glm::mat4(1);
  return glm::rotate(glm::mat4(), rotationAngle, rotationAxis);
}

void warbirdCollision() { // warbird collision checking, 
  for (int i = 0; i < 5; i++) { // check all astral bodies
    if (glm::distance(glm::vec3(warbird.transform.translateMatrix[3]), astralObjects[i].translation) < warbird.transform.size + astralObjects[i].size + 10.0f) warbird.active = false;
  }
  for (int i = 0; i < 5; i++) { // check all Unum's missiles
    if (glm::distance(glm::vec3(warbird.transform.translateMatrix[3]), uMissiles[i].transform.translation) < warbird.transform.size + uMissiles[i].transform.size) {
      warbird.active = false;
      uMissiles[i].active = false;
    }
  }
  for (int i = 0; i < 5; i++) { // check all Duo's missiles
    if (glm::distance(glm::vec3(warbird.transform.translateMatrix[3]), dMissiles[i].transform.translation) < warbird.transform.size + dMissiles[i].transform.size + 10.0f) {
      warbird.active = false;
      dMissiles[i].active = false;
    }
  }
}

void missileCollision( projectile * missile ) { // warbird missile checking
  if (glm::distance(glm::vec3(missile->transform.translateMatrix[3]), warbird.transform.translation) < missile->transform.size + warbird.transform.size + 20.0f) { // check warbird
    warbird.active = false;
    missile->active = false;
  }
  for (int i = 0; i < 2; i++) { //check missile sites
    if (glm::distance(glm::vec3(missile->transform.translateMatrix[3]), missileSites[i].transform.translation) < missile->transform.size + missileSites[i].transform.size + 10.0f) {
      missileSites[i].active = false;
      missile->active = false;
    }
  }
  for (int i = 0; i < 5; i++) { // check unum missiles
    if (glm::distance(glm::vec3(missile->transform.translateMatrix[3]), uMissiles[i].transform.translation) < missile->transform.size + uMissiles[i].transform.size + 10.0f) {
      missile->active = false;
      uMissiles[i].active = false;
    }
  }
  for (int i = 0; i < 5; i++) { // check duo missiles
    if (glm::distance(glm::vec3(missile->transform.translateMatrix[3]), dMissiles[i].transform.translation) < missile->transform.size + dMissiles[i].transform.size + 10.0f) {
      missile->active = false;
      dMissiles[i].active = false;
    }
  }
}

void uMissileCollision( projectile * missile ) { // unum missile checking ( no warbird since warbird does that )
  for (int i = 0; i < 2; i++) { //check missile sites
    if (glm::distance(glm::vec3(missile->transform.translateMatrix[3]), missileSites[i].transform.translation) < missile->transform.size + missileSites[i].transform.size + 10.0f) {
      missileSites[i].active = false;
      missile->active = false;
    }
  }
  for (int i = 0; i < 5; i++) { // check duo missiles
    if (glm::distance(glm::vec3(missile->transform.translateMatrix[3]), dMissiles[i].transform.translation) < missile->transform.size + dMissiles[i].transform.size + 10.0f) {
      missile->active = false;
      dMissiles[i].active = false;
    }
  }
}

void dMissileCollision( projectile * missile ) { // unum missile checking ( no warbird since warbird does that )
  for (int i = 0; i < 2; i++) { //check missile sites
    if (glm::distance(glm::vec3(missile->transform.translateMatrix[3]), missileSites[i].transform.translation) < missile->transform.size + missileSites[i].transform.size + 10.0f) {
      missileSites[i].active = false;
      missile->active = false;
    }
  }
  for (int i = 0; i < 5; i++) { // check unum missiles
    if (glm::distance(glm::vec3(missile->transform.translateMatrix[3]), uMissiles[i].transform.translation) < missile->transform.size + uMissiles[i].transform.size + 10.0f) {
      missile->active = false;
      uMissiles[i].active = false;
    }
  }
}



void init() {
  // Shaders
  shaderProgram = loadShaders(vertexShaderFile, fragmentShaderFile);
  glUseProgram(shaderProgram);


  // Generate Vertex <variable> Objects
  glGenVertexArrays(numModels, VAO);
  glGenBuffers(numModels, VBO);
  // Load Models
  for (int i = 0; i < numModels; i++) {
    modelBoundingRadius[i] = loadModelBuffer(modelFile[i], numVertices[i], VAO[i], VBO[i], shaderProgram, vPosition[i], vColor[i], vNormal[i], "vPosition", "vColor", "vNormal");
  }

  // astral body data
  glm::vec3 astralTranslate[5] = {
    glm::vec3(0,0,0),
    glm::vec3(4000.0f,0,0),
    glm::vec3(9000.0f,0,0),
    glm::vec3(2000.0f,0,0),
    glm::vec3(4000.0f,0,0),
  };
  float astralScale[5] = {
    2000.0f,
    200.0f,
    400.0f,
    100.0f,
    150.0f
  };
  float astralOrbits[5] = {
    0,
    0.004f,
    0.002f,
    -0.002f,
    -0.004f
  };
  float astralRotations[5] = {
    0.05f,
    0.004f,
    0.002f,
    -0.002f,
    -0.004f
  };
  // Astral Body initializations
  for (int i = 0; i < 5; i++) {
    astralObjects[i].modelIndex = i;
    astralObjects[i].translateMatrix = glm::translate(glm::mat4(), astralTranslate[i]);
    astralObjects[i].rotationMatrix = glm::rotate(glm::mat4(), 0.0f, glm::vec3(0, 1, 0));
    astralObjects[i].orbitalMatrix = glm::rotate(glm::mat4(), 0.0f, glm::vec3(0, 1, 0));
    astralObjects[i].size = astralScale[i];
    astralObjects[i].scaleMatrix = glm::scale(glm::mat4(), glm::vec3( astralScale[i] * 1/modelBoundingRadius[astralObjects[i].modelIndex] ));
    astralObjects[i].orbitSpeed = astralOrbits[i];
    astralObjects[i].rotationSpeed = astralRotations[i];
    if (i > 2) astralObjects[i].parent = &astralObjects[2];
  }
  
  // Warbird Initialization
  warbird.transform.modelIndex = 5;
  warbird.transform.translateMatrix = glm::translate(glm::mat4(), glm::vec3(15000.0f, 0, 0));
  warbird.transform.orbitalMatrix = glm::rotate(glm::mat4(), 0.0f, glm::vec3(0, 1, 0));
  warbird.transform.rotationMatrix = glm::rotate(glm::mat4(), 0.0f, glm::vec3(0, 1, 0));
  warbird.transform.size = 100.0f;
  warbird.transform.scaleMatrix = glm::scale(glm::mat4(), glm::vec3( warbird.transform.size * 1/modelBoundingRadius[warbird.transform.modelIndex] ));

  // Missile Site Initializations
  for (int i = 0; i < 2; i++) {
    missileSites[i].transform.modelIndex = 7;
    missileSites[i].transform.parent = &astralObjects[i + 1];
    missileSites[i].transform.translateMatrix = glm::translate(glm::mat4(), glm::vec3(0, astralScale[i + 1], 0));
    missileSites[i].transform.orbitalMatrix = glm::rotate(glm::mat4(), 0.0f, glm::vec3(0, 1, 0));
    missileSites[i].transform.rotationMatrix = glm::rotate(glm::mat4(), 0.0f, glm::vec3(0, 1, 0));
    missileSites[i].transform.size = 30.0f;
    missileSites[i].transform.scaleMatrix = glm::scale(glm::mat4(), glm::vec3(missileSites[i].transform.size * 1 / modelBoundingRadius[missileSites[i].transform.modelIndex]));
  }

  // Warbird Missiles initiliazati0ns
  for (int i = 0; i < 9; i++) {
    missiles[i].transform.modelIndex = 6;
    missiles[i].transform.translateMatrix = glm::translate(glm::mat4(), glm::vec3(0, 0, 0));
    missiles[i].transform.orbitalMatrix = glm::rotate(glm::mat4(), 0.0f, glm::vec3(0, 1, 0));
    missiles[i].transform.rotationMatrix = glm::rotate(glm::mat4(), 0.0f, glm::vec3(0, 1, 0));
    missiles[i].transform.size = 25.0f;
    missiles[i].transform.scaleMatrix = glm::scale(glm::mat4(), glm::vec3(missiles[i].transform.size * 1 / modelBoundingRadius[missiles[i].transform.modelIndex]));
  }

  // Unum Missiles initiliazati0ns
  for (int i = 0; i < 5; i++) {
    uMissiles[i].transform.modelIndex = 6;
    uMissiles[i].transform.translateMatrix = glm::translate(glm::mat4(), glm::vec3(0, 0, 0));
    uMissiles[i].transform.orbitalMatrix = glm::rotate(glm::mat4(), 0.0f, glm::vec3(0, 1, 0));
    uMissiles[i].transform.rotationMatrix = glm::rotate(glm::mat4(), 0.0f, glm::vec3(0, 1, 0));
    uMissiles[i].transform.size = 25.0f;
    uMissiles[i].transform.scaleMatrix = glm::scale(glm::mat4(), glm::vec3(uMissiles[i].transform.size * 1 / modelBoundingRadius[uMissiles[i].transform.modelIndex]));
  }

  // Duo Missiles initiliazati0ns
  for (int i = 0; i < 5; i++) {
    dMissiles[i].transform.modelIndex = 6;
    dMissiles[i].transform.translateMatrix = glm::translate(glm::mat4(), glm::vec3(0, 0, 0));
    dMissiles[i].transform.orbitalMatrix = glm::rotate(glm::mat4(), 0.0f, glm::vec3(0, 1, 0));
    dMissiles[i].transform.rotationMatrix = glm::rotate(glm::mat4(), 0.0f, glm::vec3(0, 1, 0));
    dMissiles[i].transform.scaleMatrix = glm::scale(glm::mat4(), glm::vec3(1000.0f));
    dMissiles[i].transform.size = 25.0f;
    dMissiles[i].transform.scaleMatrix = glm::scale(glm::mat4(), glm::vec3(dMissiles[i].transform.size * 1 / modelBoundingRadius[dMissiles[i].transform.modelIndex]));
  }



  //Assign Shader Handles
  MVP = glGetUniformLocation(shaderProgram, "ModelViewProjection");
  NormalMatrix = glGetUniformLocation(shaderProgram, "NormalMatrix");
  ViewMatrix = glGetUniformLocation(shaderProgram, "ViewMatrix");
  Texture = glGetUniformLocation(shaderProgram, "Texture");
  UseTexture = glGetUniformLocation(shaderProgram, "UseTexture");
  IfRuber = glGetUniformLocation(shaderProgram, "IfRuber");
  printf("Texture:%d", loadRawTexture(Texture, "starfield.raw", 1024, 1024));

  
  /*printf("Shader program variable locations:\n");
  printf("  vPosition = %d  vColor = %d  vNormal = %d MVP = %d Texture = %d\n",
  glGetAttribLocation( shaderProgram, "vPosition" ),
  glGetAttribLocation( shaderProgram, "vColor" ),
  glGetAttribLocation( shaderProgram, "vNormal" ), MVP,
  glGetAttribLocation( shaderProgram, "Texture" ) );*/
  

  // Initialize default View Matrix since I'm not sure if update will call before display
  viewMatrix = glm::lookAt(
    glm::vec3(0.0f, 10000.0f, 20000.0f),  // eye position
    glm::vec3(0),                   // look at position
    glm::vec3(0.0f, 1.0f, 0.0f)); // up vect0r

  timerDelay = updateSpeed[currentUpdateSpeed]; // Set default update rate

  // Set render state variables
  glEnable(GL_DEPTH_TEST);
  glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
}

void updateTitle() {
  char missileTitle[50]; // String for missile coutns
  char fullTitle[100]; // Full title to pass to glutSetWindowTitle
  char updTitle[50]; // String for UPS (no not that one) string
  char frameTitle[50]; // String for FPS string
  // generat strings
  sprintf(updTitle, "Updates/sec %3d  ", (int)(1000.0f / timerDelay));
  sprintf(frameTitle, "Frames/sec %04d  ", (int)(1000.0f / (currentTime - previousTime)));
  sprintf(missileTitle, "Missiles Warbird %2d Unum %2d Duo %2d  ", warbird.missiles, missileSites[0].missiles, missileSites[1].missiles);
  //clear title and concatenate appropriate strings
  strcpy(fullTitle, "");
  if (!warbird.active) {
    strcat(fullTitle, "Cadet resigns from War College!");
  }
  else if ( !missileSites[0].active && !missileSites[1].active) {
    strcat(fullTitle, "Cadet passes flight training");
  }
  else {
    strcat(fullTitle, missileTitle);
    strcat(fullTitle, camTitle);
    strcat(fullTitle, updTitle);
    strcat(fullTitle, frameTitle);
  }
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
  case 'x': case 'X':
    if (currentCamera == 0) currentCamera = 4;
    else currentCamera = (currentCamera - 1) % numCameras;
    break;
  case 't': case 'T': // set update speed
    timerDelay = updateSpeed[(++currentUpdateSpeed) % 4];
    updateTitle();
    break;
  case 's': case 'S': // set warbird speed
    warbird.currentSpeed = (warbird.currentSpeed + 1) % 3;
    break;
  case 'w': case 'W': // warp to appropriate camera
    if (warbird.active) { 
      warbird.transform.translateMatrix = glm::translate(glm::mat4(), glm::vec3((astralObjects[warbird.warpPoint + 1].orbitalMatrix*glm::translate(glm::mat4(), eyeCameras[warbird.warpPoint + 3]))[3]));
      warbird.transform.rotationMatrix = rotateToward(glm::vec3(warbird.transform.rotationMatrix[2]), astralObjects[warbird.warpPoint + 1].translation - glm::vec3(warbird.transform.translateMatrix[3])) * warbird.transform.rotationMatrix;
      warbird.warpPoint = (warbird.warpPoint + 1) % 2;
    }
    break;
  case 'f': case 'F': // fire missiles
    if ( warbird.active && warbird.missiles > 0 && !missiles[warbird.missiles].active ) {
      missiles[warbird.missiles - 1].transform.translateMatrix = warbird.transform.translateMatrix;
      missiles[warbird.missiles - 1].transform.rotationMatrix = warbird.transform.rotationMatrix;
      missiles[warbird.missiles - 1].active = true;
      warbird.missiles -= 1;
    }
    break;
  case 'g': case 'G': // toggle gravity
    gravityOn = !gravityOn;
    break;
  }
}
void special(int key, int x, int y) {
  // arrow key logic ( key down event )
  int mod = glutGetModifiers();
  switch (key) {
  case GLUT_KEY_UP:
    if (mod == GLUT_ACTIVE_CTRL) inputs.pitchDown = true;
    else inputs.forwards = true;
    break;
  case GLUT_KEY_DOWN:
    if (mod == GLUT_ACTIVE_CTRL) inputs.pitchUp = true;
    else inputs.backwards = true;
    break;
  case GLUT_KEY_RIGHT:
    if (mod == GLUT_ACTIVE_CTRL) inputs.rollLeft = true;
    else inputs.yawLeft = true;
    break;
  case GLUT_KEY_LEFT:
    if (mod == GLUT_ACTIVE_CTRL) inputs.rollRight = true;
    else inputs.yawRight = true;
    break;
  }
}
void specialUp(int key, int x, int y) {
   //arrow key logic ( key up event )
  switch (key) {
  case GLUT_KEY_UP:
    inputs.forwards = inputs.pitchDown = false;
    break;
  case GLUT_KEY_DOWN:
    inputs.backwards = inputs.pitchUp = false;
    break;
  case GLUT_KEY_RIGHT:
    inputs.yawLeft = inputs.rollLeft = false;
    break;
  case GLUT_KEY_LEFT:
    inputs.yawRight = inputs.rollRight = false;
    break;
  }
}

void update(int i) {
  glutTimerFunc(timerDelay, update, 1);

  // Increments rotational and revolution angles
  for (int i = 0; i < 5; i++) {
    astralObjects[i].orbitalMatrix *= glm::rotate(glm::mat4(), astralObjects[i].orbitSpeed, glm::vec3(0.0f, 1.0f, 0.0f));
    astralObjects[i].rotationMatrix *= glm::rotate(glm::mat4(), astralObjects[i].rotationSpeed, glm::vec3(0.0f, 1.0f, 0.0f));
  }

  // missile data
  float missileRange = 3000.0f;
  float missileSpeed = 10.0f;
  //Warbird Missiles
  for (int m = 0; m < 9; m++) {
    if (missiles[m].active) {
      if (gravityOn) missiles[m].transform.translateMatrix *= glm::translate(glm::mat4(), glm::normalize(astralObjects[0].translation - glm::vec3(missiles[m].transform.translateMatrix[3]) * gravity)); // apply gravity
      missiles[m].transform.translateMatrix *= glm::translate(glm::mat4(), glm::vec3(missiles[m].transform.rotationMatrix[2])*missileSpeed ); // move forwards
      missiles[m].life += 1; // increment life
      if (missiles[m].life >= 200) {
        missileCollision(&missiles[m]); // check collisions
        if (glm::distance(glm::vec3(missiles[m].transform.translateMatrix[3]), missileSites[0].transform.translation) < missileRange) { // if unum site is in range
          missiles[m].target = &missileSites[0].transform;
          if (glm::distance(glm::vec3(missiles[m].transform.translateMatrix[3]), missileSites[1].transform.translation) < missileRange && glm::distance(glm::vec3(missiles[m].transform.translateMatrix[3]), missileSites[1].transform.translation) < glm::distance(glm::vec3(missiles[m].transform.translateMatrix[3]), missileSites[0].transform.translation)) {
            missiles[m].target = &missileSites[1].transform; // if duo site is closer
          }
        }
        else { 
          if (glm::distance(glm::vec3(missiles[m].transform.translateMatrix[3]), missileSites[1].transform.translation) < missileRange) {
            missiles[m].target = &missileSites[1].transform; // if duo site is in range
          }
        }
      }
      if (missiles[m].life == 2000) missiles[m].active = false; // destroy self at end of life
      if ( missiles[m].target != NULL ) missiles[m].transform.rotationMatrix = rotateToward(glm::vec3(missiles[m].transform.rotationMatrix[2]), missiles[m].target->translation - glm::vec3(missiles[m].transform.translateMatrix[3])) * missiles[m].transform.rotationMatrix; // orient towards target
    }
  }

  //Unum missile site
  if (missileSites[0].active && glm::distance(glm::vec3(missileSites[0].transform.translation), glm::vec3(warbird.transform.translateMatrix[3])) < missileRange) { // if warbird in range
    if (missileSites[0].missiles > 0 && ( !(uMissiles[missileSites[0].missiles].active) || missileSites[0].missiles == 5 ) ) { // fire missile
      uMissiles[missileSites[0].missiles - 1].transform.translateMatrix = glm::translate(glm::mat4(), missileSites[0].transform.translation );
      uMissiles[missileSites[0].missiles - 1].transform.rotationMatrix = rotateToward(glm::vec3(uMissiles[missileSites[0].missiles - 1].transform.rotationMatrix[2]), warbird.transform.translation - missileSites[0].transform.translation) * uMissiles[missileSites[0].missiles - 1].transform.rotationMatrix;
      uMissiles[missileSites[0].missiles - 1].active = true;
      missileSites[0].missiles -= 1;
    }
  }

  //Duo missile site
  if (missileSites[1].active && glm::distance(glm::vec3(missileSites[1].transform.translation), glm::vec3(warbird.transform.translateMatrix[3])) < missileRange) { // if warbird in range
    if (missileSites[1].missiles > 0 && ( !(dMissiles[missileSites[1].missiles].active) || missileSites[1].missiles == 5 ) ) { // fire missile
      dMissiles[missileSites[1].missiles - 1].transform.translateMatrix = glm::translate(glm::mat4(), missileSites[1].transform.translation);
      dMissiles[missileSites[1].missiles - 1].transform.rotationMatrix = rotateToward(glm::vec3(dMissiles[missileSites[1].missiles - 1].transform.rotationMatrix[2]), warbird.transform.translation - missileSites[1].transform.translation) * dMissiles[missileSites[1].missiles - 1].transform.rotationMatrix;
      dMissiles[missileSites[1].missiles - 1].active = true;
      missileSites[1].missiles -= 1;
    }
  }

  // Unum Missiles
  for (int m = 0; m < 5; m++) {
    if (uMissiles[m].active) {
      uMissiles[m].transform.translateMatrix *= glm::translate(glm::mat4(), glm::vec3(uMissiles[m].transform.rotationMatrix[2])*missileSpeed); // move forward
      if (gravityOn) uMissiles[m].transform.translateMatrix *= glm::translate(glm::mat4(), glm::normalize(astralObjects[0].translation - glm::vec3(uMissiles[m].transform.translateMatrix[3]) * gravity)); // apply gravity
      uMissiles[m].life += 1; // increment life
      if (uMissiles[m].life >= 200) {
        uMissileCollision( &uMissiles[m]);
        if (glm::distance(glm::vec3(uMissiles[m].transform.translateMatrix[3]), glm::vec3(warbird.transform.translateMatrix[3])) < missileRange) {
          if ( warbird.active) uMissiles[m].target = &warbird.transform; // if warbird is in range
          else uMissiles[m].target = NULL;
        }
      }
      if (uMissiles[m].life == 2000) uMissiles[m].active = false; // destroy on end of life
      if (uMissiles[m].target != NULL) {
        uMissiles[m].transform.rotationMatrix = rotateToward(glm::vec3(uMissiles[m].transform.rotationMatrix[2]), uMissiles[m].target->translation - glm::vec3(uMissiles[m].transform.translateMatrix[3])) * uMissiles[m].transform.rotationMatrix; // orient towards target
      }
    }
  }

  //Duo Missiles
  for (int m = 0; m < 5; m++) {  //same structure as above
    if (dMissiles[m].active) {
      dMissiles[m].transform.translateMatrix *= glm::translate(glm::mat4(), glm::vec3(dMissiles[m].transform.rotationMatrix[2])*missileSpeed);
      if (gravityOn) dMissiles[m].transform.translateMatrix *= glm::translate(glm::mat4(), glm::normalize(astralObjects[0].translation - glm::vec3(dMissiles[m].transform.translateMatrix[3]) * gravity));
      dMissiles[m].life += 1;
      if (dMissiles[m].life >= 200) {
        dMissileCollision(&dMissiles[m]);
        if (glm::distance(glm::vec3(dMissiles[m].transform.translateMatrix[3]), glm::vec3(warbird.transform.translateMatrix[3])) < missileRange) {
          if (warbird.active) dMissiles[m].target = &warbird.transform;
          else dMissiles[m].target = NULL;
        }
      }
      if (dMissiles[m].life == 2000) dMissiles[m].active = false;
      if (dMissiles[m].target != NULL) {
        dMissiles[m].transform.rotationMatrix = rotateToward(glm::vec3(dMissiles[m].transform.rotationMatrix[2]), dMissiles[m].target->translation - glm::vec3(dMissiles[m].transform.translateMatrix[3])) * dMissiles[m].transform.rotationMatrix;
      }
    }
  }
  
  //Warbird
  if (warbird.active) {
    warbirdCollision(); // warbird collisions
    // movement and rotation logic
    if (inputs.forwards) {
      warbird.transform.translateMatrix *= glm::translate(glm::mat4(), glm::vec3(warbird.transform.rotationMatrix[2] * warbird.speedSettings[warbird.currentSpeed]));
    }
    if (inputs.backwards) {
      warbird.transform.translateMatrix *= glm::translate(glm::mat4(), glm::vec3(warbird.transform.rotationMatrix[2] * -1.0f * warbird.speedSettings[warbird.currentSpeed]));
    }
    if (inputs.yawLeft) {
      warbird.transform.rotationMatrix = glm::rotate(glm::mat4(), -0.02f, glm::vec3(warbird.transform.rotationMatrix[1])) * warbird.transform.rotationMatrix;
    }
    if (inputs.yawRight) {
      warbird.transform.rotationMatrix = glm::rotate(glm::mat4(), 0.02f, glm::vec3(warbird.transform.rotationMatrix[1])) * warbird.transform.rotationMatrix;
    }
    if (inputs.pitchDown) {
      warbird.transform.rotationMatrix = glm::rotate(glm::mat4(), 0.02f, glm::vec3(warbird.transform.rotationMatrix[0])) * warbird.transform.rotationMatrix;
    }
    if (inputs.pitchUp) {
      warbird.transform.rotationMatrix = glm::rotate(glm::mat4(), -0.02f, glm::vec3(warbird.transform.rotationMatrix[0])) * warbird.transform.rotationMatrix;
    }
    if (inputs.rollLeft) {
      warbird.transform.rotationMatrix = glm::rotate(glm::mat4(), 0.02f, glm::vec3(warbird.transform.rotationMatrix[2])) * warbird.transform.rotationMatrix;
    }
    if (inputs.rollRight) {
      warbird.transform.rotationMatrix = glm::rotate(glm::mat4(), -0.02f, glm::vec3(warbird.transform.rotationMatrix[2])) * warbird.transform.rotationMatrix;
    }
    if ( gravityOn ) warbird.transform.translateMatrix *= glm::translate(glm::mat4(), glm::normalize(astralObjects[0].translation - glm::vec3(warbird.transform.translateMatrix[3]) * gravity));
  }

  updateTitle();

}

void display() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear Buffer

  // Generate View Matrix
  glm::vec3 eye = eyeCameras[currentCamera];
  glm::vec3 at = glm::vec3(0);
  switch (currentCamera) {
  case 0: // General Camera
    viewMatrix = glm::lookAt(eye, at, glm::vec3(0, 1, 0));
    strcpy(camTitle, "Cam: Front");
    break;
  case 1: //Topview Camera
    viewMatrix = glm::lookAt(eye, at, glm::vec3(0, 0, -1));
    strcpy(camTitle, "Cam: Top");
    break;
  case 2: // Warbird Camera
    eye = warbird.transform.translation + glm::vec3((glm::translate(glm::mat4(), glm::vec3(warbird.transform.rotationMatrix[1]) * 300.0f) * glm::translate(glm::mat4(), glm::vec3(warbird.transform.rotationMatrix[2]) * -1000.0f))[3]);
    at = warbird.transform.translation + glm::vec3((glm::translate(glm::mat4(), glm::vec3(warbird.transform.rotationMatrix[1]) * 300.0f))[3]);
    viewMatrix = glm::lookAt(eye, at, glm::vec3(warbird.transform.rotationMatrix[1]));
    strcpy(camTitle, "Cam: Warbird");
    break;
  case 3: // Unum Camera (Ruber eclipses a lot but wait you'll see it eventually)
    eye = (astralObjects[1].orbitalMatrix*glm::translate(glm::mat4(), eye))[3]; // Make translate matrix, apply rotation transform, then extract world position
    at = astralObjects[1].translation; // Unums Position
    viewMatrix = glm::lookAt(eye, at, glm::vec3(0, 1, 0));
    strcpy(camTitle, "Cam: Unum");
    break;
  case 4: // Duo Camera
    eye = (astralObjects[2].orbitalMatrix*glm::translate(glm::mat4(), eye))[3]; // Make translate matrix, apply rotation transform, then extract world position
    at = astralObjects[2].translation; // Unums Position
    viewMatrix = glm::lookAt(eye, at, glm::vec3(0, 1, 0));
    strcpy(camTitle, "Cam: Duo");
    break;
  }

  //Starfield
  glm::mat3 dummy3Matrix = glm::mat3(); //identity matrices to set in shader
  glm::mat4 dummy4Matrix = glm::mat4();
  glUniform1f(UseTexture, true);
  glUniformMatrix4fv(MVP, 1, GL_FALSE, glm::value_ptr(dummy4Matrix));
  glUniformMatrix3fv(NormalMatrix, 1, GL_FALSE, glm::value_ptr(dummy3Matrix));
  glUniformMatrix4fv(ViewMatrix, 1, GL_FALSE, glm::value_ptr(dummy4Matrix));
  glBindVertexArray(VAO[8]);
  glDrawArrays(GL_TRIANGLES, 0, numVertices[8]);

  glClear( GL_DEPTH_BUFFER_BIT ); //Clear Buffer so it's always behind

  //Astral Bodies
  glm::mat4 modelMatrix;
  for (int m = 0; m < 5; m++) {
    modelMatrix = astralObjects[m].orbitalMatrix * astralObjects[m].translateMatrix * astralObjects[m].rotationMatrix *  astralObjects[m].scaleMatrix;
    if (astralObjects[m].parent != NULL) modelMatrix = astralObjects[m].parent->orbitalMatrix * astralObjects[m].parent->translateMatrix * modelMatrix;
    astralObjects[m].translation = glm::vec3(modelMatrix[3]);

    ModelViewProjectionMatrix = projectionMatrix * viewMatrix * modelMatrix;
    normalMatrix = glm::mat3(viewMatrix * modelMatrix);
    glUniformMatrix4fv(MVP, 1, GL_FALSE, glm::value_ptr(ModelViewProjectionMatrix));
    glUniformMatrix3fv(NormalMatrix, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    glUniformMatrix4fv(ViewMatrix, 1, GL_FALSE, glm::value_ptr(viewMatrix));
    glUniform1f(UseTexture, false);
    if ( m== 0 ) glUniform1f(IfRuber, true);
    else glUniform1f(IfRuber, false);
    glBindVertexArray(VAO[astralObjects[m].modelIndex]);
    glDrawArrays(GL_TRIANGLES, 0, numVertices[astralObjects[m].modelIndex]);
  }

  if (warbird.active) {
    modelMatrix = warbird.transform.orbitalMatrix * warbird.transform.translateMatrix * warbird.transform.rotationMatrix * warbird.transform.scaleMatrix;
    warbird.transform.translation = glm::vec3(modelMatrix[3]);
    ModelViewProjectionMatrix = projectionMatrix * viewMatrix * modelMatrix;
    normalMatrix = glm::mat3(viewMatrix * modelMatrix);
    glUniformMatrix4fv(MVP, 1, GL_FALSE, glm::value_ptr(ModelViewProjectionMatrix));
    glUniformMatrix3fv(NormalMatrix, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    glUniformMatrix4fv(ViewMatrix, 1, GL_FALSE, glm::value_ptr(viewMatrix));
    glBindVertexArray(VAO[warbird.transform.modelIndex]);
    glDrawArrays(GL_TRIANGLES, 0, numVertices[warbird.transform.modelIndex]);
  }

  // Missile Sites
  for (int m = 0; m < 2; m++) {
    if (missileSites[m].active) {
      modelMatrix = missileSites[m].transform.orbitalMatrix * missileSites[m].transform.translateMatrix * missileSites[m].transform.rotationMatrix *  missileSites[m].transform.scaleMatrix;
      if (missileSites[m].transform.parent != NULL) modelMatrix = missileSites[m].transform.parent->orbitalMatrix * missileSites[m].transform.parent->translateMatrix * modelMatrix;
      missileSites[m].transform.translation = glm::vec3(modelMatrix[3]);

      ModelViewProjectionMatrix = projectionMatrix * viewMatrix * modelMatrix;
      normalMatrix = glm::mat3(viewMatrix * modelMatrix);
      glUniformMatrix4fv(MVP, 1, GL_FALSE, glm::value_ptr(ModelViewProjectionMatrix));
      glUniformMatrix3fv(NormalMatrix, 1, GL_FALSE, glm::value_ptr(normalMatrix));
      glUniformMatrix4fv(ViewMatrix, 1, GL_FALSE, glm::value_ptr(viewMatrix));
      glBindVertexArray(VAO[missileSites[m].transform.modelIndex]);
      glDrawArrays(GL_TRIANGLES, 0, numVertices[missileSites[m].transform.modelIndex]);
    }
  }

  //Ship Missiles
  for (int m = 0; m < 9; m++) {
    if (missiles[m].active) {
      modelMatrix = missiles[m].transform.orbitalMatrix * missiles[m].transform.translateMatrix * missiles[m].transform.rotationMatrix *  missiles[m].transform.scaleMatrix;
      if (missiles[m].transform.parent != NULL) modelMatrix = missiles[m].transform.parent->orbitalMatrix * missiles[m].transform.parent->translateMatrix * modelMatrix;
      missiles[m].transform.translation = glm::vec3(modelMatrix[3]);

      ModelViewProjectionMatrix = projectionMatrix * viewMatrix * modelMatrix;
      normalMatrix = glm::mat3(viewMatrix * modelMatrix);
      glUniformMatrix4fv(MVP, 1, GL_FALSE, glm::value_ptr(ModelViewProjectionMatrix));
      glUniformMatrix3fv(NormalMatrix, 1, GL_FALSE, glm::value_ptr(normalMatrix));
      glUniformMatrix4fv(ViewMatrix, 1, GL_FALSE, glm::value_ptr(viewMatrix));
      glBindVertexArray(VAO[missiles[m].transform.modelIndex]);
      glDrawArrays(GL_TRIANGLES, 0, numVertices[missiles[m].transform.modelIndex]);
    }
  }

  //Unum Missiles
  for (int m = 0; m < 5; m++) {
    if (uMissiles[m].active) {
      modelMatrix = uMissiles[m].transform.orbitalMatrix * uMissiles[m].transform.translateMatrix * uMissiles[m].transform.rotationMatrix *  uMissiles[m].transform.scaleMatrix;
      if (uMissiles[m].transform.parent != NULL) modelMatrix = uMissiles[m].transform.parent->orbitalMatrix * uMissiles[m].transform.parent->translateMatrix * modelMatrix;
      uMissiles[m].transform.translation = glm::vec3(modelMatrix[3]);

      ModelViewProjectionMatrix = projectionMatrix * viewMatrix * modelMatrix;
      normalMatrix = glm::mat3(viewMatrix * modelMatrix);
      glUniformMatrix4fv(MVP, 1, GL_FALSE, glm::value_ptr(ModelViewProjectionMatrix));
      glUniformMatrix3fv(NormalMatrix, 1, GL_FALSE, glm::value_ptr(normalMatrix));
      glUniformMatrix4fv(ViewMatrix, 1, GL_FALSE, glm::value_ptr(viewMatrix));
      glBindVertexArray(VAO[uMissiles[m].transform.modelIndex]);
      glDrawArrays(GL_TRIANGLES, 0, numVertices[uMissiles[m].transform.modelIndex]);
    }
  }

  //Duo Missiles
  for (int m = 0; m < 5; m++) {
    if (dMissiles[m].active) {
      modelMatrix = dMissiles[m].transform.orbitalMatrix * dMissiles[m].transform.translateMatrix * dMissiles[m].transform.rotationMatrix *  dMissiles[m].transform.scaleMatrix;
      if (dMissiles[m].transform.parent != NULL) modelMatrix = dMissiles[m].transform.parent->orbitalMatrix * dMissiles[m].transform.parent->translateMatrix * modelMatrix;
      dMissiles[m].transform.translation = glm::vec3(modelMatrix[3]);

      ModelViewProjectionMatrix = projectionMatrix * viewMatrix * modelMatrix;
      normalMatrix = glm::mat3(viewMatrix * modelMatrix);
      glUniformMatrix4fv(MVP, 1, GL_FALSE, glm::value_ptr(ModelViewProjectionMatrix));
      glUniformMatrix3fv(NormalMatrix, 1, GL_FALSE, glm::value_ptr(normalMatrix));
      glUniformMatrix4fv(ViewMatrix, 1, GL_FALSE, glm::value_ptr(viewMatrix));
      glBindVertexArray(VAO[dMissiles[m].transform.modelIndex]);
      glDrawArrays(GL_TRIANGLES, 0, numVertices[dMissiles[m].transform.modelIndex]);
    }
  }

  previousTime = currentTime; 
  currentTime = glutGet(GLUT_ELAPSED_TIME); // time management
  glutSwapBuffers();
}

int main(int argc, char* argv[]) {
  glutInit(&argc, argv); //Initialize GLUT
  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
  glutInitWindowSize(800, 600);
  glutInitContextVersion(3, 3);
  glutInitContextProfile(GLUT_CORE_PROFILE);
  glutCreateWindow("Project Phase 2 This should change");
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
  glutSpecialFunc(special);
  glutSpecialUpFunc(specialUp);
  glutTimerFunc(timerDelay, update, 1);
  glutIdleFunc(display);
  glutMainLoop();
  return 0;
}
