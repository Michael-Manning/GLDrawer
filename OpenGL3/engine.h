#pragma once
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <Box2D/Box2D.h>
#include <vector>

#include <stb/stb_truetype.h>

using namespace std;
using namespace glm;

#define TEXTFILE 256;

//struct fontAsset {
//	unsigned char bitmapBuffer[512 * 512];
//	vec2 uvOffset[95]; //location of the letter within the generated bitmap
//	vec2 uvScale[95]; //size ofthe letter within the bitmap
//	vec2 quadScale[95]; //size the letter should be displayed
//	float alignment[95]; //vertical offset of each letter
//	float tallestLetter = 0; //tallest letter in pixels
//	float alignmentOffset = 0; //pixels in which the tallest letter sets below the alignemt. Required for precise global positioning
//	float spaceOff;
//	GLuint id;
//	bool init;
//	const char * filePath;
//
//	fontAsset(const char * filepath);
//	fontAsset() { init = false; }
//	void loadTexture();
//	void dipose();
//};

//these 2 need to be the same for correct kerning
const float fnt_height_px = 64.0f;
const uint32_t fnt_size = 64;

const uint32_t fnt_atlas_padding = 1;
const uint32_t fnt_atlas_width = 1024;
const uint32_t fnt_atlas_height = 1024;
const uint32_t fnt_oversample_x = 2;
const uint32_t fnt_oversample_y = 2;
const uint32_t fnt_first_char = ' ';
const uint32_t fnt_last_char = '~';
const uint32_t fnt_char_count = '~' - ' ';

struct fontAsset {
	GLuint tex_atlas;
	stbtt_fontinfo info;
	float scaleFactor;
	float tallestLetter;
	int ascent;
	int descent;
	int line_gap;
	int baseline;
	stbtt_packedchar * packed_chars;
	stbtt_aligned_quad quads[fnt_last_char];
	const char * filePath;

	bool init;

	fontAsset(const char * filepath);
	fontAsset() { 
		init = false; 
		scaleFactor = 0.0f;
		tallestLetter = 0.0f;
		ascent = 0;
		descent = 0;
		line_gap = 0;
		packed_chars = NULL;
	}
	//void loadTexture();
	//void dipose();
};


//while images are small, the same image should be prevented from be loaded multiple times into the same OpnGL context
struct imgAsset {
	const char * filePath;
	GLuint ID;
	bool init;

	imgAsset(const char * filepath);
	imgAsset() { init = false; }
};

class GO;
struct textData {
	string text;
	const char * filepath;
	vec4 color;
	float height;
	int justification;
	bool boundMode;
	bool useKerning;

	int TextLength = 0;
	float * letterTransData;
	float * letterUVData;
	int hashIndex = -1; //privately accessd
	vec2 lastLetterPos;

	textData(string Text, float textHeight, vec4 Color, int Justification, const char * path, bool bound = false, bool kerning = false);
	textData() { lastLetterPos = vec2(0); };
	vec2 letterPosAtIndexNDC(int index);
	int getHashIndex();
	void dispose();
};

struct animationData {
	int sheetSize = 0, cellSize, cells, cellsPerLine = 0;
	float frequency, iTime = 0;
	vec2 * UVS; //pre computed UV positions
	bool play = true;
	bool repeat = true;
	int nextFrame = 0;

	animationData(int size, int cellsPerLine, float freq);
	void dispose();
};

struct imgData {
	const char * filepath;
	animationData* adata;
	float opacity = 1;
	vec2 UVscale;
	vec2 UVpos;
	vec4 tint;

	//will include settings for stretch to fit, tilling .ect
	imgData(const char* path, vec4 tint = vec4(0), vec2 uvpos = vec2(0),  vec2 uvscale = vec2(1) );
	imgData() { tint = vec4(0); UVpos = vec2(0);  UVscale = vec2(1); adata = NULL; }
	int getHashIndex();
	int hashIndex = -1; //privately accessd
};

struct polyData {
	vec4 fColor;
	vec4 bColor;
	
	float bWidth;
	int sides;

	polyData(vec4 fCol = vec4(1), vec4 bCol = vec4(0), float BWidth = 0, int Sides = 4);
};


struct Particle {
	vec2 pos, vel;
	unsigned char r, g, b, a; // Color
	float size;
	float life; 
	Particle(vec2 Pos, float x, float y, float Life, float startSize, vec4 startCol) {
		pos = Pos;
		vel = vec2(x, y);
		size = startSize;
		life = Life;
		r = (int)startCol.r * 255;
		g = (int)startCol.g * 255;
		b = (int)startCol.b * 255;
		a = (int)startCol.a * 255;
	}
	Particle() {
		life = 1;
	}
};

float RandomFloat(float a, float b);

//everything needs a default with C++/CLI
class ParticleSystem {
public:
	//data
	Particle * container;
	float * positionSizeData;
	unsigned char * colorData;
	float UVScale = 0;
	float * UVData; //per particle UVs
	vec2 * UVS; //UVs of the sprite sheet
	int count = 0;

	//settings
	int MaxParticles = 0;
	vec2 spawnLocation;	
	float startSize = 0, endSize = 0, lifeLength = 0, lifePrecision = 0;
	bool burstMode = false, continuous = true;
	vec4 startCol, endCol;
	float spread = 0, angle = 0, speed = 0, speedPrecision = 0;
	int spawnRate = 0; //#new particles per frame in continuous
	float internalDelta = 0; //smooths out spawn rates, especially at high framerates
	vec2 gravity;
	vec2 extraStartVelocity;
	float radius = 0;
	bool relitivePosition = true;
	bool drawBehindShape = true;

	ParticleSystem(int maxCount, float LifeLength);
	ParticleSystem() {};
	const char * filepath; //used by parent GO imgdata
	void setTexture(const char * Texture);
	void setAnimation(const char * Texture, int resolution, int tilesPerLine);
	void updateParticles(float delta, float extraAngle); //angle to match gameobject
	bool disposed = true;
	void dispose(){
		if (disposed)
			return;
		disposed = true;
		delete container;
		delete positionSizeData;
		delete colorData;
		delete UVData;
		if(textureMode)
			delete UVS;
	}

	imgData img;
	bool textureMode = false;
	bool tileMode = false;
	bool dead = false;
	int tileCount;
	int resolution;
	bool burstTrigger = true;
};

const float phScale = 100; //pixel/unit ratio for physics

bool testCirc(vec2 circPos, float rad, vec2 test);
bool testRect(vec2 rectPos, vec2 scale, float angle, vec2 Test);

class rigBody {
public:
	GO * link;
	b2World * world;
	b2Body * body;
	b2FixtureDef fixtureDef;
	bool kinematic = false;
	bool setPositionFlag = false; //for C# transform setting
	bool collisionEnter = false;
	bool collisionExit = false;
	char * tag;
	char * collisionTag = NULL; //tag of what was touched

	void addForce(vec2 force);
	void addTorque(float torque);
	void setVelocity(vec2 velocity);
	vec2 GetVelocity();
	rigBody(b2World * World, GO * Link, int type, float friction = 0.8f, bool Kinimatic = false, bool trigger = false, char * tag = NULL);
};

//GameObject
class GO {
public:
	GO * parent;
	polyData * p;
	imgData * i;
	textData * t;
	ParticleSystem * ps;

	rigBody * body;

	vec2 position;
	vec2 scale;
	float angle;
	float rSpeed = 0;
	int drawIndex = 0;
	bool hidden = false;

	vec2 getGlobalPosition();

	GO(vec2 pos = vec2(0), vec2 Scale = vec2(0), float Angle = 0, float rotationSpeed = 0);
	GO(polyData * poly, vec2 pos = vec2(0), vec2 Scale = vec2(0), float Angle = 0, float rotationSpeed = 0);
	GO(imgData * img, vec2 pos = vec2(0), vec2 Scale = vec2(0), float Angle = 0, float rotationSpeed = 0);
	GO(textData * text, vec2 pos = vec2(0), vec2 Scale = vec2(0), float Angle = 0, float rotationSpeed = 0);
	GO(ParticleSystem * part, vec2 pos = vec2(0), vec2 Scale = vec2(0), float Angle = 0, float rotationSpeed = 0);
};


extern vector<const char *> fontHashLookup;
extern vector<const char *> imgHashLookup;
extern b2BodyDef bodyDef; //for physics
b2Vec2 toB2(vec2 v);

//box2d collision events
class MyContactListener : public b2ContactListener
{
public:
	void BeginContact(b2Contact* contact);
	void EndContact(b2Contact* contact);
	void PreSolve(b2Contact* contact, const b2Manifold* oldManifold) {}
	void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse) {}
};

class RayCastCallback : public b2RayCastCallback
{
public:
	RayCastCallback() : m_fixture(NULL) {
	}

	float32 ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float32 fraction) {
		m_fixture = fixture;
		m_point = point;
		m_normal = normal;
		m_fraction = fraction;
		return fraction;
	}

	b2Fixture* m_fixture;
	b2Vec2 m_point;
	b2Vec2 m_normal;
	float32 m_fraction;

};

//for input rollover and events
struct inputDescription {
	int btn, action, read;
	bool operator==(const inputDescription& b) {
		return (action == b.action && btn == b.btn);
	}
};

//very slight performance boost
struct Pixel {
	int xy, r, g, b, a;
};

class GLCanvas {
public :
	GLFWwindow * window;

	//input manager stuff
	void onKeyboard(int key, int scancode, int action, int mods);
	void onMouse(int button, int action, int mods);
	void onCursor();
	int keyBufferLength = 0, MouseBufferLength = 0;
	inputDescription keyBuffer[10];
	inputDescription mouseBuffer[10];

	bool mouseMoveFlag = false;

	int keyStates[348];
	int LeftMouseState, RightMouseState;
	void clearStates();

	bool getKey(char key);
	bool getKeyDown(char key);
	bool getKeyUp(char key);

	bool getKey(int key);
	bool getKeyDown(int key);
	bool getKeyUp(int key);

	vec2 getMousePos();
	bool getMouse(int button);
	bool getMouseDown(int button);
	bool getMouseUp(int button);


	//backend only
	const int expectedTextLength = 200; //number of letters to preallocated VRAM (can handle larger at runtime)
	void setVisible(bool visible);
	bool usePackedShaders = false;
	float aspect; //canvas aspect ratio
	vec2 resolutionV2f; //resolution as a vecc2
	//vec2 prevWindowSize; //used to detect window resizing
	bool windowSizeChanged = false;
	float currTime; //latest GLFW time response
	float prevTime;
	float currentFPS;
	float LastRenderTime;
	textData infoText;//used to display debug info on the canvas
	GO infoGO; //used to display debug info on the canvas
	float debugTimer = 0; //used to time frequency of debug information updates
	float debugUpdateFreq = 0.2; //how many seconds to wait between info updates
	std::string debugString;
	volatile bool setPixelFlag = false; //used to determin if work needs to be done based on wether a pixel was set since the previous frame
	volatile bool setPixelCopyFlag = false;//used to pause the main thread if it tries to write setpixel data while being coppied
	volatile bool shapeCopyFlag = false; //main thread interupt to prevent loss while coppying the shape buffer for shapes added that frame
	bool firstFrame = true;

	//used by back/middle end
	const char * title = "Running from Native C++    ";
	bool titleDetails = true; //toggle for fps, render time, and shape count
	bool disposed = false; //to prevent multiple cleanups
	bool clearColorFlag = true; //resets the back buffer in the next from if true
	bool debugMode = false;
	bool reSizeEvent = false; //window resize
	void setWindowSize(int w, int h);
	int resolutionWidth;
	int resolutionHeight;
	bool clearShapeFlag = false;
	bool windowTitleFlag = true; //setting the title takes insanely long, so it should only be updated if there's a change
	bool centerOffset = false;// 0,0 is bottom left of the string
	vec2 camera;
	vec2 cameraZoom = vec2(1);
	int ParticleLimit = 10000; //max particles per system. Required for memory allocation
	vec4 backCol;
	bool managed = false; //wether code is being run from the wrapper vs native

	//backend only
	int loadShader(const char * vertexFilename, const char * fragmentFilename); //part of the canvas class to link the packed shaders boolean
	void setFont(GO * g);
	void DsetFont(GO * g);
	void setTexture(GO *, GLuint textureLocation);
	void setPolygon(GO * g); 
	void createSetPixelTexture();
	void loadImageAsset(const char * filepath); //like setTexture, but only loads the image to memory for later use
	void drawParticleSystem(GO * g, float deltaTime, mat4 *global = NULL);
	void clearSetPixelData(bool makeOnly = false);
	void setGOTransform(GO * s, GLuint aspect, GLuint scale, GLuint pos, GLuint rot, GLuint zoom);
	void LocalTransformHelper(GO * child, mat4 * m);
	void saveCanvasAsImage(const char * fileName);
	mat4 getLocalTransform(GO * child);
	void drawGameobjectShape(GO * g); //automatically applies localized transformations
	GLCanvas();
	bool Borderd = false; //skips writing window title

	//used by back/middle end
	void setPos(int x, int y);
	int createCanvas(int width, int height, bool borderd, vec3 backCol, bool Vsync, bool cursorHidden = false);
	void addGO(GO* g);
	void setBBPixel(int x, int y,int r, int g, int b, int a);
	void setBBPixelFast(int x, int y,int r, int g, int b, int a);
	void setBBShape(GO g); 
	void dispose();
	vec3 getPixel(int x, int y);
	void updateDebugInfo();
	HWND getNativeHWND();
	void focus();
	void mainloop(bool render = true, bool focusContext = true); //steps the program forward (and maybe renders the scene)

	//physics
	b2World world = b2World(b2Vec2(0, -10)); //world with default gravity
	MyContactListener contactListener;
	RayCastCallback raycastCB;
	bool raycast(vec2 start, vec2 end); //has the ability to report what it hit, but doesn't right now
	int32 velocityIterations = 50; //6
	int32 positionIterations = 20; //3
	float32 timeStep = 1.0f / 60.0f;

	vector<GO*> GameObjects;
	vector<GO*> GOBuffer;
	vector<GO*> GORemoveBuffer;
	vector<GO> BBQue;

	vector<const char *> assetBuffer;
	vector<fontAsset> fonts;
	vector<imgAsset> imgs;
	unsigned char* setPixelData;
	unsigned char* setPixelMask; //optimizes set pixel
	vector<Pixel> spTransferBuffer; //stores set requests while sending data to the GPU
	GLuint setPixelDataID;
	GLuint setPixelMaskID;

	int PolygonShaderProgram, 
		textureShaderProgram, 
		fontShaderProgram, 
		ParticleShaderProgram,
		setPixelShaderProgram;
	GLuint VBO, VAO, EBO;
	GLuint PPosVBO, PColVBO, PVertVBO, PuvVBO; //for particle systems
	GLuint FTranVBO, FuvVBO; //for text
	GLuint texture;
	GLuint fboIdA;
	GLuint fboTextIdA;
	GLuint fboIdB;
	GLuint fboTextIdB;

	//polygon shader uniforms
	GLuint PxformUniformLocation;
	GLuint PColorUniformLocation;
	GLuint PbordColorUniformLocation;
	GLuint PsideCountUniformLocation;
	GLuint PshapeScaleUniformLocation;
	GLuint PborderWidthUniformLocation;
	//GLuint PtextureUniformLocation;
	GLuint PtimeUniformLocation;

	GLuint PmPosUniformLocation;
	GLuint PmScaleUniformLocation;
	GLuint PmRotUniformLocation;
	//GLuint PUVscaleUniformLocation;
	//GLuint PUVposUniformLocation;
	GLuint PaspectUniformLocation;
	GLuint PzoomUniformLocation;

	//texture shader uniforms
	GLuint FtextureUniformLocation;
	GLuint FColorUniformLocation; //tinting

	GLuint FOpacityUniformLocation;
	GLuint FxformUniformLocation;
	GLuint FmPosUniformLocation;
	GLuint FmScaleUniformLocation;
	GLuint FmRotUniformLocation;
	GLuint FUVscaleUniformLocation;
	GLuint FUVposUniformLocation;
	GLuint FaspectUniformLocation;
	GLuint FzoomUniformLocation;

	//set pixel shader uniforms
	GLuint SPtextureUniformLocation;
	GLuint SPTextureMaskUniformLocation;

	GLuint SPxformUniformLocation;
	GLuint SPPosUniformLocation;
	GLuint SPScaleUniformLocation;
	GLuint SPRotUniformLocation;
	GLuint SPUVscaleUniformLocation;
	GLuint SPUVposUniformLocation;
	GLuint SPaspectUniformLocation;
	GLuint SPzoomUniformLocation;

	//font shader uniforms
	GLuint FonttextureUniformLocation;
	GLuint FontColorUniformLocation;
	GLuint FonttimeUniformLocation;

	GLuint FontmPosUniformLocation;
	GLuint FontmScaleUniformLocation;
	GLuint FontmRotUniformLocation;
	GLuint FontUVscaleUniformLocation;
	GLuint FontUVposUniformLocation;
	GLuint FontaspectUniformLocation;
	GLuint FontxformUniformLocation;
	GLuint FontzoomUniformLocation;

	//particle shader uniforms
	GLuint ParticlePosUniformLocation;
	GLuint ParticleResUniformLocation; //particle systems themselves don't have accsess to the canvas resolution
	GLuint ParticleTextureUniformLocation;
	GLuint ParticleUVScaleUniformLocation;
	GLuint ParticlexformUniformLocation;
};
