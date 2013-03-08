typedef enum
{
  FALSE = 0,
  TRUE,
} bool;

typedef struct
{
  int x, y;
} Point;
#define NULL_POINT {0,0}

typedef struct
{
  float x, y;
} FPoint;
#define NULL_FPOINT {0.0f,0.0f}

typedef struct
{
  Point a, b;
} Rectangle;
#define NULL_RECTANGLE {NULL_POINT, NULL_POINT}

typedef struct
{
  FPoint a, b;
} FRectangle;
#define NULL_FRECTANGLE {NULL_FPOINT, NULL_FPOINT}

typedef struct
{
  int r,g,b,a;
} Color;
#define NULL_COLOR {0xff,0xff,0xff,0xff}

typedef enum
{
  IMAGE_FONT = 0,
  IMAGE_MAX,
}  ImageID;
#define NULL_IMAGEID (0);

typedef struct
{
  Point pos;
  Point size;
  ImageID image;
} SpriteData;
#define NULL_SPRITEDATA {NULL_POINT, NULL_POINT, 0}

typedef enum
{
  DIR_UP=0,
  DIR_RIGHT,
  DIR_DOWN,
  DIR_LEFT,
} Direction;

int min(int a, int b);
int max(int a, int b);
int minmax(int lower, int upper, int n);
Point pointAddPoint(Point a, Point b);
FPoint fpointAddFPoint(FPoint a, FPoint b);
Rectangle rectangleAddPoint(Rectangle a, Point b);
FRectangle frectangleAddFPoint(FRectangle a, FPoint b);
Point pointInverse(Point a);
FPoint fpointInverse(FPoint a);
Point pointMultiply(Point a, int n);
FPoint fpointMultiply(FPoint a, float f);
Rectangle rectangleMultiply(Rectangle a, int n);
FRectangle frectangleMultiply(FRectangle a, float n);

FPoint pointToFPoint(Point in);
Point fpointToPoint(FPoint in);
FRectangle rectangleToFRectangle(Rectangle in);
Rectangle frectangleToRectangle(FRectangle in);

bool rectangleIntersect(Rectangle a, Rectangle b);
bool frectangleIntersect(FRectangle a, FRectangle b);
bool pointInRectangle(Point p, Rectangle r);
bool fpointInFRectangle(FPoint p, FRectangle r);

Point getFrameFromAscii(char c, int colour);
