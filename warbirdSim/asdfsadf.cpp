/*

indexPyramid.cpp
Shaders:  indexPyramidVertex.glsl  amd indexPyramidFragment.glsl

Example C OpenGL Core 3.3 example using:
VAOs   Vertex Array Objects
VBOs   Vertex Buffer Objects
IBOs   Index Buffer Objects

Based on Pyramid and
Angel & Shreiner's Introduction to Modern OpenGL Programming,
SIGGRAPH 2012
http://www.cs.unm.edu/~angel/SIGGRAPH12/SIGGRAPH_2012.pptx
Shreiner et.al. OpenGL Programming Guide 8th Edition code ch 3 draws
http://www.opengl-redbook.com/Code/oglpg-8th-edition.zip

Uses include465.hpp, freeglut, glew, and glm.

Note the black band around the base in the top view.  This is due to
the averaging of the normals.  The bottom view does not have this problem.
See class notes on ProceduralModeling.

Mike Barnes
9/24/2015
*/

# define __Windows__ // define your target operating system
# include "../includes465/include465.hpp"

const GLuint NumVertices = 18;
// display state and "state strings" for title display
bool perspective = true;
GLfloat aspectRatio;
char viewStr[25] = " front view";
char projectStr[25] = " perspective";
char baseStr[60] = "465 indexed DrawElements Example {o, p, b, f, t} : ";
char titleStr[75];

GLuint vao, vbo, ibo;  // VertexArrayObject, VertexBufferObject, IndexBufferObject
GLuint vPosition, vColor, vNormal;  // VertexAttribPointers
GLuint shaderProgram;
char * vertexShaderFile = "diffuseVertex.glsl";
char * fragmentShaderFile = "diffuseFragment.glsl";
// GLuint modelView, projection ;  // handles for uniform model - view  and projection matrices
GLuint NormalMatrix, MVP;  // ModelViewProjection handle
glm::mat4 projectionMatrix;     // set in display
glm::mat4 modelViewMatrix;      // set in display()
glm::mat3 normalMatrix;         // set in display()
glm::mat4 modelViewProjectionMatrix;  // set in display()
glm::mat4 rotationMatrix;       // set in init() and spin()

                                // vectors and values for lookAt
glm::vec3 eye, at, up;

// coordinates for "model"
static const GLfloat point[] = {
  -5.0f, -8.0f,  5.0f, 1.0f,  // 0 front left bottom 
  5.0f, -8.0f,  5.0f, 1.0f,  // 1 front right bottom
  5.0f, -8.0f, -5.0f, 1.0f,  // 2 back right bottom
  -5.0f, -8.0f, -5.0f, 1.0f,  // 3 back left bottom
  0.0f,  8.0f,  0.0f, 1.0f }; // 4 apex


                              // indexes to reuse vertex values
static const unsigned int indices[] = { // 6 faces, 3 vertices per face
  0, 1, 4, // Front Face 
  1, 2, 4,   // Right Face 
  2, 3, 4,  // Back Face 
  3, 0, 4,  // Left Face 
  3, 1, 0,  // Front Bottom Face 
  3, 2, 1 }; // Back Bottom Face 


             // vertex normals averaged from shared surface normals
static GLfloat normal[15];

// Diffuse colors for each vertex
static const GLfloat diffuseColor[] = {
  1.0f, 0.0f, 0.0f, 1.0f,  // 0, red
  0.0f, 1.0f, 0.0f, 1.0f,  // 1, green
  0.0f, 0.0f, 1.0f, 1.0f,  // 2, blue
  0.8f, 0.8f, 0.8f, 1.0f,  // 3, light gray
  0.8f, 0.8f, 0.8f, 1.0f   // 4, light gray
};

// Given a unitNormal, set the 3 coordinates in the normal array 
void makeNormal(int i, glm::vec3 unitNormal) {
  normal[i] = unitNormal.x;
  normal[i + 1] = unitNormal.y;
  normal[i + 2] = unitNormal.z;
}


void init(void) {

  // make normals from vertices shared surface normals 
  // bottom normals flipped "up" for top view, as there is no bottom view.
  makeNormal(0, glm::normalize(glm::vec3(-0.00, 0.30, 0.95) + glm::vec3(0.00, -1.00, 0.00) + glm::vec3(-0.95, 0.30, 0.00))); // front left bottom
  makeNormal(3, glm::normalize(glm::vec3(-0.00, 0.30, 0.95) + glm::vec3(0.00, -1.00, 0.00) +
    glm::vec3(0.00, -1.00, 0.00) + glm::vec3(0.95, 0.30, 0.00))); // front right bottom
  makeNormal(6, glm::normalize(glm::vec3(0.00, 0.30, -0.95) + glm::vec3(0.00, -1.00, 0.00) + glm::vec3(0.95, 0.30, 0.00))); // back right bottom
  makeNormal(9, glm::normalize(glm::vec3(0.00, 0.30, -0.95) + glm::vec3(0.00, -1.00, 0.00) +
    glm::vec3(0.00, -1.00, 0.00) + glm::vec3(-0.95, 0.30, 0.00))); // back left bottom
  makeNormal(12, glm::normalize(glm::vec3(-0.00, 0.30, 0.95) + glm::vec3(0.00, 0.30, -0.95) +
    glm::vec3(0.95, 0.30, 0.00) + glm::vec3(-0.95, 0.30, 0.00))); // apex


  shaderProgram = loadShaders(vertexShaderFile, fragmentShaderFile);
  glUseProgram(shaderProgram);

  // set up the indices buffer
  glGenBuffers(1, &ibo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  // set up the vertex attributes
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  //  initialize a buffer object
  glGenBuffers(1, &vbo);
  // glEnableVertexAttribArray(vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(point) + sizeof(diffuseColor) + sizeof(normal), NULL, GL_STATIC_DRAW);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(point), point);
  glBufferSubData(GL_ARRAY_BUFFER, sizeof(point), sizeof(diffuseColor), diffuseColor);
  glBufferSubData(GL_ARRAY_BUFFER, sizeof(point) + sizeof(diffuseColor), sizeof(normal), normal);

  // set up vertex arrays (after shaders are loaded)
  vPosition = glGetAttribLocation(shaderProgram, "vPosition");
  glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
  glEnableVertexAttribArray(vPosition);

  vColor = glGetAttribLocation(shaderProgram, "vColor");
  glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(point)));
  glEnableVertexAttribArray(vColor);

  vNormal = glGetAttribLocation(shaderProgram, "vNormal");
  glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(point) + sizeof(diffuseColor)));
  glEnableVertexAttribArray(vNormal);

  NormalMatrix = glGetUniformLocation(shaderProgram, "NormalMatrix");
  MVP = glGetUniformLocation(shaderProgram, "MVP");


  // If you want to see what these values are...
  printf("vbo %3d ibo %3d\n", vbo, ibo);
  printf("locations: vPosition %3d, vColor %3d, vNormal %3d, NormalMatrix %3d, MVP %3d\n",
    vPosition, vColor, vNormal, NormalMatrix, MVP);

  rotationMatrix = glm::mat4();

  // initially use a front view
  eye = glm::vec3(20.0f, 20.0f, 50.0f);   // eye is 50 "out of screen" from origin
  at = glm::vec3(0.0f, 0.0f, 0.0f);   // looking at origin
  up = glm::vec3(0.0f, 1.0f, 0.0f);   // camera'a up vector

                                      // set render state values
  glCullFace(GL_BACK);  // show only front faces
  glEnable(GL_DEPTH_TEST);
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
}

void reshape(int width, int height) {
  glViewport(0, 0, width, height);
  aspectRatio = (GLfloat)width / (GLfloat)height;
  printf("reshape: width = %4d height = %4d aspect = %5.2f \n", width, height, aspectRatio);
}

void display(void) {
  // printf("display %s %s \n", projectStr, viewStr);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  // update model view projection matrix
  modelViewMatrix = glm::lookAt(eye, at, up) * rotationMatrix;
  if (perspective)  // 60 degrees = 1.047 radians
    projectionMatrix = glm::perspective(glm::radians(60.0f), aspectRatio, 1.0f, 1000.0f);
  else
    projectionMatrix = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 1.0f, 1000.0f);
  // update for vertex shader
  modelViewProjectionMatrix = projectionMatrix * modelViewMatrix;
  glUniformMatrix4fv(MVP, 1, GL_FALSE, glm::value_ptr(modelViewProjectionMatrix));
  normalMatrix = glm::mat3(modelViewMatrix);
  glUniformMatrix3fv(NormalMatrix, 1, GL_FALSE, glm::value_ptr(normalMatrix));

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
  // draw 
  glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, BUFFER_OFFSET(0));
  glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y) {
  switch (key) {
  case 033: case 'q':  case 'Q': exit(EXIT_SUCCESS); break;
  case 'b': case 'B':  // front view
    eye = glm::vec3(0.0f, -50.0f, 0.0f);
    at = glm::vec3(0.0f, 0.0f, 0.0f);
    up = glm::vec3(0.0f, 0.0f, 1.0f);
    strcpy(viewStr, " bottom view");
    break;
  case 'f': case 'F':  // front view
    eye = glm::vec3(-50.0f, 20.0f, 0.0f);
    at = glm::vec3(0.0f, 0.0f, 0.0f);
    up = glm::vec3(0.0f, 1.0f, 0.0f);
    strcpy(viewStr, " front view");
    break;
  case 't': case 'T':  // top view
    eye = glm::vec3(0.0f, 50.0f, 0.0f);
    at = glm::vec3(0.0f, 0.0f, 0.0f);
    up = glm::vec3(0.0f, 0.0f, -1.0f);
    strcpy(viewStr, " top view");
    break;
  case 'p': case 'P': // set perspective projection
    perspective = true;
    strcpy(projectStr, " perspective");
    break;
  case 'o': case 'O': // set orthographic perspective projection
    perspective = false;
    strcpy(projectStr, " orthographic");
    break;
  }
  strcpy(titleStr, baseStr);
  strcat(strcat(titleStr, projectStr), viewStr);
  glutSetWindowTitle(titleStr);
  glutPostRedisplay();
}

// for use with Idle and intervalTimer functions 
// to set rotation
void spin(int i) {
  glutTimerFunc(40, spin, 1);
  rotationMatrix = glm::rotate(rotationMatrix, 0.01f, glm::vec3(0, 1, 0));
  glutPostRedisplay();
}

// free OpenGL resources
void cleanUp(void) {
  glDeleteBuffers(1, &vbo);
  glDeleteBuffers(1, &ibo);
  printf("cleaned up\n");
}

int main(int argc, char* argv[]) {
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
  glutInitWindowSize(800, 600);
  glutInitContextVersion(3, 3);
  glutInitContextProfile(GLUT_CORE_PROFILE);
  glutCreateWindow("465 indexed DrawElements Example {o, p, b, f, t} : perspective front view");
  // initialize and verify glew
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
  // initialize scene
  init();
  // set glut callback functions
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(keyboard);
  glutTimerFunc(40, spin, 1);
  glutCloseFunc(cleanUp);  // freeglut only
                           // Note: glutSetOption is only available with freeglut
  glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE,
    GLUT_ACTION_GLUTMAINLOOP_RETURNS);
  glutMainLoop();
  printf("exit main() \n");
  return 0;
}

