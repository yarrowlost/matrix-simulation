const int size = 16;

struct PV {
  PV() = default;
  PV(float x, float y)
    : x(x), y(y) {}
  float x;
  float y;
};

struct Movable {
  PV coord;
  PV speed;
};


struct Hero {  //: public Movable {
  int hp;
};


struct Enemy : public Movable {
  bool alive = false;
};

struct Bullet : public Movable {
  bool active = false;
};


Hero h;
Enemy enemy[6];
unsigned long enemySpawnTimer = millis();

void spawnEnemy() {
  for (int i = 0; i < 6; ++i) {
    auto& e = enemy[i];
    if (!e.alive) {
      e.alive = true;
      e.coord.x = 0;
      e.speed.x = 0.3 + random(10)/30.0;
      return;
    }
  }
}
void moveEnemy() {
  for (int i = 0; i < 6; ++i) {
    auto& e = enemy[i];
    if (e.alive) {
      e.coord.x += e.speed.x;
      if (e.coord.x > 15 || e.coord.x < 0) {
        e.speed.x *= -1;
      }
    }
  }
}
void drawEnemy() {
  for (int i = 0; i < 6; ++i) {
    auto& e = enemy[i];
    if (e.alive) {
      matrix.drawPixel(e.coord.x, i, matrix.Color(200, 0, 0));
    }
  }
}

void makeFrame(float angle) {
  //draw hero
  int hx = map(angle, -30, 30, 0, 15);
  if (hx < 0) {
    hx = 0;
  }
  if (hx > 15) {
    hx = 15;
  }
  matrix.drawPixel(hx, 14, matrix.Color(0, 200, 0));

  //spawn enemy
  if (millis() - enemySpawnTimer > 5000) {
    enemySpawnTimer = millis();
    spawnEnemy();
  }
  //move enemy
  moveEnemy();
  //draw enemy
  drawEnemy();
}