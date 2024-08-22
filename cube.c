// compile with:                       clang cube.c -o cube
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>    
#include <termios.h>   
#include <fcntl.h>     
#include <stdlib.h>    

#define M_PI_2 1.57079632679  // Define M_PI_2 as half of pi

float A = 0, B = 0, C = 0;
int width = 80, height = 40;
float ZBuffer[80 * 40];
char buffer[80 * 40];
int BackgroundASCII = ' ';
int DistanceFromCam = 200;
float horizontalOffset;
float K1 = 80; 
float incrementSpeed = 1;

// Function to calculate X, Y, Z coordinates considering rotation
float calculateX(float i, float j, float k) {
    return i * cos(B) * cos(C) + j * (sin(A) * sin(B) * cos(C) - cos(A) * sin(C)) + k * (cos(A) * sin(B) * cos(C) + sin(A) * sin(C) );

    // First row of the combined rotation matrix . Go to this link to see the rotation matrices: https://en.wikipedia.org/wiki/Rotation_matrix
}

float calculateY(float i, float j, float k) {
    return i * cos(B) * sin(C) + j * (sin(A) * sin(C) * sin(C) + cos(A) * cos(C)) + k * (cos(A) * sin(B) * sin(C) - sin(A) * cos(C));

    // Second row of the combined rotation matrix. 
}

float calculateZ(float i, float j, float k) {
    return i * -sin(B) + j * (sin(A) * cos(B)) + k * (cos(A) * cos(B));

    // Third row of the combined rotation matrix. 
}



// Function to handle cube surface rendering
void CalculateForSurface(float CubeX, float CubeY, float CubeZ, int ch) {
    float x = calculateX(CubeX, CubeY, CubeZ); // We put arguments in this to set the vaule of i, j and k. This shapes the rotation based on the coordinatess.
    float z = calculateZ(CubeX, CubeY, CubeZ);

    float y = calculateY(CubeX, CubeY, CubeZ);

    z += DistanceFromCam;

    float ooz = 1 / z;

    int xp = (int)(width / 2 + horizontalOffset + K1 * ooz * x);
    int yp = (int)(height / 2 + K1 * ooz * y);

    if (xp >= 0 && xp < width && yp >= 0 && yp < height) {
        int idx = xp + yp * width;
        if (ooz > ZBuffer[idx]) {
            ZBuffer[idx] = ooz;
            buffer[idx] = ch;
        }
    }

    // This checks if a letter should be in front of another one, if it is is, it sets it Zbuffer to ooz, and then draws the character based on the arguments in the main function.
}

// Non-blocking keyboard input for macOS
int kbhit(void) {
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }

    return 0;
}

// Handle keyboard inputs to control cube's position
void rotateCube(char key) {
    switch (key) {
        case 'w': // Move cube closer
            if (K1 < 200) {
                K1 += 3;

                // This makes the cube bigger when you move closer to it which gives the illusion that you are walking up to it when you press w.
            }
            break;
        case 's': // Move cube farther
            if (K1 > 0) {
                K1 -= 3;

                // // This makes the cube smaller when you move closer to it which gives the illusion that you are walking away from it when you press s.
            }
            break;
        case 'a': // Move cube to the left
            horizontalOffset += 3;
            break;
        case 'd': // Move cube to the right
            horizontalOffset -= 3;
            break;
    }
}

int main() {
    printf("\x1b[2J\x1b[H"); // Clear the screen and move cursor to home position

    horizontalOffset = (width / 2);

    while (1) {
        memset(buffer, BackgroundASCII, width * height);
        for (int i = 0; i < width * height; i++) {
            ZBuffer[i] = -INFINITY; // Initialize ZBuffer to a large negative value
        }

        float cubeSize = 30;

        for (float cubeX = -cubeSize; cubeX < cubeSize; cubeX += incrementSpeed) {
            for (float cubeY = -cubeSize; cubeY < cubeSize; cubeY += incrementSpeed) {
                CalculateForSurface(cubeX, cubeY, -cubeSize, '@'); // Creates surfaces for the cube by putting different arguments in.
                CalculateForSurface(cubeSize, cubeY, cubeX, '$');
                CalculateForSurface(-cubeSize, cubeY, -cubeX, '~');
                CalculateForSurface(-cubeX, cubeY, cubeSize, '#');
                CalculateForSurface(cubeX, -cubeSize, -cubeY, ';');
                CalculateForSurface(cubeX, cubeSize, cubeY, '+');
            }
        }

        printf("\x1b[H"); // Move cursor to the top-left corner
        for (int i = 0; i < height; i++) {
            fwrite(buffer + i * width, 1, width, stdout);
            putchar('\n'); 
            // This prints the characters in the terminal
        }

        usleep(50000); // Sleep for 50ms


        // These rotate the cube by changing the vaule for each axis(A, B, C)
        A += 0.1; // Rotation for X axis
        B += 0.1; // Rotation for Y axis
        C += 0.1; // Rotation for Z axis

        if (kbhit()) {
            char key = getchar();
            rotateCube(key);
        }
    }

    return 0;
}
