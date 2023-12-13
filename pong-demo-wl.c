#include "window-wl.h"
#include "drawing.h"
#include <stdio.h>

#define WIDTH 600
#define HEIGHT 400

typedef struct {
    int x;
    int y;
    int xVelocity;
    int yVelocity;
    int color;
    int radius;
} Ball;

typedef struct {
    int x;
    int y;
    int height;
    int width;
    char upPressed;
    char downPressed;
    int color;
} Player;

char collide(Ball ball, Player player) {
    return (ball.x > player.x) && (ball.x < player.width + player.x) && (ball.y > player.y) && (ball.y < player.height + player.y);
}

Ball ball;
Player player1;
Player player2;

void draw() {
    rectangle(0, 0, WIDTH, HEIGHT, 0x00000000);
    rectangle(player1.x, player1.y, player1.width, player1.height, player1.color);
    rectangle(player2.x, player2.y, player2.width, player2.height, player2.color);
    circle(ball.x, ball.y, ball.radius, ball.color);
}

void key_change(unsigned int key, unsigned int state) {
    if (state == 1) {
        if (key == 17) {
            player1.upPressed = 1;
        }
        if (key == 31) {
            player1.downPressed = 1;
        }
        if (key == 103) {
            player2.upPressed = 1;
        }
        if (key == 108) {
            player2.downPressed = 1;
        }
    }
    if (state == 0) {
        if (key == 17) {
            player1.upPressed = 0;
        }
        if (key == 31) {
            player1.downPressed = 0;
        }
        if (key == 103) {
            player2.upPressed = 0;
        }
        if (key == 108) {
            player2.downPressed = 0;
        }
    }
}

int main() {
    initWindow(WIDTH, HEIGHT, "Pong", draw, key_change);

    ball.x = 300;
    ball.y = 200;
    ball.xVelocity = 2;
    ball.yVelocity = 2;
    ball.color = 0x00ffffff;
    ball.radius = 5;
    
    player1.x = 10;
    player1.y = 175;
    player1.width = 10;
    player1.height = 50;
    player1.upPressed = 0;
    player1.downPressed = 0;
    player1.color = 0x00ffffff;

    player2.x = 580;
    player2.y = 175;
    player2.width = 10;
    player2.height = 50;
    player2.upPressed = 0;
    player2.downPressed = 0;
    player2.color = 0x00ffffff;

    while (dispatchEvents() != -1) {
        if (player1.upPressed) {
            player1.y -= 3;
        }
        if (player1.downPressed) {
            player1.y += 3;
        }
        if (player2.upPressed) {
            player2.y -= 3;
        }
        if (player2.downPressed) {
            player2.y += 3;
        }
        if ((ball.y > 395) || (ball.y < 5)) {
            ball.yVelocity = -ball.yVelocity;
        }
        if (ball.x < 5) {
            printf("Player 2 wins!\n");
            break;
        }
        if (ball.x > 595) {
            printf("Player 1 wins!\n");
            break;
        }
        if (collide(ball, player1) || collide(ball, player2)) {
            ball.xVelocity = -ball.xVelocity;
        }
        ball.x += ball.xVelocity;
        ball.y += ball.yVelocity;
    }
}

