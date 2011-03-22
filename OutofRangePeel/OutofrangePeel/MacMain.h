/*
 *  MacMain.h
 *  OutofrangePeel
 */

#ifdef __cplusplus
extern "C" {
#endif
	
int core_initialize();

int core_callback_animate();
int core_callback_disp();

int core_callback_mouse(int button, int state, float x, float y);
int core_callback_mousemove(float x, float y);
int core_callback_mousepassivemove(float x, float y);
int core_callback_mousewheel(int button, float dir, float x, float y);
	
int core_callback_resize(int width, int height);

#ifdef __cplusplus
}
#endif
