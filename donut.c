#include <conio.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

#define DELTA_A -0.005
#define DELTA_B 0.09

#define DELTA_THETA 0.08
#define DELTA_PHI 0.03
// Radius of the torus circle
#define R 11
// Radius of the "tube"
#define r 3
#define DISTANCE 17.0
#define FOCAL_LENGTH 1.3

// View range
#define MIN_X -5.0
#define MAX_X -MIN_X
#define MIN_Y -2.0
#define MAX_Y -MIN_Y

#define WIDTH 40
#define HEIGHT 30

#define BRIGHTNESS_CHARS ".,:;!?$@"
#define CHAR_COUNT 8

#define TERMINAL_FONT_RATIO 2.2

void renderFrame(double A, double B);

int main(void) {
   double A = 0;
   double B = 0;

   printf("\x1b[2J");
   while(1) {
      A += DELTA_A;
      B += DELTA_B;
      renderFrame(A, B);
      if(A > 2 * M_PI) A = 0.0;
      if(B > 2 * M_PI) B = 0.0;
      usleep(30000);
   }

   return 0;
}

void renderFrame(double A, double B) {
   double depthBuffer[WIDTH][HEIGHT];
   memset(depthBuffer, 0, WIDTH * HEIGHT * sizeof(double));
   char renderBuffer[WIDTH][HEIGHT];
   memset(renderBuffer, ' ', WIDTH * HEIGHT);
   
   double cosA = cos(A);
   double sinA = sin(A);
   double cosB = cos(B);
   double sinB = sin(B);

   for(double theta = 0.0; theta < 2 * M_PI; theta += DELTA_THETA) {
      double RPlusrCosTheta = R + r * cos(theta);
      double rSinTheta = r * sin(theta);
      for(double phi = 0.0; phi < 2 * M_PI; phi += DELTA_PHI) {
         double cosPhi = cos(phi);
         double sinPhi = sin(phi);

         // To make a torus:
         // 1. Make a circle of radius r which is R units away from the origin
         //    -> x = r * cos(theta) + R, y = r * sin(theta)
         // 2. Rotate that circle phi radians around the y (up) axis using a rotation matrix
         double xTorus = cosPhi * RPlusrCosTheta;
         double yTorus = rSinTheta;
         double zTorus = sinPhi * RPlusrCosTheta;

         // Rotate by A radians around the x axis
         // Rotate by B radians around the z axis
         double xRotated = xTorus * cosB - yTorus * sinB;
         double yRotated = xTorus * cosA * sinB + yTorus * cosA * cosB - zTorus * sinA;
         double zRotated = xTorus * sinA * sinB + yTorus  * sinA * cosB + zTorus * cosA;
      
         // Make sure the torus is visible
         zRotated += DISTANCE;

         // Perspective projection
         if (zRotated == 0.0) continue;
         double xProjected = (xRotated * FOCAL_LENGTH) / zRotated;
         double yProjected = (yRotated * FOCAL_LENGTH) / zRotated;
         double ooz = 1.0 / zRotated;

         // Characters are generally 2.2 times as tall as they are wide.
         // This preserves a 1:1 ratio
         xProjected *= TERMINAL_FONT_RATIO;

         if (xProjected < MIN_X || xProjected > MAX_X || yProjected < MIN_Y || yProjected > MAX_Y) continue;
         int xIndex = WIDTH * ((xProjected - MIN_X) / (MAX_X - MIN_X));
         int yIndex = HEIGHT * ((yProjected - MIN_Y) / (MAX_Y - MIN_Y));

         // Only display the closest point
         if(ooz > depthBuffer[xIndex][yIndex]) {
            depthBuffer[xIndex][yIndex] = ooz;

            double magnitude = sqrt(xRotated * xRotated + yRotated * yRotated + zRotated * zRotated);
            double brightness = 0.57735 * (xRotated + yRotated + zRotated) / magnitude;
            int brightnessIndex = brightness > 0 ? CHAR_COUNT * brightness : 0;

            renderBuffer[xIndex][yIndex] = BRIGHTNESS_CHARS[brightnessIndex];
         }
      }
   }

   printf("\x1b[H");
   for(unsigned char y = 0; y < HEIGHT; ++y) {
      for(unsigned char x = 0; x < WIDTH; ++x) {
         putchar(renderBuffer[x][y]);
      }
      putchar('\n');
   }
}