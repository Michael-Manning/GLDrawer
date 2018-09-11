#pragma once

#include<gl3w/gl3w.h>
#include<GL/GLU.h>
#include<GLFW/glfw3.h>
#include<link.h>
#include<Input.h>
#include <glm/glm.hpp>
#include<Windows.h>
#include <msclr\marshal_cppstd.h>

using namespace std;
using namespace System;

using namespace System::Runtime::InteropServices;

namespace GL3DrawerCLR {
	const char* toCchar(String ^ s) {
		return (const char*)(Marshal::StringToHGlobalAnsi(s)).ToPointer();
	}

	public ref struct vec2 {
	public:
		float x, y;
		vec2(float X, float Y) {
			x = X;
			y = Y;
		}
		vec2() {
			x = 0;
			y = 0;
		}
		vec2(glm::vec2 v2) {
			x = v2.x;
			y = v2.y;
		}
		vec2(vec2 % v) {
			x = v.x;
			y = v.y;
		}
	};

	public ref struct RGBA {
		float r, g, b, a;
		//implicit conversion to glm vec4
		operator glm::vec4() { return glm::vec4(r,g,b,a); }
		RGBA(float R, float G, float B, float A) {
			r = R;
			g = G;
			b = B;
			a = A;
		}
		RGBA(vec4 v4) {
			r = v4.x;
			g = v4.y;
			b = v4.z;
			a = v4.w;
		}
		RGBA(RGBA % coppier) {
			r = coppier.r;
			g = coppier.g;
			b = coppier.b;
			a = coppier.a;
		}
	};
	public ref struct Rect {
	public:
		property vec2 ^ Pos {
			vec2 ^ get() {
				return gcnew vec2(r->pos);
			}
			void set(vec2 ^ v) {
				r->pos = glm::vec2(v->x, v->y);
			}
		}
		property vec2 ^ Scale {
			vec2 ^ get() {
				return gcnew vec2(r->scale);
			}
			void set(vec2 ^ v) {
				r->scale = glm::vec2(v->x, v->y);
			}
		}
		property float Angle {
			float get() {
				return r->angle;
			}
			void set(float value) {
				r->angle = value;
			}
		}		
		property RGBA ^ Color {
			RGBA ^ get() {
				return gcnew RGBA(r->color);
			}
			void set(RGBA ^ value) {
				r->color = value;
			}
		}
		property RGBA ^ BorderColor {
			RGBA ^ get() {
				return gcnew RGBA(r->borderColor);
			}
			void set(RGBA ^ value) {
				r->borderColor = value;
			}
		}
		property float BordWidth {
			float get() {
				return r->borderW;
			}
			void set(float f) {
				r->borderW = f;
			}
		}
		property float rSpeed {
			float get() {
				return r->rotSpeed;
			}
			void set(float value) {
				r->rotSpeed = value;
			}
		}
		property bool hidden {
			bool get() {
				return r->hidden;
			}
			void set(bool value) {
				r->hidden = value;
			}
		}
		property int sides {
			int get() {
				return r->sides;
			}
			void set(int value) {
				r->sides = value;
			}
		}
		property int justification {
			int get() {
				return r->justification;
			}
			void set(int value) {
				r->justification = value;
			}
		}
		property System::String ^ text {
			System::String ^ get() {
				return gcnew String(r->text.c_str());
			}
			void set(System::String ^ text) {
				r->text = msclr::interop::marshal_as<std::string>(text);
			}
		}
		property System::String ^ filepath {
			System::String ^ get() {
				return gcnew String(r->path);
			}
		}

		Rect(float x, float y, float w, float h, float angle ,RGBA ^ color, RGBA ^ borderColor, float borderWidth, float rotationSpeed) {
			r = new rect(glm::vec2(x, y), glm::vec2(w, h), angle, color, borderColor, borderWidth, rotationSpeed);
		}
		Rect(float x, float y, float w, float h, float angle, RGBA ^ color, RGBA ^ borderColor, float borderWidth, float rotationSpeed, int sides) {
			r = new rect(glm::vec2(x, y), glm::vec2(w, h), angle, color, borderColor, borderWidth, rotationSpeed, sides);
		}
		//As a texture
		Rect(System::String ^ filePath, float x, float y, float w, float h, float angle, RGBA ^ color, RGBA ^ borderColor, float borderWidth, float rotationSpeed) {
			const char* cpath = toCchar(filePath);
			r = new rect(cpath, glm::vec2(x, y), glm::vec2(w, h), angle, color, borderColor, borderWidth, rotationSpeed);
		}
		//As a font
		Rect(System::String ^ text, float x, float y, float height, RGBA ^ color, int justification, Rect ^ bound, System::String ^ font, float angle, float rotationSpeed) {
			const char* cpath = toCchar(font);
			std::string unmanagedText = msclr::interop::marshal_as<std::string>(text);
			r = new rect(cpath, unmanagedText, text->Length, glm::vec2(x, y), height, justification, bound->r, angle, color, vec4(), 0.0f, rotationSpeed);
		}
		Rect(System::String ^ text, float x, float y, float height, RGBA ^ color, int justification, System::String ^ font, float angle, float rotationSpeed) {
			const char* cpath = toCchar(font);
			std::string unmanagedText = msclr::interop::marshal_as<std::string>(text);
			r = new rect(cpath, unmanagedText, text->Length, glm::vec2(x, y), height, justification, NULL, angle, color, vec4(), 0.0f, rotationSpeed);
		}

		void dispose();
		int index;
		rect* r;
	};

	public delegate void keyCallback(int, int, int);
	public delegate void mouseCallback(int,int,int);
	public delegate void mouseMoveCallback();
	public ref class InputManagerCLR {
	public:
		InputManagerCLR(BaseGLD * base);
		
		void setKeyCallback(keyCallback^ fp);
	    static keyCallback^ csharpKeyCallback;
		void setMouseCallback(mouseCallback^ fp);
	    mouseCallback^ csharpMouseCallback;
		void setMouseMoveCallback(mouseMoveCallback^ fp);
		mouseMoveCallback^ csharpMouseMoveCallback;

	    property int LeftMouseState {int get() { return im->LeftMouseState; }}
		property int RightMouseState {int get() { return im->RightMouseState; }}	

		bool getKey(char key);
		bool getKeyDown(char key);
		bool getKeyUp(char key);

		bool getKey(int key);
		bool getKeyDown(int key);
		bool getKeyUp(int key);

		bool getMouse(int button);
		bool getMouseDown(int button);
		bool getMouseUp(int button);
		vec2^ getMousePos();

		void dispose() {
			delete im;
		}
	private:
		InputManager * im;
		BaseGLD * link;
	};
	public ref class GLDWrapper
	{
	public:
		GLDWrapper(bool packShaders);
		InputManagerCLR ^ Input;
		int createCanvas(int width, int height, bool borderd, RGBA ^ backColor, bool Vsync, bool debugMode);
		void setPos(int x, int y);
		void setVisible(bool visible);
		void focusWindow();
		int addRect(Rect ^);
		void mainloop(); //returns 1 if the window should close
		void cleaarNullRects();
		void removeRect(Rect ^ r);
		void swapOrder(int indexA, int indexB);
		void addBBShape(Rect ^ r) {
			base->BBQue.push_back(*r->r);
		}
		void setBBpixel(int x, int y, RGBA ^ col) {
			base->setBBPixel(glm::vec2(x, y), (vec4)col);
		}
		RGBA ^ getPixel(int x, int y) {
			vec3 c = base->getPixel(x, y);
			return gcnew RGBA(vec4(c, 1));
		}
		//this gets the pointer to the native HWND window from a fully initialised canvas
		IntPtr getNativeHWND() {
			HWND hwnd = base->getNativeHWND();
			return IntPtr(hwnd);
		}
		void clearBB() {
			base->clearColorFlag = true;
		}

		void dispose() {
			base->cleanup();
			//Input->dispose();
		//	delete base->Input;
			delete base;
		}
		property bool shouldClose {
			bool get() {
				return base->closeFlag;
			}
			void set(bool value) { //assumed to be true
				base->closeFlag = value;
			}
		}
		property String ^ title {
			String ^ get() {
				return gcnew String(base->title);
			}
			void set(String ^ s) {
				base->title = toCchar(s);
			}
		}
		property RGBA ^ backColor {
			RGBA ^ get() {
				return gcnew RGBA(base->backCol);
			}
			void set(RGBA ^ color) {
				base->backCol = vec4(color->r, color->g, color->b, 1.0);
			}
		}
		property bool titleDetails {
			void set(bool value) {
				base->titleDetails = value;
			}
		}
		
	private:
		BaseGLD * base;
	};
}
