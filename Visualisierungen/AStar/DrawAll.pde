void drawAll() {
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

	for(;;) {
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
			// noFill();
		}
	}

	// ivec2 p1 = new ivec2(13, 11);
	ivec2 p1 = start;

	for(;;) {
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