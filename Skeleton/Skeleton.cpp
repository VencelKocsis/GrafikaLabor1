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

const char* const vertexSource = R"(
	#version 330				// Shader 3.3
	precision highp float;		// normal floats, makes no difference on desktop computers

	uniform mat4 MVP;			// uniform variable, the Model-View-Projection transformation matrix
	layout(location = 0) in vec2 vp;	// Varying input: vp = vertex position is expected in attrib array 0

	void main() {
		gl_Position = vec4(vp.x, vp.y, 0, 1) * MVP;		// transform vp from modeling space to normalized device space
	}
)";

const char* const fragmentSource = R"(
	#version 330			// Shader 3.3
	precision highp float;	// normal floats, makes no difference on desktop computers
	
	uniform vec3 color;		// uniform variable, the color of the primitive
	out vec4 outColor;		// computed color of the current pixel

	void main() {
		outColor = vec4(color, 1);	// computed color is the color of the primitive
	}
)";

GPUProgram gpuProgram;
bool isDrawingPoints = false;
bool isDrawingLine = false;
bool isMovingLine = false;
bool isIntersect = false;

// class Object: OpenGL diasor copy telibe: nem biztos
class PointCollection {
public:
	std::vector<vec3> points;
	unsigned int vaoPointCollection, vboPointCollection;
	unsigned int vaoLine, vboLine;
	std::vector<vec3> selectedPoints;

	float MVPtransf[4][4] = { 1, 0, 0, 0,    
								  0, 1, 0, 0,    
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
			float deltaX = points[i].x - cX;
			float deltaY = points[i].y - cY;
			if (deltaX < 0.0f) {
				deltaX = (-1.0f) * deltaX;
			}
			if (deltaY < 0.0f) {
				deltaY = (-1.0f) * deltaY;
			}
			if (deltaX < 0.02 && deltaY < 0.02) {
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
	vec2 wTranslate;
public:
	Line(const vec3& p0, const vec3& p1, const vec3& color = vec3(0.0f, 1.0f, 1.0f), float width = 3.0f) {
		this->p0 = p0;
		this->p1 = p1;
		this->color = color;
		this->width = width;

		//vec3 direction = p1 - p0;

		/* ezek helyett az egyenes implicit egyenletéből ki kell számolni külön az x, y értékeit
		* be kell helyettesíteni x és y helyére pl az 1-et és kiszámolni az x, y értékeit külön
		* valamelyiknek (x, y) értelmes (-1, 1) között kell lennie
		* mert így a kapott pontban fogja metszeni az egyenes a 600x600 négyzet szélét
		* és megadjuk a négyzet szélén lévő egyenes koordinátáit
		*/ 

		std::vector<vec3> vertices = computeIntersectionPointsWithSquare(p0, p1);
		this->p0 = vertices[0];
		this->p1 = vertices[1];

		glGenVertexArrays(1, &vaoLine);
		glBindVertexArray(vaoLine);

		glGenBuffers(1, &vboLine);
		glBindBuffer(GL_ARRAY_BUFFER, vboLine);

		//std::vector<vec3> vertices = { this->p0, this->p1 };
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

	const vec3 getP0() const {
		return this->p0;
	}
	const vec3 getP1() const {
		return this->p1;
	}

	const vec3 getNDCP0() const {
		float cX = -1 * (2.0f * this->p0.x / windowHeight - 1);
		float cY = 1.0f - 2.0f * this->p0.y / windowHeight;
		return vec3(cX, cY, 0.0f);
	}

	const vec3 getNDCP1() const {
		float cX = -1 * (2.0f * this->p1.x / windowWidth - 1);
		float cY = 1.0f - 2.0f * this->p1.y / windowHeight;
		return vec3(cX, cY, 0.0f);
	}

	const vec3 getDirection() const { return this->p1 - this->p0; }
	void updateEndPoints(const vec3& newP0, const vec3& newP1) {
		this->p0 = newP0;
		this->p1 = newP1;
	}

	void AddTranslation(vec2 wT) { wTranslate = wTranslate + wT; }
	void updateVertices() {
		vec3 newP0 = this->p0 + vec3(wTranslate.x, wTranslate.y, 0.0f);
		vec3 newP1 = this->p1 + vec3(wTranslate.x, wTranslate.y, 0.0f);

		//std::vector<vec3> vertices = { newP0, newP1 };
		std::vector<vec3> vertices = computeIntersectionPointsWithSquare(newP0, newP1);
		glBindBuffer(GL_ARRAY_BUFFER, vboLine);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vec3), &vertices[0], GL_DYNAMIC_DRAW);
	}

	std::vector<vec3> computeIntersectionPointsWithSquare(const vec3& p0, const vec3& p1) const {
		float minX = -1.0f, maxX = 1.0f, minY = -1.0f, maxY = 1.0f;

		float dx = p1.x - p0.x;
		float dy = p1.y - p0.y;

		std::vector<vec3> points;

		// Az egyenes paraméteres egyenlete: x = p0.x + t*dx, y = p0.y + t*dy
		// Ahol t egy paraméter, ami a [0, 1] intervallumba esik, és ahol az egyenes mindkét végpontjának x és y koordinátája adott.

		// A négyzet bal és jobb oldalának ellenőrzése
		for (float x : {minX, maxX}) {
			float t = (x - p0.x) / dx;
			float y = p0.y + t * dy;
			points.push_back(vec3(x, y, 0.0f));
		}

		// Ha az egyenes nem metszi a négyzetet, akkor ellenőrizzük a négyzet felső és alsó oldalát is
		if (points.size() < 2) {
			points.clear();
			for (float y : {minY, maxY}) {
				float t = (y - p0.y) / dy;
				float x = p0.x + t * dx;
				points.push_back(vec3(x, y, 0.0f));
			}
		}

		return points;
	}
};

class LineCollection {
private: 
	std::vector<Line> lines;
	std::vector<Line*> selectedLines;
public:
	LineCollection() {}

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

	Line* isCursorOnLine(float cX, float cY) {
		vec3 cursor = vec3(cX, cY, 0);

		for (Line& line : lines) {
			if (isPointOnLine(cursor, line.getP0(), line.getP1())) {
				return &line;
			}
		}
		return nullptr;
	}

	void selectLine(float cX, float cY) {
		selectedLines.push_back(isCursorOnLine(cX, cY));
		for (Line& line : lines) {
			printLineEquations(line.getNDCP0(), line.getNDCP1());
		}
	}

	Line& getSelectedLine(int i) { return lines[i]; }

	int getSelectedLinesSize() { return selectedLines.size(); }

	vec3 getIntersectionPoint(const Line& line1, const Line& line2) {
		
		vec3 p1 = line1.getP0();
		vec3 d1 = normalize(line1.getDirection());
		vec3 p2 = line2.getP0();
		vec3 d2 = normalize(line2.getDirection());

		float t = ((p2.x - p1.x) * d2.y - (p2.y - p1.y) * d2.x) / ((d1.x * d2.y) - (d1.y * d2.x));
		float iX = p1.x + d1.x * t;
		float iY = p1.y + d1.y * t;

		vec3 intersection = vec3(iX, iY, 0.0f);

		printf("\nt: %3.2f, iX: %3.2f, iY: %3.2f", t, iX, iY);

		return intersection;
	}

	void clearSelectedLines() { selectedLines.clear(); }
};

PointCollection* pointCollection;
LineCollection* lineCollection;
Line* selectedLine;

void onInitialization() {
	glViewport(0, 0, windowWidth, windowHeight);
	
	pointCollection = new PointCollection();
	lineCollection = new LineCollection();
	
	// create program for the GPU
	gpuProgram.create(vertexSource, fragmentSource, "outColor");
}

void onDisplay() {
	glClearColor(0.25f, 0.25f, 0.25f, 1.0f);// background color
	glClear(GL_COLOR_BUFFER_BIT); // clear frame buffer

	lineCollection->drawLines();
	pointCollection->drawPoint();	

	glutSwapBuffers(); // exchange buffers for double buffering
}

void onKeyboard(unsigned char key, int pX, int pY) {
	if (key == 'p') {
		isDrawingPoints = true;
		isDrawingLine = false;
		isMovingLine = false;
		isIntersect = false;
	}
	else if (key == 'l') {
		isDrawingPoints = false;
		isDrawingLine = true;
		isMovingLine = false;
		isIntersect = false;
	}
	else if (key == 'm') {
		isDrawingPoints = false;
		isDrawingLine = false;
		isMovingLine = true;
		isIntersect = false;
	}
	else if (key == 'i') {
		isDrawingPoints = false;
		isDrawingLine = false;
		isMovingLine = false;
		isIntersect = true;
	}
}

void onKeyboardUp(unsigned char key, int pX, int pY) {}

void onMouseMotion(int pX, int pY) {	
	float cX = 2.0f * pX / windowWidth - 1;
	float cY = 1.0f - 2.0f * pY / windowHeight;
	printf("\nMouse moved to (%3.2f, %3.2f)\n", cX, cY);
	
	static float prevX = 0.0f;
	static float prevY = 0.0f;

	if (!isMovingLine || prevX == 0 || prevY == 0) {
		prevX = cX;
		prevY = cY;
	}

	if (isMovingLine && selectedLine) {
		float deltaX = cX - prevX;
		float deltaY = cY - prevY;
		
		prevX = cX;
		prevY = cY;

		selectedLine->AddTranslation(vec2(deltaX, deltaY));
		selectedLine->updateVertices();

		glutPostRedisplay();
	}
}

void onMouse(int button, int state, int pX, int pY) {
	float cX = 2.0f * pX / windowWidth - 1;
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
				selectedLine = lineCollection->isCursorOnLine(cX, cY);
				printf("\nLine selected");
			}
			else {
				printf("\nNo Line selected");
			}
		}
		else if (isIntersect && lineCollection->getSelectedLinesSize() < 2) {
			printf("\nIntersect");
			lineCollection->selectLine(cX, cY);
			printf("\n lines selected number: %d", lineCollection->getSelectedLinesSize());
			if (lineCollection->getSelectedLinesSize() == 2) {
				printf("\nlines have intersection");
				printf("\ngetSelectedLine1: ");
				lineCollection->printLineEquations(lineCollection->getSelectedLine(0).getNDCP0(), lineCollection->getSelectedLine(0).getNDCP1());
				printf("\ngetSelectedLine2: ");
				lineCollection->printLineEquations(lineCollection->getSelectedLine(1).getNDCP0(), lineCollection->getSelectedLine(1).getNDCP1());
				vec3 intersection = lineCollection->getIntersectionPoint(lineCollection->getSelectedLine(0), lineCollection->getSelectedLine(1));
				pointCollection->addPoint(intersection.x, intersection.y);
				glutPostRedisplay();
				lineCollection->clearSelectedLines();
			}
		}
	} else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
		isMovingLine = false;	
	}
}

void onIdle() {
	long time = glutGet(GLUT_ELAPSED_TIME);
}
