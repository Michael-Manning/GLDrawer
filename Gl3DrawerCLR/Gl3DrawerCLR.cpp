#include "stdafx.h"
#include <gl3w/gl3w.h>
#include "Gl3DrawerCLR.h"
#include <OpenGL3/link.cpp>
#include <OpenGL3/Input.cpp>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
using namespace std;

GL3DrawerCLR::GLDWrapper::GLDWrapper(bool packShaders) {
	base = new BaseGLD();
	base->usePackedShaders = packShaders; //the fininall DLL need to be portable. Set this to true on export
	Input = gcnew InputManagerCLR(base);	
}
int GL3DrawerCLR::GLDWrapper::createCanvas(int width, int height, bool borderd, RGBA ^ backColor, bool Vsync, bool DebugMode) { 
	cout << "Running window from C# wrapper \n";
	base->debugMode = DebugMode;
    initialized = !base->createCanvas(width, height, !borderd, vec3(backColor->r, backColor->g, backColor->b), Vsync);
	return initialized;
}
void GL3DrawerCLR::Rect::dispose() {
	delete r;
}
void GL3DrawerCLR::GLDWrapper::clearNullRects() {
	base->removeNullShapes();
}
void GL3DrawerCLR::GLDWrapper::removeRect(Rect ^ r) {
	base->removeRect(r->r);
}
void GL3DrawerCLR::GLDWrapper::swapOrder(int a, int b) {
	base->swapOrder(a, b);
}
void GL3DrawerCLR::GLDWrapper::setPos(int x, int y) {
	base->setPos(x, y);
}
void GL3DrawerCLR::GLDWrapper::setVisible(bool visible) {
	base->setVisible(visible);
}
void GL3DrawerCLR::GLDWrapper::focusWindow() {
	base->focus();
}
int GL3DrawerCLR::GLDWrapper::addRect(Rect ^ r) {
	base->addRect(r->r);
	return 1; 
}
bool GL3DrawerCLR::GLDWrapper::checkLoaded(Rect ^ r) {
	for (int i = 0; i < base->rects.size(); i++)
		if (base->rects[i] == r->r)
			return true;
	return false;
}
void GL3DrawerCLR::GLDWrapper::setBBpixel(int x, int y, RGBA ^ col) {
	base->setBBPixel(x, y, vec4(col->r, col->g, col->b, col->a));
}
void GL3DrawerCLR::GLDWrapper::setBBShape(Rect ^ r) {
	base->setBBShape(*r->r);
}
GL3DrawerCLR::RGBA ^ GL3DrawerCLR::GLDWrapper::getPixel(int x, int y) {
	vec3 c = base->getPixel(x, y);
	return gcnew RGBA(vec4(c, 1));
}
void GL3DrawerCLR::GLDWrapper::addBBShape(Rect ^ r) {
	base->BBQue.push_back(*r->r);
}
IntPtr GL3DrawerCLR::GLDWrapper::getNativeHWND() {
	HWND hwnd = base->getNativeHWND();
	return IntPtr(hwnd);
}
void GL3DrawerCLR::GLDWrapper::clearBB() {
	base->clearColorFlag = true;
}
void GL3DrawerCLR::GLDWrapper::clearShapes() {
	base->clearShapeFlag = true;
}
int GL3DrawerCLR::GLDWrapper::getRectIndex(Rect ^ r) {
	return base->getDrawIndex(r->r);
}
void GL3DrawerCLR::GLDWrapper::setWindowSize(int x, int y) {
	base->setWindowSize(x, y);
}
void GL3DrawerCLR::GLDWrapper::dispose() {
	base->cleanup();
	//Input->dispose();
//	delete base->Input;
	delete base;
}

void GL3DrawerCLR::GLDWrapper::mainloop(bool render) {
	if (shouldClose)
		return;

	//check to see if GLFW has triggered any events (up to 10 per frame) and call the corosponding events in C#
	for (int i = 0; i < 10; i++)
	{
		if (!base->Input->mouseBuffer[i].read) 
			if(Input->csharpMouseCallback)
				Input->csharpMouseCallback(base->Input->mouseBuffer[i].btn, base->Input->mouseBuffer[i].action, 1);
		base->Input->mouseBuffer[i].read = true;

		if (!base->Input->keyBuffer[i].read) {
			if (i > 0 && base->Input->keyBuffer[i - 1] == base->Input->keyBuffer[i]) {
				//make this not stupid
			}

			else if (Input->csharpKeyCallback) {
				Input->csharpKeyCallback(base->Input->keyBuffer[i].btn, base->Input->keyBuffer[i].action, 1);
			}			
		}
		base->Input->keyBuffer[i].read = true;
	}
	if (base->Input->mouseMoveFlag) {
		base->Input->mouseMoveFlag = false;
		Input->csharpMouseMoveCallback();
	}
	base->mainloop(render);
}

bool GL3DrawerCLR::InputManagerCLR::getKey(char key) {
	return im->getKey(key);
}
bool GL3DrawerCLR::InputManagerCLR::getKeyDown(char key) {
	return im->getKeyDown(key);
}
bool GL3DrawerCLR::InputManagerCLR::getKeyUp(char key) {
	return im->getKeyUp(key);
}
bool GL3DrawerCLR::InputManagerCLR::getKey(int key) {
	return im->getKey(key);
}
bool GL3DrawerCLR::InputManagerCLR::getKeyDown(int key) {
	return im->getKeyDown(key);
}
bool GL3DrawerCLR::InputManagerCLR::getKeyUp(int key) {
	return im->getKeyUp(key);
}

bool GL3DrawerCLR::InputManagerCLR::getMouse(int btn) {
	return im->getMouse(btn);
}
bool GL3DrawerCLR::InputManagerCLR::getMouseDown(int btn) {
	return im->getMouseDown(btn);
}
bool GL3DrawerCLR::InputManagerCLR::getMouseUp(int btn) {
	return im->getMouseUp(btn);
}
GL3DrawerCLR::vec2^ GL3DrawerCLR::InputManagerCLR::getMousePos() {
	return gcnew vec2(im->getMousePos());
}

void cppKeyCallback(GLFWwindow * window, int key, int scancode, int action, int mods) {
	if (GL3DrawerCLR::InputManagerCLR::csharpKeyCallback) {
		GL3DrawerCLR::InputManagerCLR::csharpKeyCallback(key, action, scancode);
	}
}
void GL3DrawerCLR::InputManagerCLR::setKeyCallback(keyCallback^ fp) {
	csharpKeyCallback = fp;
}
void GL3DrawerCLR::InputManagerCLR::setMouseCallback(mouseCallback^ fp) {
	csharpMouseCallback = fp;
}
void GL3DrawerCLR::InputManagerCLR::setMouseMoveCallback(mouseMoveCallback^ fp) {
	csharpMouseMoveCallback = fp;
}
GL3DrawerCLR::InputManagerCLR::InputManagerCLR(BaseGLD * base) {
	im = new InputManager();
	base->Input = im;
	link = base;
}