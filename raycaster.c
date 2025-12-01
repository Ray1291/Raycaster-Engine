#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glut.h>

//Compile with gcc raycaster.c -o a.out -lglut -lGL -lGLU -lm

#define GAME_WINDOW_HEIGHT 512
#define GAME_WINDOW_WIDTH 1024

#define PI 3.1415926535
#define P2 PI/2
#define P3 3*PI/2
#define DR 0.0174533

#define ANGLEOFFSET 0.001

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

float player_x_position,player_y_position, player_x_delta, player_y_delta, player_angle; //player pos
float player_x_velocity, player_y_velocity;

int mapX = 8, mapY = 8, map_size = 64;
int map[] = {
    1, 1, 1, 1, 1, 1, 1, 1,  
    1, 0, 0, 0, 0, 0, 0, 1, 
    1, 0, 0, 0, 1, 1, 0, 1, 
    1, 0, 0, 0, 0, 1, 0, 1,  
    1, 0, 0, 1, 0, 1, 0, 1,  
    1, 0, 0, 0, 0, 0, 0, 1,  
    1, 0, 1, 1, 1, 1, 0, 1,  
    1, 1, 1, 1, 1, 1, 1, 1   
};

typedef struct 
{
    int w, a, s, d; //on or off
}ButtonKeys; ButtonKeys Keys;


float degToRad(float a) {return a*M_PI/180.0;}

float FixAngle(float a) 
{ 
    if(a > 359) {
         a-=360;
    } 
    if (a < 0) {
        a+=360;
    } 
    return a;
}

void drawPlayer()
{
    glColor3f(1,1,0);
    glPointSize(8);
    glBegin(GL_POINTS);
    glVertex2i(player_x_position,player_y_position);
    glEnd();

    glLineWidth(3);
    glBegin(GL_LINES);
    glVertex2i(player_x_position, player_y_position);
    glVertex2i(player_x_position + player_x_delta * 5, player_y_position + player_y_delta * 5);
    glEnd();
}

void drawMap2d()
{
    int x, y, xo, yo;

    for(y = 0; y < mapY; y++){
        for(x = 0; x < mapX; x++){
            if(map[y*mapX+x] == 1) {glColor3f(1,1,1);} else {glColor3f(0,0,0);}
            xo = x*map_size; yo = y*map_size;
            glBegin(GL_QUADS);
            glVertex2i(xo            +1, yo            +1);
            glVertex2i(xo            +1, yo + map_size -1);
            glVertex2i(xo + map_size -1, yo+map_size   -1);
            glVertex2i(xo+map_size   -1, yo            +1);
            glEnd();
        }
    }
}

float distance(float ax, float ay, float bx, float by, float ang)
{
    return cos(degToRad(ang))*(bx-ax)-sin(degToRad(ang))*(by-ay);
}

void drawRays3d()
{
    int ray, mx, my, mp, dof, side; float ray_x_position, ray_y_position, ray_angle, x_offset, y_offset, distance_total, vertical_x, vertical_y;

    ray_angle = FixAngle(player_angle+30);

    for(ray = 0; ray < 60; ray++){
        /*check vertical*/
        dof = 0;
        side = 0;
        float distance_vertical = 100000;
        float Tan = tan(degToRad(ray_angle));

        if(cos(degToRad(ray_angle)) > ANGLEOFFSET) {
            ray_x_position = (((int) player_x_position >> 6) << 6) + 64;
            ray_y_position = (player_x_position - ray_x_position) * Tan + player_y_position;
            x_offset = 64; 
            y_offset = -x_offset*Tan;
        }

        else if(cos(degToRad(ray_angle)) < -(ANGLEOFFSET)) {
            ray_x_position = (((int) player_x_position >> 6) << 6) - 0.0001;
            ray_y_position = (player_x_position - ray_x_position) * Tan + player_y_position;
            x_offset = -64;
            y_offset = -x_offset * Tan;
        }
        else {
            ray_x_position = player_x_position;
            ray_y_position = player_y_position;

            dof = 8;
        }

        while(dof < 8)
        {
            mx = (int) (ray_x_position) >> 6;
            my = (int) (ray_y_position) >> 6; 
            mp = my *mapX + mx;

            if(mp > 0 && mp < mapX * mapY && map[mp] == 1){
                distance_vertical = cos(degToRad(ray_angle)) * (ray_x_position - player_x_position) 
                - sin(degToRad(ray_angle)) * (ray_y_position - player_y_position);

                dof = 8;
            } //wall was hit
            else{
                ray_x_position += x_offset;
                ray_y_position += y_offset;
                dof += 1;
            }
        }
        vertical_x = ray_x_position;
        vertical_y = ray_y_position;

        /*check horizontal*/
        dof = 0;
        Tan = 1.0/Tan;
        float distance_horizontal = 100000;

        if(sin(degToRad(ray_angle)) > ANGLEOFFSET) {
            ray_y_position = (((int) player_y_position >> 6) << 6) - 0.0001;
            ray_x_position = (player_y_position - ray_y_position) * Tan + player_x_position;
            y_offset = -64; 
            x_offset = -y_offset*Tan;
        }
        else if(sin(degToRad(ray_angle)) < -(ANGLEOFFSET)) {
            ray_y_position = (((int) player_y_position >> 6) << 6) + 64;
            ray_x_position = (player_y_position - ray_y_position) * Tan + player_x_position;
            y_offset = 64;
            x_offset = -y_offset * Tan;
        }
        else {
            ray_x_position = player_x_position;
            ray_y_position = player_y_position;

            dof = 8;
        }

        while(dof < 8)
        {
            mx = (int) (ray_x_position) >> 6;
            my = (int) (ray_y_position) >> 6; 
            mp = my *mapX + mx;

            if(mp > 0 && mp < mapX * mapY && map[mp] == 1){
                distance_horizontal = cos(degToRad(ray_angle)) * (ray_x_position - player_x_position) 
                - sin(degToRad(ray_angle)) * (ray_y_position - player_y_position);
                dof = 8;
            } //hit wall
            else{
                ray_x_position += x_offset;
                ray_y_position += y_offset;
                dof += 1;
            }
        }

        glColor3f(0.5,0,0);  
        if(distance_vertical < distance_horizontal){
            ray_x_position = vertical_x;
            ray_y_position = vertical_y;
            distance_horizontal = distance_vertical; 
            glColor3f(0.9,0,0);   
        }

        glLineWidth(3); glBegin(GL_LINES); glVertex2i(player_x_position,player_y_position); glVertex2i(ray_x_position, ray_y_position); glEnd();

        /*DRAW 3d WALLS*/
        
        int camera_angle = FixAngle(player_angle - ray_angle); 
        distance_horizontal = distance_horizontal * cos(degToRad(camera_angle));
        int lineHeight = (map_size*320) / (distance_horizontal);
        if(lineHeight > 320) {
            lineHeight = 320;
        }
        int lineOffset = 160 - (lineHeight >> 1);

        glLineWidth(8); glEnable(GL_LINE_SMOOTH); glBegin(GL_LINES); glVertex2i(ray * 8 + 530, lineOffset); glVertex2i(ray * 8 + 530, lineHeight + lineOffset); glEnd();
        ray_angle = (FixAngle(ray_angle - 1));
    }
}

void ButtonDown(unsigned char key, int x, int y)
{
    if (key == 'a') {Keys.a = 1;}
    if (key == 'd') {Keys.d = 1;}
    if (key == 's') {Keys.s = 1;}
    if (key == 'w') {Keys.w = 1;}
}

void ButtonUp(unsigned char key, int x, int y)
{
    if (key == 'a') {Keys.a = 0;}
    if (key == 'd') {Keys.d = 0;}
    if (key == 's') {Keys.s = 0;}
    if (key == 'w') {Keys.w = 0;}
}

void init()
{
    glClearColor(0.3,0.3,0.3,0);
    gluOrtho2D(0 ,GAME_WINDOW_WIDTH, GAME_WINDOW_HEIGHT, 0);

    player_x_position = 300; player_y_position = 300;  player_angle = 90;        
    player_x_delta = cos(degToRad(player_angle)); 
    player_y_delta = -sin(degToRad(player_angle));

}

float frame1 = 0;

void draw_horizon()
{
    glColor3f(0,1,1); glBegin(GL_QUADS); glVertex2i(526,  0); glVertex2i(1006,  0); glVertex2i(1006,160); glVertex2i(526,160); glEnd();	
    glColor3f(0,0,1); glBegin(GL_QUADS); glVertex2i(526,160); glVertex2i(1006,160); glVertex2i(1006,320); glVertex2i(526,320); glEnd();	 	
}


void display()
{
    //fps
    int currentTime = glutGet(GLUT_ELAPSED_TIME);
    float deltaTime = (currentTime - frame1) / 1000.0f;
    frame1 = currentTime;

    //printf("%f\n", fps);

    if(Keys.a == 1) {
        player_angle += 120.0f * deltaTime;
        player_angle = FixAngle(player_angle);
        player_x_delta = cos(degToRad(player_angle));
        player_y_delta = -sin(degToRad(player_angle));
    }

    if(Keys.d == 1) {
        player_angle -= 120.0f * deltaTime;
        player_angle = FixAngle(player_angle);
        player_x_delta = cos(degToRad(player_angle));
        player_y_delta = -sin(degToRad(player_angle));
    }

    int x_offset = 0;
    if(player_x_delta < 0) {
        x_offset = -20;
    }
    else {x_offset = 20;}

    int y_offset = 0;
    if(player_y_delta < 0) {
        y_offset = -20;
    }
    else {y_offset = 20;}

    int grid_x_pos = player_x_position/64.0;
    int grid_add_x_offset = (player_x_position+x_offset)/64.0;
    int grid_sub_x_offset = (player_x_position-x_offset)/64.0;
        
    int grid_y_pos = player_y_position/64.0;
    int grid_add_y_offset = (player_y_position+y_offset)/64.0;
    int grid_sub_y_offset = (player_y_position-y_offset)/64.0;


    if(Keys.w == 1) {
        if(map[grid_y_pos * mapX + grid_add_x_offset] == 0) {
            player_x_position += player_x_delta * 100.0f * deltaTime;
        }
        if(map[grid_add_y_offset * mapX + grid_x_pos] == 0) {
            player_y_position += player_y_delta * 100.0f * deltaTime;        
    }


    }

    if(Keys.s == 1) {
        if(map[grid_y_pos * mapX + grid_sub_x_offset] == 0) {
            player_x_position += player_x_delta * 100.0f * deltaTime;
        }
        if(map[grid_add_y_offset * mapX + grid_x_pos] == 0) {
            player_y_position += player_y_delta * 100.0f * deltaTime;     
        }
    }

    glutPostRedisplay();

    glClear(GL_COLOR_BUFFER_BIT);
    drawMap2d();
    drawPlayer();
    draw_horizon();
    drawRays3d();
    glutSwapBuffers();
}

void resize(int w, int h)
{
    glutReshapeWindow(GAME_WINDOW_WIDTH,GAME_WINDOW_HEIGHT);
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(GAME_WINDOW_WIDTH, GAME_WINDOW_HEIGHT);
    glutInitWindowPosition(200,200);
    glutCreateWindow("My Raycaster - V1.0");
    printf("TESTING");
    init();
    glutDisplayFunc(display);
    glutReshapeFunc(resize);
    glutKeyboardFunc(ButtonDown);
    glutKeyboardUpFunc(ButtonUp);
    glutMainLoop();
    return 0;
}