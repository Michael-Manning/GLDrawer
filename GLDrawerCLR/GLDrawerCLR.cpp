#include "stdafx.h"
#include <gl3w/gl3w.h>
#include "GLDrawerCLR.h"
#include <OpenGL3/engine.cpp>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>
#include <Box2D/Box2D.h>

using System::Array;
using namespace std;
using namespace glm;

GLDrawerCLR::unmanaged_Canvas::unmanaged_Canvas(bool packShaders) {
	base = new GLCanvas();
	base->usePackedShaders = packShaders; //the fininall DLL need to be portable. Set this to true on export	
	base->managed = true;
}
int GLDrawerCLR::unmanaged_Canvas::createCanvas(int width, int height, bool borderd, unmanaged_color ^ backColor, bool Vsync, bool DebugMode) {
	if(DebugMode)
		cout << "Running window from C# wrapper \n";
	base->debugMode = DebugMode;
	initialized = !base->createCanvas(width, height, !borderd, vec3(backColor->r, backColor->g, backColor->b), Vsync);
	return initialized;
}

void GLDrawerCLR::unmanaged_Canvas::mainloop(bool render, bool focusContext) {

	//mouse events
	if (csharpMouseCallback)
		for (int i = 0; i < base->MouseBufferLength; i++)
			csharpMouseCallback(base->mouseBuffer[i].btn, base->mouseBuffer[i].action, 1);
	
	//key events
	if (csharpKeyCallback)
		for (int i = 0; i < base->keyBufferLength; i++)
			csharpKeyCallback(base->keyBuffer[i].btn, base->keyBuffer[i].action, 1);
	//mouse move event
	if (base->mouseMoveFlag) {
		base->mouseMoveFlag = false;
		csharpMouseMoveCallback();
	}
	base->keyBufferLength = 0;
	base->MouseBufferLength = 0;

	base->mainloop(render, focusContext);
}
bool GLDrawerCLR::unmanaged_Canvas::checkLoaded(unmanaged_GO ^ g) {
	for (int i = 0; i < base->GameObjects.size(); i++)
		if (base->GameObjects[i] == g->iGO)
			return true;
	return false;
}


//nothing below this point has any significant functionality beyond keeping clutter out of the header file

GLDrawerCLR::unmanaged_polyData::unmanaged_polyData(unmanaged_color ^ FColor, unmanaged_color ^ BColor, float Bwidth, int Sides) {
	p = new polyData(FColor, BColor, Bwidth, Sides);
}
void GLDrawerCLR::unmanaged_polyData::dispose()
{
	delete p;
}
GLDrawerCLR::unmanaged_textData::unmanaged_textData(System::String ^ text, float textHight, unmanaged_color ^ color, int just, System::String ^ path, bool bound, bool kerning)
{
	t = new textData(CString(text), textHight, color, just, toCchar(path), bound, kerning);
}

void GLDrawerCLR::unmanaged_textData::dispose()
{
	delete t;
}

GLDrawerCLR::unmanaged_GO::unmanaged_GO()
{
	iGO = new GO(vec2(0), vec2(0), 0, 0);
}

GLDrawerCLR::unmanaged_GO::unmanaged_GO(float posx, float posy, float scalex, float scaley, float angle, float rotSpeed)
{
	iGO = new GO(vec2(posx, posy), vec2(scalex, scaley), angle, rotSpeed);
}

GLDrawerCLR::unmanaged_GO::unmanaged_GO(unmanaged_polyData ^ p, float posx, float posy, float scalex, float scaley, float angle, float rotSpeed)
	: unmanaged_GO(posx, posy, scalex, scaley, angle, rotSpeed)
{
	iGO->p = p->p;
}

GLDrawerCLR::unmanaged_GO::unmanaged_GO(unmanaged_imgData ^ i, float posx, float posy, float scalex, float scaley, float angle, float rotSpeed)
	: unmanaged_GO(posx, posy, scalex, scaley, angle, rotSpeed)
{
	iGO->i = i->i;
}

GLDrawerCLR::unmanaged_GO::unmanaged_GO(unmanaged_textData ^ t, float posx, float posy, float scalex, float scaley, float angle, float rotSpeed)
	: unmanaged_GO(posx, posy, scalex, scaley, angle, rotSpeed)
{
	iGO->t = t->t;
}

GLDrawerCLR::unmanaged_GO::unmanaged_GO(unmanaged_PS ^ ps, float posx, float posy, float scalex, float scaley, float angle, float rotSpeed)
	: unmanaged_GO(posx, posy, scalex, scaley, angle, rotSpeed)
{
	iGO->ps = ps->iPS;
}

void GLDrawerCLR::unmanaged_GO::dispose()
{
	if (disposed) {

#ifdef _DEBUG
		cout << "Debug warning: multiple disposal attempts on GO detected\n";
#endif
		return;
	}

	delete iGO;
	disposed = true;
}

void GLDrawerCLR::unmanaged_Canvas::removeGO(unmanaged_GO ^ g) {
	base->GORemoveBuffer.push_back(g->iGO);
}
void GLDrawerCLR::unmanaged_Canvas::swapOrder(int a, int b) {
	//base->swapOrder(a, b);
}
void GLDrawerCLR::unmanaged_Canvas::setPos(int x, int y) {
	base->setPos(x, y);
}
void GLDrawerCLR::unmanaged_Canvas::setVisible(bool visible) {
	base->setVisible(visible);
}
void GLDrawerCLR::unmanaged_Canvas::focusWindow() {
	base->focus();
}
void GLDrawerCLR::unmanaged_Canvas::setBBpixel(int x, int y, int r, int g, int b, int a){
	base->setBBPixel(x, y, r, g, b, a);
}
void GLDrawerCLR::unmanaged_Canvas::setBBpixelFast(int x, int y, int r, int g, int b, int a) {
	base->setBBPixelFast(x, y, r, g, b, a);
}
void GLDrawerCLR::unmanaged_Canvas::setBBShape(unmanaged_GO ^ g) {
	base->setBBShape(*g->iGO);
}
GLDrawerCLR::unmanaged_color ^ GLDrawerCLR::unmanaged_Canvas::getPixel(int x, int y) {
	vec3 c = base->getPixel(x, y);
	return gcnew unmanaged_color(vec4(c, 1));
}

IntPtr GLDrawerCLR::unmanaged_Canvas::getNativeHWND() {
	HWND hwnd = base->getNativeHWND();
	return IntPtr(hwnd);
}
void GLDrawerCLR::unmanaged_Canvas::clearBB() {
	base->clearColorFlag = true;
}
void GLDrawerCLR::unmanaged_Canvas::clearShapes() {
	base->clearShapeFlag = true;
}
void GLDrawerCLR::unmanaged_Canvas::setWindowSize(int x, int y) {
	base->setWindowSize(x, y);
}
bool GLDrawerCLR::unmanaged_Canvas::getKey(char key) {
	return base->getKey(key);
}
bool GLDrawerCLR::unmanaged_Canvas::getKeyDown(char key) {
	return base->getKeyDown(key);
}
bool GLDrawerCLR::unmanaged_Canvas::getKeyUp(char key) {
	return base->getKeyUp(key);
}
bool GLDrawerCLR::unmanaged_Canvas::getKey(int key) {
	return base->getKey(key);
}
bool GLDrawerCLR::unmanaged_Canvas::getKeyDown(int key) {
	return base->getKeyDown(key);
}
bool GLDrawerCLR::unmanaged_Canvas::getKeyUp(int key) {
	return base->getKeyUp(key);
}
bool GLDrawerCLR::unmanaged_Canvas::getMouse(int btn) {
	return base->getMouse(btn);
}
bool GLDrawerCLR::unmanaged_Canvas::getMouseDown(int btn) {
	return base->getMouseDown(btn);
}
bool GLDrawerCLR::unmanaged_Canvas::getMouseUp(int btn) {
	return base->getMouseUp(btn);
}
GLDrawerCLR::unmanaged_vec2 ^ GLDrawerCLR::unmanaged_Canvas::getMousePos() {
	return gcnew unmanaged_vec2(base->getMousePos());
}
void GLDrawerCLR::unmanaged_Canvas::setKeyCallback(keyCallback^ fp) {
	csharpKeyCallback = fp;
}
void GLDrawerCLR::unmanaged_Canvas::setMouseCallback(mouseCallback^ fp) {
	csharpMouseCallback = fp;
}
void GLDrawerCLR::unmanaged_Canvas::setMouseMoveCallback(mouseMoveCallback^ fp) {
	csharpMouseMoveCallback = fp;
}

void GLDrawerCLR::unmanaged_PS::setAnimation(System::String ^ filepath, int resolution, int tilesPerLine) {
	iPS->setAnimation(toCchar(filepath), resolution, tilesPerLine);
}
void GLDrawerCLR::unmanaged_PS::dispose()
{
	iPS->dispose();
	delete iPS;
}
void GLDrawerCLR::unmanaged_PS::setTexture(System::String ^ filepath) {
	iPS->setTexture(toCchar(filepath));
}

void GLDrawerCLR::unmanaged_imgData::setAnimation(int size, int cellsPerLine, float freq)
{
	ad = new animationData(size, cellsPerLine, freq);
	i->adata = ad;
}

inline GLDrawerCLR::unmanaged_imgData::unmanaged_imgData(System::String ^ filePath)
{
	i = new imgData(toCchar(filePath));
	i->tint = vec4(0);
}

void GLDrawerCLR::unmanaged_imgData::dispose()
{
	delete i;
	if (animated)
		delete ad;
}

GLDrawerCLR::unmanaged_rigBody::unmanaged_rigBody(unmanaged_Canvas ^ can, unmanaged_GO ^ Link, int type, float friction, bool kinematic, bool sensor)
{
	ibod = new rigBody(&(can->base->world), Link->iGO, type, friction, kinematic, sensor);
}

GLDrawerCLR::unmanaged_rigBody::unmanaged_rigBody(unmanaged_Canvas ^ can, unmanaged_GO ^ Link, int type, float friction, bool kinematic, bool sensor, System::String ^ tag)
{
	ibod = new rigBody(&(can->base->world), Link->iGO, type, friction, kinematic, sensor, toChar(tag));
}

void GLDrawerCLR::unmanaged_rigBody::disable()
{
	ibod->body->SetActive(false);
	ibod->body->SetAwake(false);
	ibod->body->SetUserData(NULL);
}

System::String ^ GLDrawerCLR::unmanaged_rigBody::getCollisionTag()
{
	return gcnew System::String(ibod->collisionTag);
}

GLDrawerCLR::unmanaged_vec2 ^ GLDrawerCLR::unmanaged_rigBody::getVelocity() {

	vec2 v = ibod->GetVelocity();
	return gcnew unmanaged_vec2(v.x, v.y);
}

void GLDrawerCLR::unmanaged_rigBody::dispose()
{
	if (disposed)
		return;
	ibod->world->DestroyBody(ibod->body);
	disposed = true;
	delete ibod;
}
