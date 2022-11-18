final static int WIDTH = 28;
final static int HEIGHT = 31;

class ivec2 {
	ivec2(int x, int y) {
		this.x = x;
		this.y = y;
	}
	int x, y;
}

final static String[] map = new String[] {
	"WWWWWWWWWWWWWWWWWWWWWWWWWWWW",
	"W............WW............W",
	"W.WWWW.WWWWW.WW.WWWWW.WWWW.W",
	"WoWWWW.WWWWW.WW.WWWWW.WWWWoW",
	"W.WWWW.WWWWW.WW.WWWWW.WWWW.W",
	"W..........................W",
	"W.WWWW.WW.WWWWWWWW.WW.WWWW.W",
	"W.WWWW.WW.WWWWWWWW.WW.WWWW.W",
	"W......WW....WW....WW......W",
	"WWWWWW.WWWWW WW WWWWW.WWWWWW",
	"WWWWWW.WWWWW WW WWWWW.WWWWWW",
	"WWWWWW.WW          WW.WWWWWW",
	"WWWWWW.WW WWW  WWW WW.WWWWWW",
	"WWWWWW.WW W      W WW.WWWWWW",
	"      .   W      W   .      ",
	"WWWWWW.WW W      W WW.WWWWWW",
	"WWWWWW.WW WWWWWWWW WW.WWWWWW",
	"WWWWWW.WW          WW.WWWWWW",
	"WWWWWW.WW WWWWWWWW WW.WWWWWW",
	"WWWWWW.WW WWWWWWWW WW.WWWWWW",
	"W............WW............W",
	"W.WWWW.WWWWW.WW.WWWWW.WWWW.W",
	"W.WWWW.WWWWW.WW.WWWWW.WWWW.W",
	"Wo..WW................WW..oW",
	"WWW.WW.WW.WWWWWWWW.WW.WW.WWW",
	"WWW.WW.WW.WWWWWWWW.WW.WW.WWW",
	"W......WW....WW....WW......W",
	"W.WWWWWWWWWW.WW.WWWWWWWWWW.W",
	"W.WWWWWWWWWW.WW.WWWWWWWWWW.W",
	"W..........................W",
	"WWWWWWWWWWWWWWWWWWWWWWWWWWWW"};

boolean[][] walls;

class Target {
	float x, y;
	boolean dragged;

	Target(float x, float y) {
		this.x = x;
		this.y = y;
		this.dragged = false;
	}
}

final int TARGET_RADIUS = 30;
boolean targetDragged = false;
Target[] targets = new Target[]{ new Target(13, 11), new Target(13, 13) }; // von, zu
ivec2 start = new ivec2(13, 11);
ivec2 stop = new ivec2(13, 13);


boolean instantUpdate = true;
int stepDelay = 5;
boolean pause = false;

// live:
int numSteps;

void setup() {
	size(600, 800);
	frameRate(60);
	surface.setResizable(true);

	walls = new boolean[HEIGHT][WIDTH];
	for(int y = 0; y < HEIGHT; y++)
		for(int x = 0; x < WIDTH; x++)
			walls[y][x] = map[y].charAt(x) == 'W';
}

void mousePressed() {
	for(int i = 0; i < 2; i++) {
		if(targetDragged==false && dist(mouseX, mouseY, (targets[i].x+.5)*width/WIDTH, (targets[i].y+.5)*height/HEIGHT) <= TARGET_RADIUS) {
			targetDragged = true;
			targets[i].dragged = true;
		}
	}
}

void mouseReleased() {
	targetDragged = false;
	targets[0].dragged = false;
	targets[1].dragged = false;
	start.x = constrain(round(targets[0].x), 0, WIDTH-1);
	start.y = constrain(round(targets[0].y), 0, HEIGHT-1);
	stop.x = round(targets[1].x);
	stop.y = round(targets[1].y);
	targets[0].x = (int)start.x;
	targets[0].y = (int)start.y;
	targets[1].x = (int)stop.x;
	targets[1].y = (int)stop.y;
	numSteps = 0;
}

void mouseDragged() {
	final float cw = width * 1. / WIDTH;
	final float ch = height * 1. / HEIGHT;

	for(int i = 0; i < 2; i++) {
		if(targets[i].dragged) {
			targets[i].x += (mouseX - pmouseX) / cw;
			targets[i].y += (mouseY - pmouseY) / ch;
		}
	}

	// live update:
	if(instantUpdate) {
		start.x = constrain(round(targets[0].x), 0, WIDTH-1);
		start.y = constrain(round(targets[0].y), 0, HEIGHT-1);
		stop.x = round(targets[1].x);
		stop.y = round(targets[1].y);
		resetDists();
	}
}

void keyPressed() {
	switch (key) {
		case 'i':
			instantUpdate ^= true;
			break;
		
		case '+':
			stepDelay++;
			break;

		case '-':
			if(stepDelay>1)
				stepDelay--;
			break;

		case 'p':
			pause = !pause;
			break;

		case 'a':
			numSteps--;
			break;

		case 'd':
			numSteps++;
			break;

		case 'r':
			targets = new Target[]{ new Target(13, 11), new Target(13, 13) }; // von, zu
			break;
	}
}


int[][] distMap;

void drawToStep(int step) {
	final float cw = width * 1. / WIDTH;
	final float ch = height * 1. / HEIGHT;

	ArrayList<ivec2> lastSet = calcStartDists();

	for(ivec2 p : lastSet) {
		noStroke();
		fill(0, 255, 0);
		rect(p.x*cw, p.y*ch, cw, ch);
		fill(0);
		text(distMap[p.y][p.x], p.x*cw + cw/2, p.y*ch + ch/2);
		noFill();
	}

	for(; step>0; step--) {
		ArrayList<ivec2> newLastSet = new ArrayList<ivec2>();

		for(ivec2 p : lastSet) {
			final int currentDist = distMap[p.y][p.x] + 1;

			if(p.x-1 >= 0 && walls[p.y][p.x-1] == false && distMap[p.y][p.x-1] == -1) { // Keine Wand links
				distMap[p.y][p.x-1] = currentDist;
				newLastSet.add(new ivec2(p.x-1, p.y));
			}
			if(p.x+1 <= WIDTH-1 && walls[p.y][p.x+1] == false && distMap[p.y][p.x+1] == -1) { // Keine Wand rechts
				distMap[p.y][p.x+1] = currentDist;
				newLastSet.add(new ivec2(p.x+1, p.y));
			}
			if(p.y-1 >= 0 && walls[p.y-1][p.x] == false && distMap[p.y-1][p.x] == -1) { // Keine Wand oben
				distMap[p.y-1][p.x] = currentDist;
				newLastSet.add(new ivec2(p.x, p.y-1));
			}
			if(p.y+1 <= HEIGHT-1 && walls[p.y+1][p.x] == false && distMap[p.y+1][p.x] == -1) { // Keine Wand unten
				distMap[p.y+1][p.x] = currentDist;
				newLastSet.add(new ivec2(p.x, p.y+1));
			}
		}

		if(newLastSet.size() == 0)
			break;

		lastSet = newLastSet;

		for(ivec2 p : lastSet) {
			colorMode(HSB);
			noStroke();
			fill((distMap[p.y][p.x]*5) % 256, 175, 175);
			rect(p.x*cw, p.y*ch, cw, ch);
			colorMode(RGB);

			fill(0);
			text(distMap[p.y][p.x], p.x*cw + cw/2, p.y*ch + ch/2);
		}
	}

	// ivec2 p1 = new ivec2(13, 11);
	ivec2 p1 = start;

	for(; step>0; step--) {
		ivec2 p2 = null;
		int minDist = distMap[p1.y][p1.x];

		if(p1.x-1 >= 0 && walls[p1.y][p1.x-1]==false && distMap[p1.y][p1.x-1] < minDist) {
			minDist = distMap[p1.y][p1.x-1];
			p2 = new ivec2(p1.x-1, p1.y);
		}
		if(p1.x+1 <= WIDTH-1 && walls[p1.y][p1.x+1]==false && distMap[p1.y][p1.x+1] < minDist) {
			minDist = distMap[p1.y][p1.x+1];
			p2 = new ivec2(p1.x+1, p1.y);
		}

		if(p1.y-1 >= 0 && walls[p1.y-1][p1.x]==false && distMap[p1.y-1][p1.x] < minDist) {
			minDist = distMap[p1.y-1][p1.x];
			p2 = new ivec2(p1.x, p1.y-1);
		}
		if(p1.y+1 <= HEIGHT-1 && walls[p1.y+1][p1.x]==false && distMap[p1.y+1][p1.x] < minDist) {
			minDist = distMap[p1.y+1][p1.x];
			p2 = new ivec2(p1.x, p1.y+1);
		}

		if(p2 == null)
			break;

		strokeWeight(3);
		stroke(255);
		line(p1.x * cw + cw/2, p1.y*ch + ch/2, p2.x*cw + cw/2, p2.y*ch + ch/2);

		p1 = p2;
	}
}

void draw() {
	background(50);

	rectMode(CORNER);
	textSize(12);
	textAlign(CENTER, CENTER);

	resetDists();

	drawWalls();

	if(instantUpdate) {
		drawAll();
	} else {
		drawToStep(numSteps);
		if(frameCount % stepDelay == 0 && !pause)
			numSteps++;
	}

	drawTargets();

	textSize(17);
	textAlign(LEFT, TOP);
	// fill(0, 255, 0);
	fill(255);
	text("Instant Mode[i]: " + instantUpdate, 10, 10);
	text("Step Delay[+/-]: " + stepDelay, 10, 30);
	text("Pause: " + pause, 10, 50);
}




ArrayList<ivec2> calcStartDists() {
	ArrayList<ivec2> lastSet = new ArrayList<ivec2>(); // zuletzt ausgefuellte Felder

	// final int toX = mouseX * WIDTH / width;
	// final int toY = mouseY * HEIGHT / height;
	final int toX = stop.x;
	final int toY = stop.y;

	// Zellgroesse fuer visualisierung
	final float cw = width * 1. / WIDTH;
	final float ch = height * 1. / HEIGHT;

	if(toY >= 0 && toY < HEIGHT && toX >= 0 && toX < WIDTH && walls[toY][toX] == false) {
		distMap[toY][toX] = 0;
		lastSet.add(new ivec2(toX, toY));
	} else { // falls Zielposition in einer Wand oder ausserhalb des Spielfeldes liegt:
		for(int currentDist = 0; lastSet.size() == 0; currentDist++) {
			for(int xRel = -currentDist; xRel <= currentDist; xRel++) {
				for(int yFac = -1; yFac <= 1; yFac += 2) {
					final int x = toX + xRel;
					final int y = toY + yFac * (currentDist - abs(xRel));

					if(x < 0 || x > WIDTH-1 || y < 0 || y > HEIGHT-1) // Punkte ausserhalb des spielfeldes ueberspringen
						continue;

					if(walls[y][x]) { // Waende ueberspringen
            			colorMode(HSB);
						noFill();
						strokeWeight(1);
						stroke(currentDist*20 % 256, 255, 255);
						rect(x*cw, y*ch, cw, ch);
            			colorMode(RGB);

						fill(200);
						text(currentDist, x*cw + cw/2, y*ch + ch/2);
						continue;
					}


					distMap[y][x] = currentDist;
					lastSet.add(new ivec2(x, y));
				}
			}
		}
	}

	return lastSet;
}











void drawWalls() {
	final float cw = width * 1. / WIDTH;
	final float ch = height * 1. / HEIGHT;

	for(int yi = 0; yi < HEIGHT; yi++) {
		for(int xi = 0; xi < WIDTH; xi++) {
			final int x = xi * width / WIDTH;
			final int y = yi * height / HEIGHT;

			if(walls[yi][xi]) {
				noStroke();
				fill(50, 50, 255);
				rect(x, y, cw+1, ch+1);
			}
		}
	}
}

void drawTargets() {
	final float cw = width * 1. / WIDTH;
	final float ch = height * 1. / HEIGHT;

	noFill();
	strokeWeight(3);
	stroke(255, 255, 0);
	for(int i = 0; i < 2; i++)
		ellipse(round(targets[i].x)*cw + cw/2, round(targets[i].y)*ch + ch/2, TARGET_RADIUS*2, TARGET_RADIUS*2);
}

void resetDists() {
	distMap = new int[HEIGHT][WIDTH];
	for(int i = 0; i < WIDTH * HEIGHT; i++)
		distMap[i/WIDTH][i%WIDTH] = -1;
}