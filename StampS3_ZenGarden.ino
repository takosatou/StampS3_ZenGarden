/*
  A Zen garden simulator inspired by Sisyphus
  using M5Stack Stamp S3 and a round LCD display designed by Uruka Technology.

  Copyright (c) 2023 Takashi Satou

  Released under the MIT License, except LFGX_ESP32_SPI_GC9A01.hpp
*/

#include "LFGX_ESP32_SPI_GC9A01.hpp"
#include <Arduino.h>
#include <stdlib.h>
#include <math.h>

#define PIN_BUTTON 0

bool pressed = false;

void button_init() {
  pinMode(PIN_BUTTON, INPUT);
}

bool button_clicked() {
  bool npressed = !digitalRead(PIN_BUTTON);
  if (pressed && !npressed) {
    pressed = false;
    return true;
  }
  pressed = npressed;
  return false;
}

const int W = 240; // max width
const int H = 240; // max height
const int R = W / 2;

LGFX_ESP32_SPI_GC9A01 lcd(W, H);
LGFX_Sprite ball(&lcd);

int colormap[256]; // colormap for display
bool go_outer = true;
double cx = 0, cy = 0;
double mod0, mod1, mod2;
int counter = 0;

const int WAIT = 3;
int wait = WAIT;

const int FORK_N = 1;
const int BALL_R = 4;
const int BALL_SZ = BALL_R * 2 + 1;

const unsigned int BALL_PAT[] = {
  0xf0, 0xf0, 0xf8, 0xff, 0xff,
  0xe0, 0xe0, 0xe8, 0xf0, 0xff,
  0xb8, 0xd0, 0xd8, 0xe8, 0xf8,
  0xa0, 0xb0, 0xe0, 0xe0, 0xf8,
  0xa0, 0xa0, 0xc8, 0xd8, 0xf8,
  0xa0, 0xb0, 0xe0, 0xe0, 0xf8,
  0xb8, 0xd0, 0xd8, 0xe8, 0xf8,
  0xe0, 0xe0, 0xe8, 0xf0, 0xff,
  0xf0, 0xf0, 0xf8, 0xff, 0xff,
};

void init_colormap() {
  for (int i = 0; i < 256; i++) {
    const int v = int(pow((i / 255.0), 2) * 255.0);
    colormap[i] = lcd.color565(v, v, v);
  }
}

void init_ball() {
  ball.createSprite(BALL_R + 1, BALL_SZ * FORK_N);
  ball.setPivot(0, BALL_R + (FORK_N / 2) * BALL_SZ);

  for (int n = 0; n < FORK_N; n++) {
    int i = 0;
    for (int y = -BALL_R; y <= BALL_R; y++) {
      for (int x = 0; x <= BALL_R; x++, i++) {
        ball.drawPixel(x, y + BALL_R + n * BALL_SZ, colormap[BALL_PAT[i]]);
      }
    }
  }
}

void init_param() {
  mod0 = (random() % 100 + 1) / 100.0;
  mod1 = (random() % 100) / 300.0;
  mod2 = (random() % 100) / 300.0;
  counter = 0;
  USBSerial.printf("%f %f %f\n", mod0, mod1, mod2);
}

void draw_garden() {
  double len = sqrt(cx * cx + cy * cy);
  double mod = sin(counter * mod0) *  mod1 + cos(counter * mod0) *  mod2;
  double dir0 = atan2(cy, cx) + PI / 2 + mod + (go_outer ? -0.01 : (0.01 + atan2(1, len)));
  double cs = cos(dir0);
  double sn = sin(dir0);

  cx += cs;
  cy += sn;

  ball.pushRotateZoom(cx + R, cy + R, dir0 * 180 / PI, 1.0, 1.0,
		      colormap[255]);
  counter++;

  const int M = BALL_SZ;
  if (go_outer && len > R - BALL_SZ ||
      !go_outer && len < BALL_R) {
    go_outer = !go_outer;
    init_param();
  }

  if (wait > 0) {
    delay(wait);
  }
}

void setup() {
  USBSerial.begin(9600);
  USBSerial.println("Zen Garden");
  button_init();
  lcd.init();
  init_colormap();
  init_ball();

  init_param();

  // draw initial screen
  lcd.setTextColor(TFT_WHITE);
  lcd.drawString("Zen Garden", 90, 80, 2);
  delay(1000);
  lcd.fillScreen(colormap[240]);
}

void loop() {
  draw_garden();

  if (button_clicked()) {
    wait = WAIT - wait; // toggle 0 and WAIT
  }
}
