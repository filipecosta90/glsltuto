#include <GL/glew.h>
#ifdef __APPLE__
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif
#include <GL/glut.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>

#include "chemfiles.hpp"


#define ROTATE 1
#define RENDER_SPHERES_INSTEAD_OF_VERTICES 1

#define XYZFMT "%-4s %11.6lf %11.6lf %11.6lf"

GLuint  prog_hdlr;
GLint location_attribute_0, location_viewport;

 int NATOMS        = 1000;
const int SCREEN_WIDTH  = 1024;
const int SCREEN_HEIGHT = 1024;
const float camera[]           = {40,0,40};
const float light0_position[4] = {1,1,1,0};
std::vector<std::vector<float> > atoms;
float angle = 0.72;

float rand_minus_one_one() {
  return (float)rand()/(float)RAND_MAX*(rand()>RAND_MAX/2?1:-1);
}

float rand_zero_one() {
  return (float)rand()/(float)RAND_MAX;
}

float cur_camera[] = {0,0,0};

void render_scene(void) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity();
  cur_camera[0] = cos(angle)*camera[0]+sin(angle)*camera[2];
  cur_camera[1] = camera[1];
  cur_camera[2] = cos(angle)*camera[2]-sin(angle)*camera[0];
#if ROTATE
  angle+=0.01;
#endif
  gluLookAt(cur_camera[0], cur_camera[1], cur_camera[2], 0,  0, 0, 0, 1, 0);

#if RENDER_SPHERES_INSTEAD_OF_VERTICES
  for (int i=0; i<NATOMS; i++) {
    glColor3f(atoms[i][4], atoms[i][5], atoms[i][6]);
    glPushMatrix();
    glTranslatef(atoms[i][0], atoms[i][1], atoms[i][2]);
    glutSolidSphere(atoms[i][3], 16, 16);
    glPopMatrix();
  }
#else
  glUseProgram(prog_hdlr);
  GLfloat viewport[4];
  glGetFloatv(GL_VIEWPORT, viewport);
  glUniform4fv(location_viewport, 1, viewport);
  glBegin(GL_POINTS);
  for (int i=0; i<NATOMS; i++) {
    glColor3f(atoms[i][4], atoms[i][5], atoms[i][6]);
    glVertexAttrib1f(location_attribute_0, atoms[i][3]);
    glVertex3f(atoms[i][0], atoms[i][1], atoms[i][2]);
  }
  glEnd();
  glUseProgram(0);
#endif

  glutSwapBuffers();
}

void process_keys(unsigned char key, int x, int y) {
  if (27==key) {
    exit(0);
  }
}

void change_size(int w, int h) {
  float ratio = (1.0*w)/(!h?1:h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(90, ratio, .1, 100);
  //    glOrtho(-1,1,-1,1,-2,2);
  glMatrixMode(GL_MODELVIEW);
  glViewport(0, 0, w, h);
}

#if !RENDER_SPHERES_INSTEAD_OF_VERTICES
void printInfoLog(GLuint obj) {
  int log_size = 0;
  int bytes_written = 0;
  glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &log_size);
  if (!log_size) return;
  char *infoLog = new char[log_size];
  glGetProgramInfoLog(obj, log_size, &bytes_written, infoLog);
  std::cerr << infoLog << std::endl;
  delete [] infoLog;
}

bool read_n_compile_shader(const char *filename, GLuint &hdlr, GLenum shaderType) {
  std::ifstream is(filename, std::ios::in|std::ios::binary|std::ios::ate);
  if (!is.is_open()) {
    std::cerr << "Unable to open file " << filename << std::endl;
    return false;
  }
  long size = is.tellg();
  char *buffer = new char[size+1];
  is.seekg(0, std::ios::beg);
  is.read (buffer, size);
  is.close();
  buffer[size] = 0;

  hdlr = glCreateShader(shaderType);
  glShaderSource(hdlr, 1, (const GLchar**)&buffer, NULL);
  glCompileShader(hdlr);
  std::cerr << "info log for " << filename << std::endl;
  printInfoLog(hdlr);
  delete [] buffer;
  return true;
}

void setShaders(GLuint &prog_hdlr, const char *vsfile, const char *fsfile) {
  GLuint vert_hdlr, frag_hdlr;
  read_n_compile_shader(vsfile, vert_hdlr, GL_VERTEX_SHADER);
  read_n_compile_shader(fsfile, frag_hdlr, GL_FRAGMENT_SHADER);

  prog_hdlr = glCreateProgram();
  glAttachShader(prog_hdlr, frag_hdlr);
  glAttachShader(prog_hdlr, vert_hdlr);

  glLinkProgram(prog_hdlr);
  std::cerr << "info log for the linked program" << std::endl;
  printInfoLog(prog_hdlr);
}
#endif


int main(int argc, char **argv) {


  chemfiles::Trajectory trajectory("../tdmap_roi.xyz");
  auto frame = trajectory.read();
  NATOMS = frame.natoms();
  std::cout << "There are " << frame.natoms() << " atoms in the frame" << std::endl;

  auto positions = frame.positions();
  auto topology = frame.topology();

    for (size_t i=0; i<frame.natoms(); i++) {
      auto atom = topology[i];
      std::cout << "atom " << i << " " << atom.name() << "vdw_radius " << atom.vdw_radius() << " covalent_radius " << atom.covalent_radius() << std::endl;

      std::vector<float> tmp;
      tmp.push_back(positions[i][0] );
      tmp.push_back(positions[i][1] );
      tmp.push_back(positions[i][2] );
      tmp.push_back( atom.covalent_radius() ); // radius
      tmp.push_back( atom.color_r());
      tmp.push_back( atom.color_g());
      tmp.push_back( atom.color_b());
      atoms.push_back(tmp);
    }

  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowPosition(100,100);
  glutInitWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);
  glutCreateWindow("GLSL tutorial");
  glClearColor(0.0,0.0,1.0,1.0);

  glutDisplayFunc(render_scene);
  glutReshapeFunc(change_size);
  glutKeyboardFunc(process_keys);
#if ROTATE
  glutIdleFunc(render_scene);
#endif

  glEnable(GL_COLOR_MATERIAL);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glLightfv(GL_LIGHT0, GL_POSITION, light0_position);

#if !RENDER_SPHERES_INSTEAD_OF_VERTICES
  glewInit();
  if (GLEW_ARB_vertex_shader && GLEW_ARB_fragment_shader && GL_EXT_geometry_shader4)
    std::cout << "Ready for GLSL - vertex, fragment, and geometry units" << std::endl;
  else {
    std::cout << "No GLSL support" << std::endl;
    exit(1);
  }
  setShaders(prog_hdlr, "shaders/vert_shader.glsl", "shaders/frag_shader.glsl");

  location_attribute_0   = glGetAttribLocation(prog_hdlr, "R");          // radius
  location_viewport = glGetUniformLocation(prog_hdlr, "viewport"); // viewport

  glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
#endif
  glutMainLoop();
  return 0;
}
