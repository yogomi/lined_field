// Copyright 2013 Makoto Yano

#include <iostream>
#include <math.h>
#include <stdlib.h>

#ifdef __APPLE__
#include <GLUT/glut.h>
#include <OpenGL/glu.h>
#include <OpenGL/gl.h>
#elif __linux__
#include <GL/glut.h>
#include <GL/glu.h>
#include <GL/gl.h>
#endif

#include "Leap.h"

#include "Quaternion.h"
#include "pen_line.h"
#include "field_line.h"
#include "hand_input_controller.h"

field_line::FieldLine *background_line;

/////////////////////////////////
// for Leap

static hand_controller::HandInputController hand_input_controller;

/////////////////////////////////
// for OpenGL

static int gMouseX = 0;
static int gMouseY = 0;

static const int QUIT_VALUE(99);

void reshape_func(int width, int height);

void apply_world_quaternion(Quaternion &q) {
  GLfloat m[16];
  float x2 = q[1] * q[1] * 2.0f;
  float y2 = q[2] * q[2] * 2.0f;
  float z2 = q[3] * q[3] * 2.0f;
  float xy = q[1] * q[2] * 2.0f;
  float yz = q[2] * q[3] * 2.0f;
  float zx = q[3] * q[1] * 2.0f;
  float xw = q[1] * q[0] * 2.0f;
  float yw = q[2] * q[0] * 2.0f;
  float zw = q[3] * q[0] * 2.0f;

  m[0] = 1.0f - y2 - z2;
  m[1] = xy + zw;
  m[2] = zx - yw;
  m[3] = 0.0f;

  m[4] = xy - zw;
  m[5] = 1.0 - z2 - x2;
  m[6] = yz + xw;
  m[7] = 0.0f;

  m[8] = zx + yw;
  m[9] = yz - xw;
  m[10] = 1.0f - x2 - y2;
  m[11] = 0.0f;

  m[12] = 0.0f;
  m[13] = 0.0f;
  m[14] = 0.0f;
  m[15] = 1.0f;
  glMultMatrixf(m);
}

void display_func(void) {
  hand_input_controller.process_input();

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(
    60.0f,
    (float) glutGet(GLUT_WINDOW_WIDTH)
    / (float) glutGet(GLUT_WINDOW_HEIGHT), 2.0f, 200000.0f
  );
  gluLookAt(0, 0, 0
      , 0, 0,  -300, 0, 1, 0);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glTranslated( hand_input_controller.camera_x_position
      , -hand_input_controller.camera_y_position
      , -hand_input_controller.camera_z_position);
  apply_world_quaternion(hand_input_controller.world_x_quaternion);
  apply_world_quaternion(hand_input_controller.world_y_quaternion);

  background_line->draw();

  glPushAttrib(GL_LIGHTING_BIT);
  GLfloat lmodel_ambient[] = { 1.0, 1.0, 1.0, 1.0 };
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
  for (pen_line::LineList::iterator line = hand_input_controller.penline_list.begin()
      ; line != hand_input_controller.penline_list.end(); line++) {
    if (line->size() > 3) {
      glPushMatrix();
      glLineWidth(3);
      pen_line::Line::iterator point = line->begin();
      glColor3f(point->x, point->y, point->z);
      ++point;
      glBegin(GL_LINE_STRIP);
      for(; point != line->end(); point++) {
        glVertex3f(point->x, point->y, point->z);
      }
      glEnd();
      glPopMatrix();
    }
  }
  glPopAttrib();

  glutSwapBuffers();
}

void key_func(unsigned char key, int x, int y) {
  switch (toupper(key)) {
  case 'Q':
    exit(0);
    break;
  case 'I':
    hand_input_controller.initialize_world_position();
    break;
  }
}

void skey_func(int key, int x, int y) {
  switch (key) {
  case GLUT_KEY_UP:
    break;
  case GLUT_KEY_DOWN:
    break;
  case GLUT_KEY_LEFT:
    break;
  case GLUT_KEY_RIGHT:
    break;

  }

  glutPostRedisplay();
}

void reshape_func(int width, int height) {
  glViewport(0, 0, width, height);
}

static void MouseCallback(int button, int state, int x, int y) {
  gMouseX = x;
  gMouseY = y;
}

static void MotionCallback(int x, int y) {
  gMouseX = x;
  gMouseY = y;
}

static void mainMenuCB(int value) {
  if (value == QUIT_VALUE) {
    exit(0);
  }
}

void idle_func(void) {
  glutPostRedisplay();
}

static void ExitNx() {
}

static void init() {
  glutDisplayFunc(display_func);

  glutReshapeFunc(reshape_func);

  glutKeyboardFunc(key_func);
  glutSpecialFunc(skey_func);
  glutMouseFunc(MouseCallback);
  glutMotionFunc(MotionCallback);
  MotionCallback(0, 0);

  glutIdleFunc(idle_func);

  glEnable(GL_DEPTH_TEST);

  atexit(ExitNx);

  GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
  GLfloat mat_shininess[] = { 100.0 };
  GLfloat light_position[] = { 1.0, 1.0, 1.0, 0.0 };
  GLfloat white_light[] = { 1.0, 1.0, 1.0, 0.0 };
  GLfloat lmodel_ambient[] = { 0.1, 0.1, 0.1, 1.0 };
  glClearColor(0, 0, 0, 1.0f);
  glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
  glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
  glLightfv(GL_LIGHT0, GL_POSITION, light_position);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, white_light);
  glLightfv(GL_LIGHT0, GL_SPECULAR, white_light);
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
  glShadeModel(GL_SMOOTH);

  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_COLOR_MATERIAL);
  background_line = new field_line::FieldLine();
  hand_input_controller.initialize_world_position();
  float hard = Leap::PI / 32;
  float s = sin(hard);
  Quaternion rotate_quaternion(cos(hard), 1*s, 0*s, 0*s);
}


int main(int argc, char *argv[])
{
  std::cout << "start" << std::endl;

  glutInit(&argc, argv);

  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);

  glutInitWindowSize(800, 600);

  int mainHandle = glutCreateWindow("Sample 1");
  glutSetWindow(mainHandle);

  init();
  using namespace std;
  glutMainLoop();

  std::cout << "end" << std::endl;
  return 0;
}

