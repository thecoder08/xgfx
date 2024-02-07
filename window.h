#define KEY_CHANGE 1
#define MOUSE_MOVE 2
#define MOUSE_BUTTON 3
#define WINDOW_CLOSE 4

typedef struct {
  int width;
  int height;
  int depth;
  int stride;
  int size;
  int* data;
} Image;

typedef struct {
    int type;
    unsigned int key;
    unsigned int state;
} KeyChangeEvent;

typedef struct {
    int type;
    int x;
    int y;
} MouseMoveEvent;

typedef struct {
    int type;
    unsigned int button;
    unsigned int state;
} MouseButtonEvent;

typedef struct {
    int type;
} WindowCloseEvent;

typedef union {
    int type;
    KeyChangeEvent keychange;
    MouseMoveEvent mousemove;
    MouseButtonEvent mousebutton;
    WindowCloseEvent windowclose;
} Event;

void initWindow(int width, int height, const char* title);
void updateWindow();
int checkWindowEvent(Event* event);