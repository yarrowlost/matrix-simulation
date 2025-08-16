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
  int hp = 3;
  int x;
};


struct Enemy : public Movable {
  bool alive = false;
};


struct Bullet : public Movable {
  bool active = false;
};
const int enemyBulletNum = 5;
const int heroBulletNum = 3;
Bullet hb[heroBulletNum];
Bullet eb[enemyBulletNum];
const int enemyNum = 6;
Hero h;
Enemy enemy[enemyNum];
unsigned long enemySpawnTimer = millis();
unsigned long enemyFireTimer = millis();

void spawnEnemy() {
  for (int i = 0; i < enemyNum; ++i) {
    auto& e = enemy[i];
    if (!e.alive) {
      e.alive = true;
      e.coord.x = 0;
      e.speed.x = 0.3 + random(10) / 30.0;
      return;
    }
  }
}

void moveEnemy() {
  for (int i = 0; i < enemyNum; ++i) {
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
  for (int i = 0; i < enemyNum; ++i) {
    auto& e = enemy[i];
    if (e.alive) {
      matrix.drawPixel(e.coord.x, i, matrix.Color(200, 0, 0));
    }
  }
}


void spawnBullet(Bullet* ba, int size, int x, int y, float speed);

void spawnBullet(Bullet* ba, int size, int x, int y, float speed) {

  for (int i = 0; i < size; ++i) {
    auto& b = ba[i];
    if (!b.active) {
      b.active = true;
      b.coord.x = x;
      b.coord.y = y;
      b.speed.x = 0;
      b.speed.y = speed;

      return;
    }
  }
}

void drawBullets(Bullet* ba, int size, uint16_t color);
void drawBullets(Bullet* ba, int size, uint16_t color) {
  for (int i = 0; i < size; ++i) {
    const auto& b = ba[i];
    if (b.active) {
      matrix.drawPixel(b.coord.x, b.coord.y, color);
    }
  }
}

void moveBullets(Bullet* ba, int size);
void moveBullets(Bullet* ba, int size) {
  for (int i = 0; i < size; ++i) {
    auto& b = ba[i];
    if (b.active) {
      b.coord.y += b.speed.y;
      if (b.coord.y > 15 || b.coord.y < 0) {
        b.active = false;
      }
    }
  }
}

int pressed = false;
void enemyAttacks() {
  for (int i = 0; i < enemyNum; ++i) {
    if (((int)enemy[i].coord.x) == h.x && enemy[i].alive) {
      spawnBullet(eb, enemyBulletNum, h.x, i + 1, 1);
    }
  }
}

bool collision(Bullet* ba, int size, int x, int y);
bool collision(Bullet* ba, int size, int x, int y) {
  for (int i = 0; i < size; ++i) {
    auto& b = ba[i];
    Serial.print("check ");
    Serial.print(i);
    Serial.print(" ");
    Serial.print(static_cast<int>(b.coord.y));
    Serial.print(" ");
    Serial.println(y);
    if (static_cast<int>(b.coord.x) == x && static_cast<int>(b.coord.y) == y && b.active) {
      b.active = false;
      return true;
    }
  }
  return false;
}

void killEnemy() {
  for (int i = 0; i < enemyNum; ++i) {
    auto& e = enemy[i];
    if (e.alive && collision(hb, heroBulletNum, e.coord.x, i)) {
      e.alive = false;
      Serial.println("Enemy killed " + i);
    }
  }
}

bool gameOver = false;

void makeFrame(float angle) {
  //draw hero
  int hx = map(angle, -25, 25, 0, 15);
  if (hx < 0) {
    hx = 0;
  }
  if (hx > 15) {
    hx = 15;
  }
  h.x = hx;


  //spawn enemy
  if (millis() - enemySpawnTimer > 5000) {
    enemySpawnTimer = millis();
    spawnEnemy();
  }
  //move enemy
  moveEnemy();


  if (millis() - enemyFireTimer > 200) {
    enemyFireTimer = millis();
    enemyAttacks();
  }


  //draw enemy
  drawEnemy();
  //fire
  if (!digitalRead(18) && pressed == false) {
    spawnBullet(hb, heroBulletNum, hx, 13, -1);
    pressed = true;
  } else if (digitalRead(18)) {
    pressed = false;
  }
  //draw bullets
  moveBullets(hb, heroBulletNum);
  moveBullets(eb, enemyBulletNum);

  if (collision(eb, enemyBulletNum, hx, 14)) {
    h.hp--;
    if (h.hp == 0) {
      gameOver = true;
    }
  }
  matrix.drawPixel(hx, 14, matrix.Color(0, 0, map(h.hp, 0, 3, 0, 250)));
  killEnemy();

  drawBullets(hb, heroBulletNum, matrix.Color(0xf5, 0x9b, 0) / 2);
  drawBullets(eb, enemyBulletNum, matrix.Color(0xf5, 0xf0, 0) / 2);

  if (gameOver == true) {
    for (int i = 0; i < 16; ++i) {
      for (int j = 0; j < 16; ++j) {
        matrix.drawPixel(j, i, matrix.Color(150, 0, 0));
      }
      delay(180);
      matrix.show();
    }
    while (digitalRead(18)){
      delay(180);
    }
    for(int i = 0; i < enemyBulletNum; ++i){
      eb[i].active = false;
    }
    for(int i = 0; i < heroBulletNum; ++i){
      hb[i].active = false;
    }
    for(int i = 0; i < enemyNum; ++i){
      enemy[i].alive = false;
    }
    h.hp = 3;
    gameOver = false;
  }
}
