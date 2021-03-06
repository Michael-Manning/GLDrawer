#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <tchar.h>
#include <math.h>
#include <assert.h>

#include <gl3w/gl3w.h>

#include <GL/GLU.h>
#include <GLFW/glfw3.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "engine.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#define STB_TRUETYPE_IMPLEMENTATION 
#include <stb/stb_truetype.h>

#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <Box2D/Box2D.h>

#include "shaders.h"

using namespace std;
using namespace glm;

////////Usefull macros

template <typename F> struct _scope_exit_t {
	_scope_exit_t(F f) : f(f) {}
	~_scope_exit_t() { f(); }
	F f;
};
template <typename F> _scope_exit_t<F> _make_scope_exit_t(F f) {
	return _scope_exit_t<F>(f);
};
#define _concat_impl(arg1, arg2) arg1 ## arg2
#define _concat(arg1, arg2) _concat_impl(arg1, arg2)
#define defer(code) \
	auto _concat(scope_exit_, __COUNTER__) = _make_scope_exit_t([=](){ code; })

//replaces glfunction() with gl(function()) and prints errors automatically
#ifdef NDEBUG
#define gl(OPENGL_CALL) \
	gl##OPENGL_CALL
#else
#define gl(OPENGL_CALL) \
	gl##OPENGL_CALL; \
	{ \
		int potentialGLError = glGetError(); \
		if (potentialGLError != GL_NO_ERROR) \
		{ \
			fprintf(stderr, "OpenGL Error '%s' (%d) gl" #OPENGL_CALL " " __FILE__ ":%d\n", (const char*)gluErrorString(potentialGLError), potentialGLError, __LINE__); \
			assert(potentialGLError == GL_NO_ERROR && "gl" #OPENGL_CALL); \
		} \
	}
#endif
#define polyTransUniforms PaspectUniformLocation, PmScaleUniformLocation, PmPosUniformLocation, PmRotUniformLocation, PzoomUniformLocation
#define textTransUniforms FaspectUniformLocation, FmScaleUniformLocation, FmPosUniformLocation, FmRotUniformLocation, FzoomUniformLocation
#define fontTransUniforms FontaspectUniformLocation, FontmScaleUniformLocation, FontmPosUniformLocation, FontmRotUniformLocation
#define FBOUniforms() 		gl(Uniform2f(FmScaleUniformLocation, 1.0, 1.0f));\
							gl(Uniform2f(FmPosUniformLocation, 0, 0));\
							gl(Uniform1f(FmRotUniformLocation, 0.0f));\
							gl(Uniform2f(FUVscaleUniformLocation, 1, 1));\
							gl(Uniform2f(FUVposUniformLocation, 0, 0));\
							gl(Uniform1f(FaspectUniformLocation, aspect));\
						    gl(Uniform4f(FColorUniformLocation, 0, 0,0,0));\
							gl(Uniform2f(FzoomUniformLocation, 1, 1));\
							gl(Uniform1f(FOpacityUniformLocation, 1));\

#define asizei(a) (int)(sizeof(a)/sizeof(a[0]))

//defined in header
vector<const char *> fontHashLookup;
vector<const char *> imgHashLookup;

const unsigned int shapeIndices[] = {  // note that we start from 0!
	0, 3, 1,  // first Triangle
	1, 3, 2   // second Triangle
};

const vec3 rectVertices[] = {
	{ -1.0f,  1.0f,  0.0f, },  // top left
{ 1.0f,  1.0f,  0.0f, },  // top right
{ 1.0f, -1.0f,  0.0f },  // bottom right
{ -1.0f, -1.0f,  0.0f, },  // bottom left
{ -1.0f,  1.0f,  0.0f, },  // top left
};

const GLfloat particleVertices[] = {
 -1.0f, -1.0f, 0.0f,
 1.0f, -1.0f, 0.0f,
 -1.0f, 1.0f, 0.0f,
 1.0f, 1.0f, 0.0f,
};
ParticleSystem::ParticleSystem(int maxCount, float LifeLength) {
	container = new Particle[maxCount];
	positionSizeData = new float[maxCount * 3]();
	colorData = new unsigned char[maxCount * 4]();
	UVData = new float[maxCount * 2]();
	MaxParticles = maxCount;
	lifeLength = LifeLength;
	spawnRate = MaxParticles / lifeLength;
	spawnLocation = vec2();
	gravity = vec2();
	extraStartVelocity = vec2();
	startCol = vec4();
	endCol = vec4();
	spawnLocation = vec2();
	disposed = false;
}
void ParticleSystem::updateParticles(float delta, float extraAngle) {
	int newParticles = 0;
	if (burstMode) {
		if (burstTrigger) {
			newParticles = MaxParticles;
			burstTrigger = false;
		}
	}
	else {
		newParticles = spawnRate * internalDelta;
		if (newParticles == 0 && count != MaxParticles) {
			internalDelta += delta;
		}
		else
			internalDelta = 0;
	}

	//if (count == MaxParticles)
		//internalDelta = 0;

	if (!burstMode || newParticles > 0) {
		if (newParticles + count > MaxParticles)
			newParticles = MaxParticles - count;

		for (int i = 0; i < newParticles + count; i++)
		{
			if ((continuous && container[i].life == 0) || (i >= count && i < newParticles + count)) {
				float newangle = angle + RandomFloat(-spread / 2, spread / 2) + extraAngle;
				float newspeed = speed + RandomFloat(-speedPrecision / 2, speedPrecision / 2);
				float newLife = lifeLength - RandomFloat(0, lifePrecision);
				vec2 newPos = spawnLocation + vec2(RandomFloat(-radius / 2, radius / 2), RandomFloat(-radius / 2, radius / 2));
				container[i] = Particle(newPos, cos(newangle) * newspeed + extraStartVelocity.x, sin(newangle) * newspeed + extraStartVelocity.y, newLife, startSize, startCol);
			}
		}
	}


	count += newParticles;

	bool notDead = !(count > 0);
	for (int i = 0; i < count; i++) {
		Particle * p = container + i; // shortcut
		if (p->life > 0)
			notDead = true;

		p->life -= delta;
		if (p->life < 0)
			p->life = 0;
		p->pos += p->vel * delta;
		p->vel += +gravity * delta;

		float progess = (p->life / lifeLength);
		p->size = startSize * progess + endSize * (1.0f - progess);
		p->r = (startCol.r * progess + endCol.r * (1.0f - progess)) * 250;
		p->g = (startCol.g * progess + endCol.g * (1.0f - progess)) * 250;
		p->b = (startCol.b * progess + endCol.b * (1.0f - progess)) * 250;
		p->a = (startCol.a * progess + endCol.a * (1.0f - progess)) * 250;

		positionSizeData[3 * i + 0] = p->pos.x;
		positionSizeData[3 * i + 1] = p->pos.y;
		positionSizeData[3 * i + 2] = p->size;

		colorData[4 * i + 0] = p->r;
		colorData[4 * i + 1] = p->g;
		colorData[4 * i + 2] = p->b;
		colorData[4 * i + 3] = p->a;

		if (tileMode) {
			UVData[i * 2] = UVS[(int)(tileCount * progess)].x;
			UVData[i * 2 + 1] = UVS[(int)(tileCount * progess)].y;
		}
		else if (textureMode) {
			UVData[i * 2] = 0.5f;
			UVData[i * 2 + 1] = 0.5f;
		}

	}

	if (!notDead) {
		if (!continuous) {
			dead = true;
			dispose();
		}
		else if (burstMode)
			burstTrigger = true;
	}
}

void ParticleSystem::setTexture(const char * Texture) {
	filepath = Texture;
	textureMode = true;
	UVScale = 1.0;
}
void ParticleSystem::setAnimation(const char * Texture, int WH, int TPL) {
	filepath = Texture;
	tileMode = true;
	textureMode = true;
	UVScale = WH / TPL;
	tileCount = TPL * TPL;
	UVS = new vec2[tileCount];
	resolution = WH;
	for (int i = 0; i < tileCount; i++)
	{
		float x = i % TPL * UVScale + (UVScale / 2);
		float y = i / TPL * UVScale + (UVScale / 2);
		UVS[i] = vec2(x / WH, y / WH);

	}
}

float RandomFloat(float a, float b) {
	float random = ((float)rand()) / (float)RAND_MAX;
	float diff = b - a;
	float r = random * diff;
	return a + r;
}


void GLCanvas::addGO(GO * g) {
	GOBuffer.push_back(g);
}

const char * defaultFilepath = "../data/"; //use if the shader folder is outside of the bin dishapeory

int GLCanvas::loadShader(const char * vertexFilename, const char * fragmentFilename) {

	const char * vertexShaderSource;
	const char * fragmentShaderSource;
	string str, str2; //can't be local

	//if the packed shaders are being used, the filename is a pointer the actual packed shader. The variable name should be changed
	if (usePackedShaders) {
		vertexShaderSource = vertexFilename;
		fragmentShaderSource = fragmentFilename;
	}
	else {
		ostringstream sstream;

		ifstream fs((string)defaultFilepath + "Shaders\\" + (string)vertexFilename + ".glsl");
		sstream << fs.rdbuf();
		str = sstream.str();
		vertexShaderSource = str.c_str();

		if (str == "") {
			std::cout << "ERROR: Failed to load vertex shader source \"" << vertexFilename << "\"" << "\n" << std::endl;
		}

		ostringstream sstream2;
		ifstream fs2((string)defaultFilepath + "Shaders\\" + (string)fragmentFilename + ".glsl");
		sstream2 << fs2.rdbuf();
		str2 = sstream2.str();
		fragmentShaderSource = str2.c_str();

		if (str2 == "") {
			std::cout << "ERROR: Failed to load fragment shader source \"" << fragmentFilename << "\"" << "\n" << std::endl;
		}
	}
	int shaderProgram;

	int vertexShader = gl(CreateShader(GL_VERTEX_SHADER));
	gl(ShaderSource(vertexShader, 1, &vertexShaderSource, NULL));
	gl(CompileShader(vertexShader));

	int success;
	char infoLog[512];
	gl(GetShaderiv(vertexShader, GL_COMPILE_STATUS, &success));
	if (!success)
	{
		gl(GetShaderInfoLog(vertexShader, 512, NULL, infoLog));
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	int fragmentShader = gl(CreateShader(GL_FRAGMENT_SHADER));
	gl(ShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL));
	gl(CompileShader(fragmentShader));

	gl(GetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success));
	if (!success)
	{
		gl(GetShaderInfoLog(fragmentShader, 512, NULL, infoLog));
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	shaderProgram = gl(CreateProgram());
	gl(AttachShader(shaderProgram, vertexShader));
	gl(AttachShader(shaderProgram, fragmentShader));
	gl(LinkProgram(shaderProgram));

	gl(GetProgramiv(shaderProgram, GL_LINK_STATUS, &success));
	if (!success) {
		gl(GetProgramInfoLog(shaderProgram, 512, NULL, infoLog));
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}

	gl(DeleteShader(vertexShader));
	gl(DeleteShader(fragmentShader));
	return shaderProgram;
}

void GLCanvas::dispose() {
	if (disposed)
		return;
	//delete set pixel buffer
//	delete setPixelData;
	//delete font asset heap data
	//for (int i = 0; i < fonts.size(); i++)
	//	fonts[i].dipose();
	//delte text heap data
	for (int i = 0; i < GameObjects.size(); i++)
		if (GameObjects[i]->t)
			GameObjects[i]->t->dispose();
	//glfwTerminate();
	disposed = true;
}
void GLCanvas::setWindowSize(int w, int h) {
	glfwSetWindowSize(window, w, h);
}

void GLCanvas::setPos(int x, int y)
{
	glfwSetWindowPos(window, x, y);
}
void GLCanvas::setVisible(bool visible) {
	if (!visible)
		glfwHideWindow(window);
	else
		glfwShowWindow(window);
}
void GLCanvas::focus() {
	if (window)
		glfwFocusWindow(window);
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	static_cast<GLCanvas*>(glfwGetWindowUserPointer(window))->onKeyboard(key, scancode, action, mods);
}
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	static_cast<GLCanvas*>(glfwGetWindowUserPointer(window))->onMouse(button, action, mods);
}
void mouseMoveCallback(GLFWwindow* window, double x, double y) {
	static_cast<GLCanvas*>(glfwGetWindowUserPointer(window))->onCursor();
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	static_cast<GLCanvas*>(glfwGetWindowUserPointer(window))->onMouse(3, (int)yoffset, 0);
}
//unexpected resize + set pixel = death
void window_size_callback(GLFWwindow* window, int width, int height) {
	GLCanvas * base = static_cast<GLCanvas*>(glfwGetWindowUserPointer(window));
	base->reSizeEvent = true;
	base->windowSizeChanged = true;
}


polyData::polyData(vec4 fCol, vec4 bCol, float BWidth, int Sides)
{
	fColor = fCol;
	bColor = bCol;
	bWidth = BWidth;
	sides = Sides;
}
imgData::imgData(const char * path, vec4 tintCol, vec2 uvpos, vec2 uvscale)
{
	filepath = path;
	tint = tintCol;
	UVpos = uvpos;
	UVscale = uvscale;
	adata = NULL;
}
animationData::animationData(int size, int cell, float freq)
{
	sheetSize = size;
	cells = cell * cell;
	cellSize = size / cell;
	cellsPerLine = cell;

	UVS = new vec2[cells];
	frequency = freq;

	for (int i = 0; i < cells; i++)
	{
		float x = i % cellsPerLine * cellSize;
		float y = -i / cellsPerLine * cellSize;
		UVS[i] = vec2(x / sheetSize, y / sheetSize);
	}
}


int imgData::getHashIndex() {
	if (hashIndex == -1) {
		if (imgHashLookup.size() == 0) {
			hashIndex = 0;
			imgHashLookup.push_back(filepath);
			return 0;
		}
		else {
			for (int i = 0; i < imgHashLookup.size(); i++) {
				if (strcmp(imgHashLookup[i], filepath) == 0) {
					hashIndex = i;
					return i;
				}
			}
			imgHashLookup.push_back(filepath);
			return imgHashLookup.size() - 1;
		}
	}
	else
		return hashIndex;
}
textData::textData(string Text, float textHeight, vec4 Color, int Justification, const char * path, bool bound, bool kerning)
{
	text = Text;
	filepath = path;
	height = textHeight;
	color = Color;
	boundMode = bound;
	justification = Justification;
	lastLetterPos = vec2(0);
	useKerning = kerning;
}
vec2 textData::letterPosAtIndexNDC(int index)
{
	if (!TextLength || index > TextLength)
		return vec2();

	return vec2(letterTransData[index * 4 + 0],
		letterTransData[index * 4 + 1]);
}
void textData::dispose()
{
	delete letterTransData;
	delete letterUVData;
}

int textData::getHashIndex() {
	if (hashIndex == -1) {
		if (fontHashLookup.size() == 0) {
			hashIndex = 0;
			fontHashLookup.push_back(filepath);
			return 0;
		}
		else {
			for (int i = 0; i < fontHashLookup.size(); i++) {
				if (strcmp(fontHashLookup[i], filepath) == 0) {
					hashIndex = i;
					return i;
				}
			}
			fontHashLookup.push_back(filepath);
			return fontHashLookup.size() - 1;
		}
	}
	else
		return hashIndex;
}


void mLocalTransformHelper(GO * child, mat4 * m) {
	if (!child->parent)
		return;
	if (child->parent->parent)
		mLocalTransformHelper(child->parent, m);

	else {
		//if the end of the parent chain has been reached, the aspect ratio correction gets applied here
	// 	*m = translate(*m, vec3(-camera / resolutionV2f * 2.0f, 0.0f));
		*m = translate(*m, vec3((child->parent->position), 0.0f));
		//	*m = scale(*m, vec3(aspect, 1.0f, 1.0f));
	}

	*m = rotate(*m, child->parent->angle, vec3(0.0f, 0.0f, 1.0f));
	*m = translate(*m, vec3((child->position), 0.0f));
}

mat4 makeLocalTransform(GO * child) {
	mat4 m(1.0f);
	mLocalTransformHelper(child, &m);
	m = rotate(m, child->angle, vec3(0.0f, 0.0f, 1.0f));

	return m;
}

vec2 GO::getGlobalPosition()
{

	if (!parent)
		return position;
	mat4 m = makeLocalTransform(this);


	vec3 newPosition(m[3]);
	return vec2(newPosition.x, newPosition.y);

}

GO::GO(vec2 pos, vec2 Scale, float Angle, float rotationSpeed) {
	position = pos;
	scale = Scale;
	angle = Angle;
	rSpeed = rotationSpeed;
	p = NULL;
	i = NULL;
	t = NULL;
	ps = NULL;
	parent = NULL;
	body = NULL;
}
GO::GO(polyData * poly, vec2 pos, vec2 Scale, float Angle, float rotationSpeed) : GO(pos, Scale, Angle, rotationSpeed)
{
	p = poly;
}
GO::GO(imgData * img, vec2 pos, vec2 Scale, float Angle, float rotationSpeed) : GO(pos, Scale, Angle, rotationSpeed)
{
	i = img;
}
GO::GO(textData * text, vec2 pos, vec2 Scale, float Angle, float rotationSpeed) : GO(pos, Scale, Angle, rotationSpeed)
{
	t = text;
}

GO::GO(ParticleSystem * part, vec2 pos, vec2 Scale, float Angle, float rotationSpeed) : GO(pos, Scale, Angle, rotationSpeed)
{
	ps = part;
}

HWND GLCanvas::getNativeHWND() {
	return glfwGetWin32Window(window);
}

void genFramBuffer(GLuint * textID, GLuint * fboID, int width, int height) {
	glGenTextures(1, textID);
	glGenFramebuffers(1, fboID);
	{
		gl(BindTexture(GL_TEXTURE_2D, *textID));
		defer(glBindTexture(GL_TEXTURE_2D, 0));
		gl(TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
		gl(TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		gl(TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
		gl(TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
		gl(TexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0));

		gl(BindFramebuffer(GL_FRAMEBUFFER, *fboID));
		defer(glBindFramebuffer(GL_FRAMEBUFFER, 0));

		// attach the texture to FBO color attachment point
		gl(FramebufferTexture2D(GL_FRAMEBUFFER,        // 1. fbo target: GL_FRAMEBUFFER 
			GL_COLOR_ATTACHMENT0,  // 2. attachment point
			GL_TEXTURE_2D,         // 3. tex target: GL_TEXTURE_2D
			*textID,             // 4. tex ID
			0));                    // 5. mipmap level: 0(base)

								   // check FBO status
		GLenum status = gl(CheckFramebufferStatus(GL_FRAMEBUFFER));
		assert(status == GL_FRAMEBUFFER_COMPLETE);

		gl(BindTexture(GL_TEXTURE_2D, *textID));
		gl(TexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0));
		gl(BindTexture(GL_TEXTURE_2D, 0));

		gl(BindFramebuffer(GL_FRAMEBUFFER, *fboID));

		gl(ClearColor(0.0f, 0.0f, 0.0f, 1.0f));
		gl(Clear(GL_COLOR_BUFFER_BIT));
	}
}

void GLCanvas::onKeyboard(int key, int scancode, int action, int mods) {
	//Likely redundant
	//if (keyStates[key] == action)
		//return;
	//for (int i = 0; i < 10; i++){}
	//	if (keyBuffer[i].read)
	//		keyBuffer[i] = inputDescription{ key,action, false };
	keyBuffer[keyBufferLength] = inputDescription{ key,action, false };
	if (keyBufferLength < 10)
		keyBufferLength++;

	keyStates[key] = action;
}
void GLCanvas::onMouse(int button, int action, int mods) {
	if (button == 0)
		LeftMouseState = action;
	else if (button == 1)
		RightMouseState = action;
	//for (int i = 0; i < 10; i++)
	//	if (mouseBuffer[i].read)
	//		mouseBuffer[i] = inputDescription{ button,action, false };
	mouseBuffer[MouseBufferLength] = inputDescription{ button,action, false };
	if (MouseBufferLength < 10)
		MouseBufferLength++;
}
void GLCanvas::onCursor() {
	mouseMoveFlag = true;
}

void GLCanvas::clearStates() {
	fill(keyStates, keyStates + 348, 2);
	LeftMouseState = 2;
	RightMouseState = 2;
}

bool GLCanvas::getKey(char key) {
	return glfwGetKey(window, toupper(key));
}
bool GLCanvas::getKeyDown(char key) {
	return (keyStates[toupper(key)] == 1);
}
bool GLCanvas::getKeyUp(char key) {
	return (keyStates[toupper(key)] == 0);
}
//same thing but int32
bool GLCanvas::getKey(int key) {
	return glfwGetKey(window, key);
}
bool GLCanvas::getKeyDown(int key) {
	return (keyStates[key] == 1);
}
bool GLCanvas::getKeyUp(int key) {
	return (keyStates[key] == 0);
}

vec2 GLCanvas::getMousePos() {
	double x, y;
	glfwGetCursorPos(window, &x, &y);
	return vec2((float)x, (float)y);
}
bool GLCanvas::getMouse(int button) {
	return(glfwGetMouseButton(window, button) == 1);
}
bool GLCanvas::getMouseDown(int button) {

	if (button == 0)
		return (LeftMouseState == 1);
	if (button == 1)
		return (RightMouseState == 1);
	return false;
}
bool GLCanvas::getMouseUp(int button) {
	if (button == 0)
		return (LeftMouseState == 0);
	else if (button == 1)
		return (RightMouseState == 0);
	return false;
}

void GLCanvas::saveCanvasAsImage(const char * fileName)
{
	//getPixel(100, 100);


	unsigned char * pixels = new unsigned char[3 * resolutionWidth * resolutionHeight];
	gl(ReadPixels(0, 0, resolutionWidth, resolutionHeight, GL_RGB, GL_UNSIGNED_BYTE, pixels));

	stbi_flip_vertically_on_write(true);
	stbi_write_bmp(fileName, resolutionWidth, resolutionHeight, 3, pixels);

	delete pixels;
}

int GLCanvas::createCanvas(int width, int height, bool borderd, vec3 backcol, bool Vsync, bool cursorHidden)
{
	resolutionWidth = width;
	resolutionHeight = height;

	backCol = vec4(backcol, 1);
	int glfwInitResult = glfwInit();
	if (glfwInitResult != GLFW_TRUE)
	{
		fprintf(stderr, "glfwInit returned false\n");
		return 1;
	}

	Borderd = borderd;
	glfwWindowHint(GLFW_DECORATED, borderd);
	glfwWindowHint(GLFW_FLOATING, !borderd);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	window = glfwCreateWindow(resolutionWidth, resolutionHeight, "demo", NULL, NULL);

	if (!window)
	{
		fprintf(stderr, "failed to open glfw window. is opengl 3.2 supported?\n");
		return 1;
	}

	glfwPollEvents();
	glfwMakeContextCurrent(window);
	if (cursorHidden)
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

	int gl3wInitResult = gl3wInit();
	//return 0;
	if (gl3wInitResult != 0)
	{
		fprintf(stderr, "gl3wInit returned error code %d\n", gl3wInitResult);
		return 1;
	}

	//only usefull during c++ degug?
	glfwSetWindowUserPointer(window, this);
	glfwSetKeyCallback(window, KeyCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);
	glfwSetCursorPosCallback(window, mouseMoveCallback);
	glfwSetWindowSizeCallback(window, window_size_callback);
	glfwSetScrollCallback(window, scroll_callback);

	//If the program is being run from the CLR program, packed shaders will be turned on. 
	//the sources are from shaders.h which is automatically generated from the original .glsl files
	if (usePackedShaders) {
		PolygonShaderProgram = loadShader(RectVertex, PolygonFragment);
		textureShaderProgram = loadShader(RectVertex, TextureFragment);
		setPixelShaderProgram = loadShader(simpleRect, SetPixelFragment);
		fontShaderProgram = loadShader(FontVertex, FontFragment);
		ParticleShaderProgram = loadShader(ParticleVertex, ParticleFragment);
	}
	else {
		PolygonShaderProgram = loadShader("RectVertex", "PolygonFragment");
		textureShaderProgram = loadShader("RectVertex", "TextureFragment");
		fontShaderProgram = loadShader("FontVertex", "FontFragment");
		ParticleShaderProgram = loadShader("ParticleVertex", "ParticleFragment");
		setPixelShaderProgram = loadShader("simpleRect", "SetPixelFragment");
	}


	//create VBOs for particle shader
	glGenBuffers(1, &PVertVBO);
	glBindBuffer(GL_ARRAY_BUFFER, PVertVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(particleVertices), particleVertices, GL_STATIC_DRAW);

	// The VBO containing the positions and sizes of the particles
	glGenBuffers(1, &PPosVBO);
	glBindBuffer(GL_ARRAY_BUFFER, PPosVBO);
	// Initialize with empty (NULL) buffer : it will be updated later, each frame.
	glBufferData(GL_ARRAY_BUFFER, ParticleLimit * 3 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);

	// The VBO containing the colors of the particles
	glGenBuffers(1, &PColVBO);
	glBindBuffer(GL_ARRAY_BUFFER, PColVBO);
	glBufferData(GL_ARRAY_BUFFER, ParticleLimit * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);

	// The VBO containing the colors of the particles
	glGenBuffers(1, &PuvVBO);
	glBindBuffer(GL_ARRAY_BUFFER, PuvVBO);
	glBufferData(GL_ARRAY_BUFFER, expectedTextLength * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);

	//Position and scale VBO for letters in text
	glGenBuffers(1, &FTranVBO);
	glBindBuffer(GL_ARRAY_BUFFER, FTranVBO);
	glBufferData(GL_ARRAY_BUFFER, expectedTextLength * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);

	//Position and scale VBO for letters in text
	glGenBuffers(1, &FuvVBO);
	glBindBuffer(GL_ARRAY_BUFFER, FuvVBO);
	glBufferData(GL_ARRAY_BUFFER, expectedTextLength * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);

	//create VAO for shape shaders
	gl(GenVertexArrays(1, &VAO));
	gl(GenBuffers(1, &VBO));
	gl(GenBuffers(1, &EBO));

	gl(BindVertexArray(VAO));

	gl(BindBuffer(GL_ARRAY_BUFFER, VBO));
	gl(BufferData(GL_ARRAY_BUFFER, sizeof(rectVertices), rectVertices, GL_STATIC_DRAW));

	gl(BindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO));
	gl(BufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(shapeIndices), shapeIndices, GL_STATIC_DRAW));

	gl(VertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(sizeof(float) * 3)));
	gl(EnableVertexAttribArray(0));

	gl(BindBuffer(GL_ARRAY_BUFFER, 0));
	gl(BindVertexArray(0));

	gl(Enable(GL_BLEND));
	gl(BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

	//polygon shader uniforms
	PxformUniformLocation = gl(GetUniformLocation(PolygonShaderProgram, "xform"));
	PColorUniformLocation = gl(GetUniformLocation(PolygonShaderProgram, "Color"));
	PbordColorUniformLocation = gl(GetUniformLocation(PolygonShaderProgram, "bordColor"));
	PsideCountUniformLocation = gl(GetUniformLocation(PolygonShaderProgram, "sideCount"));
	PshapeScaleUniformLocation = gl(GetUniformLocation(PolygonShaderProgram, "shapeScale"));
	PborderWidthUniformLocation = gl(GetUniformLocation(PolygonShaderProgram, "bordWidth"));

	PtimeUniformLocation = gl(GetUniformLocation(PolygonShaderProgram, "iTime"));
	PzoomUniformLocation = gl(GetUniformLocation(PolygonShaderProgram, "zoom"));

	PmPosUniformLocation = gl(GetUniformLocation(PolygonShaderProgram, "position"));
	PmScaleUniformLocation = gl(GetUniformLocation(PolygonShaderProgram, "scale"));
	PmRotUniformLocation = gl(GetUniformLocation(PolygonShaderProgram, "rotation"));
	PaspectUniformLocation = gl(GetUniformLocation(PolygonShaderProgram, "aspect"));

	//fbo shader uniforms
	FtextureUniformLocation = gl(GetUniformLocation(textureShaderProgram, "Text"));
	FColorUniformLocation = gl(GetUniformLocation(textureShaderProgram, "tint"));

	FmPosUniformLocation = gl(GetUniformLocation(textureShaderProgram, "position"));
	FmScaleUniformLocation = gl(GetUniformLocation(textureShaderProgram, "scale"));
	FmRotUniformLocation = gl(GetUniformLocation(textureShaderProgram, "rotation"));
	FUVscaleUniformLocation = gl(GetUniformLocation(textureShaderProgram, "scaleOffset"));
	FUVposUniformLocation = gl(GetUniformLocation(textureShaderProgram, "posOffset"));
	FaspectUniformLocation = gl(GetUniformLocation(textureShaderProgram, "aspect"));
	FxformUniformLocation = gl(GetUniformLocation(textureShaderProgram, "xform"));
	FzoomUniformLocation = gl(GetUniformLocation(textureShaderProgram, "zoom"));
	FOpacityUniformLocation = gl(GetUniformLocation(textureShaderProgram, "opacity"));

	//set pixel shader uniforms
	SPtextureUniformLocation = gl(GetUniformLocation(setPixelShaderProgram, "Text"));
	SPTextureMaskUniformLocation = gl(GetUniformLocation(setPixelShaderProgram, "Mask"));

	//SPPosUniformLocation = gl(GetUniformLocation(setPixelShaderProgram, "position"));
	//SPScaleUniformLocation = gl(GetUniformLocation(setPixelShaderProgram, "scale"));
	//SPRotUniformLocation = gl(GetUniformLocation(setPixelShaderProgram, "rotation"));
	//SPUVscaleUniformLocation = gl(GetUniformLocation(setPixelShaderProgram, "scaleOffset"));
	//SPUVposUniformLocation = gl(GetUniformLocation(setPixelShaderProgram, "posOffset"));
	//SPaspectUniformLocation = gl(GetUniformLocation(setPixelShaderProgram, "aspect"));
	//SPxformUniformLocation = gl(GetUniformLocation(setPixelShaderProgram, "xform"));
	//SPzoomUniformLocation = gl(GetUniformLocation(setPixelShaderProgram, "zoom"));

	//font shader uniforms
	FonttextureUniformLocation = gl(GetUniformLocation(fontShaderProgram, "Text"));
	FontColorUniformLocation = gl(GetUniformLocation(fontShaderProgram, "Color"));
	FonttimeUniformLocation = gl(GetUniformLocation(fontShaderProgram, "iTime"));

	FontmPosUniformLocation = gl(GetUniformLocation(fontShaderProgram, "position"));
	FontmScaleUniformLocation = gl(GetUniformLocation(fontShaderProgram, "scale"));
	FontmRotUniformLocation = gl(GetUniformLocation(fontShaderProgram, "rotation"));
	FontUVscaleUniformLocation = gl(GetUniformLocation(fontShaderProgram, "scaleOffset"));
	FontUVposUniformLocation = gl(GetUniformLocation(fontShaderProgram, "posOffset"));
	FontaspectUniformLocation = gl(GetUniformLocation(fontShaderProgram, "aspect"));
	FontxformUniformLocation = gl(GetUniformLocation(fontShaderProgram, "xform"));
	FontzoomUniformLocation = gl(GetUniformLocation(fontShaderProgram, "zoom"));

	//particle shader uniforms
	ParticlePosUniformLocation = gl(GetUniformLocation(ParticleShaderProgram, "position"));
	ParticleResUniformLocation = gl(GetUniformLocation(ParticleShaderProgram, "iResolution"));
	ParticleTextureUniformLocation = gl(GetUniformLocation(ParticleShaderProgram, "Text"));
	ParticleUVScaleUniformLocation = gl(GetUniformLocation(ParticleShaderProgram, "UVScale"));
	ParticlexformUniformLocation = gl(GetUniformLocation(ParticleShaderProgram, "xform"));

	if (!Vsync)
		glfwSwapInterval(0);
	else
		glfwSwapInterval(1);

	genFramBuffer(&fboTextIdA, &fboIdA, width, height);
	genFramBuffer(&fboTextIdB, &fboIdB, width, height);

	//used to display debug information on the canvas.
	if (debugMode) {
		infoText = textData("this will be replaced by some info", 20, vec4(1), 0, "c:\\windows\\fonts\\arial.ttf");
		infoGO = GO(vec2(170, resolutionV2f.y - 20));
		infoGO.t = &infoText;
	}

	setPixelData = new unsigned char[width * height * 4];
	//	setPixelMask = new unsigned char[width * height];
	clearSetPixelData();
	world.SetContactListener(&contactListener);

	camera = vec2();
	prevTime = (float)glfwGetTime();
	return 0;
}
void GLCanvas::clearSetPixelData(bool maskOnly) {
	if (!maskOnly) {
		int length = resolutionWidth * resolutionHeight * 4;
		memset(setPixelData, 0, length * sizeof(*setPixelData));

	}

	//int length = resolutionWidth * resolutionHeight;
	//memset(setPixelMask, 0, length * sizeof(*setPixelMask));

	//int length = resolutionWidth * resolutionHeight * 4;
	//for (int i = 0; i < length; i++)
	//	setPixelData[i] = 0;
}


fontAsset::fontAsset(const char * filepath) {
	filePath = filepath;
	FILE * test;
	gl(GenTextures(1, &tex_atlas));
	gl(BindTexture(GL_TEXTURE_2D, tex_atlas));
	gl(TexImage2D(GL_TEXTURE_2D, 0, GL_RGB, fnt_atlas_width, fnt_atlas_height, 0, GL_RED, GL_BYTE, NULL));
	gl(TexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	gl(TexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	gl(TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	gl(TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	// gl(GenerateMipmap(GL_TEXTURE_2D));
	gl(BindTexture(GL_TEXTURE_2D, 0));


	u8* tt_buf = NULL;
	{
		FILE* fp = fopen(filePath, "rb");
		fseek(fp, 0, SEEK_END);
		size_t size = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		tt_buf = (u8*)malloc(size);
		fread(tt_buf, size, 1, fp);
		fclose(fp);
	}


	stbtt_InitFont(&info, tt_buf, 0);
	scaleFactor = stbtt_ScaleForPixelHeight(&info, fnt_height_px);
	stbtt_GetFontVMetrics(&info, &ascent, &descent, &line_gap);
	baseline = (int)(ascent*scaleFactor);


	packed_chars = (stbtt_packedchar*)malloc(sizeof(*packed_chars)*fnt_char_count);


	u8* fnt_atlas = (u8*)malloc(sizeof(*fnt_atlas)*fnt_atlas_width*fnt_atlas_height);
	{
		stbtt_pack_context fnt_context;

		STBTT_assert(stbtt_PackBegin(&fnt_context, fnt_atlas, fnt_atlas_width, fnt_atlas_height, 0, fnt_atlas_padding, NULL));
		//{
		//	free(tt_buf);
		//	free(fnt_atlas);
		//	free(file_paths);
		//	return;
		//}
		stbtt_PackSetOversampling(&fnt_context, fnt_oversample_x, fnt_oversample_y);
		STBTT_assert(stbtt_PackFontRange(&fnt_context, tt_buf, 0, fnt_size, fnt_first_char, fnt_char_count, packed_chars));
		//{
		//	free(tt_buf);
		//	stbtt_PackEnd(&fnt_context);
		//	free(fnt_atlas);
		//	free(file_paths);
		//	return;
		//}

	//	free(tt_buf);
		stbtt_PackEnd(&fnt_context);
	}
	gl(BindTexture(GL_TEXTURE_2D, tex_atlas));
	gl(PixelStorei(GL_UNPACK_ALIGNMENT, 1));
	gl(TexImage2D(GL_TEXTURE_2D, 0, GL_RED, fnt_atlas_width, fnt_atlas_height, 0, GL_RED, GL_UNSIGNED_BYTE, fnt_atlas));
	const GLint swizzle[] = { GL_ONE, GL_ONE, GL_ONE, GL_RED };
	gl(TexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzle));
	gl(PixelStorei(GL_UNPACK_ALIGNMENT, 4));
	gl(BindTexture(GL_TEXTURE_2D, 0));

	free(fnt_atlas);

	float unused_offset_y, xof;
	for (int i = fnt_first_char + 1; i < fnt_last_char; i++)
	{
		const char ch = i;
		stbtt_aligned_quad quad;
		stbtt_GetPackedQuad(packed_chars, fnt_atlas_height, fnt_atlas_height, ch - fnt_first_char, &xof, &unused_offset_y, &quad, 1);
		quads[i] = quad;
		tallestLetter = glm::max(tallestLetter, (quad.y1 - quad.y0));
		float tttttttttt = -quad.y1 + (quad.y1 - quad.y0);
	}
}


//this could be moved into the constuctor
//void fontAsset::loadTexture() {
//	gl(GenTextures(1, &id));
//	gl(BindTexture(GL_TEXTURE_2D, id));
//	//	gl(TexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 512, 512, 0, GL_RGBA, GL_UNSIGNED_BYTE, tempBitmap));
//	gl(TexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 512, 512, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, bitmapBuffer));
//	// can free temp_bitmap at this point
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//}
//
//void fontAsset::dipose()
//{
//	//	delete bitmapBuffer;
//		//delete [] uvOffset;
//		//delete uvScale;
//		//delete quadScale;
//		//delete alignment;
//}


//this could be moved into the img constructor, same as font
void loadTexture(const char * path, GLuint * id) {

	stbi_set_flip_vertically_on_load(true);
	int imW, imH, comp;
	unsigned char* image = stbi_load(path, &imW, &imH, &comp, STBI_rgb_alpha);
	if (image == NULL) {
		fprintf(stderr, "failed to load image\n");
		return;
	}
	gl(GenTextures(1, id));
	gl(BindTexture(GL_TEXTURE_2D, *id));
	gl(TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
	gl(TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
	gl(TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	gl(TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	gl(TexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imW, imH, 0, GL_RGBA, GL_UNSIGNED_BYTE, image));
	gl(BindTexture(GL_TEXTURE_2D, 0));

	stbi_image_free(image);
}

imgAsset::imgAsset(const char * FilePath) {
	filePath = FilePath;
	loadTexture(FilePath, &ID);
	init = true;
}

void GLCanvas::setBBPixel(int x, int y, int r, int g, int b, int a) {

	if (a == 0)
		return;

	typedef unsigned char byte;
	while (setPixelCopyFlag) {} //thread safety (no other way i could find with C++/CLI)

	int gridXY = (x * 4) + (resolutionWidth * y * 4);

	//if more than one pixel is set in the same frame (very likely) This block will run
	if (setPixelFlag) {

		//get preexisting values for manual color blending
		byte baseR = setPixelData[gridXY];
		byte baseG = setPixelData[++gridXY];
		byte baseB = setPixelData[++gridXY];
		byte tempA = setPixelData[++gridXY];

		//set values normally if preexisiting value is transparent
		if (tempA == 0)
		{
			gridXY -= 3;
			setPixelData[gridXY] = r;
			setPixelData[++gridXY] = g;
			setPixelData[++gridXY] = b;
			setPixelData[++gridXY] = a;
		}
		//perform additive color blending on pixel
		else {
			float baseA = tempA / 255.0f;

			float addedA = a / 255.0f;
			float mixA = 1.0f - (1.0f - a) * (1.0f - baseA);
			float ratio = (1.0f - addedA / mixA);
			byte mixR = (r * addedA / mixA) + (baseR * baseA * ratio);

			gridXY -= 3;
			setPixelData[gridXY] = (r * addedA / mixA) + (baseR * baseA * ratio);
			setPixelData[++gridXY] = (g * addedA / mixA) + (baseG * baseA * ratio);
			setPixelData[++gridXY] = (b * addedA / mixA) + (baseB * baseA * ratio);
			setPixelData[++gridXY] = 255 * mixA;
		}
	}
	else {
		setPixelData[gridXY] = r;
		setPixelData[++gridXY] = g;
		setPixelData[++gridXY] = b;
		setPixelData[++gridXY] = a;
	}

	setPixelFlag = true;
}
void GLCanvas::setBBPixelFast(int x, int y, int r, int g, int b, int a) {
	while (windowSizeChanged) {}

	int gridXY = (x * 4) + (resolutionWidth * y * 4);

	if (!setPixelCopyFlag) {
		setPixelFlag = true;
		setPixelData[gridXY] = r;
		setPixelData[++gridXY] = g;
		setPixelData[++gridXY] = b;
		setPixelData[++gridXY] = a;
	}
	//if the array is currently being transfered to the GPU, send it later as to not lock the thread
	else
		spTransferBuffer.push_back({ gridXY, r, g, b, a });
}

void GLCanvas::setBBShape(GO g) {
	BBQue.push_back(g);
}

void GLCanvas::setPolygon(GO * g) {
	gl(Uniform1f(PaspectUniformLocation, aspect));
	gl(Uniform4f(PColorUniformLocation, g->p->fColor.r, g->p->fColor.g, g->p->fColor.b, g->p->fColor.a));
	gl(Uniform1i(PsideCountUniformLocation, g->p->sides));
	gl(Uniform2f(PshapeScaleUniformLocation, g->scale.x, g->scale.y));
	gl(Uniform1f(PborderWidthUniformLocation, g->p->bWidth));
	gl(Uniform4f(PbordColorUniformLocation, g->p->bColor.r, g->p->bColor.g, g->p->bColor.b, g->p->bColor.a));
	gl(Uniform1f(PtimeUniformLocation, currTime));
	gl(Uniform2f(PzoomUniformLocation, cameraZoom.x, cameraZoom.y));
}

void GLCanvas::createSetPixelTexture()
{
	gl(DeleteTextures(1, &setPixelDataID));

	gl(GenTextures(1, &setPixelDataID));
	gl(BindTexture(GL_TEXTURE_2D, setPixelDataID));
	gl(TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
	gl(TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
	gl(TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	gl(TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	gl(TexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, resolutionWidth, resolutionHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, setPixelData));
	gl(BindTexture(GL_TEXTURE_2D, 0));
}

void GLCanvas::setGOTransform(GO * g, GLuint Aspect, GLuint scale, GLuint pos, GLuint rot, GLuint zoom) {
	//transform
	gl(Uniform1f(Aspect, aspect));
	gl(Uniform2f(scale, g->scale.x / resolutionV2f.x, g->scale.y / resolutionV2f.y));
	gl(Uniform2f(pos, (g->position.x - camera.x) / resolutionWidth * 2.0f, (g->position.y - camera.y) / resolutionHeight * 2.0f));
	gl(Uniform1f(rot, g->angle - g->rSpeed * currTime));
	gl(Uniform2f(zoom, cameraZoom.x, cameraZoom.y));
}

void GLCanvas::LocalTransformHelper(GO * child, mat4 * m) {
	if (!child->parent)
		return;
	if (child->parent->parent)
		LocalTransformHelper(child->parent, m);

	else {
		//if the end/begining of the parent chain has been reached, the aspect ratio correction gets applied here
		*m = translate(*m, vec3(-camera / resolutionV2f * 2.0f, 0.0f));
		*m = translate(*m, vec3((child->parent->position) / resolutionV2f * 2.0f, 0.0f));
		*m = scale(*m, vec3(aspect, 1.0f, 1.0f));
	}

	*m = rotate(*m, child->parent->angle, vec3(0.0f, 0.0f, 1.0f));
	*m = translate(*m, vec3((child->position) / resolutionV2f * 2.0f, 0.0f));
}

mat4 GLCanvas::getLocalTransform(GO * child) {
	mat4 m(1.0f);
	LocalTransformHelper(child, &m);
	m = rotate(m, child->angle + child->rSpeed * currTime, vec3(0.0f, 0.0f, 1.0f));

	return m;
}

GLCanvas::GLCanvas()
{
	fill(keyStates, keyStates + 348, 2);
	LeftMouseState = 2;
	RightMouseState = 2;
	for (int i = 0; i < 10; i++)
	{
		keyBuffer[i].read = true;
		mouseBuffer[i].read = true;
	}
}

void GLCanvas::mainloop(bool render, bool focusContext) {
	if (disposed)
		return;
	
	if(focusContext)
		glfwMakeContextCurrent(window);
	
	clearStates();
	glfwPollEvents();
	if (!managed) {
		keyBufferLength = 0;
		MouseBufferLength = 0;
	}

	if (glfwWindowShouldClose(window)) {
		dispose();
		glfwDestroyWindow(window);
		return;
	}
	//	return;
	currTime = (float)glfwGetTime();
	const float deltaTime = currTime - prevTime;
	LastRenderTime = floorf(deltaTime * 1000); //possibly reduntant variable
	currentFPS = 1.0f / deltaTime;
	prevTime = currTime;

	glfwGetWindowSize(window, &resolutionWidth, &resolutionHeight);
	resolutionV2f = vec2((float)resolutionWidth, (float)resolutionHeight);
	aspect = resolutionV2f.y / resolutionV2f.x;

	//window is minimized
	if (resolutionWidth == 0 || resolutionHeight == 0)
		return;

	//update physics
	world.Step(timeStep, velocityIterations, positionIterations);

	gl(Viewport(0, 0, resolutionWidth, resolutionHeight));

	//if the window size is changed, the back buffer and setpixel buffer needs to be re-created 
	if (windowSizeChanged) {
		gl(DeleteTextures(1, &fboTextIdA));
		gl(DeleteTextures(1, &fboTextIdB));
		gl(DeleteFramebuffers(1, &fboTextIdA));
		gl(DeleteFramebuffers(1, &fboTextIdB));

		genFramBuffer(&fboTextIdA, &fboIdA, resolutionWidth, resolutionHeight);
		genFramBuffer(&fboTextIdB, &fboIdB, resolutionWidth, resolutionHeight);

		delete setPixelData;
		//	delete setPixelMask;
		setPixelData = new unsigned char[resolutionWidth * resolutionHeight * 4];
		//	setPixelMask = new unsigned char[resolutionWidth * resolutionHeight];
		clearSetPixelData();
		createSetPixelTexture();
		spTransferBuffer.clear();
	}

	//bind the back writing buffer
	gl(BindTexture(GL_TEXTURE_2D, 0));
	gl(BindFramebuffer(GL_FRAMEBUFFER, fboIdA));

	//clear the buffer
	if (clearColorFlag || windowSizeChanged) {
		gl(ClearColor(backCol.r, backCol.g, backCol.b, 1.0f));
		gl(Clear(GL_COLOR_BUFFER_BIT));
		clearColorFlag = false;
		//if(render)
		//	setBBShape(shape(resolutionV2f / 2.0f, resolutionV2f * 2.0f, 0, vec4(backCol.r, backCol.g, backCol.b, 1), vec4(), 0, 0));
	}

	for (int i = 0; i < assetBuffer.size(); i++)
	{
		loadImageAsset(assetBuffer[i]);
	}

	mat4 empty(0);

	//draw shapes to the back buffer
	gl(BindVertexArray(VAO));
	gl(UseProgram(PolygonShaderProgram));
	if (render) {
		for (int i = 0; i < BBQue.size(); i++)
		{
			GO * g = &BBQue[i];
			if (g->hidden)
				continue;

			if (g->i) {
				gl(UseProgram(textureShaderProgram));
				setTexture(g, FtextureUniformLocation);
				setGOTransform(g, textTransUniforms);
				gl(DrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0));
				gl(UseProgram(PolygonShaderProgram));
			}
			else if (g->t) {
				setFont(g);
				gl(DrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, g->t->TextLength));
				gl(UseProgram(PolygonShaderProgram));
			}
			else if (g->p) {
				setPolygon(g);
				setGOTransform(g, polyTransUniforms);
				setPolygon(g);
				gl(DrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0));
			}


		}
		BBQue.clear();
	}

	if (firstFrame) {
		createSetPixelTexture();
	}

	for (int i = 0; i < spTransferBuffer.size(); i++)
	{
		Pixel p = spTransferBuffer[i];
		setPixelData[p.xy] = p.r;
		setPixelData[p.xy + 1] = p.g;
		setPixelData[p.xy + 2] = p.b;
		setPixelData[p.xy + 3] = p.a;
	}
	spTransferBuffer.clear();


	//with the writing buffer still bound, create a texture from the setpixel data buffer and paste it to the back buffer
	if (setPixelFlag && render) {
		setPixelCopyFlag = true; //pause other thread while pixels are coppied to buffer
		gl(BindVertexArray(VAO));
		gl(UseProgram(setPixelShaderProgram));

		gl(BindTexture(GL_TEXTURE_2D, setPixelDataID));
		gl(TexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, resolutionWidth, resolutionHeight, GL_RGBA, GL_UNSIGNED_BYTE, setPixelData));

		gl(DrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0));
		gl(BindTexture(GL_TEXTURE_2D, 0));
		//gl(DeleteTextures(1, &setPixelDataID));

		clearSetPixelData(false);
		setPixelFlag = false;
		setPixelCopyFlag = false; //resume other thread
	}

	//transfer the back write buffer to the read buffer
	{
		gl(BindVertexArray(VAO));
		gl(UseProgram(textureShaderProgram));

		gl(BindFramebuffer(GL_FRAMEBUFFER, fboIdB));
		gl(Viewport(0, 0, resolutionWidth, resolutionHeight));

		gl(ActiveTexture(GL_TEXTURE0 + 0));
		gl(BindTexture(GL_TEXTURE_2D, fboTextIdA));

		FBOUniforms();
		gl(UniformMatrix4fv(FxformUniformLocation, 1, GL_FALSE, value_ptr(empty)));
		gl(DrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0));
		gl(BindTexture(GL_TEXTURE_2D, 0));

		gl(UseProgram(0));
		gl(BindVertexArray(0));
	}

	//transform the back buffer and draw it as a background for the front buffer
	{
		gl(BindFramebuffer(GL_FRAMEBUFFER, 0));
		gl(Disable(GL_CULL_FACE));
		gl(Viewport(0, 0, resolutionWidth, resolutionHeight));

		gl(BindVertexArray(VAO));
		gl(UseProgram(textureShaderProgram));
		gl(Uniform1f(FaspectUniformLocation, aspect));

		gl(ActiveTexture(GL_TEXTURE0 + 0));
		gl(BindTexture(GL_TEXTURE_2D, fboTextIdB));

		//transform
		gl(Uniform2f(FmScaleUniformLocation, 1.0, 1.0f));
		gl(Uniform2f(FmPosUniformLocation, 0, 0));
		gl(Uniform1f(FmRotUniformLocation, 0.0f));

		gl(UniformMatrix4fv(FxformUniformLocation, 1, GL_FALSE, value_ptr(empty)));
		gl(Uniform2f(FzoomUniformLocation, 1, 1));
		gl(Uniform1f(FOpacityUniformLocation, 1));

		gl(DrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0));
		gl(BindTexture(GL_TEXTURE_2D, 0));
		gl(UseProgram(0));
		gl(BindVertexArray(0));
	}


	//thread safe transfer object buffer to canvas draw list
	if (clearShapeFlag) {
		GameObjects.clear();
		clearShapeFlag = false;
	}

	//thread safe transfer of gameobjects
	for (int i = 0; i < GOBuffer.size(); i++)
		GameObjects.push_back(GOBuffer[i]);
	int newGOs = GOBuffer.size();
	GOBuffer.clear();

	//thread safe removal of gameobjects
	for (int j = 0; j < GORemoveBuffer.size(); j++)
		for (int i = 0; i < GameObjects.size(); i++)
			if (GameObjects[i] == GORemoveBuffer[j])
				GameObjects.erase(GameObjects.begin() + i);
	GORemoveBuffer.clear();

	int length = GameObjects.size();

	for (int i = 0; i < length; i++)
	{
		bool change = false;
		int s = GameObjects.size() - 1; //only works when declared outside the loop?
		for (int j = 0; j < s; j++)
		{
			if (GameObjects[j]->drawIndex < GameObjects[j + 1]->drawIndex) {
				GO * temp = GameObjects[j];
				GameObjects[j] = GameObjects[j + 1];
				GameObjects[j + 1] = temp;
				change = true;
			}
		}
		if (!change)
			break;
	}
	
	gl(BindVertexArray(VAO));
	gl(UseProgram(PolygonShaderProgram));




	//if (Dfonts.size() > 0) {
	//	gl(UseProgram(textureShaderProgram));

	//	GO g = GO(vec2(0), vec2(1024));

	//	gl(Uniform1f(FaspectUniformLocation, aspect));
	//	gl(BindTexture(GL_TEXTURE_2D, Dfonts[0].tex_atlas));
	//	gl(Uniform1i(FtextureUniformLocation, 0));


	//	setGOTransform(&g, textTransUniforms);
	//	gl(UniformMatrix4fv(FxformUniformLocation, 1, GL_FALSE, value_ptr(empty)));
	//	gl(DrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0));
	//}

	if (render) {


		//draw Gameobjects
		int GoSize = GameObjects.size();
		mat4 m; //used if parent transforms are involved
		for (int i = 0; i < GoSize; ++i)
		{
			GO * g = GameObjects[i];

#ifdef _DEBUG
			int shapeCount = 0;
			shapeCount += g->p != nullptr;
			shapeCount += (g->i != nullptr) && (g->ps == nullptr); //particle systems might also have a texture
			shapeCount += g->t != nullptr;
			shapeCount += g->ps != nullptr;
			if (shapeCount > 1)
				cout << "Debug warning : GO with multiple draw targets detected\n";
#endif

			if (g->hidden)
				continue;

			if (g->parent)
				m = getLocalTransform(g);

			//update physics
			if (g->body) {
				if (g->body->setPositionFlag) {
					g->body->body->SetTransform(toB2(g->position), g->angle);
					g->body->setPositionFlag = false;
				}
				else {
					b2Vec2 pos = g->body->body->GetPosition();
					g->position = vec2(pos.x, pos.y) * phScale;
					g->angle = g->body->body->GetAngle();
				}
			}
			//particle system
			if (g->ps) {
				if (g->parent)
					drawParticleSystem(g, deltaTime, &m);
				else
					drawParticleSystem(g, deltaTime, NULL);
				gl(BindVertexArray(VAO));
				gl(UseProgram(PolygonShaderProgram));
			}
			//sprite
			else if (g->i) {
				gl(UseProgram(textureShaderProgram));
				setTexture(g, FtextureUniformLocation);
				gl(Uniform1f(FOpacityUniformLocation, g->i->opacity));
				//animation
				if (g->i->adata) {
					animationData * a = g->i->adata;

					int current = a->nextFrame;
					if (a->play)
						current = a->cells * (a->iTime / a->frequency);

					gl(Uniform2f(FUVposUniformLocation, a->UVS[current].x, a->UVS[current].y));
					float sc = a->cellSize / (float)a->sheetSize;
					gl(Uniform2f(FUVscaleUniformLocation, sc, sc));

					if (a->play)
						a->iTime += deltaTime;

					if (a->iTime > a->frequency) {
						a->iTime = 0;
						if (!a->repeat)
							a->play = false;
					}
				}
				//no animation
				else {
					gl(Uniform2f(FUVposUniformLocation, g->i->UVpos.x, g->i->UVpos.y));
					gl(Uniform2f(FUVscaleUniformLocation, g->i->UVscale.x, g->i->UVscale.y));
				}

				if (g->parent) {
					m = scale(m, vec3(g->scale / resolutionV2f, 1.0f));
					gl(UniformMatrix4fv(FxformUniformLocation, 1, GL_FALSE, value_ptr(m)));
					gl(Uniform2f(FzoomUniformLocation, cameraZoom.x, cameraZoom.y));
					gl(DrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0));
				}
				else {
					setGOTransform(g, textTransUniforms);
					gl(UniformMatrix4fv(FxformUniformLocation, 1, GL_FALSE, value_ptr(empty)));
					gl(DrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0));
				}
				gl(UseProgram(PolygonShaderProgram));
			}
			//text
			//else if (g->Dt) {
			//	DsetFont(g);
			//	//setFont(g);
			//	if (g->parent) {
			//		gl(UniformMatrix4fv(FontxformUniformLocation, 1, GL_FALSE, value_ptr(m)));
			//		gl(DrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, g->t->TextLength));
			//	}
			//	else {
			//		gl(UniformMatrix4fv(FontxformUniformLocation, 1, GL_FALSE, value_ptr(empty)));
			//		gl(DrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, g->Dt->TextLength));
			//	}
			//	gl(UseProgram(PolygonShaderProgram));
			//}
			else if (g->t) {
				setFont(g);
				if (g->parent) {
					gl(UniformMatrix4fv(FontxformUniformLocation, 1, GL_FALSE, value_ptr(m)));
					gl(DrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, g->t->TextLength));
				}
				else {
					gl(UniformMatrix4fv(FontxformUniformLocation, 1, GL_FALSE, value_ptr(empty)));
					gl(DrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, g->t->TextLength));
				}
				gl(UseProgram(PolygonShaderProgram));
			}
			//polygon
			else if (g->p) {
				gl(UseProgram(PolygonShaderProgram));
				setPolygon(g);
				if (g->parent) {
					m = scale(m, vec3(g->scale / resolutionV2f, 1.0f));
					gl(UniformMatrix4fv(PxformUniformLocation, 1, GL_FALSE, value_ptr(m)));
					gl(Uniform2f(PzoomUniformLocation, cameraZoom.x, cameraZoom.y));
					gl(DrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0));
				}
				else {
					setGOTransform(g, polyTransUniforms);
					gl(UniformMatrix4fv(PxformUniformLocation, 1, GL_FALSE, value_ptr(empty)));
					//gl(Uniform1f(PmRotUniformLocation, 0)); //anti aliasing optimization
					gl(DrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0));
				}
			}
		}
		if (debugMode) {
			infoGO.position = vec2(170, resolutionV2f.y - 30);
			infoText.text = debugString;
			setFont(&infoGO);
			gl(DrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, infoText.TextLength));
		}
	}
	gl(UseProgram(0));
	gl(BindVertexArray(0));

	windowSizeChanged = false;

	if(render)
		glfwSwapBuffers(window);
	updateDebugInfo();

	if (windowTitleFlag && Borderd) {
		if (titleDetails)
			glfwSetWindowTitle(window, (title + debugString).c_str());
		else
			glfwSetWindowTitle(window, title);
		windowTitleFlag = false;
	}

	firstFrame = false;
}

//checks if image is loaded to memory. assigns it so and loads it if not
void GLCanvas::setTexture(GO * g, GLuint textureLocation) {
	int index = g->i->getHashIndex();
	if (index + 1 > imgs.size())
		imgs.resize(index + 1);
	if (!&imgs[index] || !imgs[index].init)
		imgs[index] = imgAsset(g->i->filepath);

	gl(Uniform1f(FaspectUniformLocation, aspect));
	gl(BindTexture(GL_TEXTURE_2D, imgs[index].ID));
	//temp fix
	if (g->ps == NULL)
		gl(Uniform4f(FColorUniformLocation, g->i->tint.r, g->i->tint.g, g->i->tint.b, g->i->tint.a));
	gl(Uniform1i(textureLocation, 0));
}


//similar to setTexture, but only loads to memory, no uniforms
void GLCanvas::loadImageAsset(const char * filepath) {

	for (int i = 0; i < imgs.size(); i++)
		if (strcmp(imgs[i].filePath, filepath) == 0)
			return;

	//wasn't found in memory, so load it now
	imgs.push_back(imgAsset(filepath)); //constructor calls loadTexture
	imgHashLookup.push_back(filepath);
}

void GLCanvas::setFont(GO * g) {
	gl(UseProgram(fontShaderProgram));

	gl(Uniform1f(FontaspectUniformLocation, aspect));
	gl(Uniform4f(FontColorUniformLocation, g->t->color.r, g->t->color.g, g->t->color.b, g->t->color.a));
	gl(Uniform1f(FonttimeUniformLocation, currTime));


	int HashIndex = g->t->getHashIndex();
	if (HashIndex + 1 > fonts.size())
		fonts.resize(HashIndex + 1);
	if (!&fonts[HashIndex] || !fonts[HashIndex].init) {
		fonts[HashIndex] = fontAsset(g->t->filepath);
		//fonts[HashIndex].loadTexture();
	}

	fontAsset * selected = &fonts[HashIndex];
	gl(BindTexture(GL_TEXTURE_2D, selected->tex_atlas));
	gl(Uniform1i(FonttextureUniformLocation, 0));

	float xof = 0; //x offset of each letter
	float x = 0, y = 0; //required by stbtt
	float scaleRatio = g->t->height / selected->tallestLetter;
	float totalWidth = 0;
	int lineCount = 1; //number of lines in the string (# of '\n')
	int lineNum = 0;  //current line being worked on
	char c; //used to store each letter for calculations
	bool boundMode = g->t->boundMode; //bounding box mode
	int maxLines; //Only used for bounding box mode
	int recordIndex = 0; //which line number is the
	int Xreference; // iether scaled boundry.X or record
	int textLength = g->t->text.length();
	float * lineLengths; //array of pixel lengths for each line
	bool lineBreak = false; //runtime flag for inserting new lines in bound mode
	bool inverted = cameraZoom.y < 0;

	float letterPosX, letterPosY, finalScaleX, finalScaleY; //used ever for letter. Allocated here for performance

	//much faster to convert the string to a char *
	char * text = new char[g->t->text.size() + 1];
	memcpy(text, g->t->text.c_str(), g->t->text.size());

	//allocate an arrays to be coppied to the VBOs
	if (textLength != g->t->TextLength) {
		//if containers are not being allocated for the first time, delete the previuos data
		if (g->t->TextLength > 0)
			g->t->dispose();
		g->t->letterTransData = new float[textLength * 4]();
		g->t->letterUVData = new float[textLength * 4]();
		g->t->TextLength = textLength;
	}


	if (!boundMode) {
		for (int i = 0; i < textLength; i++)
			if (text[i] == '\n')
				lineCount++;
	}

	else
		lineCount = g->scale.y / g->t->height;

	lineLengths = new float[lineCount + 10];


	/////////////////////////////////////////////////////////////

	float unused_offset_y = 0.0f;





	float of;
	{
		const char ch = '}';

		stbtt_aligned_quad quad;
		stbtt_GetPackedQuad(selected->packed_chars, fnt_atlas_height, fnt_atlas_height, ch - fnt_first_char, &xof, &unused_offset_y, &quad, 1);

		of = (quad.y1 - quad.y0);
		//of = quad.y0;
	}




	float sdif = g->t->height / of; //selected->tallestLetter;

	xof = 0.0f;
	unused_offset_y = 0.0f;

	float record = 0; //longest line of text
	for (int txt_i = 0, lineCounter = 1; txt_i < textLength; ++txt_i) {
		const char ch = text[txt_i];

		stbtt_aligned_quad quad;
		stbtt_GetPackedQuad(selected->packed_chars, fnt_atlas_height, fnt_atlas_height, ch - fnt_first_char, &xof, &unused_offset_y, &quad, 1);

		if (!boundMode) {
			if (ch == '\n' || txt_i == textLength - 1) {
				record = xof > record ? xof : record;
				lineLengths[lineCounter - 1] = xof;
				lineCounter++;
				xof = 0;
			}
		}
		else {
			if (ch == '\n' || txt_i == textLength - 1) {
				lineLengths[lineCounter - 1] = xof;
				lineCounter++;
				xof = 0;
			}
			else if (xof > g->scale.x / sdif) {
				lineLengths[lineCounter - 1] = xof;
				lineCounter++;
				xof = 0;
				txt_i--;
			}
			if (lineCounter > lineCount)
				break;
		}
	}


	float maxOffset = 0;
	for (int txt_i = fnt_first_char + 1; txt_i < fnt_last_char; ++txt_i) {
		const char ch = txt_i;

		stbtt_aligned_quad quad;
		stbtt_GetPackedQuad(selected->packed_chars, fnt_atlas_height, fnt_atlas_height, ch - fnt_first_char, &xof, &unused_offset_y, &quad, 1);

		maxOffset = glm::max(maxOffset, -quad.y1 * sdif + ((quad.y1 - quad.y0)  *sdif));
	}

	if (!boundMode) {
		if (g->t->justification == 0)
			xof = -record / 2;
		else if (g->t->justification == 1)
			xof = -lineLengths[lineNum] / 2;
		else if (g->t->justification == 2)
			xof = -record / 2 + (record - lineLengths[lineNum]);
	}
	else {
		if (g->t->justification == 0)
			xof = 0;
		else if (g->t->justification == 1)
			xof = g->scale.x / sdif / 2 - lineLengths[lineNum] / 2;
		else if (g->t->justification == 2)
			xof = g->scale.x / sdif - lineLengths[lineNum];
	}

	bool lineCut = false;
	//lineNum--; //compensation for first execution

	for (int txt_i = 0; txt_i < textLength; ++txt_i) {
		const char ch = text[txt_i];

		//handle new line
		if (ch == '\n' || lineCut) {
			lineNum++;
			lineCut = false;

			if (!boundMode) {
				if (g->t->justification == 0)
					xof = -record / 2;
				else if (g->t->justification == 1)
					xof = -lineLengths[lineNum] / 2;
				else if (g->t->justification == 2)
					xof = -record / 2 + (record - lineLengths[lineNum]);
			}
			else {
				if (g->t->justification == 0)
					xof = 0;
				else if (g->t->justification == 1)
					xof = g->scale.x / sdif / 2 - lineLengths[lineNum] / 2;
				else if (g->t->justification == 2)
					xof = g->scale.x / sdif - lineLengths[lineNum];

				if (boundMode && lineNum + 1 > lineCount)
					break;
			}

			continue;
		}

		stbtt_aligned_quad quad;
		stbtt_GetPackedQuad(selected->packed_chars, fnt_atlas_height, fnt_atlas_height, ch - fnt_first_char, &xof, &unused_offset_y, &quad, 0);

		if (boundMode && xof > g->scale.x / sdif) {
			lineCut = true;
			txt_i -= 2;
			continue;
		}

		const float uvx = (quad.s1 - quad.s0) / 2;
		const float uvy = (quad.t1 - quad.t0) / 2;
		g->t->letterUVData[txt_i * 4 + 0] = quad.s0 + uvx;
		g->t->letterUVData[txt_i * 4 + 1] = quad.t0 + uvy;
		g->t->letterUVData[txt_i * 4 + 2] = uvx;
		g->t->letterUVData[txt_i * 4 + 3] = uvy;

		const float quad_width_px = (quad.x1 - quad.x0)  * sdif;
		const float quad_height_px = (quad.y1 - quad.y0)  *sdif;

		float temmppp = -quad.y1 * sdif +
			quad_height_px;


		//position X and Y
		g->t->letterTransData[txt_i * 4 + 0] = (quad.x0 * sdif + 0.5f*quad_width_px) / (resolutionWidth / 2);
		
		//the y value of each letter is fairly complicated
		g->t->letterTransData[txt_i * 4 + 1] =
			(	
				(	//gives vertical allignment but breaks vertical position		
					!inverted ?
					- quad.y1 * sdif
					+ quad_height_px
					:
					+ quad.y1 * sdif
					- quad_height_px			
				)			
				+(
					!inverted ?
					-maxOffset
					:
					+maxOffset
				)
				+(
					!inverted ?
					-g->t->height * lineNum * 2
					:
					+g->t->height * lineNum * 2
				)
				+(
					!boundMode ?
					(
						!inverted ?
						g->t->height * (lineCount - 1)
						:
						-g->t->height * (lineCount - 1)
					)			
					:
					(
						!inverted ?
						g->scale.y / 1
						- g->t->height
						:
						-g->scale.y / 1
						+g->t->height
					)
				))
			/ (resolutionHeight / 1);

		//Scale X and Y
		g->t->letterTransData[txt_i * 4 + 2] = quad_width_px / resolutionWidth;
		g->t->letterTransData[txt_i * 4 + 3] = -quad_height_px / resolutionHeight;

		//if (text[txt_i + 1])
		//	xof += selected->scaleFactor * stbtt_GetCodepointKernAdvance(&selected->info, text[txt_i], text[txt_i + 1]);
	}

	//update position buffer
	glBindBuffer(GL_ARRAY_BUFFER, FTranVBO);
	glBufferData(GL_ARRAY_BUFFER, textLength * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, textLength * sizeof(GLfloat) * 4, g->t->letterTransData);

	glBindBuffer(GL_ARRAY_BUFFER, FuvVBO);
	glBufferData(GL_ARRAY_BUFFER, textLength * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, textLength * sizeof(GLfloat) * 4, g->t->letterUVData);

	//vertices
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, PVertVBO);
	glVertexAttribPointer(
		1,
		3,
		GL_FLOAT,
		GL_FALSE,
		0,
		(void*)0
	);
	// position and scale
	glEnableVertexAttribArray(5);
	glBindBuffer(GL_ARRAY_BUFFER, FTranVBO);
	glVertexAttribPointer(
		5,
		4,
		GL_FLOAT,
		GL_FALSE,
		0,
		(void*)0
	);
	// UV position and scale
	glEnableVertexAttribArray(6);
	glBindBuffer(GL_ARRAY_BUFFER, FuvVBO);
	glVertexAttribPointer(
		6,
		4,
		GL_FLOAT,
		GL_FALSE,
		0,
		(void*)0
	);

	glVertexAttribDivisor(1, 0);
	glVertexAttribDivisor(5, 1);
	glVertexAttribDivisor(6, 1);

	if (boundMode) {
		gl(Uniform2f(FontmPosUniformLocation, ((g->position.x - camera.x) - g->scale.x / 2) / resolutionWidth * 2.0f, (g->position.y - camera.y /* + g->scale.y / 2*/) / resolutionHeight * 2.0f));
	}
	else {
		gl(Uniform2f(FontmPosUniformLocation, (g->position.x - camera.x) / resolutionWidth * 2.0f, (g->position.y - camera.y) / resolutionHeight * 2.0f));
	}
	gl(Uniform1f(FontmRotUniformLocation, g->angle - g->rSpeed * currTime));

	gl(Uniform2f(FontzoomUniformLocation, cameraZoom.x, cameraZoom.y));

	delete lineLengths;
	delete text;
}

void GLCanvas::drawParticleSystem(GO * g, float deltaTime, mat4 * global)
{
	if (g->ps->dead)
		return;

	gl(UseProgram(ParticleShaderProgram));

	//if (ps.relitivePosition) {
	if (global) {
		//vec4 resulting = vec4(g->position, 0, 1) * *global;
		//g->ps->spawnLocation = vec2(resulting.y, resulting.x);
		if (g->parent->parent)
			g->ps->spawnLocation = g->position + g->parent->position + g->parent->parent->position;
		else
			g->ps->spawnLocation = g->position + g->parent->position;
		g->ps->angle = g->angle + g->parent->angle;
	}
	else
		g->ps->spawnLocation = g->position;
	//}
	g->ps->updateParticles(deltaTime, g->angle);

	if (g->ps->dead)
		return;

	//update position buffer
	gl(BindBuffer(GL_ARRAY_BUFFER, PPosVBO));
	gl(BufferData(GL_ARRAY_BUFFER, g->ps->MaxParticles * 3 * sizeof(GLfloat), NULL, GL_STREAM_DRAW));
	gl(BufferSubData(GL_ARRAY_BUFFER, 0, g->ps->count * sizeof(GLfloat) * 3, g->ps->positionSizeData));

	//update color buffer
	gl(BindBuffer(GL_ARRAY_BUFFER, PColVBO));
	gl(BufferData(GL_ARRAY_BUFFER, g->ps->MaxParticles * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW));
	gl(BufferSubData(GL_ARRAY_BUFFER, 0, g->ps->count * sizeof(GLubyte) * 4, g->ps->colorData));

	//update UVbuffer
	gl(BindBuffer(GL_ARRAY_BUFFER, PuvVBO));
	gl(BufferData(GL_ARRAY_BUFFER, g->ps->MaxParticles * 2 * sizeof(GLfloat), NULL, GL_STREAM_DRAW));
	gl(BufferSubData(GL_ARRAY_BUFFER, 0, g->ps->count * sizeof(GLfloat) * 2, g->ps->UVData));

	//vertices
	gl(EnableVertexAttribArray(1));
	gl(BindBuffer(GL_ARRAY_BUFFER, PVertVBO));
	gl(VertexAttribPointer(
		1,
		3,
		GL_FLOAT,
		GL_FALSE,
		0,
		(void*)0
	));
	// position and size
	gl(EnableVertexAttribArray(2));
	gl(BindBuffer(GL_ARRAY_BUFFER, PPosVBO));
	gl(VertexAttribPointer(
		2,
		3, // x + y + size + size => 3
		GL_FLOAT,
		GL_FALSE,
		0,
		(void*)0
	));
	//colors
	gl(EnableVertexAttribArray(3));
	gl(BindBuffer(GL_ARRAY_BUFFER, PColVBO));
	gl(VertexAttribPointer(
		3,
		4, // size : r + g + b + a => 4
		GL_UNSIGNED_BYTE,
		GL_TRUE, // normalized?
		0,
		(void*)0
	));
	// UV Data
	gl(EnableVertexAttribArray(4));
	gl(BindBuffer(GL_ARRAY_BUFFER, PuvVBO));
	gl(VertexAttribPointer(
		4,
		2, // x + y
		GL_FLOAT,
		GL_FALSE,
		0,
		(void*)0
	));

	gl(VertexAttribDivisor(1, 0)); // particles vertices : always reuse the same 4 vertices -> 0
	gl(VertexAttribDivisor(2, 1)); // positions : one per quad (its center) -> 1
	gl(VertexAttribDivisor(3, 1)); // color : one per quad -> 1
	gl(VertexAttribDivisor(4, 1)); // uvs

	//wether the particles care about where the particle system moves after being spawned
	if (g->ps->relitivePosition) {
		gl(Uniform2f(ParticlePosUniformLocation, -camera.x, -camera.y));
	}
	else if (!g->ps->relitivePosition) {
		gl(Uniform2f(ParticlePosUniformLocation, (g->ps->spawnLocation.x - camera.x), (g->ps->spawnLocation.y - camera.y)));
	}

	gl(Uniform2f(ParticleResUniformLocation, resolutionWidth, resolutionHeight));

	if (g->ps->textureMode) {
		if (!g->i) {
			g->ps->img = imgData(g->ps->filepath);
			g->i = &g->ps->img; //link the GO img pointer to reference the actual data in the particle system
		}
		setTexture(g, ParticleTextureUniformLocation);

		if (g->ps->tileMode)
			glUniform1f(ParticleUVScaleUniformLocation, g->ps->UVScale / g->ps->resolution);
		else
			glUniform1f(ParticleUVScaleUniformLocation, 1.0);
	}
	else
		gl(Uniform1f(ParticleUVScaleUniformLocation, 0));

	gl(DrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, g->ps->count));

	gl(BindTexture(GL_TEXTURE_2D, 0));
}

vec3 GLCanvas::getPixel(int x, int y) {
	glfwMakeContextCurrent(window);

	y = -y + resolutionHeight;
	unsigned char pixel[3] = { 0 };
	gl(ReadPixels(x, y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, &pixel));
	return vec3(pixel[0], pixel[1], pixel[2]);
}

void GLCanvas::updateDebugInfo() {
	if (debugTimer < currTime - debugUpdateFreq || debugTimer == 0)
		debugTimer = currTime;
	else
		return;
	std::stringstream sstm;
	sstm.precision(0);
	sstm.setf(std::ios::fixed);
	sstm << "Render Time: " << LastRenderTime << "ms  FPS: " << ceil(currentFPS) << "  Shapes: " << (GameObjects.size());
	debugString = sstm.str();
	windowTitleFlag = true;
}





////////////// physics stuff

b2CircleShape circCollider;
b2PolygonShape rectCollider;
b2BodyDef bodyDef; //defined in header

b2Vec2 toB2(vec2 v) {
	return b2Vec2(v.x / phScale, v.y / phScale);
}

bool testCirc(vec2 circPos, float rad, vec2 test)
{
	circCollider.m_radius = rad / phScale / 2;
	return circCollider.TestPoint(b2Transform(toB2(circPos), b2Rot()), toB2(test));
	return true;
}

bool testRect(vec2 rectPos, vec2 scale, float angle, vec2 test)
{
	rectCollider.SetAsBox(scale.x / phScale / 2, scale.y / phScale / 2);
	return rectCollider.TestPoint(b2Transform(toB2(rectPos), b2Rot(angle)), toB2(test));
	return true;
}

rigBody::rigBody(b2World * World, GO * Link, int type, float friction, bool Kinimatik, bool trigger, char * Tag)
{
	link = Link;
	world = World;
	kinematic = Kinimatik;
	tag = Tag;

#ifdef _DEBUG
	if (link->scale.x < 1 || link->scale.y < 1) {
		cout << "Debug warning: < 1 pixel rigidbody scale detected\n";
		return;
	}
#endif

	fixtureDef.isSensor = trigger;

	//as rect
	if (type == 1) {
		// Define the dynamic body. We set its position and call the body factory.
		bodyDef.position.Set((link->position.x / phScale), (link->position.y / phScale));
		bodyDef.angle = link->angle;
		body = world->CreateBody(&bodyDef);

		// Define another box shape for our dynamic body.
		b2PolygonShape dynamicBox;
		dynamicBox.SetAsBox(link->scale.x / phScale / 2.0f, link->scale.y / phScale / 2.0f);

		// Define the dynamic body fixture.
		fixtureDef.shape = &dynamicBox;

		// Set the box density to be non-zero, so it will be dynamic.
		fixtureDef.density = 1.0f;
		// Override the default friction.
		fixtureDef.friction = friction;

		// Add the shape to the body.
		body->CreateFixture(&fixtureDef);
	}
	//as circle
	else {
		bodyDef.position.Set((link->position.x / phScale), (link->position.y / phScale));
		body = world->CreateBody(&bodyDef);


		b2CircleShape dynamicCirc;
		dynamicCirc.m_radius = link->scale.x / phScale / 2;

		fixtureDef.shape = &dynamicCirc;
		fixtureDef.density = 1.0f;
		fixtureDef.friction = friction;

		body->CreateFixture(&fixtureDef);
	}

	if (kinematic)
		body->SetType(b2_kinematicBody);
	else
		body->SetType(b2_dynamicBody);

	body->SetUserData(Link);

}

void rigBody::addForce(vec2 force)
{
	body->ApplyForceToCenter(b2Vec2(force.x, force.y), true);
}

void rigBody::addTorque(float torque)
{
	body->ApplyTorque(torque, false);
}

void rigBody::setVelocity(vec2 velocity)
{
	body->SetLinearVelocity(toB2(velocity));
}

vec2 rigBody::GetVelocity()
{
	b2Vec2 v = body->GetLinearVelocity();
	return vec2(v.x, v.y);
}

void MyContactListener::BeginContact(b2Contact * contact)
{
	b2Fixture* fixtureA = contact->GetFixtureA();
	b2Body* bodyA = fixtureA->GetBody();
	GO* actorA = (GO*)bodyA->GetUserData();

	b2Fixture* fixtureB = contact->GetFixtureB();
	b2Body* bodyB = fixtureB->GetBody();
	GO* actorB = (GO*)bodyB->GetUserData();

	actorA->body->collisionEnter = true;
	actorB->body->collisionEnter = true;
	actorA->body->collisionExit = false;
	actorB->body->collisionExit = false;
	actorA->body->collisionTag = actorB->body->tag;
	actorB->body->collisionTag = actorA->body->tag;
}

void MyContactListener::EndContact(b2Contact * contact)
{
	b2Fixture* fixtureA = contact->GetFixtureA();
	b2Body* bodyA = fixtureA->GetBody();
	GO* actorA = (GO*)bodyA->GetUserData();

	b2Fixture* fixtureB = contact->GetFixtureB();
	b2Body* bodyB = fixtureB->GetBody();
	GO* actorB = (GO*)bodyB->GetUserData();

	actorA->body->collisionEnter = false;
	actorB->body->collisionEnter = false;
	actorA->body->collisionExit = true;
	actorB->body->collisionExit = true;
}
bool GLCanvas::raycast(vec2 start, vec2 end)
{
	RayCastCallback rc;
	world.RayCast(&rc, toB2(start), toB2(end));
	if (rc.m_fixture) {
		return true;
	}
	return false;

	//world.RayCast(&raycastCB, toB2(start), toB2(end));
	//return raycastCB.m_fixture;

	//this version could report what was hit

	//	can.world.RayCast(&can.raycastCB, toB2(pos), toB2(pos - vec2(0, 500)));
//	if (can.raycastCB.m_fixture) {
//		GO* g = (GO*)(can.raycastCB.m_fixture->GetBody()->GetUserData());
//		g->p->fColor = vec4(1);
//	}
}




//void GLCanvas::setFont(GO * g) {
//	gl(UseProgram(fontShaderProgram));
//
//	gl(Uniform1f(FontaspectUniformLocation, aspect));
//	gl(Uniform4f(FontColorUniformLocation, g->t->color.r, g->t->color.g, g->t->color.b, g->t->color.a));
//	gl(Uniform1f(FonttimeUniformLocation, currTime));
//
//
//	int HashIndex = g->t->getHashIndex();
//	if (HashIndex + 1 > fonts.size())
//		fonts.resize(HashIndex + 1);
//	if (!&fonts[HashIndex] || !fonts[HashIndex].init) {
//		fonts[HashIndex] = fontAsset(g->t->filepath);
//		fonts[HashIndex].loadTexture();
//	}
//
//	fontAsset * selected = &fonts[HashIndex];
//	gl(BindTexture(GL_TEXTURE_2D, selected->id));
//	gl(Uniform1i(FonttextureUniformLocation, 0));
//
//	float xof = 0; //x offset of each letter
//	float x = 0, y = 0; //required by stbtt
//	float scaleRatio = g->t->height / selected->tallestLetter;
//	float totalWidth = 0;
//	int lineCount = 1; //number of lines in the string (# of '\n')
//	int lineNum = 0;  //current line being worked on
//	char c; //used to store each letter for calculations
//	bool boundMode = g->t->boundMode; //bounding box mode
//	int maxLines; //Only used for bounding box mode
//	float record = 0; //longest line of text
//	int recordIndex = 0; //which line number is the
//	int Xreference; // iether scaled boundry.X or record
//	int textLength = g->t->text.length();
//	float * lineLengths; //array of pixel lengths for each line
//	bool lineBreak = false; //runtime flag for inserting new lines in bound mode
//
//	float letterPosX, letterPosY, finalScaleX, finalScaleY; //used ever for letter. Allocated here for performance
//
//	//much faster to convert the string to a char *
//	char * text = new char[g->t->text.size() + 1];
//	memcpy(text, g->t->text.c_str(), g->t->text.size());
//
//	//allocate an arrays to be coppied to the VBOs
//	if (textLength != g->t->TextLength) {
//		//if containers are not being allocated for the first time, delete the previuos data
//		if (g->t->TextLength > 0)
//			g->t->dispose();
//		g->t->letterTransData = new float[textLength * 4]();
//		g->t->letterUVData = new float[textLength * 4]();
//		g->t->TextLength = textLength;
//	}
//
//
//	//pre calculate the pixel lengths of the final lines
//	for (int i = 0; i < textLength; i++)
//		if (text[i] == '\n')
//			lineCount++;
//
//	lineLengths = new float[lineCount];
//
//	for (int i = 0, lineCounter = 1; i < textLength; i++)
//	{
//		c = text[i];
//		//actual letter lengths
//		if (c != '\n' && c != ' ');
//		totalWidth += selected->quadScale[c - 32].x;
//		//add in distace for space characters
//		if (c == ' ')
//			totalWidth += selected->spaceOff;
//		if (c == '\n' || i == textLength - 1) {
//			record = totalWidth > record ? totalWidth : record;
//			lineLengths[lineCounter - 1] = totalWidth;
//			lineCounter++;
//			totalWidth = 0;
//		}
//	}
//
//	if (boundMode) {
//		maxLines = g->scale.y / g->t->height;
//		if (lineCount > maxLines)
//			lineCount = maxLines;
//		//g->pos = g->pos + vec2(-g->scale.x / (float)2,
//			//g->scale.y / (float)2 - selected->tallestLetter / (float)2);
//		Xreference = g->scale.x / scaleRatio;
//	}
//	else Xreference = record;
//
//
//	for (int i = 0; i < textLength; i++)
//	{
//		c = text[i];
//
//		//handle new line
//		if (c == '\n') {
//			lineNum++;
//			if (boundMode && lineNum + 1 > maxLines)
//				break;
//			continue;
//		}
//		//set the cursor for the new line
//		if (i == 0 || text[i - 1] == '\n' || lineBreak) {
//			if (g->t->justification == 0 || (boundMode && lineLengths[lineNum] > g->scale.x / scaleRatio))
//				xof = 0;
//			else if (g->t->justification == 1)
//				xof = (Xreference - lineLengths[lineNum]) / 2;
//			else {
//				xof = (Xreference - lineLengths[lineNum]);
//			}
//			lineBreak = false;
//		}
//
//		g->t->letterUVData[i * 4 + 0] = selected->uvOffset[c - 32].x;
//		g->t->letterUVData[i * 4 + 1] = selected->uvOffset[c - 32].y;
//		g->t->letterUVData[i * 4 + 2] = selected->uvScale[c - 32].x;
//		g->t->letterUVData[i * 4 + 3] = selected->uvScale[c - 32].y;
//
//		vec2 letterScale = selected->quadScale[c - 32];
//
//		xof += letterScale.x / 2; //move the scale by  width of current chashapeor
//		if (boundMode && xof *scaleRatio > g->scale.x) {
//			lineNum++;
//			lineBreak = true;
//			if (boundMode && lineNum + 1 > maxLines)
//				break;
//			i--;
//			continue;
//		}
//
//		float xTranslate = xof;
//		float yTranslate = selected->alignment[c - 32] - selected->tallest`Letter * lineNum;
//
//		//calculate final position of the letter
//		{
//			if (!boundMode) {
//				xTranslate -= record / 2;
//				yTranslate += selected->alignmentOffset;
//				yTranslate += (selected->tallestLetter * lineCount) / 2;
//			}
//			else {
//				//yTranslate += g->scale.y/2;
//			}
//			//	yTranslate += (selected->tallestLetter * lineCount) / 2;
//
//			letterPosX = (xTranslate * scaleRatio) / resolutionWidth * 2.0f;
//			letterPosY = (yTranslate * scaleRatio) / resolutionHeight * 2.0f;
//
//			if (boundMode)
//				g->t->lastLetterPos = vec2((xTranslate - selected->alignment[c - 32]) * scaleRatio - g->scale.x / 2,
//				(yTranslate - selected->alignment[c - 32]) * scaleRatio + g->scale.y / 2);
//			else
//				g->t->lastLetterPos = vec2((xTranslate - selected->alignment[c - 32]) * scaleRatio,
//				(yTranslate - selected->alignment[c - 32]) * scaleRatio);
//		}
//		//final scale of the letter
//		{
//			finalScaleX = letterScale.x * scaleRatio / resolutionWidth;
//			finalScaleY = letterScale.y * scaleRatio / resolutionHeight;
//		}
//
//		//transform
//		g->t->letterTransData[i * 4 + 0] = letterPosX;
//		g->t->letterTransData[i * 4 + 1] = letterPosY;
//		g->t->letterTransData[i * 4 + 2] = finalScaleX;
//		g->t->letterTransData[i * 4 + 3] = finalScaleY;
//
//		if (c == ' ')
//			xof += selected->spaceOff;
//		xof += letterScale.x / 2;
//	}
//	//update position buffer
//	glBindBuffer(GL_ARRAY_BUFFER, FTranVBO);
//	glBufferData(GL_ARRAY_BUFFER, textLength * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);
//	glBufferSubData(GL_ARRAY_BUFFER, 0, textLength * sizeof(GLfloat) * 4, g->t->letterTransData);
//
//	glBindBuffer(GL_ARRAY_BUFFER, FuvVBO);
//	glBufferData(GL_ARRAY_BUFFER, textLength * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);
//	glBufferSubData(GL_ARRAY_BUFFER, 0, textLength * sizeof(GLfloat) * 4, g->t->letterUVData);
//
//	//vertices
//	glEnableVertexAttribArray(1);
//	glBindBuffer(GL_ARRAY_BUFFER, PVertVBO);
//	glVertexAttribPointer(
//		1,
//		3,
//		GL_FLOAT,
//		GL_FALSE,
//		0,
//		(void*)0
//	);
//	// position and scale
//	glEnableVertexAttribArray(5);
//	glBindBuffer(GL_ARRAY_BUFFER, FTranVBO);
//	glVertexAttribPointer(
//		5,
//		4,
//		GL_FLOAT,
//		GL_FALSE,
//		0,
//		(void*)0
//	);
//	// UV position and scale
//	glEnableVertexAttribArray(6);
//	glBindBuffer(GL_ARRAY_BUFFER, FuvVBO);
//	glVertexAttribPointer(
//		6,
//		4,
//		GL_FLOAT,
//		GL_FALSE,
//		0,
//		(void*)0
//	);
//
//	glVertexAttribDivisor(1, 0);
//	glVertexAttribDivisor(5, 1);
//	glVertexAttribDivisor(6, 1);
//
//	if (boundMode) {
//		gl(Uniform2f(FontmPosUniformLocation, ((g->position.x - camera.x) - g->scale.x / 2) / resolutionWidth * 2.0f, (g->position.y - camera.y + g->scale.y / 2) / resolutionHeight * 2.0f));
//	}
//	else {
//		gl(Uniform2f(FontmPosUniformLocation, (g->position.x - camera.x) / resolutionWidth * 2.0f, (g->position.y - camera.y) / resolutionHeight * 2.0f));
//	}
//	gl(Uniform1f(FontmRotUniformLocation, g->angle - g->rSpeed * currTime));
//
//	gl(Uniform1f(FontzoomUniformLocation, cameraZoom));
//
//	delete lineLengths;
//	delete text;
//}