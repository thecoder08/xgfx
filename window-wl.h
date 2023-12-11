void initWindow(int width, int height, const char* title, void (*paint_pixels)(), void (*key_change)(unsigned int key, unsigned int state));
int dispatchEvents();
void closeWindow();