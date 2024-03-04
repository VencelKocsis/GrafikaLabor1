//=============================================================================================
// Mintaprogram: Zöld háromszög. Ervenyes 2019. osztol.
//
// A beadott program csak ebben a fajlban lehet, a fajl 1 byte-os ASCII karaktereket tartalmazhat, BOM kihuzando.
// Tilos:
// - mast "beincludolni", illetve mas konyvtarat hasznalni
// - faljmuveleteket vegezni a printf-et kiveve
// - Mashonnan atvett programresszleteket forrasmegjeloles nelkul felhasznalni es
// - felesleges programsorokat a beadott programban hagyni!!!!!!! 
// - felesleges kommenteket a beadott programba irni a forrasmegjelolest kommentjeit kiveve
// ---------------------------------------------------------------------------------------------
// A feladatot ANSI C++ nyelvu forditoprogrammal ellenorizzuk, a Visual Studio-hoz kepesti elteresekrol
// es a leggyakoribb hibakrol (pl. ideiglenes objektumot nem lehet referencia tipusnak ertekul adni)
// a hazibeado portal ad egy osszefoglalot.
// ---------------------------------------------------------------------------------------------
// A feladatmegoldasokban csak olyan OpenGL fuggvenyek hasznalhatok, amelyek az oran a feladatkiadasig elhangzottak 
// A keretben nem szereplo GLUT fuggvenyek tiltottak.
//
// NYILATKOZAT
// ---------------------------------------------------------------------------------------------
// Nev  : Kocsis Vencel
// Neptun : IGMACF
// ---------------------------------------------------------------------------------------------
// ezennel kijelentem, hogy a feladatot magam keszitettem, es ha barmilyen segitseget igenybe vettem vagy
// mas szellemi termeket felhasznaltam, akkor a forrast es az atvett reszt kommentekben egyertelmuen jeloltem.
// A forrasmegjeloles kotelme vonatkozik az eloadas foliakat es a targy oktatoi, illetve a
// grafhazi doktor tanacsait kiveve barmilyen csatornan (szoban, irasban, Interneten, stb.) erkezo minden egyeb
// informaciora (keplet, program, algoritmus, stb.). Kijelentem, hogy a forrasmegjelolessel atvett reszeket is ertem,
// azok helyessegere matematikai bizonyitast tudok adni. Tisztaban vagyok azzal, hogy az atvett reszek nem szamitanak
// a sajat kontribucioba, igy a feladat elfogadasarol a tobbi resz mennyisege es minosege alapjan szuletik dontes.
// Tudomasul veszem, hogy a forrasmegjeloles kotelmenek megsertese eseten a hazifeladatra adhato pontokat
// negativ elojellel szamoljak el es ezzel parhuzamosan eljaras is indul velem szemben.
//=============================================================================================
#include "framework.h"	

// vertex shader in GLSL: It is a Raw string (C++11) since it contains new line characters
const char* const vertexSource = R"(
	#version 330				// Shader 3.3
	precision highp float;		// normal floats, makes no difference on desktop computers

	uniform mat4 MVP;			// uniform variable, the Model-View-Projection transformation matrix
	layout(location = 0) in vec2 vp;	// Varying input: vp = vertex position is expected in attrib array 0

	void main() {
		gl_Position = vec4(vp.x, vp.y, 0, 1) * MVP;		// transform vp from modeling space to normalized device space
	}
)";

// fragment shader in GLSL
const char* const fragmentSource = R"(
	#version 330			// Shader 3.3
	precision highp float;	// normal floats, makes no difference on desktop computers
	
	uniform vec3 color;		// uniform variable, the color of the primitive
	out vec4 outColor;		// computed color of the current pixel

	void main() {
		outColor = vec4(color, 1);	// computed color is the color of the primitive
	}
)";

#define MIN(a,b) ((a) < (b) ? (a) : (b))

GPUProgram gpuProgram;//vertex and fragment shaders
bool isDrawingPoints = false;
bool isDrawingLine = false;
bool isMovingLine = false;

class PointCollection {
public:
	std::vector<vec3> points;
	unsigned int vaoPointCollection, vboPointCollection;
	unsigned int vaoLine, vboLine;
	std::vector<vec3> selectedPoints;

	float MVPtransf[4][4] = { 1, 0, 0, 0,    // MVP matrix, 
								  0, 1, 0, 0,    // row-major!
								  0, 0, 1, 0,
								  0, 0, 0, 1 };

public:
	PointCollection() {
		glGenVertexArrays(1, &vaoPointCollection);
		glBindVertexArray(vaoPointCollection);

		glGenBuffers(1, &vboPointCollection);
		glBindBuffer(GL_ARRAY_BUFFER, vboPointCollection);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
	}

	void addPoint(float cX, float cY) {
		vec3 wVertex = vec3(cX, cY, 0);
		points.push_back(wVertex);
	}

	void drawPoint() {
		int location = glGetUniformLocation(gpuProgram.getId(), "color");
		glUniform3f(location, 1.0f, 0.0f, 0.0f); // 3 floats

		location = glGetUniformLocation(gpuProgram.getId(), "MVP");	// Get the GPU location of uniform variable MVP
		glUniformMatrix4fv(location, 1, GL_TRUE, &MVPtransf[0][0]);	// Load a 4x4 row-major float matrix to the specified location

		if (points.size() > 0) {
			glBindVertexArray(vaoPointCollection);
			glBindBuffer(GL_ARRAY_BUFFER, vboPointCollection);
			glBufferData(GL_ARRAY_BUFFER, points.size() * 3 * sizeof(float), &points[0], GL_DYNAMIC_DRAW);
			if (location >= 0) glUniform3f(location, 1, 0, 0);
			glPointSize(10.0f);
			glDrawArrays(GL_POINTS, 0, points.size());
		}
	}

	bool pointExists(float cX, float cY) {
		for (int i = 0; i < points.size(); i++) {
			if (abs(points[i].x - cX) < 0.02 && abs(points[i].y - cY) < 0.02) {
				return true;
			}
		}
		return false;
	}

	void selectPoint(float cX, float cY) {
		if (pointExists(cX, cY)) {
			addSelectedPoint(cX, cY);
		}
		else {
			printf("\nThe Selected Point does not exists. Select a valid point");
		}
	}

	void addSelectedPoint(float cX, float cY) { selectedPoints.push_back(vec3(cX, cY, 0.0f)); }

	void clearSelectedPoints() { selectedPoints.clear(); }

	vec3 getSelectedPointsbyIndex(int i) {
		if (selectedPoints.size() != 0)
			return selectedPoints[i];
		else
			return vec3(-1, -1, -1);
	}

	int getSelectedPointsSize() { return selectedPoints.size(); }

	int getSize() { return points.size(); }
};

class Line {
private:
	vec3 p0, p1, color;
	float width;
	GLuint vaoLine, vboLine;
public:
	Line(const vec3& p0, const vec3& p1, const vec3& color = vec3(0.0f, 1.0f, 1.0f), float width = 3.0f) {
		this->p0 = p0;
		this->p1 = p1;
		this->color = color;
		this->width = width;

		vec3 direction = p1 - p0;

		this->p0 = p0 - direction * 600;
		this->p1 = p0 + direction * 600;

		glGenVertexArrays(1, &vaoLine);
		glBindVertexArray(vaoLine);

		glGenBuffers(1, &vboLine);
		glBindBuffer(GL_ARRAY_BUFFER, vboLine);

		std::vector<vec3> vertices = { this->p0, this->p1 };
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vec3), &vertices[0], GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(0);
	}

	void draw() {
		glLineWidth(width);

		glUniform3fv(glGetUniformLocation(gpuProgram.getId(), "color"), 1, &color.x);

		glBindVertexArray(vaoLine);

		glDrawArrays(GL_LINES, 0, 2);
	}

	const vec3 getP0() { return this->p0; }
	const vec3 getP1() { return this->p1; }
};

class LineCollection {
private: 
	std::vector<Line> lines;
public:
	LineCollection() {

	}

	void addLine(const vec3& p0, const vec3& p1, const vec3& color, float width) {
		lines.push_back(Line(p0, p1, color, width));
		printLineEquations(p0, p1);
	}

	void drawLines() {
		for (Line& line : lines) {
			line.draw();
		}
	}

	void printLineEquations(const vec3 p0, const vec3 p1) {
		float a = p1.y - p0.y;
		float b = p0.x - p1.x;
		float c = p0.x * p1.y - p1.x * p0.y;

		float dx = p1.x - p0.x;
		float dy = p1.y - p0.y;

		printf("\n\tImplicit: %3.2fx + %3.2fy + %3.2f = 0", a, b, c);
		printf("\n\tParametric: %3.2f + %3.2ft, y = %3.2f + %3.2ft\n", p0.x, dx, p0.y, dy);
	}

	bool isPointOnLine(vec3 point, vec3 p0, vec3 p1) {
		float d = length(cross(p1 - p0, p0 - point)) / length(p1 - p0);
		return d < 0.02;
	}

	bool isCursorOnLine(float cX, float cY) {
		vec3 cursor = vec3(cX, cY, 0);

		for (Line& line : lines) {
			if (isPointOnLine(cursor, line.getP0(), line.getP1())) {
				return true;
			}
		}
		return false;
	}
};

PointCollection* pointCollection;
LineCollection* lineCollection;

// Initialization, create an OpenGL context
void onInitialization() {
	glViewport(0, 0, windowWidth, windowHeight);
	
	pointCollection = new PointCollection();
	lineCollection = new LineCollection();
	
	// create program for the GPU
	gpuProgram.create(vertexSource, fragmentSource, "outColor");
}

// Window has become invalid: Redraw
void onDisplay() {
	glClearColor(0.25f, 0.25f, 0.25f, 1.0f);// background color
	glClear(GL_COLOR_BUFFER_BIT); // clear frame buffer

	pointCollection->drawPoint();
	lineCollection->drawLines();

	glutSwapBuffers(); // exchange buffers for double buffering
}

// Key of ASCII code pressed
void onKeyboard(unsigned char key, int pX, int pY) {
	if (key == 'p') {
		isDrawingPoints = true;
		isDrawingLine = false;
	}
	if (key == 'l') {
		isDrawingPoints = false;
		isDrawingLine = true;
	}
	if (key == 'm') {
		isDrawingPoints = false;
		isDrawingLine = false;
		isMovingLine = true;
	}
	if (key == 'i') {

	}
}

// Key of ASCII code released
void onKeyboardUp(unsigned char key, int pX, int pY) {
}

// Move mouse with key pressed
void onMouseMotion(int pX, int pY) {	// pX, pY are the pixel coordinates of the cursor in the coordinate system of the operation system
	// Convert to normalized device space
	float cX = 2.0f * pX / windowWidth - 1;	// flip y axis
	float cY = 1.0f - 2.0f * pY / windowHeight;
	printf("Mouse moved to (%3.2f, %3.2f)\n", cX, cY);
}

// Mouse click event
void onMouse(int button, int state, int pX, int pY) { // pX, pY are the pixel coordinates of the cursor in the coordinate system of the operation system
	// Convert to normalized device space
	float cX = 2.0f * pX / windowWidth - 1;	// flip y axis
	float cY = 1.0f - 2.0f * pY / windowHeight;

	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		if (isDrawingLine && pointCollection->getSelectedPointsSize() < 2) {
			pointCollection->selectPoint(cX, cY);
			printf("\nDefine Lines");

			if (pointCollection->getSelectedPointsSize() == 2) {
				printf("\nLine added");
				lineCollection->addLine(pointCollection->getSelectedPointsbyIndex(0), pointCollection->getSelectedPointsbyIndex(1), vec3(0.0f, 1.0f, 1.0f), 3.0f);
				glutPostRedisplay();
				pointCollection->clearSelectedPoints();
			}
		}
		else if (isDrawingPoints) {
			pointCollection->addPoint(cX, cY);
			glutPostRedisplay();
			printf("\nPoint (%3.2f, %3.2f) added", cX, cY);
		}
		else if (isMovingLine) {
			if (lineCollection->isCursorOnLine(cX, cY)) {
				printf("\nLine selected");
			}
			else {
				printf("\nNo Line selected");
			}
		}
	}
}

// Idle event indicating that some time elapsed: do animation here
void onIdle() {
	long time = glutGet(GLUT_ELAPSED_TIME); // elapsed time since the start of the program
}
