#include "main.h"
#include <math.h>

GLFWwindow *_window;
Point _resolution;
Point _windowSize;
int _pixel_scale;
Rectangle _drawRect;
bool _shouldClose;
ImageID _nextImage;
bool _heldInputs[INPUT_MAX];
bool _pressedInputs[INPUT_MAX];

typedef struct
{
  GLuint gluint;
  Point size;
} Image;
Image _images[IMAGE_MAX];
#define NULL_IMAGE {0, NULL_POINT}

void _sys_close_callback();

void sys_init(Point resolution, int pixel_scale)
{
  bool result;
  int i;
  GLuint image_ids[IMAGE_MAX];
  _resolution = resolution;
  _pixel_scale = pixel_scale;
  _shouldClose = false;
  _nextImage = 0;
  
  LOG("Initialize GLFW");

  result = glfwInit();
  if(!result)
  {
    LOG("Failed to initialize GLFW");
    exit( EXIT_FAILURE );
  }
  _window = glfwCreateWindow( _resolution.x*_pixel_scale, _resolution.y*_pixel_scale,
                             "Sunk Coast", NULL, NULL );
  if(!_window)
  {
    LOG("Failed to create GLFW window");
    exit( EXIT_FAILURE );
  }
  glfwMakeContextCurrent( _window );
  
  // Detect key presses between calls to GetKey
  glfwSetInputMode( _window, GLFW_STICKY_KEYS, GLFW_TRUE );
  
  // Enable vertical sync (on cards that support it)
  glfwSwapInterval( 1 );
  
  glfwSetWindowCloseCallback(_window, _sys_close_callback);
  
  LOG("Initialize DevIL");
  
  // devil init  
  ilInit();
  
  glGenTextures(IMAGE_MAX, image_ids);
  Image nullImage = NULL_IMAGE;
  for(i=0; i<IMAGE_MAX; i++)
  {
    _images[i] = nullImage;
    _images[i].gluint = image_ids[i];
  }

  srand(time(NULL));
}

void _sys_close_callback()
{
  return;
}

bool sys_shouldClose()
{
  return _shouldClose;
}

void sys_close()
{
  glfwTerminate();
}

double sys_getTime()
{
  return glfwGetTime();
}

void sys_drawInit(Color col)
{
  Point maxPixelSize = NULL_POINT;
  Point drawRectSize = NULL_POINT;
  
  glfwGetWindowSize( _window, &_windowSize.x, &_windowSize.y);
  glViewport( 0, 0, _windowSize.x, _windowSize.y ); 
  
  glClearColor((float)col.r/255.0f, (float)col.g/255.0f, (float)col.b/255.0f, (float)col.a/255.0f);
  glClear( GL_COLOR_BUFFER_BIT );
  
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, _windowSize.x, _windowSize.y,0, -1,1);
  
  maxPixelSize.x = _windowSize.x/_resolution.x;
  maxPixelSize.y = _windowSize.y/_resolution.y;
  _pixel_scale = min(maxPixelSize.x, maxPixelSize.y);
  drawRectSize.x = _pixel_scale*_resolution.x;
  drawRectSize.y = _pixel_scale*_resolution.y;
  _drawRect.a.x = (_windowSize.x-drawRectSize.x)/2;
  _drawRect.a.y = (_windowSize.y-drawRectSize.y)/2;
  _drawRect.b.x = _drawRect.a.x+drawRectSize.x;
  _drawRect.b.y = _drawRect.a.y+drawRectSize.y;
}

Point _sys_worldPointToScreenPoint(Point p)
{
  p.x *= _pixel_scale;
  p.y *= _pixel_scale;
  p.x += _drawRect.a.x;
  p.y += _drawRect.a.y;
  return p;
}

Rectangle _sys_worldRectangleToScreenRectangle(Rectangle r)
{
  r.a = _sys_worldPointToScreenPoint(r.a);
  r.b = _sys_worldPointToScreenPoint(r.b);
  return r;
}

void _sys_drawRectangle(Rectangle rect, Color col)
{
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  glColor4f((float)col.r/255.0f, (float)col.g/255.0f, (float)col.b/255.0f, (float)col.a/255.0f);
  glBegin(GL_QUADS);
  glVertex3f(rect.a.x, rect.a.y,1.0f);
  glVertex3f(rect.b.x, rect.a.y,1.0f);
  glVertex3f(rect.b.x, rect.b.y,1.0f);
  glVertex3f(rect.a.x, rect.b.y,1.0f);
  glEnd();
  glDisable(GL_BLEND);
}

void sys_drawRectangle(Rectangle rect, Color col)
{
  rect = _sys_worldRectangleToScreenRectangle(rect);
  _sys_drawRectangle(rect, col);
}

void sys_drawFinish()
{
  Color black = {0x17,0x14,0x1b,255};
  Rectangle rectNBar = {{0,0}, {_windowSize.x, _drawRect.a.y}};
  Rectangle rectEBar = {{_drawRect.b.x,0}, {_windowSize.x, _windowSize.y}};
  Rectangle rectSBar = {{0,_drawRect.b.y}, {_windowSize.x, _windowSize.y}};
  Rectangle rectWBar = {{0,0}, {_drawRect.a.x, _windowSize.y}};

  _sys_drawRectangle(rectNBar, black);
  _sys_drawRectangle(rectEBar, black);
  _sys_drawRectangle(rectSBar, black);
  _sys_drawRectangle(rectWBar, black);
  
  glfwSwapBuffers( _window );
  glfwPollEvents();
}

ImageID sys_loadImage(const char *name)
{
  bool success;
  if(_nextImage >= IMAGE_MAX)
  {
    LOG("Ran out of image slots.");
    _shouldClose = true;
    return 0;
  }
  
  ILuint iluint;
  
  LOG("Loading %s.", name);
  ilGenImages(1, &iluint);
  ilBindImage(iluint);
  success = ilLoadImage(name);
  if(!success)
  {
    LOG("Failed to load spritesheet.");
    _shouldClose = true;
    return 0;
  }
  
  success = ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
  if(!success)
  {
    LOG("Failed to convert image.");
    _shouldClose = true;
    return 0;
  }
  
  // Bind image to opengl
  glBindTexture(GL_TEXTURE_2D, _images[_nextImage].gluint);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 

  int sizex = ilGetInteger(IL_IMAGE_WIDTH);
  int sizey = ilGetInteger(IL_IMAGE_HEIGHT);
  if((sizex & (sizex-1)) != 0)
    LOG("WARNING: Image %s width %d is not power of 2, some gfx cards do not support this!", name, sizex);
  if((sizey & (sizey-1)) != 0)
    LOG("WARNING: Image %s height %d is not power of 2, some gfx cards do not support this!", name, sizey);
  _images[_nextImage].size.x = sizex;
  _images[_nextImage].size.y = sizey;
  
    
  glTexImage2D(GL_TEXTURE_2D, 0, ilGetInteger(IL_IMAGE_BPP),
               _images[_nextImage].size.x, _images[_nextImage].size.y,
               0, ilGetInteger(IL_IMAGE_FORMAT), GL_UNSIGNED_BYTE, ilGetData());  
 
  // Image now in graphics memory, can ditch il copy
  ilDeleteImages(1, &iluint);
  
  LOG("Bound to %d (%d)", _nextImage, _images[_nextImage]);
  _nextImage++;
  return _nextImage-1;
}

Point sys_imageSize(ImageID id)
{
  return _images[id].size;
}

void sys_drawSprite(SpriteData spriteData, Point frame, Point pos)
{
    Rectangle drawRect = NULL_RECTANGLE;
    FRectangle spriteRect = NULL_RECTANGLE;
    Image image = _images[spriteData.image];
    
    drawRect.a.x = pos.x * spriteData.size.x;
    drawRect.a.y = pos.y * spriteData.size.y;
    drawRect.b = pointAddPoint(drawRect.a, spriteData.size);
    drawRect = _sys_worldRectangleToScreenRectangle(drawRect);
    
    spriteRect.a.x = (float)(spriteData.pos.x + spriteData.size.x*frame.x);
    spriteRect.a.y = (float)(spriteData.pos.y + spriteData.size.y*frame.y);
    spriteRect.b.x = (float)(spriteRect.a.x + spriteData.size.x);
    spriteRect.b.y = (float)(spriteRect.a.y + spriteData.size.y);
    spriteRect.a.x /= image.size.x;
    spriteRect.a.y /= image.size.y;
    spriteRect.b.x /= image.size.x;
    spriteRect.b.y /= image.size.y;
    
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, _images[spriteData.image].gluint);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    
    glColor4f(1.0, 1.0, 1.0, 1.0);
    glBegin(GL_QUADS);
    glTexCoord2f(spriteRect.a.x, spriteRect.a.y);
    glVertex2f(drawRect.a.x, drawRect.a.y);
    glTexCoord2f(spriteRect.b.x , spriteRect.a.y);
    glVertex2f(drawRect.b.x, drawRect.a.y);
    glTexCoord2f(spriteRect.b.x, spriteRect.b.y);
    glVertex2f(drawRect.b.x, drawRect.b.y);
    glTexCoord2f(spriteRect.a.x, spriteRect.b.y);
    glVertex2f(drawRect.a.x, drawRect.b.y);
    glEnd();
    
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
}

void sys_drawString(Point pos, const char* string, int len, int colour)
{
  int i;
  SpriteData spriteData = {{0,0}, {8,15}, IMAGE_FONT};
  for(i=0; i<len; i++)
  {
    if(string[i] == 0)
      break;
    Point frame = getFrameFromAscii(string[i], colour);
    sys_drawSprite(spriteData, frame, pos);
    pos.x++;
  }
}

bool sys_inputDown(Input input)
{
  return _heldInputs[input];
}

bool sys_inputPressed(Input input)
{
  return _pressedInputs[input];
}

Point sys_mousePos()
{
  Point out;
  double x, y;
  glfwGetCursorPos(_window, &x, &y);
  out.x = floor(x);
  out.y = floor(y);
  out = pointAddPoint(out, pointInverse(_drawRect.a));  
  out.x /= _pixel_scale;
  out.y /= _pixel_scale;
  return out;
}

bool _sys_pressed(int key)
{
  return glfwGetKey(_window, key) == GLFW_PRESS;
}


void sys_update()
{
  int i;
  bool _oldInputs[INPUT_MAX];
  for(i=0; i<INPUT_MAX; i++)
    _oldInputs[i] = _heldInputs[i];
    
  _heldInputs[INPUT_UP] = _sys_pressed(GLFW_KEY_UP) || _sys_pressed('K') || _sys_pressed('W') || _sys_pressed(GLFW_KEY_KP_8);
  _heldInputs[INPUT_RIGHT] = _sys_pressed(GLFW_KEY_RIGHT) || _sys_pressed('L') || _sys_pressed('D') || _sys_pressed(GLFW_KEY_KP_6);
  _heldInputs[INPUT_DOWN] = _sys_pressed(GLFW_KEY_DOWN) || _sys_pressed('J') || _sys_pressed('S') || _sys_pressed(GLFW_KEY_KP_2);
  _heldInputs[INPUT_LEFT] = _sys_pressed(GLFW_KEY_LEFT) || _sys_pressed('H') || _sys_pressed('A') || _sys_pressed(GLFW_KEY_KP_4);
  _heldInputs[INPUT_DIVE] = _sys_pressed('V');
  _heldInputs[INPUT_RISE] = _sys_pressed('R');
  _heldInputs[INPUT_PICKUP] = _sys_pressed('P');
  _heldInputs[INPUT_DROP] = _sys_pressed('O');
  _heldInputs[INPUT_USE] = _sys_pressed('U');
  _heldInputs[INPUT_ESC] = _sys_pressed(GLFW_KEY_ESCAPE);

  for(i=0; i<10; i++)
    _heldInputs[INPUT_0 + i] = glfwGetKey(_window, '0'+i) == GLFW_PRESS;

  _heldInputs[INPUT_ANY] = false;
  for(i=0; i<INPUT_ANY; i++)
    if(_heldInputs[i])
      _heldInputs[INPUT_ANY] = true;
  
  for(i=0; i<INPUT_MAX; i++)
    _pressedInputs[i] = !_oldInputs[i] && _heldInputs[i];
}

int sys_randint(int max)
{
  if(max <= 0) return max;
  return rand() % max;
}
