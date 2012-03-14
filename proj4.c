// Project 2: by Mark (andrus) and Shir (shiryehoshua)
//
// Please see Mark's dir for source (andrus)
//
//
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h> // For UCHAR_MAX and friends...

#define __gl_h_
#define GLFW_NO_GLU // Tell glfw.h not to include GLU header
#include <GL/glfw.h>
#undef GLFW_NO_GLU
#undef __gl_h_

#include <AntTweakBar.h>

// Local includes
#include "callbacks.h"
#include "matrixFunctions.h"
#include "spot.h"
#include "types.h"

// NOTE: this is how we support our stack of shaders; we define each we want to load in
//       `glInitContext()' of our program, load them once, and leave them attached until the app
//       terminates
const char *vertFnames[NUM_PROGRAMS], // Our list of shaders (populated in `contextGLInit()');
           *fragFnames[NUM_PROGRAMS]; // see `types.h' for the definition of NUM_PROGRAMS
int programIds[NUM_PROGRAMS+1];       // List of corresponding program ids (for `glUseProgram()')

// Global context
context_t *gctx = NULL;

// Values for tweak bar
TwType twBumpMappingModes, twFilteringModes, twObjects, twCubeMaps, twShaders;
TwEnumVal twBumpMappingModesEV[]={{Disabled, "Disabled"},
                                  {Bump, "Bump"},
                                  {Parallax, "Parallax"}},
          twFilteringModesEV[]  ={{Nearest, "Nearest"},
                                  {Linear, "Linear"},
                                  {NearestWithMipmap, "NearestWithMipmap"},
                                  {LinearWithMipmap, "LinearWithMipmap"}},
          twObjectsEV[]          ={{Sun, "Sun"},
                                  {Mercury, "Mercury"},
                                  {Venus, "Venus"},
                                  {Earth, "Earth"},
                                  {Mars, "Mars"},
                                  {Jupiter, "Jupiter"},
                                  {Saturn, "Saturn"},
                                  {Uranus, "Uranus"},
                                  {Neptune, "Neptune"},
                                  {Pluto, "Pluto"}},
          twCubeMapsEV[]         ={{CubeSample, "cube-sample.png"},
                                  {CubeCool, "cube-cool.png"},
                                  {CubePlace, "cube-place.png"}},
          twShadersEV[]          ={{PhongShader, "phong.{vert,frag}"},
                                  {CubeShader, "cube.{vert,frag}"},
                                  {SpotlightShader, "spotlight.{vert,frag}"}};


// NOTE: we'd prefer to only draw one shape at a time, while keeping a sphere and
//       square in memory. This variable gets referenced in contextDraw and does just
//       that...

/* Creates a context around geomNum spotGeom's and
   imageNum spotImage's */
context_t *contextNew(unsigned int geomNum, unsigned int imageNum) {
  const char me[]="contextNew";
  context_t *ctx;
  unsigned int gi;
  
  ctx = (context_t *)calloc(1, sizeof(context_t));
  if (!ctx) {
    spotErrorAdd("%s: couldn't alloc context?", me);
    return NULL;
  }

  ctx->vertFname = NULL;
  ctx->fragFname = NULL;
  if (geomNum) {
    ctx->geom = (spotGeom **)calloc(geomNum, sizeof(spotGeom*));
    if (!ctx->geom) {
      spotErrorAdd("%s: couldn't alloc %u geoms", me, geomNum);
      free(ctx); return NULL;
    }
    for (gi=0; gi<geomNum; gi++) {
      ctx->geom[gi] = NULL;
    }
  } else {
    ctx->geom = NULL;
  }
  ctx->geomNum = geomNum;
  if (imageNum) {
    ctx->image = (spotImage **)calloc(imageNum, sizeof(spotImage*));
    if (!ctx->image) {
      spotErrorAdd("%s: couldn't alloc %u images", me, imageNum);
      free(ctx); return NULL;
    }
    for (gi=0; gi<imageNum; gi++) {
      ctx->image[gi] = spotImageNew();
    }
  } else {
    ctx->image = NULL;
  }
  ctx->imageNum = imageNum;
  SPOT_V3_SET(ctx->bgColor, 0.0f, 0.0f, 0.0f);
  SPOT_V3_SET(ctx->lightDir, 1.0f, 0.0f, 0.0f);
  SPOT_V3_SET(ctx->lightColor, 1.0f, 1.0f, 1.0f);
  ctx->running = 1;
  ctx->program = 0;
  ctx->winSizeX = 1500;
  ctx->winSizeY = 900;
  ctx->tbarSizeX = 200;
  ctx->tbarSizeY = 300;
  ctx->tbarMargin = 20;
  ctx->lastX = ctx->lastY = -1;
  ctx->buttonDown = 0;
  ctx->shiftDown = 0;
  ctx->Zspread = 0.003;

  // create the objects
  ctx->geom[0] = spotGeomNewSphere(); // Sun
  ctx->geom[1] = spotGeomNewSphere(); // Mercury
  ctx->geom[2] = spotGeomNewSphere(); // Venus
  ctx->geom[3] = spotGeomNewSphere(); // Earth
  ctx->geom[4] = spotGeomNewSphere(); // Mars
  ctx->geom[5] = spotGeomNewSphere(); // Jupiter
  ctx->geom[6] = spotGeomNewSphere(); // Saturn
  ctx->geom[7] = spotGeomNewSphere(); // Uranus
  ctx->geom[8] = spotGeomNewSphere(); // Neptune
  ctx->geom[9] = spotGeomNewSphere(); // Pluto

  // color the objects
  SPOT_V3_SET(ctx->geom[0]->objColor, 1.0f, 0.5f, 0.0f); // Sun
  SPOT_V3_SET(ctx->geom[1]->objColor, 0.8f, 0.8f, 0.8f); // Mercury
  SPOT_V3_SET(ctx->geom[2]->objColor, 0.7f, 0.7f, 1.0f); // Venus
  SPOT_V3_SET(ctx->geom[3]->objColor, 0.1f, 0.7f, 1.0f); // Earth
  SPOT_V3_SET(ctx->geom[4]->objColor, 1.0f, 0.5f, 0.0f); // Mars
  SPOT_V3_SET(ctx->geom[5]->objColor, 1.0f, 0.7f, 0.1f); // Jupiter
  SPOT_V3_SET(ctx->geom[6]->objColor, 0.8f, 0.8f, 1.0f); // Saturn
  SPOT_V3_SET(ctx->geom[7]->objColor, 0.2f, 0.8f, 1.0f); // Uranus
  SPOT_V3_SET(ctx->geom[8]->objColor, 1, 1, 0); // Neptune
  SPOT_V3_SET(ctx->geom[9]->objColor, 1, 1, 0); // Pluto

  // set object radius
  ctx->geom[0]->radius = 0.000f;
  ctx->geom[1]->radius = 2.105f;
  ctx->geom[2]->radius = 2.322f; 
  ctx->geom[3]->radius = 2.500f;
  ctx->geom[4]->radius = 2.838f;
  ctx->geom[5]->radius = 5.210f;
  ctx->geom[6]->radius = 8.007f;
  ctx->geom[7]->radius = 12.23f;
  ctx->geom[8]->radius = 14.25f;
  ctx->geom[9]->radius = 18.34f;

  // set object orbit axis
  SPOT_V3_SET(ctx->geom[0]->orbitAxis, 0.0f, 0.0f, 0.0f); 
  SPOT_V3_SET(ctx->geom[1]->orbitAxis, 0.0f, 1.0f, 0.0f); 
  SPOT_V3_SET(ctx->geom[2]->orbitAxis, 0.0f, 1.0f, 0.0f); 
  SPOT_V3_SET(ctx->geom[3]->orbitAxis, 0.0f, 1.0f, 0.0f); 
  SPOT_V3_SET(ctx->geom[4]->orbitAxis, 0.0f, 1.0f, 0.0f); 
  SPOT_V3_SET(ctx->geom[5]->orbitAxis, 0.0f, 1.0f, 0.0f); 
  SPOT_V3_SET(ctx->geom[6]->orbitAxis, 0.0f, 1.0f, 0.0f); 
  SPOT_V3_SET(ctx->geom[7]->orbitAxis, 0.0f, 1.0f, 0.0f); 
  SPOT_V3_SET(ctx->geom[8]->orbitAxis, 0.0f, 1.0f, 0.0f); 
  SPOT_V3_SET(ctx->geom[9]->orbitAxis, 0.0f, 1.0f, 0.0f); 

  for (gi=0; gi < geomNum; gi ++) {
    translateGeomU(ctx->geom[gi], ctx->geom[gi]->radius);
  }

  // set axialThetaPerSec
  ctx->geom[0]->axialThetaPerSec = 0.01f;
  ctx->geom[1]->axialThetaPerSec = 0.01f; 
  ctx->geom[2]->axialThetaPerSec = 0.01f; 
  ctx->geom[3]->axialThetaPerSec = 0.01f; 
  ctx->geom[4]->axialThetaPerSec = 0.01f; 
  ctx->geom[5]->axialThetaPerSec = 0.01f; 
  ctx->geom[6]->axialThetaPerSec = 0.01f; 
  ctx->geom[7]->axialThetaPerSec = 0.01f; 
  ctx->geom[8]->axialThetaPerSec = 0.01f; 
  ctx->geom[9]->axialThetaPerSec = 0.01f; 

  // set orbitThetaPerSec
  ctx->geom[0]->orbitThetaPerSec = 0.01f;
  ctx->geom[1]->orbitThetaPerSec = 0.01f;
  ctx->geom[2]->orbitThetaPerSec = 0.01f;
  ctx->geom[3]->orbitThetaPerSec = 0.01f;
  ctx->geom[4]->orbitThetaPerSec = 0.01f;
  ctx->geom[5]->orbitThetaPerSec = 0.01f;
  ctx->geom[6]->orbitThetaPerSec = 0.01f;
  ctx->geom[7]->orbitThetaPerSec = 0.01f;
  ctx->geom[8]->orbitThetaPerSec = 0.01f;
  ctx->geom[9]->orbitThetaPerSec = 0.01f;

  // scale the objects so that they resemble true dimensions
  scaleGeom(ctx->geom[0], 2.000f); // Sun
  scaleGeom(ctx->geom[1], 0.035f); // Mercury
  scaleGeom(ctx->geom[2], 0.086f); // Venus
  scaleGeom(ctx->geom[3], 0.091f); // Earth
  scaleGeom(ctx->geom[4], 0.048f); // Mars
  scaleGeom(ctx->geom[5], 1.027f); // Jupiter
  scaleGeom(ctx->geom[6], 0.836f); // Saturn
  scaleGeom(ctx->geom[7], 0.337f); // Uranus
  scaleGeom(ctx->geom[8], 0.326f); // Neptune
  scaleGeom(ctx->geom[9], 0.016f); // Pluto

  // set orientation, and lighting constants
  for (gi=0; gi < geomNum; gi ++) {
    SPOT_V4_SET(ctx->geom[gi]->quaternion, 1.0f, 0.0f, 0.0f, 0.0f);
    rotate_model_ith(ctx->geom[gi], 0.75, 0);
    ctx->geom[gi]->Kd = 0.4;
    ctx->geom[gi]->Ks = 0.3;
    ctx->geom[gi]->Ka = 0.3;
  }

  // load images
  spotImageLoadPNG(ctx->image[0], "textimg/sun.png");     // Sun
  spotImageLoadPNG(ctx->image[1], "textimg/mercury.png"); // Mercury 
  spotImageLoadPNG(ctx->image[2], "textimg/venus.png");   // Venus
  spotImageLoadPNG(ctx->image[3], "textimg/earth.png");   // Earth
  spotImageLoadPNG(ctx->image[4], "textimg/mars.png");    // Mars
  spotImageLoadPNG(ctx->image[5], "textimg/jupiter.png"); // Jupiter
  spotImageLoadPNG(ctx->image[6], "textimg/saturn.png");  // Saturn
  spotImageLoadPNG(ctx->image[7], "textimg/uranus.png");  // Uranus
  spotImageLoadPNG(ctx->image[8], "textimg/neptune.png"); // Neptune
  spotImageLoadPNG(ctx->image[9], "textimg/pluto.png");   // Pluto

  ctx->ticDraw = -1;
  ctx->ticMouse = -1;
  ctx->thetaPerSecU = 0;
  ctx->thetaPerSecV = 0;
  ctx->thetaPerSecN = 0;
  ctx->onlyN = 0;

  ctx->angleU = 0;
  ctx->angleV = 0;
  ctx->angleN = 0;

  ctx->gi = 5;

  return ctx;
}

// NOTE: it makes sense to let this be its own function, since we need to call it upon changing
//       gctx->program in our shaders
void setUnilocs() {
  /* Learn (once) locations of uniform variables that we will
     frequently set */
#define SET_UNILOC(V) gctx->uniloc.V = glGetUniformLocation(gctx->program, #V)
      SET_UNILOC(lightDir);
      SET_UNILOC(spotPoint);
      SET_UNILOC(penumbra);
      SET_UNILOC(rStart);
      SET_UNILOC(rEnd);
      SET_UNILOC(spotUp);
      SET_UNILOC(lightColor);
      SET_UNILOC(modelMatrix);
      SET_UNILOC(normalMatrix);
      SET_UNILOC(viewMatrix);
      SET_UNILOC(inverseViewMatrix);
      SET_UNILOC(projMatrix);
      SET_UNILOC(objColor);
      SET_UNILOC(gi);
      SET_UNILOC(Ka);
      SET_UNILOC(Kd);
      SET_UNILOC(Ks);
      SET_UNILOC(gouraudMode);
      SET_UNILOC(seamFix);
      SET_UNILOC(shexp);
      SET_UNILOC(sampler0);
      SET_UNILOC(sampler1);
      SET_UNILOC(sampler2);
      SET_UNILOC(sampler3);
      SET_UNILOC(sampler4);
      SET_UNILOC(sampler5);
      SET_UNILOC(sampler6);
      SET_UNILOC(sampler7);
      SET_UNILOC(sampler8);
      SET_UNILOC(sampler9);
      SET_UNILOC(Zu);
      SET_UNILOC(Zv);
      SET_UNILOC(Zspread);
#undef SET_UNILOC;
}

int contextGLInit(context_t *ctx) {
  const char me[]="contextGLInit";
  unsigned int ii, i;

  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glDisable(GL_CULL_FACE); // No backface culling for now
  glEnable(GL_DEPTH_TEST); // Yes, do depth testing

  /* Create shader program.  Note that the names of per-vertex attributes
     are specified here.  This includes  vertPos and vertNorm from last project
     as well as new vertTex2 (u,v) per-vertex texture coordinates, and the
     vertTang per-vertex surface tangent 3-vector. */

  // NOTE: here is our shader "stack"; the ID_${shader} definitions allow easy retrieval of the
  //       program id from the programIds array after `glLinkProgram' calls
  vertFnames[ID_CUBE]="cube.vert";
  fragFnames[ID_CUBE]="cube.frag";
  vertFnames[ID_SIMPLE]="simple.vert";
  fragFnames[ID_SIMPLE]="simple.frag";
  vertFnames[ID_PHONG]="phong.vert";
  fragFnames[ID_PHONG]="phong.frag";
  vertFnames[ID_TEXTURE]="texture.vert";
  fragFnames[ID_TEXTURE]="texture.frag";
  vertFnames[ID_BUMP]="bump.vert";
  fragFnames[ID_BUMP]="bump.frag";
  vertFnames[ID_PARALLAX]="parallax.vert";
  fragFnames[ID_PARALLAX]="parallax.frag";
  vertFnames[ID_SPOTLIGHT]="spotlight.vert";
  fragFnames[ID_SPOTLIGHT]="spotlight.frag";

  // NOTE: we loop for as many shaders as are in our "stack" (NUM_PROGRAMS), and then once more
  //       to pull in whatever shader was passed in via the terminal (or not, if we have
  //       ctx->vertName==NULL)
  const char *vertFname, *fragFname;
  for (i=0; i<=NUM_PROGRAMS-(ctx->vertFname==NULL?1:0); i++) {
    // NOTE: consider this the "invoked" or default shader paseed via the terminal; it will be
    //       loaded last, and thus the first shader visible
    if (i==NUM_PROGRAMS) {
      vertFname=ctx->vertFname;
      fragFname=ctx->fragFname;
    // otherwise, we want to load our "stack" of shaders
    } else {
      vertFname = vertFnames[i];
      fragFname = fragFnames[i];
    }
    // NOTE: use `spotProgramNew' to handle all the `glLinkProgram' specifics; we also specify the
    //       per-vertex attributes we need
    ctx->program = spotProgramNew(vertFname, fragFname,
                                  "vertPos", spotVertAttrIndx_xyz,
                                  "vertNorm", spotVertAttrIndx_norm,
                                  "vertTex2", spotVertAttrIndx_tex2,
                                  "vertRgb", spotVertAttrIndx_rgb,
                                  "vertTang", spotVertAttrIndx_tang,
                                  /* input name, attribute index pairs
                                     MUST BE TERMINATED with NULL */
                                  NULL);
    // NOTE: we save the program id for easy retrieval from our callbacks; i here corresponds to
    //       one of ID_SIMPLE, ID_PHONG, etc., so we can reset the gctx->program to
    //       programIds[ID_${shader}] to switch shaders
    programIds[i]=ctx->program;
    if (!ctx->program) {
      spotErrorAdd("%s: couldn't create shader program", me);
      return 1;
    } else {
      printf("%d: Program (%s,%s) loaded...\n", ctx->program, vertFname, fragFname);
    }
  }

  // NOTE: the following is equivalent to hitting '1' on the keyboard; i.e. default
  //       scene
  if (ctx->vertFname==NULL) {
    gctx->program=programIds[ID_TEXTURE];
  }

  // NOTE: this sets the uniform locations for the _invoked_ shader
  setUnilocs();
  
  if (ctx->geom) {
    for (ii=0; ii<ctx->geomNum; ii++) {
      if (spotGeomGLInit(ctx->geom[ii])) {
        spotErrorAdd("%s: trouble with geom[%u]", me, ii);
        return 1;
      }
    }
  }
  if (ctx->image) {
    for (ii=0; ii<ctx->imageNum; ii++) {
				printf("ii: %d\n", ii);
      if (ctx->image[ii]->data.v) {
        // Only bother with GL init when image data has been set
/*
        if (ii=10) { //ii==4 || ii==5 || ii==6) { // 5 is our cubemap
          if (spotImageCubeMapGLInit(ctx->image[ii])) {
            spotErrorAdd("%s: trouble with image[%u]", me, ii);
            return 1;
          } else {
            printf("cubeMap: %d\n", ii);
          }
        } else 
*/
        if (spotImageGLInit(ctx->image[ii])) {
          spotErrorAdd("%s: trouble with image[%u]", me, ii);
          return 1;
        }
      }
    }
  }

  fprintf(stderr, "sucessfully initialized images\n");

  // NOTE: set to view mode (default)
  gctx->viewMode = 1;
  gctx->modelMode = 0;
  gctx->lightMode = 0;
  gctx->gouraudMode = 1;
  gctx->seamFix = 0;
  gctx->spinning = 0;
  gctx->paused = 0;
  gctx->minFilter = GL_NEAREST;
  gctx->magFilter = GL_NEAREST;
  gctx->time = 0.0f;
//  perVertexTexturing();

  // NOTE: model initializations
  SPOT_M4_IDENTITY(gctx->model.xyzw);
  SPOT_M4_IDENTITY(gctx->model.custom);

  // NOTE: camera initializations
  SPOT_M4_IDENTITY(gctx->camera.uvn);
  SPOT_M4_IDENTITY(gctx->camera.inverse_uvn);
  SPOT_M4_IDENTITY(gctx->camera.proj);
  gctx->camera.ortho = 0; // start in perspective mode
  gctx->camera.fixed = 0;
  gctx->camera.fov = 1.57079633/10; // 90 degrees
  gctx->camera.near = -20;
  gctx->camera.far = 20;
  gctx->camera.up[0] = 0;
  gctx->camera.up[1] = 1;
  gctx->camera.up[2] = 0;
  gctx->camera.from[0] = 0;
  gctx->camera.from[1] = -2.0f;
  gctx->camera.from[2] = 25.0f;
  gctx->camera.at[0] = 0;
  gctx->camera.at[1] = 0;
  gctx->camera.at[2] = 0;

  // NOTE: Mouse function intializations
  gctx->mouseFun.m = NULL;
  gctx->mouseFun.f = identity;
  gctx->mouseFun.offset=gctx->mouseFun.multiplier=gctx->mouseFun.i = 0;

  return 0;
}

int contextGLDone(context_t *ctx) {
  const char me[]="contextGLDone";
  unsigned int ii;

  if (!ctx) {
    spotErrorAdd("%s: got NULL pointer", me);
    return 1;
  }
  if (ctx->geom) {
    for (ii=0; ii<ctx->geomNum; ii++) {
      spotGeomGLDone(ctx->geom[ii]);
    }
  }
  if (ctx->image) {
    for (ii=0; ii<ctx->imageNum; ii++) {
      if (ctx->image[ii]->data.v) {
        spotImageGLDone(ctx->image[ii]);
      }
    }
  }
  return 0;
}

context_t *contextNix(context_t *ctx) {
  unsigned int ii;

  if (!ctx) {
    return NULL;
  }
  if (ctx->geom) {
    for (ii=0; ii<ctx->geomNum; ii++) {
      spotGeomNix(ctx->geom[ii]);
    }
    free(ctx->geom);
  }
  if (ctx->image) {
    for (ii=0; ii<ctx->imageNum; ii++) {
      spotImageNix(ctx->image[ii]);
    }
    free(ctx->image);
  }
  free(ctx);
  return NULL;
}

int contextDraw(context_t *ctx) {
  const char me[]="contextDraw";
  unsigned int gi;
  GLfloat modelMat[16];
/*
  if (ctx->buttonDown) {
    // When the mouse is down, use a velocity of zero 
    thetaPerSecU = 0;
    thetaPerSecV = 0;
    thetaPerSecN = 0;
  } else {
    // Otherwise, use the previous velocity 
    thetaPerSecU = ctx->thetaPerSecU;
    thetaPerSecV = ctx->thetaPerSecV;
    thetaPerSecN = ctx->thetaPerSecN;
  }


  gctx->angleU = (thetaPerSecU * dt) * 0.1;
  gctx->angleV = (thetaPerSecV * dt) * 0.1;
  gctx->angleN = (thetaPerSecN * dt) * 0.1;
  rotate_model_UV(gctx->angleU, -gctx->angleV);
  rotate_model_N(-gctx->angleN);
*/

  /* re-assert which program is being used (AntTweakBar uses its own) */
  glUseProgram(ctx->program); 

  /* background color; setting alpha=0 means that we'll see the
     background color in the render window, but upon doing
     "spotImageScreenshot(img, SPOT_TRUE)" (SPOT_TRUE for "withAlpha")
     we'll get a meaningful alpha channel, so that the image can
     recomposited with a different background, or used in programs
     (including web browsers) that respect the alpha channel */
  glClearColor(ctx->bgColor[0], ctx->bgColor[1], ctx->bgColor[2], 0.0f);
  /* Clear the window and the depth buffer */
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  /* The following will be useful when you want to use textures,
     especially two textures at once, here sampled in the fragment
     shader with "samplerA" and "samplerB".  There are some
     non-intuitive calls required to specify which texture data will
     be sampled by which sampler.  See OpenGL SuperBible (5th edition)
     pg 279.  Also, http://tinyurl.com/7bvnej3 is amusing and
     informative */

  // Sun
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, ctx->image[0]->textureId);
  glUniform1i(ctx->uniloc.sampler0, 0);

  // Mercury 
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, ctx->image[1]->textureId);
  glUniform1i(ctx->uniloc.sampler1, 1);

  // Venus
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, ctx->image[2]->textureId);
  glUniform1i(ctx->uniloc.sampler2, 2); 

  // Earth
  glActiveTexture(GL_TEXTURE3);
  glBindTexture(GL_TEXTURE_2D, ctx->image[3]->textureId);
  glUniform1i(ctx->uniloc.sampler3, 3); 

  // Mars
  glActiveTexture(GL_TEXTURE4);
  glBindTexture(GL_TEXTURE_2D, ctx->image[4]->textureId);
  glUniform1i(ctx->uniloc.sampler4, 4); 

  // Jupiter
  glActiveTexture(GL_TEXTURE5);
  glBindTexture(GL_TEXTURE_2D, ctx->image[5]->textureId);
  glUniform1i(ctx->uniloc.sampler5, 5); 

  // Saturn
  glActiveTexture(GL_TEXTURE6);
  glBindTexture(GL_TEXTURE_2D, ctx->image[6]->textureId);
  glUniform1i(ctx->uniloc.sampler6, 6); 

  // Uranus
  glActiveTexture(GL_TEXTURE7);
  glBindTexture(GL_TEXTURE_2D, ctx->image[7]->textureId);
  glUniform1i(ctx->uniloc.sampler7, 7); 

  // Neptune
  glActiveTexture(GL_TEXTURE8);
  glBindTexture(GL_TEXTURE_2D, ctx->image[8]->textureId);
  glUniform1i(ctx->uniloc.sampler8, 8); 

  // Pluto
  glActiveTexture(GL_TEXTURE9);
  glBindTexture(GL_TEXTURE_2D, ctx->image[9]->textureId);
  glUniform1i(ctx->uniloc.sampler9, 9); 

	if (!gctx->paused) {
	}

  // NOTE: we must normalize our UVN matrix
  norm_M4(gctx->camera.uvn);
	inverseUVN(gctx->camera.inverse_uvn, gctx->camera.uvn);

  // NOTE: update our unilocs
  glUniformMatrix4fv(ctx->uniloc.viewMatrix, 1, GL_FALSE, gctx->camera.uvn);
  glUniformMatrix4fv(ctx->uniloc.inverseViewMatrix, 1, GL_FALSE, gctx->camera.inverse_uvn);
  glUniformMatrix4fv(ctx->uniloc.projMatrix, 1, GL_FALSE, gctx->camera.proj);
  glUniform3fv(ctx->uniloc.lightDir, 1, ctx->lightDir);
  glUniform3fv(ctx->uniloc.lightColor, 1, ctx->lightColor);
  glUniform1i(ctx->uniloc.seamFix, ctx->seamFix);

  // set time
  double toc = spotTime();
  if (ctx->ticDraw == -1)
    ctx->ticDraw = toc;
  double dt = toc - ctx->ticDraw;
  ctx->ticDraw = toc;

  if (!gctx->paused) {
    updateScene(gctx->time, dt);
  }

  for (gi=0; gi<ctx->geomNum; gi++) {
    set_model_transform(modelMat, ctx->geom[gi]);
    glUniformMatrix4fv(ctx->uniloc.modelMatrix, 
                       1, GL_FALSE, modelMat);
    updateNormals(ctx->geom[gi]->normalMatrix, modelMat);
    glUniformMatrix3fv(ctx->uniloc.normalMatrix,
                       1, GL_FALSE, ctx->geom[gi]->normalMatrix);
    glUniform3fv(ctx->uniloc.objColor, 1, ctx->geom[gi]->objColor);
    glUniform1f(ctx->uniloc.Ka, ctx->geom[gi]->Ka);
    glUniform1f(ctx->uniloc.Kd, ctx->geom[gi]->Kd);
    glUniform1i(ctx->uniloc.gi, gi);
    spotGeomDraw(ctx->geom[gi]);
  }

/*
  // NOTE: update our geom-specific unilocs
  for (gi=sceneGeomOffset; gi<ctx->geomNum; gi++) {
    set_model_transform(modelMat, ctx->geom[gi]);
    // NOTE: we normalize the model matrix; while we may not need to, it is cheap to do so
    norm_M4(modelMat);
    glUniformMatrix4fv(ctx->uniloc.modelMatrix, 1, GL_FALSE, modelMat);
    // NOTE: we update normals in our `matrixFunctions.c' functions on a case-by-case basis
    updateNormals(gctx->geom[gi]->normalMatrix, modelMat);
    glUniformMatrix3fv(ctx->uniloc.normalMatrix, 1, GL_FALSE, ctx->geom[gi]->normalMatrix);
    //
    glUniform3fv(ctx->uniloc.objColor, 1, ctx->geom[gi]->objColor);
    glUniform1f(ctx->uniloc.Ka, ctx->geom[gi]->Ka);
    glUniform1f(ctx->uniloc.Kd, ctx->geom[gi]->Kd);
    glUniform1f(ctx->uniloc.Ks, ctx->geom[gi]->Ks);
    glUniform1i(ctx->uniloc.gi, gi);
    glUniform1f(ctx->uniloc.shexp, ctx->geom[gi]->shexp);
    spotGeomDraw(ctx->geom[gi]);
  }
  */
  /* These lines are also related to using textures.  We finish by
     leaving GL_TEXTURE0 as the active unit since AntTweakBar uses
     that, but doesn't seem to explicitly select it */
  glActiveTexture(GL_TEXTURE9);
  glBindTexture(GL_TEXTURE_2D, 9);
  glActiveTexture(GL_TEXTURE8);
  glBindTexture(GL_TEXTURE_2D, 8);
  glActiveTexture(GL_TEXTURE7);
  glBindTexture(GL_TEXTURE_2D, 7);
  glActiveTexture(GL_TEXTURE6);
  glBindTexture(GL_TEXTURE_2D, 6);
  glActiveTexture(GL_TEXTURE5);
  glBindTexture(GL_TEXTURE_2D, 5);
  glActiveTexture(GL_TEXTURE4);
  glBindTexture(GL_TEXTURE_2D, 4);
  glActiveTexture(GL_TEXTURE3);
  glBindTexture(GL_TEXTURE_2D, 3);
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, 2);
  glActiveTexture(GL_TEXTURE1); 
  glBindTexture(GL_TEXTURE_2D, 1);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, 0);

  /* You are welcome to do error-checking with higher granularity than
     just once per render, in which case this error checking loop
     should be repackaged into its own function. */
  GLenum glerr = glGetError();
  if (glerr) {
    while (glerr) {
      spotErrorAdd("%s: OpenGL error %d (%s)", me, glerr, spotGLErrorString(glerr));
      glerr = glGetError();
    }
    return 1;
  }
  return 0;
}
/*
// NOTE: we use a callback here, since toggling perVertexTexturing requires the loading
//       of different shaders (and thus updating unilocs)
static void TW_CALL setPerVertexTexturingCallback(const void *value, void *clientData) {
  gctx->perVertexTexturingMode = *((const int *) value);
  fprintf(stderr, gctx->perVertexTexturingMode ? "Per-vertex Texturing: ON\n" : "Per-vertex Texturing: OFF\n");
  if (perVertexTexturing()) {
    printf("\tLoading shader 'simple' with id=%d\n", programIds[ID_SIMPLE]);
    gctx->program=programIds[ID_SIMPLE];
  } else {
    printf("\tLoading shader 'texture' with id=%d\n", programIds[ID_TEXTURE]);
    gctx->program=programIds[ID_TEXTURE];
  }
  setUnilocs();
}
static void TW_CALL getPerVertexTexturingCallback(void *value, void *clientData) {
  *((int *) value) = gctx->perVertexTexturingMode;
}

// NOTE: we use a callback here, since toggling bumpMapping requires the loading
//       of different shaders (and thus updating unilocs); additionally, we ensure
//       parallaxMapping is off
static void TW_CALL setBumpMappingCallback(const void *value, void *clientData) {
  gctx->bumpMappingMode = *((const enum BumpMappingModes *) value);
  switch (gctx->bumpMappingMode) {
    case Bump:
      printf("\tLoading shader 'bump' with id=%d\n", programIds[ID_BUMP]);
      gctx->program=programIds[ID_BUMP];
      break;
    case Parallax:
      printf("\tLoading shader 'parallax' with id=%d\n", programIds[ID_PARALLAX]);
      gctx->program=programIds[ID_PARALLAX];
      break;
    default: // Disabled
      printf("\tLoading shader 'texture' with id=%d\n", programIds[ID_TEXTURE]);
      gctx->program=programIds[ID_TEXTURE];
  }
  setUnilocs();
}

static void TW_CALL getBumpMappingCallback(void *value, void *clientData) {
  *((int *) value) = gctx->bumpMappingMode;
}

*/
/*static void TW_CALL setCubeMapCallback(const void *value, void *clientData) {
	enum CubeMaps cubemap = *((const enum CubeMaps *) value);
	switch (cubemap) {
		case CubeSample:
			gctx->cubeMapId = 0;
			break;
		case CubeCool:
			gctx->cubeMapId = 1;
			break;
		case CubePlace:
			gctx->cubeMapId = 2;
			break;
	}
}

static void TW_CALL getCubeMapCallback(void *value, void *clientData) {
  *((int *) value) = gctx->cubeMapId;
} 
static void TW_CALL setShaderCallback(const void *value, void *clientData) {
	enum Shaders shader = *((const enum Shaders *) value);
	switch (shader) {
		case PhongShader:
			gctx->program = programIds[ID_PHONG];
			setUnilocs();
			break;
		case CubeShader:
			gctx->program = programIds[ID_CUBE];
			setUnilocs();
			break;
		case SpotlightShader:
			gctx->program = programIds[ID_SPOTLIGHT];
			setUnilocs();
			break;
	}
}

static void TW_CALL getShaderCallback(void *value, void *clientData) {
	enum Shaders shader;
	if (gctx->program==programIds[ID_PHONG]) {
		shader = PhongShader;
	} else if (gctx->program==programIds[ID_CUBE]) {
		shader = CubeShader;
	} else {
		shader = SpotlightShader;
	}
  *((int *) value) = shader;
}
*/
static void TW_CALL setObjectCallback(const void *value, void *clientData) {
	enum Objects object = *((const enum Objects *) value);
	switch (object) {
		case Sun:
			gctx->gi = 0;
			break;
		case Mercury:
			gctx->gi = 1;
			break;
		case Venus:
			gctx->gi = 2;
			break;
		case Earth:
			gctx->gi = 3;
			break;
		case Mars:
			gctx->gi = 4;
			break;
		case Jupiter:
			gctx->gi = 5;
			break;
		case Saturn:
			gctx->gi = 6;
			break;
		case Uranus:
			gctx->gi = 7;
			break;
		case Neptune:
			gctx->gi = 8;
			break;
		case Pluto:
			gctx->gi = 9;
			break;
	}
}

static void TW_CALL getObjectCallback(void *value, void *clientData) {
  *((int *) value) = gctx->gi;
}

/*
static void TW_CALL setFilteringCallback(const void *value, void *clientData) {
  gctx->filteringMode = *((const enum FilteringModes *) value);
  switch (gctx->filteringMode) {
    case Nearest:
      gctx->minFilter=GL_NEAREST;
      gctx->magFilter=GL_NEAREST;
      printf("\tGL_NEAREST\n");
      break;
    case Linear:
      gctx->minFilter=GL_LINEAR;
      gctx->magFilter=GL_LINEAR;
      printf("\tGL_LINEAR\n");
      break;
    case NearestWithMipmap:
      gctx->minFilter=GL_NEAREST_MIPMAP_NEAREST;
      gctx->magFilter=GL_NEAREST;
      printf("\tGL_NEAREST & GL_NEAREST_MIPMAP_NEAREST\n");
      break;
    case LinearWithMipmap:
      gctx->minFilter=GL_LINEAR_MIPMAP_LINEAR;
      gctx->magFilter=GL_LINEAR;
      printf("\tGL_LINEAR & GL_LINEAR_MIPMAP_LINEAR\n");
      break;
    default:
      printf("\tDEFAULT\n");
  }
  setUnilocs();
}

static void TW_CALL getFilteringCallback(void *value, void *clientData) {
  *((int *) value) = gctx->filteringMode;
}
*/
// NOTE: here are our tweak bar definitions
int updateTweakBarVars(int scene) {
  int EE=0;
  if (!EE) EE |= !TwRemoveAllVars(gctx->tbar);
  if (!EE) EE |= !TwAddVarRW(gctx->tbar, "Ka",
                             TW_TYPE_FLOAT, &(gctx->geom[0]->Ka),
                             " label='Ka' min=0.0 max=1.0 step=0.005");
  if (!EE) EE |= !TwAddVarRW(gctx->tbar, "Kd",
                             TW_TYPE_FLOAT, &(gctx->geom[0]->Kd),
                             " label='Kd' min=0.0 max=1.0 step=0.005");
  if (!EE) EE |= !TwAddVarRW(gctx->tbar, "Ks",
                             TW_TYPE_FLOAT, &(gctx->geom[0]->Ks),
                             " label='Ks' min=0.0 max=1.0 step=0.005");
  if (!EE) EE |= !TwAddVarRW(gctx->tbar, "shexp",
                             TW_TYPE_FLOAT, &(gctx->geom[0]->shexp),
                             " label='shexp' min=0.0 max=100.0 step=0.05");
/*  if (!EE) EE |= !TwAddVarCB(gctx->tbar, "shader",
                             twShaders, setShaderCallback,
                             getShaderCallback, &(gctx->program),
                             " label='shader'"); */
  if (!EE) EE |= !TwAddVarCB(gctx->tbar, "planet",
                             twObjects, setObjectCallback,
                             getObjectCallback, &(gctx->gi),
                             " label='planet'");
/*  if (!EE) EE |= !TwAddVarCB(gctx->tbar, "cubemap",
                             twCubeMaps, setCubeMapCallback,
                             getCubeMapCallback, &(gctx->cubeMapId),
                             " label='cubemap'"); */
  if (!EE) EE |= !TwAddVarRW(gctx->tbar, "bgColor",
                             TW_TYPE_COLOR3F, &(gctx->bgColor),
                             " label='bkgr color' ");
/*
  switch (scene) {
    case 1:
      if (!EE) EE |= !TwAddVarRW(
           gctx->tbar, "shading",
           TW_TYPE_BOOL8, &(gctx->gouraudMode),
           " label='shading' true=Gouraud false=Phong ");
      break;
    case 2:
      if (!EE) EE |= !TwAddVarCB(
           gctx->tbar, "perVertexTexturing",
           TW_TYPE_BOOL8, setPerVertexTexturingCallback,
           getPerVertexTexturingCallback, &(gctx->perVertexTexturingMode),
           " label='per-vertex texturing' true=Enabled false=Disabled ");
      if (!EE) EE |= !TwAddVarRW(
           gctx->tbar, "seamFix",
           TW_TYPE_BOOL8, &(gctx->seamFix),
           " label='seam fix' true=Enabled false=Disabled ");
      break;
    case 3:
      if (!EE) EE |= !TwAddVarCB(
           gctx->tbar, "filteringMode",
           twFilteringModes, setFilteringCallback,
           getFilteringCallback, &(gctx->filteringMode),
           " label='filtering mode' ");
      break;
    case 4:
      if (!EE) EE |= !TwAddVarCB(
           gctx->tbar, "bumpMappingMode",
           twBumpMappingModes, setBumpMappingCallback,
           getBumpMappingCallback, &(gctx->bumpMappingMode),
           " label='bump mapping' ");
      break;
    default:
      break;
  }
*/
  return EE;
}

int createTweakBar(context_t *ctx, int scene) {
  const char me[]="createTweakBar";
  char buff[128];
  int EE;  /* we have an error */

  EE = 0;

  // NOTE: these are nice to have
  twBumpMappingModes=TwDefineEnum("BumpMappingModes", twBumpMappingModesEV, 3);
  twFilteringModes=TwDefineEnum("FilteringModes", twFilteringModesEV, 4);
  twObjects=TwDefineEnum("Objects", twObjectsEV, 10);
  twCubeMaps=TwDefineEnum("CubeMap", twCubeMapsEV, 3);
  twShaders=TwDefineEnum("Shader", twShadersEV, 3);
  
  /* Create a tweak bar for interactive parameter adjustment */
  if (!EE) EE |= !(ctx->tbar = TwNewBar(TBAR_NAME));
  /* documentation for the TwDefine parameter strings here:
     http://www.antisphere.com/Wiki/tools:anttweakbar:twbarparamsyntax */
  /* add a message to be seen in the "help" window */
  if (!EE) EE |= !TwDefine(" GLOBAL help='This description of Project 2 "
                           "has not been changed by anyone but students "
                           "are encouraged to write something descriptive "
                           "here.' ");
  /* change location where bar will be drawn, over to the right some
     to expose more of the left edge of window.  Note that we are
     exploiting the automatic compile-time concatentation of strings
     in C, which connects TBAR_NAME with the rest of the string to
     make one contiguous string */
  sprintf(buff, TBAR_NAME " position='%d %d' ",
          ctx->winSizeX - ctx->tbarSizeX - ctx->tbarMargin,
          ctx->tbarMargin);
  if (!EE) EE |= !TwDefine(buff);
  /* adjust other aspects of the bar */
  sprintf(buff, TBAR_NAME " color='0 0 0' alpha=10 size='%d %d' ",
          ctx->tbarSizeX, ctx->tbarSizeY);
  if (!EE) EE |= !TwDefine(buff);
  
  // NOTE: we broke this section out for easy update of tweak bar vars per-scene
  if (!EE) EE |= updateTweakBarVars(scene);

  /* see also:
     http://www.antisphere.com/Wiki/tools:anttweakbar:twtype
     http://www.antisphere.com/Wiki/tools:anttweakbar:twdefineenum
  */

  if (EE) {
    spotErrorAdd("%s: AntTweakBar initialization failed:\n\t%s", me, TwGetLastError());
    return 1;
  }
  return 0;
}

void usage(const char *me) {
  fprintf(stderr, "usage: %s [<vertshader> <fragshader>]\n", me);
  fprintf(stderr, "\tCall `%s', optionally taking a default pair of vertex and fragment\n", me);
  fprintf(stderr, "\tshaders to render. Otherwise we just load our stack of shaders.\n");
}

int main(int argc, const char* argv[]) {
  const char *me;
  me = argv[0];
  // NOTE: we now allow you to either pass in an "invoked" or default shader to render, or to let
  //       us just set up our stack; hence you either pass 2 additional arguments or none at all
  // NOTE: we aren't explicity defining this functionality, but obviously `proj2 -h' will show the
  //       usage pattern
  if (1!=argc && 3!=argc) {
    usage(me);
    exit(1);
  }

  if (!(gctx = contextNew(10, 10))) {
    fprintf(stderr, "%s: context set-up problem:\n", me);
    spotErrorPrint();
    spotErrorClear();
    exit(1);
  }

  if (argc==3) {
    gctx->vertFname = argv[1];
    gctx->fragFname = argv[2];
  } else {
    // NOTE: if invoked with no shaders, set these to NULL; `contextGlInit()' will catch these
    gctx->vertFname = NULL;
    gctx->fragFname = NULL;
  }

  if (!glfwInit()) {
    fprintf(stderr, "Failed to initialize GLFW\n");
    exit(1);
  }

  /* Make sure we're using OpenGL 3.2 core.  NOTE: Changing away from
     OpenGL 3.2 core is not needed and not allowed for this project */
  glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3);
  glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 2);
  glfwOpenWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwOpenWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  if (!glfwOpenWindow(gctx->winSizeX, gctx->winSizeY, 0, 0, 0, 0, 32, 0, GLFW_WINDOW)) {
    fprintf(stderr, "Failed to open GLFW window\n");
    glfwTerminate();
    exit(1);
  }

  glfwSetWindowTitle("Project 2: Shady");
  glfwEnable(GLFW_MOUSE_CURSOR);
  glfwEnable(GLFW_KEY_REPEAT);
  glfwSwapInterval(1);

  /* Initialize AntTweakBar */
  if (!TwInit(TW_OPENGL_CORE, NULL)) {
    fprintf(stderr, "AntTweakBar initialization failed: %s\n",
            TwGetLastError());
    exit(1);
  }

  printf("GL_RENDERER   = %s\n", (char *) glGetString(GL_RENDERER));
  printf("GL_VERSION    = %s\n", (char *) glGetString(GL_VERSION));
  printf("GL_VENDOR     = %s\n", (char *) glGetString(GL_VENDOR));
  printf("PNG_LIBPNG_VER_STRING = %s\n", PNG_LIBPNG_VER_STRING);
  
  /* set-up and initialize the global context */
  if (contextGLInit(gctx)) {
    fprintf(stderr, "%s: context OpenGL set-up problem:\n", me);
    spotErrorPrint(); spotErrorClear();
    TwTerminate();
    glfwTerminate();
    exit(1);
  }

  // NOTE: when we create the tweak bar, either load in scene 1 or default, depending
  //       on whether we were passing a pair of shaders
  if (createTweakBar(gctx, (gctx->vertFname==NULL?1:0))) {
    fprintf(stderr, "%s: AntTweakBar problem:\n", me);
    spotErrorPrint(); spotErrorClear();
    TwTerminate();
    glfwTerminate();
    exit(1);
  }

  glfwSetWindowSizeCallback(callbackResize);
  glfwSetKeyCallback(callbackKeyboard);
  glfwSetMousePosCallback(callbackMousePos);
  glfwSetMouseButtonCallback(callbackMouseButton);

  /* Redirect GLFW mouse wheel events directly to AntTweakBar */
  glfwSetMouseWheelCallback((GLFWmousewheelfun)TwEventMouseWheelGLFW);
  /* Redirect GLFW char events directly to AntTweakBar */
  glfwSetCharCallback((GLFWcharfun)TwEventCharGLFW);

  /* Main loop */
  while (gctx->running) {
    // NOTE: we update UVN every step
    updateUVN(gctx->camera.uvn, gctx->camera.at, gctx->camera.from, gctx->camera.up);

    // Update time
    if (!gctx->paused) {
      gctx->time += 0.1;
      if (gctx->time == 10) { gctx->time = 0; }
    }
    /* render */
    if (contextDraw(gctx)) {
      fprintf(stderr, "%s: trouble drawing:\n", me);
      spotErrorPrint(); spotErrorClear();
      /* Can comment out "break" so that OpenGL bugs are reported but
         do not lead to the termination of the program */
      /* break; */
    }
    /* Draw tweak bar last, just prior to buffer swap */
    if (!TwDraw()) {
      fprintf(stderr, "%s: AntTweakBar error: %s\n", me, TwGetLastError());
      break;
    }
    /* Display rendering results */
    glfwSwapBuffers();
    /* NOTE: don't call glfwWaitEvents() if you want to redraw continuously */
//    glfwWaitEvents();
    /* quit if window was closed */
    if (!glfwGetWindowParam(GLFW_OPENED)) {
      gctx->running = 0;
    }
  }
  
  contextGLDone(gctx);
  contextNix(gctx);
  TwTerminate();
  glfwTerminate();

  exit(0);
}
