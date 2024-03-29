/*
 * types.h: modularizing the code (proj1.c got too big).
 *
 */
#ifndef TYPES_HAS_BEEN_INCLUDED
#define TYPES_HAS_BEEN_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __APPLE__
#  include <OpenGL/gl3.h>
#else
#  include <GL/gl3.h>
#endif

#include <AntTweakBar.h>

#include "spot.h"

/*
** A string to use as title bar name for the "tweak bar"
*/
#define TBAR_NAME "Project2-Params"

// NOTE: Shaders are populated in our main
#define NUM_PROGRAMS 7
// NOTE: easy program lookup--refer to programIds[ID_${shader}] for the id to use with
//       `glLinkProgram'
#define ID_CUBE 0
#define ID_SIMPLE 1
#define ID_PHONG 2
#define ID_TEXTURE 3
#define ID_BUMP 4
#define ID_PARALLAX 5
#define ID_SPOTLIGHT 6
#define ID_PLANETS 7

enum BumpMappingModes {Disabled, Bump, Parallax};
enum FilteringModes {Nearest, Linear, NearestWithMipmap, LinearWithMipmap};
enum Objects {Sun, Mercury, Venus, Earth, Mars, Jupiter, Saturn, Uranus, Neptune, Pluto};
enum CubeMaps {CubeSample, CubeCool, CubePlace};
enum Shaders {PhongShader, CubeShader, SpotlightShader};

/*
** The camera_t is a suggested storage place for all the parameters associated
** with determining how you see the scene, which is used to determine one of
** the transforms applied in the vertex shader.  Right now there are no helper
** functions to initialize or compute with the camera_t; that is up to you.
**
*/
typedef struct {
  GLfloat from[3],    /* location (in world-space) of eyepoint */
    at[3],            /* what point (in world-space) we are looking at */
    up[3],            /* what is up direction for eye (this is not updated to
                         the "true" up) */
    aspect,           /* the ratio of horizontal to vertical size of the view
                         window */
    fov,              /* The angle, in degrees, vertically subtended by the
                         near clipping plane */
    near, far;        /* near and far clipping plane distances.  Whether you
                         interpret these as relative to the eye "from" point
                         (the convention in graphics) or relative to the
                         "at" point (arguably more convenient) is up to you */
  int ortho,          /* (a boolean) no perspective projection: just
                         orthographic */
      fixed;
  GLfloat uvn[4*4];
  GLfloat inverse_uvn[4*4];
  GLfloat proj[4*4];

  GLfloat wf, hf;

} camera_t;

typedef struct{
  GLfloat xyzw[4*4];
  GLfloat custom[4*4];
} model_t;

typedef struct {
  GLfloat *m;
  void (*f)(GLfloat*, GLfloat*, size_t);
  GLfloat offset, multiplier;
  int i;
} mouseFun_t;

/*
** The uniloc_t is a possible place to store "locations" of shader
** uniform variables, so they can be learned once and re-used once per
** render.  Modify as you see fit! 
*/
typedef struct {
  GLint modelMatrix;  /* same name as field in spotGeom */
  GLint normalMatrix; /* same name as field in spotGeom */
  GLint objColor;     /* same name as field in spotGeom */
  GLint Ka;           /* same name as field in spotGeom */
  GLint Kd;           /* same name as field in spotGeom */
  GLint Ks;           /* same name as field in spotGeom */
  GLint shexp;        /* same name as field in spotGeom */
  GLint gi;           /* index of spotGeom object */
  /* vvvvvvvvvvvvvvvvvvvvv YOUR CODE HERE vvvvvvvvvvvvvvvvvvvvvvvv */
  GLint viewMatrix;   /* possible name of view matrix in vertex shader */
  GLint inverseViewMatrix;   /* possible name of view matrix in vertex shader */
  GLint projMatrix;   /* possible name of projection matrix in vertex shader */
  /* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */
  GLint spotPoint;    /* point in view space coords that spot comes from */
  GLint penumbra; 
	GLint spotUp;
	GLint spotMatrix;
  GLint rStart, rEnd;
  GLint lightDir;     /* same name as field in context_t */
  GLint lightColor;   /* same name as field in context_t */
  GLint gouraudMode;  /* same name as field in context_t */
  GLint seamFix;
  GLint sampler0;     /* possible name of texture sampler in fragment shader */
  GLint sampler1;     /* possible name of texture sampler in fragment shader */
  GLint sampler2;     /* possible name of texture sampler in fragment shader */
  GLint sampler3;     /* possible name of texture sampler in fragment shader */
  GLint sampler4;     /* possible name of texture sampler in fragment shader */
  GLint sampler5;     /* possible name of texture sampler in fragment shader */
  GLint sampler6;     /* possible name of texture sampler in fragment shader */
  GLint sampler7;     /* possible name of texture sampler in fragment shader */
  GLint sampler8;     /* possible name of texture sampler in fragment shader */
  GLint sampler9;     /* possible name of texture sampler in fragment shader */
  GLint cubeMap;     /* possible name of texture sampler in fragment shader */
  GLint Zu, Zv, Zspread;
} uniloc_t;

/*
** The context_t is a suggested storage place for what might otherwise be
** separate global variables (globals obscure the flow of information and are
** hence bad style).  Modify as you see fit.  Don't forget to respect the
** order of operations on spotGeom:
**     initialization: sgeom = spotGeomNewSphere(); (for example)
**                     spotGeomGLInit(sgeom);
**     rendering loop: ... spotGeomDraw(sgeom); ...
**     cleaning up:    spotGeomGLDone(sgeom);
**                     sgeom = spotGeomNix(sgeom); (sets sgeom to NULL)
*/
typedef struct {
  const char *vertFname,  /* file name of vertex shader */
    *fragFname;           /* file name of fragment shader */
  spotGeom **geom;        /* array of spotGeom's to render */
  GLint gi;               /* index of spotGeom object currently in use */
  unsigned int geomNum;   /* length of geom */
  spotImage **image;      /* array of texture images to use */
  unsigned int imageNum;  /* length of image */
  GLfloat bgColor[3];     /* background color */
  GLfloat lightDir[3];    /* direction pointing to light (at infinity) */
  GLfloat lightColor[3];  /* color of light */
  int running;            /* we exit when this is zero */
  GLint program;          /* the linked shader program */
  int winSizeX, winSizeY; /* size of rendering window */

  int tbarSizeX,          /* initial width of tweak bar */
    tbarSizeY,            /* initial height of tweak bar */
    tbarMargin;           /* margin between tweak bar and window */

  camera_t camera,        /* a camera */
    spotlight;            /* a spotlight */
  uniloc_t uniloc;        /* store of uniform locations */
  model_t model;

  int lastX, lastY;       /* coordinates of last known mouse position */
  int buttonDown,         /* mouse button is being held down */
    shiftDown;            /* shift was down at time of mouse click */
  int viewMode,           /* 1 when in view mode, 0 otherwise */
    modelMode,            /* 1 when in model mode, 0 otherwise */
    lightMode,            /* 1 when in light mode, 0 otherwise */
    gouraudMode,          /* 1 when in gouraud mode, 0 otherwise */
    perVertexTexturingMode,
    seamFix,
    spinning,
    paused,
    cubeMapId;
  enum BumpMappingModes bumpMappingMode;
  enum FilteringModes filteringMode;
  GLint minFilter, magFilter;
  TwBar *tbar;            /* pointer to the parameter "tweak bar" */
  /* (any other information about the state of mouse or keyboard
     input, geometry, camera, transforms, or anything else that may
     need to be accessed from anywhere */
  mouseFun_t mouseFun;
  /* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */
  GLfloat vspNear, vspFar,  U[3], V[3], N[3];
  GLfloat Zu, Zv, Zspread;
  double ticDraw,         /* last abs time contextDraw was called */
    ticMouse;             /* last abs time callbackMousePos was called */
  GLfloat thetaPerSecU,   /* radians per second along U */
    thetaPerSecV,         /* radians per second along V */
    thetaPerSecN;
  GLfloat time;
  GLfloat angleU,         /* abs angle of beam along U */
    angleV,               /* abs angle of beam along V */
    angleN;
  int onlyN;
} context_t;

#ifdef __cplusplus
}
#endif

#endif /* TYPES_HAS_BEEN_INCLUDED */
