#pragma once

#include<gl3w/gl3w.h>
#include<GL/GLU.h>
#include<GLFW/glfw3.h>
#include <OpenGL3/engine.h>
#include <glm/glm.hpp>
#include<Windows.h>
#include <Box2D/Box2D.h>
#include <msclr\marshal_cppstd.h>

using namespace std;
using namespace System;
using namespace System::Runtime::InteropServices;


#define CString(text) msclr::interop::marshal_as<std::string>(text) //c# string to c++ string
#define toCchar(text) ((const char*)(Marshal::StringToHGlobalAnsi(text)).ToPointer()) // c# string to constant char *
#define toChar(text) ((char*)(Marshal::StringToHGlobalAnsi(text)).ToPointer()) // c# string to char *
//These macros make creating simple properties way easier. Uses 1 line of code instead of 7. examples below
#define getSet(item, type)	{ type get(){ \
						    return item;}\
					     	void set(type value){\
						    item = value;}}
#define getSetRef(item, type)	{ type ^ get(){ \
								return gcnew type(item);}\
					     		void set(type ^ value){\
								item = value;}}
#define getSetVec(item)	        { unmanaged_vec2 ^ get(){ \
								return gcnew unmanaged_vec2(item);}\
					     		void set(unmanaged_vec2 ^ v){\
								item = glm::vec2(v->x, v->y);}}

					  
namespace GLDrawerCLR {

	public ref struct unmanaged_vec2 {
	public:
		float x, y;
		unmanaged_vec2(float X, float Y) {
			x = X;
			y = Y;
		}
		unmanaged_vec2(glm::vec2 v2) {
			x = v2.x;
			y = v2.y;
		}
	};

	public ref struct unmanaged_color {
		float r, g, b, a;
		//implicit conversion to glm vec4
		operator glm::vec4() { return glm::vec4(r,g,b,a); }
		unmanaged_color(float R, float G, float B, float A) {
			r = R;
			g = G;
			b = B;
			a = A;
		}
		unmanaged_color(vec4 v4) {
			r = v4.x;
			g = v4.y;
			b = v4.z;
			a = v4.w;
		}
		unmanaged_color(unmanaged_color % coppier) {
			r = coppier.r;
			g = coppier.g;
			b = coppier.b;
			a = coppier.a;
		}
	};
	public ref struct unmanaged_polyData {
	public:
		property unmanaged_color ^ fColor getSetRef(p->fColor, unmanaged_color);
		property unmanaged_color ^ bColor getSetRef(p->bColor, unmanaged_color);
		property float bWidth getSet(p->bWidth, float);
		property int sides getSet(p->sides, int);
		unmanaged_polyData(unmanaged_color ^ FColor, unmanaged_color ^ BColor, float Bwidth, int Sides);
		void dispose();
		polyData * p;
	};
	public ref struct unmanaged_imgData {
		property System::String ^ filepath { System::String ^ get() { return gcnew String(i->filepath); }}
		property unmanaged_color ^  tint getSetRef(i->tint, unmanaged_color);
		property unmanaged_vec2 ^ uvPos getSetVec(i->UVpos);
		property unmanaged_vec2 ^ uvScale getSetVec(i->UVscale);
		property float opacity getSet(i->opacity, float);

		void dispose();

		//animation specific
		void setAnimation(int size, int cellsPerLine, float freq);
		bool animated = false;
		unmanaged_imgData(System::String ^ filePath);
		property int nextFrame getSet(ad->nextFrame, int);
		property bool repeat getSet(ad->repeat, bool);
		property bool play getSet(ad->play, bool);

		imgData * i;
		animationData * ad;
	};
	public ref struct unmanaged_textData {
		property unmanaged_color ^ color getSetRef(t->color, unmanaged_color);
		property int justification getSet(t->justification, int);
		property unmanaged_vec2 ^ lastLetterPos getSetVec(t->lastLetterPos);
		property float height getSet(t->height, float);
		property System::String ^ filepath { System::String ^ get() { return gcnew String(t->filepath); }}
		property System::String ^ text {
			System::String ^ get() {
				return gcnew String(t->text.c_str());
			}
			void set(System::String ^ text) {
				t->text = msclr::interop::marshal_as<std::string>(text);
			}
		}
		unmanaged_textData(System::String ^ text, float textHight, unmanaged_color ^ color, int just, System::String ^ path, bool bound, bool kerning);
		void dispose();
		unmanaged_vec2 ^ letterPosAtIndexNDC(int index) { return gcnew unmanaged_vec2(t->letterPosAtIndexNDC(index));}
		textData * t;
	};

	public ref class unmanaged_PS{
	public:	
		//property int maxParticles getSet(iPS->MaxParticles, int);
		property float startSize getSet(iPS->startSize, float);
		property float endSize getSet(iPS->endSize, float);
		property float lifeLength getSet(iPS->lifeLength, float);
		property float lifePrecision getSet(iPS->lifePrecision, float);
		property float spread getSet(iPS->spread, float);
		property float angle getSet(iPS->angle, float);
		property float speed getSet(iPS->speed, float);
		property float speedPrecision getSet(iPS->speedPrecision, float);
		property float radius getSet(iPS->radius, float);
		property unmanaged_vec2 ^ gravity getSetVec(iPS->gravity);
		property unmanaged_vec2 ^ extraVelocity getSetVec(iPS->extraStartVelocity);
		property unmanaged_color ^ startCol getSetRef(iPS->startCol, unmanaged_color);
		property unmanaged_color ^ endCol getSetRef(iPS->endCol, unmanaged_color);
		property bool relitivePosition getSet(iPS->relitivePosition, bool);
		property bool drawBehindShape getSet(iPS->drawBehindShape, bool);
		property bool continuous getSet(iPS->continuous, bool);
		property bool burst getSet(iPS->burstMode, bool);
		void setTexture(System::String ^ filepath);
		void setAnimation(System::String ^ filepath, int resolution, int tilesPerLine);
		unmanaged_PS(int maxCount, float lifeLength) { iPS = new ParticleSystem(maxCount, lifeLength); }
		void dispose();
		ParticleSystem * iPS;
	};

	ref class unmanaged_Canvas;
	ref class unmanaged_GO;
	public ref class unmanaged_rigBody {
	public:
		unmanaged_rigBody(unmanaged_Canvas ^ can, unmanaged_GO ^ Link, int type, float friction, bool kinematic, bool sensor);
		unmanaged_rigBody(unmanaged_Canvas ^ can, unmanaged_GO ^ Link, int type, float friction, bool kinematic, bool sensor, System::String ^ tag);

		void addForce(float x, float y) { ibod->addForce(vec2(x, y)); }
		void addTorque(float t) { ibod->addTorque(t); }
		void setVelocity(float x, float y) { ibod->setVelocity(vec2(x, y)); }
		void disable();
		System::String ^ getCollisionTag();
		unmanaged_vec2 ^ getVelocity();
		void lockRotation(bool flag) { ibod->body->SetFixedRotation(flag); }
		property bool setPositionFlag getSet(ibod->setPositionFlag, bool);
		property bool collisionEnter getSet(ibod->collisionEnter, bool);
		property bool collisionExit getSet(ibod->collisionExit, bool);
		property float angularVelocity {
			float get(){
				return ibod->body->GetAngularVelocity();
			}
			void set(float value) {
				ibod->body->SetAngularVelocity(value);
			}
		}

		bool disposed = false;
		void dispose();
		rigBody * ibod;
	};

	public ref class unmanaged_GO {
	public:
		void setParent(unmanaged_GO ^ g) { iGO->parent = g->iGO; }
		property unmanaged_vec2 ^ position getSetVec(iGO->position);
		property float angle getSet(iGO->angle, float);
		property float rSpeed getSet(iGO->rSpeed, float);
		property unmanaged_vec2 ^ scale getSetVec(iGO->scale);
		property int drawIndex getSet(iGO->drawIndex, int);
		property bool hidden  getSet(iGO->hidden, bool);
		unmanaged_GO();
		unmanaged_GO(float posx, float posy, float scalex, float scaley, float angle, float rotSpeed);
		unmanaged_GO(unmanaged_polyData ^ p, float posx, float posy, float scalex, float scaley, float angle, float rotSpeed);
		unmanaged_GO(unmanaged_imgData ^ i, float posx, float posy, float scalex, float scaley, float angle, float rotSpeed);
		unmanaged_GO(unmanaged_textData ^ t, float posx, float posy, float scalex, float scaley, float angle, float rotSpeed);
		unmanaged_GO(unmanaged_PS ^ ps, float posx, float posy, float scalex, float scaley, float angle, float rotSpeed);
		void setBody(unmanaged_rigBody ^ y) { iGO->body = y->ibod; }
		unmanaged_vec2 ^ getGlobalPosition() { return gcnew unmanaged_vec2( iGO->getGlobalPosition()); }
		void clearParent() {
			iGO->parent = NULL; }
		bool disposed = false;
		void dispose();	
		GO * iGO;
	};

	public delegate void keyCallback(int, int, int);
	public delegate void mouseCallback(int,int,int);
	public delegate void mouseMoveCallback();


	public ref struct unmanaged_InputEvent {
	public:
		int btn = 0, action = 0, read = 0;
		unmanaged_InputEvent(int BTN, int ACTION) { btn = BTN; action = ACTION; read = 0; }
	};

	public ref class unmanaged_Canvas
	{
	public:

		//input manager stuff
		void setKeyCallback(keyCallback^ fp);
		keyCallback^ csharpKeyCallback;
		void setMouseCallback(mouseCallback^ fp);
		mouseCallback^ csharpMouseCallback;
		void setMouseMoveCallback(mouseMoveCallback^ fp);
		mouseMoveCallback^ csharpMouseMoveCallback;

		property int LeftMouseState {int get() { return base->LeftMouseState; }}
		property int RightMouseState {int get() { return base->RightMouseState; }}

		bool getKey(char key);
		bool getKeyDown(char key);
		bool getKeyUp(char key);

		bool getKey(int key);
		bool getKeyDown(int key);
		bool getKeyUp(int key);

		bool getMouse(int button);
		bool getMouseDown(int button);
		bool getMouseUp(int button);
		unmanaged_vec2 ^ getMousePos();

		//canvas stuff
		unmanaged_Canvas(bool packShaders);
		int createCanvas(int width, int height, bool borderd, unmanaged_color ^ backColor, bool Vsync, bool debugMode);
		void setPos(int x, int y);
		void setVisible(bool visible);
		void focusWindow();
		void mainloop(bool render, bool focusContext);
		void close() { glfwSetWindowShouldClose(base->window, true); }
		void removeGO(unmanaged_GO ^ r);
		void swapOrder(int indexA, int indexB);
		bool checkLoaded(unmanaged_GO ^ g);//check if shape is on the canvas
		bool initialized = false;
		void setBBpixel(int x, int y, int r, int g, int b, int a);
		void setBBpixelFast(int x, int y, int r, int g, int b, int a);
		void setBBShape(unmanaged_GO ^ g);
		unmanaged_color ^ getPixel(int x, int y);
		void tempF() { base->getPixel(100, 100); }
		IntPtr getNativeHWND(); //this gets the pointer to the native HWND window from a fully initialised canvas
		void clearBB();
		void setWindowSize(int x, int y);
		void clearShapes();
		void saveCanvasAsImage(System::String ^ path) { base->saveCanvasAsImage(toCchar(path)); }
		void addGO(unmanaged_GO ^ g) { base->addGO(g->iGO); }
		void loadImageAsset(System::String ^ path) { base->assetBuffer.push_back(toCchar(path)); }
		bool raycast(float ax, float ay, float bx, float by) { return base->raycast(vec2(ax, ay), vec2(bx, by)); }

		static bool TestCirc(float posx, float posy, float rad, float testx, float testy) {
			return testCirc(vec2(posx, posy), rad, vec2(testx, testy));
		}
		static bool TestRect(float posx, float posy, float scalex, float scaley, float angle, float testx, float testy) {
			return testRect(vec2(posx, posy), vec2(scalex, scaley), angle, vec2(testx, testy));
		}

		property bool bbCopy getSet(base->setPixelCopyFlag, bool);
		property bool reSize getSet(base->reSizeEvent, bool);
		property unmanaged_vec2 ^ zoom getSetVec(base->cameraZoom);
		property bool disposed getSet(base->disposed, bool);
		property bool centerOffset getSet(base->centerOffset, bool);	
		property bool titleDetails {void set(bool value) { base->titleDetails = value; }}
		property int shapeCount {int get() { return base->GameObjects.size(); }}
		property float ellapsedTime { float get() { return base->currTime; }}
		property unmanaged_vec2 ^ camera getSetVec(base->camera);
		property int width {int get() { return base->resolutionWidth; }void set(int w) { setWindowSize(w, height); }}
		property int height {int get() { return base->resolutionHeight; }void set(int h) { setWindowSize(width, h); }}
		property String ^ title {
			String ^ get() {
				return gcnew String(base->title);
			}
			void set(String ^ s) {
				base->title = toCchar(s);
				base->windowTitleFlag = true;
			}
		}
		property unmanaged_color ^ backColor {
			unmanaged_color ^ get() {
				return gcnew unmanaged_color(base->backCol);
			}
			void set(unmanaged_color ^ color) {
				base->backCol = vec4(color->r, color->g, color->b, 1.0);
			}
		}

		unmanaged_vec2 ^ getGravity() {
			b2Vec2 v = base->world.GetGravity();
			return gcnew unmanaged_vec2(v.x, v.y);
		}
		void setGravity(float x, float y) {
			base->world.SetGravity(b2Vec2(x,y));
		}
		
		~unmanaged_Canvas() {
			this->!unmanaged_Canvas();
		}
		!unmanaged_Canvas() {
			if (!disposed)
				close();
		}

		GLCanvas * base;
	};
}
