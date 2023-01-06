#include "window.h"
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

int main() {
    initWindow(WIDTH, HEIGHT, "Pong");
    Ball ball;
    ball.x = 300;
    ball.y = 200;
    ball.xVelocity = 1;
    ball.yVelocity = 1;
    ball.color = 0x00ffffff;
    ball.radius = 5;
    Player player1;
    player1.x = 10;
    player1.y = 175;
    player1.width = 10;
    player1.height = 50;
    player1.upPressed = 0;
    player1.downPressed = 0;
    player1.color = 0x00ffffff;
    Player player2;
    player2.x = 580;
    player2.y = 175;
    player2.width = 10;
    player2.height = 50;
    player2.upPressed = 0;
    player2.downPressed = 0;
    player2.color = 0x00ffffff;
    while (1) {
        player1.y = ball.y - 25;
        player2.y = ball.y - 25;
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
        rectangle(0, 0, WIDTH, HEIGHT, 0x00000000);
        rectangle(player1.x, player1.y, player1.width, player1.height, player1.color);
        rectangle(player2.x, player2.y, player2.width, player2.height, player2.color);
        circle(ball.x, ball.y, ball.radius, ball.color);
        updateWindow();
    }
    return 0;
}