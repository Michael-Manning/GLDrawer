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
#include "Input.h"
#include "link.h"
#include <thread>   

using namespace std;
using namespace glm;
int runBenchmark();

int main()
{
	cout << "Running from native C++ \n";

	//return runBenchmark();

	BaseGLD base;
	base.Input = &InputManager();
	if (base.createCanvas(1300, 900, true, vec3(0.3f)))
		cout << "canvas creation failed\n";
	//BaseGLD base2;
//	base2.Input = &InputManager();
	//base2.createCanvas(1000, 800, true, vec3(0.5f));

	rect g(vec2(880, 450), vec2(655, 390), 0, vec4(0.5), vec4(), 0, 0);

	rect a(vec2(300, 100), vec2(100, 200), 0, vec4(1, 0, 0, -1), vec4(), 0, 0);
	rect b(vec2(100, 100), vec2(100, 200), 0, vec4(0, 0, 1, 1), vec4(), 0, 0);
	rect k(vec2(400, 100), vec2(100, 100), 0, vec4(0, 0, 1, -1), vec4(), 0, 0, 1);
	rect c("data\\arthur-this-isnt-weed.jpg", vec2(100, 800), vec2(300, 300), 0, vec4(1, 0, 0, 1));
	rect f("data\\test_cat.png", vec2(400, 800), vec2(300, 300), 0, vec4(1, 0, 0, 1));

	rect d("c:\\windows\\fonts\\times.ttf", "}Times New Roman\nyyyyyyyyyy\nyyyyyyyyyyyyyyyyyyyy\n123456789123456789123456789123456789\n\nSixth line\nseventh Line\neighth line", 6, vec2(400, 550), 50, 2, &g, 0, vec4(1));
	//yyyyyyyyyyyyyyyyyaby
	rect e("c:\\windows\\fonts\\comic.ttf", "SAANST{[\nfancy new line which is longer thst line\nmedium sized n}ew line", 6, vec2(300, 400), 30, 2, NULL, 0, vec4(1));
	rect w(vec2(300, 400), vec2(480, 90), 0, vec4(0, 0, 0, 1), vec4(), 0, 0, 4);

	rect h(vec2(600, 100), vec2(100, 200), 0, vec4(1, 0, 0, 1), vec4(), 0, 0, 5);

	base.addRect(&g);
	base.addRect(&a);
	base.addRect(&b);
	base.addRect(&c);
	base.addRect(&d);
	base.addRect(&w);
	base.addRect(&e);
	base.addRect(&f);
	base.addRect(&k);
	base.addRect(&h);

//	base2.addRect(&a);
//	base2.addRect(&b);
//	base2.addRect(&c);
	base.setBBPixel(vec2(300, 300), vec4(1, 0, 1, 1));
	while (!base.Cleaned) {
		base.mainloop();
		if (base.Input->getMouseDown(0)) {
			vec2 pos = base.Input->getMousePos();
			vec3 col = base.getPixel(pos.x, pos.y);
			cout << "\n" << col.r << " " << col.g << " " << col.b;
			col = col / 255.0f;
			b.color = vec4(col, 1);
		}

	//	base2.mainloop();
	}
//	cin.get();


	return 0;
}

rect onScreen[10000];
int screenCount = 0;
BaseGLD can;
void renderLoop(BaseGLD * can) {
	while (!can->Cleaned) {
		can->mainloop();
	}
}
void add(rect r) {
	onScreen[screenCount] = r;
	can.addRect(onScreen + screenCount);
	screenCount++;
}
int runBenchmark() {


	can.Input = &InputManager();
	if (can.createCanvas(1000, 1000, true, vec3(0.2f), false)) {
		cout << "canvas creation failed\n";
		return 0;
	}
	
	int count = 30;
	add(rect(vec2(250, 250), vec2(500, 500),0, vec4(vec3(0.2), 1.0)));
	for (int i = 0; i < count; i++) {
		for (int j = 0; j < count; j++) {
			float scale = (1000 / count);
			add(rect(vec2(i * scale + scale/2, j * scale + scale/2), vec2(40, 20), 0, vec4(i / (float)count, 0.8, j / (float)count, 1), vec4(), 0, 5));
		}
	}
		
	while (!can.Cleaned) {
		can.mainloop();
	}
//	thread renderThread(renderLoop, &can);

	return 1;
}