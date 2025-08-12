// Define the Part class //<>//
class Part {
  color c;
  PVector speed;
  PVector pos;
  int mass = 16;

  Part(float x_, float y_, PVector speed_) {
    pos = new PVector(x_, y_);
    speed = speed_;
    c = color(160);//color(random(255), random(255), random(255));
  }



  void wallBounce() {
    // check walls
    if (pos.x > gridSize - mass || pos.x < mass) {
      speed.x = -speed.x;
      if (pos.x > gridSize/2) {
        pos.x = gridSize - mass;
      } else {
        pos.x = mass;
      }
    }

    if (pos.y > gridSize - mass || pos.y < mass) {
      speed.y = -speed.y;
      if (pos.y > gridSize/2) {
        pos.y = gridSize - mass;
      } else {
        pos.y = mass;
      }
    }
  }
}



public  enum Type {
  Solid,
    Water,
    Air
}

class Cell {
  Type type;
  PVector vel = new PVector();
}

int pixs = 18;
Cell[][] cells = new Cell[pixs][pixs];
int iterNum = 100;
float angle;

int gridSize = 540;
float overRelax = 1.9;
float density = 1000;
float dt = 1/120;
int h = gridSize/pixs;
Part[] parts = new Part[128];

void solveIncompressibility() {
  float cp = density * h / dt;
  
  for (int iter = 0; iter < iterNum; iter ++) {
    for (int i = 1; i < pixs - 1; i++) {
      for (int j = 1; j < pixs - 1; j++) {
        
        Cell c = cells[i][j];
        
        if(c.type != Type.Water){
          continue;
        }
        
        Cell left = cells[i-1][j];
        Cell right = cells[i+1][j];
        Cell top = cells[i][j-1];
        Cell bottom = cells[i][j+1];
        
        int sx0 = left.type == Type.Water ? 1 : 0;
        int sx1 = right.type == Type.Water ? 1 : 0;
        int sy0 = top.type == Type.Water ? 1 : 0; 
        int sy1 = bottom.type == Type.Water ? 1 : 0;
        int s = sx0 + sx1 + sy0 + sy1;
        if (s == 0){
          continue;
        }
        
        float div = right.vel.x - c.vel.x + top.vel.y - c.vel.y;
        //to do calc density
        float p = -div * overRelax / s;
        
        c.vel.x -= sx0 * p;
        right.vel.x += sx1 * p;
        c.vel.y -= sy0 * p;
        top.vel.y += sy1 * p;
      }
    }
  }
}

void renderPart (Part[] parts) {
  background(50);
  noFill();
  translate(width/2, height/2);
  rotate(angle);
  rect(0, 0, gridSize, gridSize);

  translate(-gridSize/2, -gridSize/2);

  for (int i = 0; i < parts.length; i++) {

    Part p = parts[i];
    if (p != null) {

      int row = (int)p.pos.x/h;
      int col = (int)p.pos.y/h;
      fill(p.c);
      rect(row * h, col * h, h, h);
      circle(p.pos.x, p.pos.y, p.mass*2);
    }
  }
}





void procPart(Part[] parts) {
  for (int i = 0; i < parts.length; i++) {
    Part p = parts[i];
    if (p != null) {


      p.pos.x = p.pos.x + p.speed.x;

      p.pos.y = p.pos.y + p.speed.y;

      PVector gravity = new PVector(0, 1);

      gravity.rotate(-angle);

      p.speed.x += gravity.x;
      p.speed.y += gravity.y;

      p.speed.setMag(p.speed.mag()*0.98);
    }
  }


  for (int i = 0; i < parts.length; ++i) {
    if (parts[i] != null) {
      parts[i].wallBounce();
    }
  }
}

// Declare the 2D array
void printParts() {
  float E = 0;
  for (int i = 0; i < parts.length; i++) {
    if (parts[i] != null) {
      //println("Part " + i + " - X: " + parts[i].pos.x + ", Y: " + parts[i].pos.y);
      E += parts[i].pos.mag() * parts[i].pos.mag();
    } else {
      //println("Part " + i + " - NULL");
    }
  }
  println("E: " + E);
}

void setup() {
  size(1000, 1000);
  rectMode(CENTER);
  // Initialize the 2D array
  for (int i = 0; i < 128; i ++) {
    parts[i] = new Part(random(490), random(490), new PVector(0, 0));
  }
  for (int i = 0; i < pixs; ++i) {
    for (int j = 0; j < pixs; ++j) {
      cells[i][j] = new Cell();
    }
  }
  for (int i = 0; i < pixs; ++i) {
    cells[i][pixs-1].type = Type.Solid;
    cells[i][0].type = Type.Solid;
  }
  for (int i = 0; i < pixs; ++i) {
    cells[pixs-1][i].type = Type.Solid;
    cells[0][i].type = Type.Solid;
  }
}

void draw() {
  renderPart(parts);
  procPart(parts);
  printParts();
}
void mouseDragged()
{
  angle = map(mouseX, 0, width, 0, 2*PI);
}
