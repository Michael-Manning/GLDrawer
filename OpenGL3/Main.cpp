#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <tchar.h>
#include <math.h>
#include <assert.h>

#include <gl3w/gl3w.h>
#include <GL/GLU.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
//#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>


#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <Box2D/Box2D.h>
#include "engine.h"
#include <thread>   

#define QUARTPI 0.78539816339f
#define HALFPI 1.57079632679f
#define PI 3.14159265359f
#define TWOPI 6.28318530718f

using namespace std;
using namespace glm;
int runBenchmark();
int runFontBenchmark();
int runParticleTest();
int runTransformTest();
int runPhysicsTest();
int fillBackBuffer();
int runPlatformerTest();
int gridThing();

//For testing GLDrawer from the backend  

int main()
{
	cout << "Running from native C++ \n";

	//return runParticleTest();
	//return runBenchmark();
	//return runFontBenchmark();
	//return runTransformTest();
	//return runPhysicsTest();
	//return runPlatformerTest();
	//return  fillBackBuffer();
	//return gridThing();

	////////////////////General test

	GLCanvas can;
	if (can.createCanvas(1300, 900, true, vec3(0.25), false, false))
		cout << "canvas creation failed\n";

	can.cameraZoom.y *= 1;

	//GLCanvas can2;
	//if (can2.createCanvas(1300, 900, true, vec3(0.25), false, false))
	//	cout << "canvas creation failed\n";

	//Data
	polyData blueRect(vec4(0, 0, 1, 1), vec4(1), 10);
	polyData greyRect(vec4(1, 1, 1, 0.4));
	polyData greenRect(vec4(0, 1, 0, 1));
	polyData redCirc(vec4(1, 0, 0, 1), vec4(1), 4, 1);
	polyData purpleOval(vec4(1, 0, 1, 1), vec4(0), 0, 1);
	polyData rainbowPent(vec4(1, 1, 1, -2), vec4(1, 1, 1, -1), 20, 5);
	polyData rainbowCirc(vec4(1, 1, 1, -1), vec4(0), 0, 1);
	polyData whiteRect;
	polyData RedRect(vec4(1), vec4(1, 0, 0, 1), 20, 4);
	polyData blackRect(vec4(0));

	imgData flowerData("../data/images/flower.jpg", vec4(), vec2(0), vec2(2));
	imgData catData("../data/images/test_cat.png", vec4(1, 1, 1, 0.5f));
	animationData animData(1024, 8, 1.f);
	imgData fireimg("../data/images/Fire_Spritesheet.png");
	fireimg.adata = &animData;

	catData.opacity = 0.1f;
	textData freeTextData("test}\ntetogglyeeeest}\ntest}}", 30, vec4(1), 2, "c:\\windows\\fonts\\comic.ttf");
	textData freeTextData2("kTg}}", 60, vec4(1), 1, "c:\\windows\\fonts\\times.ttf");
	textData boundTextData("}K7/!jgTimes toggley New Roman\nyyyyyyyyyy\nyyyyyyyyyyyyyyyyyyyy\n123456789123456789123456789123456789\n\nSixth line\nseventh Line\n tttttt\n 999999", 50, vec4(1), 0, "c:\\windows\\fonts\\times.ttf", true);

	//GameObjects
	GO flower(&flowerData, vec2(-300, 300), vec2(200));
	GO cat(&catData, vec2(-500, 300), vec2(200));
	GO fire(&fireimg, vec2(-500, -100), vec2(200));
	GO freeSans(&freeTextData, vec2(-350, 0));
	GO freeSans2(&freeTextData2, vec2(-175, 0));
	GO boundText(&boundTextData, vec2(250, -100), vec2(655, 390));
	GO textBound(&greyRect, vec2(250, -100), vec2(655, 390));

	GO shapeA(&blueRect, vec2(400, 200), vec2(200, 100), 1, 0);
	GO shapeF(&greyRect, vec2(-350, 0), vec2(500, 2), 0);
	GO shapeB(&rainbowPent, vec2(200, -400), vec2(200), 0, 0.5f);
	GO shapeC(&redCirc, vec2(300), vec2(300));
	GO shapeD(&greenRect, vec2(200), vec2(200));
	GO shapeE(&purpleOval, vec2(-200, -300), vec2(200, 100));

	GO ba(&RedRect, vec2(-300, 0), vec2(400));
	GO bb(&RedRect, vec2(0, 0), vec2(400), QUARTPI);
	GO ta(&whiteRect, vec2(-300, 0), vec2(400));
	GO tb(&whiteRect, vec2(300, 0), vec2(400), 1);

	//add GameObjects to canvas
	can.addGO(&flower);
	can.addGO(&shapeD);
	can.addGO(&cat);

	can.addGO(&shapeC);
	can.addGO(&shapeB);
	can.addGO(&shapeE);
	can.addGO(&shapeA);
	can.addGO(&shapeF);
	can.addGO(&freeSans);
	can.addGO(&freeSans2);
	can.addGO(&textBound);
	can.addGO(&boundText);
	can.addGO(&fire);
	//can.addGO(&ba);
	//
	//can.addGO(&ta);
	//can.addGO(&bb);
	//can.addGO(&tb);



	for (int j = 0; j < 900; j++)
	{
		for (int i = 0; i < 1300; i++)
		{
			float g = (float)j / 900.0f;
			float b = (float)i / 1300.0f;
			//	can.setBBPixel(i, j, vec4(1, g, b, 1));
		}
	}


	while (!can.disposed) {

		can.mainloop();
		//can2.mainloop();
		vec2 pos = can.getMousePos();
		pos.y = -pos.y + 900;
		if (can.getMouseDown(0)) {

			vec3 col = can.getPixel(pos.x, pos.y);
			cout << "\n" << col.r << " " << col.g << " " << col.b;
			col = col / 255.0f;
		}

		if (can.getKeyDown('i'))
			can.cameraZoom += 0.2f;
		if (can.getKeyDown('o'))
			can.cameraZoom -= 0.2f;
		if (can.getKeyDown('d'))
			can.camera.x += 10;
		if (can.getKeyDown('a'))
			can.camera.x -= 10;

		if (can.getKeyDown('r')) {
			freeTextData.justification = 0;
			boundTextData.justification = 0;
		}
		if (can.getKeyDown('t')) {
			freeTextData.justification = 1;
			boundTextData.justification = 1;
		}
		if (can.getKeyDown('y')) {
			freeTextData.justification = 2;
			boundTextData.justification = 2;
		}

		if (can.getKeyDown('p'))
			fire.i->adata->play = !fire.i->adata->play;
		if (can.getKeyDown('g'))
			fire.i->adata->nextFrame++;
		if (can.getKeyDown('h'))
			fire.i->adata->nextFrame--;



		can.setBBShape(GO(&rainbowCirc, pos - can.resolutionV2f / 2.0f, vec2(60.0, 60.0) + 30 * (float)(sin(can.currTime * 8) + 1.0)));
	}
	return 0;
}

/////////////Testing routines

polyData polys[10000];
GO onScreen[10000];
int screenCount = 0;
GLCanvas can;

void add(polyData p, GO g) {
	polys[screenCount] = p;
	onScreen[screenCount] = g;
	onScreen[screenCount].p = polys + screenCount;
	can.addGO(onScreen + screenCount);
	screenCount++;
}
int runBenchmark() {
	if (can.createCanvas(1000, 1000, true, vec3(0.2f), false)) {
		cout << "canvas creation failed\n";
		return 0;
	}

	int count = 100;
	float scale = (1000 / count);
	for (int i = 0; i < count; i++) {
		for (int j = 0; j < count; j++) {

			polyData p(vec4(i / (float)count, 0.8, j / (float)count, 1));
			add(p, GO(vec2(i * scale + scale / 2 - 500, j * scale + scale / 2 - 500), vec2(10, 10), 0, 2));
		}
	}
	while (!can.disposed) {
		can.mainloop();
	}
	return 1;
}

int runFontBenchmark() {
	polyData greyRect(vec4(1, 1, 1, 0.4));
	polyData blackRect(vec4(0,0,0,1));
	

	if (can.createCanvas(1024, 1024, true, vec3(0.2f), false)) {
		cout << "canvas creation failed\n";
		return 0;
	}
#if 1
	textData sample("letters all day, letters all day, letters all day, letters all day, letters all day, letters all day, letters all day, letters all day,"
		"letters all day, letters all day, letters all day, letters all day, letters all day, letters all day, letters all day, letters all day,",
		10, vec4(1), 1, "c:\\windows\\fonts\\comic.ttf");
	for (int i = 0; i < 100; i++) {
		onScreen[i] = GO(&sample, vec2(0, i * 10 + -495));
		can.addGO(onScreen + i);
	}
#else
	const char * testString = "Letters } WA Ye all day,";
	textData sample(testString, 100, vec4(1, 1, 1, 1), 1, "c:\\windows\\fonts\\arialbd.ttf");
	textData sample2(testString, 50, vec4(1, 1, 1, 1), 1, "c:\\windows\\fonts\\arialbd.ttf");
	textData sample3(testString, 150, vec4(1, 1, 1, 1), 1, "c:\\windows\\fonts\\arialbd.ttf");

	//textData sample(testString, 100, vec4(1), 1, "c:\\windows\\fonts\\times.ttf");
	//GO cool = GO(vec2(0, 0), vec2(200, 120));

	GO Dcool = GO(vec2(-400, 0), vec2(200, 120));
	Dcool.t = &sample;
	GO Dcool2 = GO(vec2(-400, -150), vec2(200, 120));
	Dcool2.t = &sample2;
	GO Dcool3 = GO(vec2(-400, 150), vec2(200, 120));
	Dcool3.t = &sample3;

	GO textBound(&greyRect, vec2(0, 0), vec2(200, 100));
	GO baseline(&blackRect, vec2(0, 0), vec2(2000, 3));

	GO textBound2(&greyRect, vec2(0, -150), vec2(200, 50));
	GO baseline2(&blackRect, vec2(0, -150), vec2(2000, 3));

	GO textBound3(&greyRect, vec2(0, 150), vec2(200, 150));
	GO baseline3(&blackRect, vec2(0, 150), vec2(2000, 3));

//	cool.t = &sample;

    can.addGO(&textBound);
    can.addGO(&baseline);
	can.addGO(&textBound2);
	can.addGO(&baseline2);
	can.addGO(&textBound3);
	can.addGO(&baseline3);

	//can.addGO(&cool);
	can.addGO(&Dcool);
	can.addGO(&Dcool2);
	can.addGO(&Dcool3);

#endif

	int tcount = 0;
	while (!can.disposed) {
		can.mainloop();
		if (tcount == 2) {
		//	can.saveCanvasAsImage("../coolpic.bmp");
		}
		tcount++;
	}
	return 1;
}

int fillBackBuffer() {
	if (can.createCanvas(1300, 900, true, vec3(0.2f), false)) {
		cout << "canvas creation failed\n";
		return 0;
	}

	for (int j = 0; j < 900; j++)
	{
		for (int i = 0; i < 1000; i++)
		{
			//can.setBBPixel(i, j, vec4(RandomFloat(0, 1), RandomFloat(0, 1), RandomFloat(0, 1), 1));
		}
	}

	while (!can.disposed) {
		can.mainloop();
	}
	return 0;
}

int runParticleTest() {
	if (can.createCanvas(1300, 900, true, vec3(0.2f), false)) {
		cout << "canvas creation failed\n";
		return 0;
	}

	//imgData cat("../data/images/test_cat.png");
	textData text("Score: ", 60, vec4(1), 1, "../data/fonts/game_over.ttf");
	imgData spaceship("../data/images/spaceship.png");

	GO score(&text, vec2(-500, 400));
	can.addGO(&score);
	GO cursor(&spaceship, vec2(), vec2(200, 200));
	can.addGO(&cursor);

	//simple swirly particles
	ParticleSystem pA(2000, 2);
	pA.angle = 0;
	pA.spread = 0.5f;
	pA.continuous = true;
	pA.burstMode = true;
	pA.gravity = vec2(0, -240.0);
	pA.startCol = vec4(0, 0, 1, 1);
	pA.endCol = vec4(1, 1, 0, 1);
	pA.startSize = 15;
	pA.endSize = 0;
	pA.speed = 300.0f;
	pA.speedPrecision = 30.0f;
	pA.lifePrecision = 0.4;
	pA.drawBehindShape = false;

	GO testA(&pA, vec2(-400, -100));
	can.addGO(&testA);

	//textured paritcles + burst mode
	ParticleSystem pB(150, 1.5f);
	pB.angle = -3.1459f / 2.0f;
	pB.spread = 0.5f;
	pB.continuous = true;
	pB.burstMode = false;
	pB.startSize = 40;
	pB.endSize = 45;
	pB.speed = 500.0f;
	pB.speedPrecision = 30.0f;
	pB.lifePrecision = 0.4;
	pB.spawnLocation = vec2(0, 0);
	pB.radius = 60;
	pB.relitivePosition = true;
	pB.setTexture("../data/images/test_cat.png");
	pB.drawBehindShape = true;

	GO testB(&pB, vec2(400, 100));
	can.addGO(&testB);

	//animated particles
	ParticleSystem fire(150, 1.5f);
	fire.angle = -3.1459f / 2.0f;
	fire.spread = 0.5f;
	fire.continuous = true;
	fire.burstMode = false;
	fire.startSize = 40;
	fire.endSize = 45;
	fire.speed = 500.0f;
	fire.speedPrecision = 30.0f;
	fire.lifePrecision = 0.4;
	fire.spawnLocation = vec2(0, 0);
	fire.radius = 60;
	fire.relitivePosition = true;
	fire.setAnimation("../data/images/Fire_Spritesheet.png", 1024, 8);
	fire.drawBehindShape = true;

	GO rocketFire(&fire);
	rocketFire.parent = &cursor;
	can.addGO(&rocketFire);

	float points = 0;
	while (!can.disposed) {
		points += 0.04;
		text.text = ("Score: " + to_string((int)points));
		can.mainloop();
		vec2 mousePos = can.getMousePos();
		mousePos.y = -mousePos.y + 900;

		testA.angle += 0.3 * can.LastRenderTime;
		cursor.position = mousePos + vec2(0.0, 20) - can.resolutionV2f / 2.0f;
	}
	return 0;
}

int runTransformTest() {

	if (can.createCanvas(1000, 1000, true, vec3(0.2f), true)) {
		cout << "canvas creation failed\n";
		return 0;
	}

	polyData blue(vec4(0, 0, 1, 1), vec4(0, 0, 0, 1), 10);
	polyData yel(vec4(1, 1, 0, 1), vec4(0, 0, 0, 1), 10);
	polyData red(vec4(1, 0, 0, 1), vec4(0, 0, 0, 1), 10);
	polyData green(vec4(0, 1, 0, 1), vec4(0, 0, 0, 1), 10);
	imgData cat("../data/images/test_cat.png");
	imgData flower("../data/images/flower.jpg");
	textData text("test", 30, vec4(1), 2, "c:\\windows\\fonts\\comic.ttf");

	//parent rotation
	//GO parent(&cat, vec2(), vec2(200));
	GO parent(&blue, vec2(), vec2(200));
	parent.drawIndex = 5;
	can.addGO(&parent);

	GO childA(&yel, vec2(170), vec2(90));
	childA.parent = &parent;
	childA.drawIndex = 4;
	can.addGO(&childA);

	//shape childBShape(vec2(), vec2(50, 50), 0, vec4(1, 0, 0, 1), vec4(0, 0, 0, 1), 3, 0.0);
	GO childB(&red, vec2(100), vec2(50));
	childB.parent = &childA;
	childB.drawIndex = 3;
	can.addGO(&childB);

	//simple swirly particles
	ParticleSystem pA(1000, 1);
	pA.angle = 0;
	pA.spread = 0.5f;
	pA.continuous = true;
	pA.burstMode = false;
	pA.gravity = vec2(0, -240.0);
	pA.startCol = vec4(0, 0, 1, 1);
	pA.endCol = vec4(1, 1, 0, 1);
	pA.startSize = 10;
	pA.endSize = 0;
	pA.speed = 300.0f;
	pA.speedPrecision = 30.0f;
	pA.lifePrecision = 0.4;
	pA.drawBehindShape = true;


	GO childC(&green, vec2(70), vec2(40), 3, 4);
	//GO childC(&text, vec2(70), vec2(0), 3, 4);
	//childC.ps = &pA;
	childC.parent = &childB;
	childC.drawIndex = 2;
	can.addGO(&childC);

	while (!can.disposed) {
		parent.angle += 0.02f;
		childA.angle += 0.04f;
		childB.angle += 0.06f;

		if (can.getKeyDown('i'))
			can.cameraZoom += 0.2f;
		if (can.getKeyDown('o'))
			can.cameraZoom -= 0.2f;
		//vec2 mousePos = can.Input->getMousePos();
		//mousePos.y = -mousePos.y + 1000;
		//can.camera = (mousePos - 500.0f) / 3.0f;

		can.mainloop();
	}

	return 0;
}




vector<rigBody> bodys;
void addBody(GO g, polyData p, bool kino = false) {
	polys[screenCount] = p;

	onScreen[screenCount] = g;
	onScreen[screenCount].p = polys + screenCount;

	bodys.push_back(rigBody(&can.world, onScreen + screenCount, 1, 0.8, kino));
	onScreen[screenCount].body = &bodys[screenCount];
	onScreen[screenCount].body->addForce(vec2(100, 0));
	onScreen[screenCount].body->addTorque(100);
	can.addGO(onScreen + screenCount);

	//bodys have no default constructor, so for the sake of the test, here's the storage solution with a vector
	for (int i = 0; i < screenCount; i++)
		onScreen[i].body = &bodys[i];

	screenCount++;
}



int runPhysicsTest() {

	polyData blueRect(vec4(0, 0, 1, 1), vec4(1), 10);
	polyData redRect(vec4(1, 0, 0, 1), vec4(1), 10);
	polyData purpCirc(vec4(1, 0, 1, 1), vec4(1), 10, 1);
	polyData invis(vec4(0));
	GO shapeA(vec2(70, 400), vec2(100, 100), 3);
	GO shapeB(vec2(30, 200), vec2(200, 100), -1);
	GO shapeC(&purpCirc, vec2(400, 0), vec2(200, 200));
	GO shapeD(&blueRect, vec2(-200, 0), vec2(200, 100), 2);

	if (can.createCanvas(1000, 1000, true, vec3(0.2f), true)) {
		cout << "canvas creation failed\n";
		return 0;
	}

	can.addGO(&shapeC);
	can.addGO(&shapeD);

	GO ground(vec2(0, -505), vec2(1000, 10));
	addBody(ground, invis, true);

	while (!can.disposed) {
		can.mainloop();
		vec2 pos = can.getMousePos();
		pos.y = -pos.y + can.resolutionHeight;
		pos.y -= can.resolutionHeight / 2;
		pos.x -= can.resolutionWidth / 2;

		if (can.getKey('r'))
			can.raycast(pos, pos - vec2(0, 500));


		if (can.getMouseDown(0)) {
			shapeA.position = pos;
			addBody(shapeA, blueRect);
		}
		else if (can.getMouseDown(1)) {
			shapeB.position = pos;
			addBody(shapeB, redRect);
		}

		if (testCirc(shapeC.position, shapeC.scale.x, pos))
			purpCirc.fColor = vec4(1, 1, 0, 1);
		else
			purpCirc.fColor = vec4(1, 0, 1, 1);
		if (testRect(shapeD.position, shapeD.scale, shapeD.angle, pos))
			blueRect.fColor = vec4(1, 1, 0, 1);
		else
			blueRect.fColor = vec4(0, 0, 1, 1);
	}

}

int runPlatformerTest() {

	polyData blueRect(vec4(0, 0, 1, 0.8));
	polyData redRect(vec4(1, 0, 0, 1));

	GO guy(&redRect, vec2(0, 30), vec2(100));
	GO ground(&blueRect, vec2(0, -350), vec2(1000, 10));

	if (can.createCanvas(1000, 800, true, vec3(0.2f), true)) {
		cout << "canvas creation failed\n";
		return 0;
	}

	can.addGO(&guy);
	can.addGO(&ground);

	rigBody a(&can.world, &guy, 1, 0.2f);
	rigBody b(&can.world, &ground, 1, 0.2f, true);
	guy.body = &a;
	ground.body = &b;

	while (!can.disposed) {
		can.mainloop();

		if (can.getKey('a')) {
			a.addForce(vec2(-30, 0));
		}
		if (can.getKey('d')) {
			a.addForce(vec2(30, 0));
		}
		if (can.getKeyDown('w')) {
			a.addForce(vec2(0, 400));
		}
	}
	return 0;
}

int gridThing() {

	//vector<GO> stuff;
	//textData texts[500];

	//polyData lineData(vec4(0, 0, 0, 1));
	//textData tdata("aaa", 30, vec4(0,0,0,1), 1, "c:\\windows\\fonts\\arialbd.ttf");

	int dims = 1000;

	if (can.createCanvas(dims, dims, true, vec3(0.0f), true)) {
		cout << "canvas creation failed\n";
		return 0;
	}

    textData td("Press C to toggley Coordinates: {0}\n X: {1} Y: {2}", 40, vec4(1), 1, "c:\\windows\\fonts\\times.ttf", false, true);
	GO tt(&td);
	can.addGO(&tt);
	//
	////settings
	//int padding = 100;
	//float thickness = 3;
	//int cellCount = 10;
	//const char * letters = "1234567891234567891234567891234567891234567891234567891234567891234567891234567891234567891234567891";
	//int letLen = strlen(letters);

	//int div = (dims - padding * 2) / (cellCount);
	//float lineLength = dims - padding * 2;

	//for (int i = 0; i < cellCount + 1; i++)
	//{
	//	float dist = (padding - dims / 2) + div * i;
	//	GO horizontal(&lineData, vec2(0, dist), vec2(lineLength, thickness));
	//	GO vertical (&lineData, vec2(dist, 0), vec2(thickness, lineLength));

	//	stuff.push_back(horizontal);
	//	stuff.push_back(vertical);
	//}
	//for (int i = 0; i < letLen && i < cellCount * cellCount; i++)
	//{
	//	float x = (padding - dims / 2) + div * (i % cellCount) + div / 4;
	//	float y = (padding - dims / 2) + div * (cellCount -1 - i / cellCount) + div / 4;

	//	char let = letters[i];
	//	texts[i] = textData(&let, 50, vec4(0, 0, 0, 1), 1, "c:\\windows\\fonts\\arialbd.ttf");
	//	GO letter(vec2(x, y));
	//	letter.t = texts + i;
	//	stuff.push_back(letter);
	//}


	//for (int i = 0; i < stuff.size(); i++)
	//{
	//	can.addGO(&stuff[i]);
	//}

//	int framecount = 0;
	while (!can.disposed) {
		can.mainloop();
	//	if (framecount == 2)
		//	can.saveCanvasAsImage("../Grid.bmp");
		//framecount++;
	}

	return 0;
}