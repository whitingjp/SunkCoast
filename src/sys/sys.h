typedef enum
{
  // game
  INPUT_UP = 0,
  INPUT_RIGHT,
  INPUT_DOWN,
  INPUT_LEFT,
  INPUT_ESC,
  INPUT_DIVE,
  INPUT_RISE,
  INPUT_PICKUP,
  INPUT_DROP,
  INPUT_USE,

  INPUT_0,
  INPUT_1,
  INPUT_2,
  INPUT_3,
  INPUT_4,
  INPUT_5,
  INPUT_6,
  INPUT_7,
  INPUT_8,
  INPUT_9,

  INPUT_ANY,

  // max
  INPUT_MAX,
} Input;
#define NULL_INPUT (INPUT_UP)

void sys_init(Point resolution, int pixel_scale);
bool sys_shouldClose();
void sys_close();
double sys_getTime();
void sys_drawInit(Color col);
void sys_drawFinish();
void sys_drawRectangle(Rectangle rectangle, Color col);
void sys_drawSprite(SpriteData sprite, Point frame, Point pos);
void sys_drawString(Point pos, const char* string, int len, int colour);
ImageID sys_loadImage(const char *name);
Point sys_imageSize(ImageID id);
bool sys_inputDown(Input input);
bool sys_inputPressed(Input input);
Point sys_mousePos();
void sys_update();
int sys_randint(int max);
