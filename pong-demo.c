#include "window.h"
#include "drawing.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define WIDTH 600
#define HEIGHT 400

#define EVENT_BUFFER_SIZE 100
XEvent eventBuffer[sizeof(XEvent) * EVENT_BUFFER_SIZE];

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
    ball.xVelocity = 5;
    ball.yVelocity = 5;
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
        // read events and handle them
        int eventsRead = checkWindowEvents(eventBuffer, EVENT_BUFFER_SIZE);
        for (int i = 0; i < eventsRead; i++) {
            XEvent event = eventBuffer[i];
            if (event.type == ClosedWindow) {
                // the window has been closed, and memory freed. Do whatever cleanup you need and then exit.
                return 0;
            }
            if (event.type == KeyPress) {
                if (event.xkey.keycode == 25) {
                    player1.upPressed = 1;
                }
                if (event.xkey.keycode == 39) {
                    player1.downPressed = 1;
                }
                if (event.xkey.keycode == 111) {
                    player2.upPressed = 1;
                }
                if (event.xkey.keycode == 116) {
                    player2.downPressed = 1;
                }
            }
            if (event.type == KeyRelease) {
                printf("up for some reason\n");
                if (event.xkey.keycode == 25) {
                    player1.upPressed = 0;
                }
                if (event.xkey.keycode == 39) {
                    player1.downPressed = 0;
                }
                if (event.xkey.keycode == 111) {
                    player2.upPressed = 0;
                }
                if (event.xkey.keycode == 116) {
                    player2.downPressed = 0;
                }
            }
        }
        if (player1.upPressed) {
            player1.y -= 5;
        }
        if (player1.downPressed) {
            player1.y += 5;
        }
        if (player2.upPressed) {
            player2.y -= 5;
        }
        if (player2.downPressed) {
            player2.y += 5;
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
        
        rectangle(0, 0, WIDTH, HEIGHT, 0x00000000);
        rectangle(player1.x, player1.y, player1.width, player1.height, player1.color);
        rectangle(player2.x, player2.y, player2.width, player2.height, player2.color);
        circle(ball.x, ball.y, ball.radius, ball.color);
        updateWindow();
        usleep(16667);
    }
}