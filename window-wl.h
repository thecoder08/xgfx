void initWindow(int width, int height, const char* title, void (*paint_pixels)(), void (*key_change)(unsigned int key, unsigned int state), void (*pointer_motion_cb)(int pointerX, int pointerY), void (*pointer_button_cb)(unsigned int button, unsigned int state));
int dispatchEvents();
void closeWindow();