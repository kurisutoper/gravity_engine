#include <iostream>
#include <vector>
#include <cmath>
#include <unistd.h>
#include <GL/glut.h>
using namespace std;
struct Particle;
float G = 6.67384*pow(10, -11);
vector<Particle> zoo;
struct Particle {
    Particle(float XPOS, float YPOS, float MASS):
             xpos(XPOS),ypos(YPOS),mass(MASS) {
        pid = pid_counter++;
        yvel = 0.0f;
        xvel = 0.0f;
        for(unsigned short int i = 0; i < 3; ++i)
            color[i] = 1.0f;
    }
    void render(void) {
        vector<vector<float> >::iterator n;
        glColor3f(color[0], color[1], color[2]);
        //to prevent leaking into the command area.
        if(xpos > 0.0f) {
            //particle
            glBegin(GL_POINTS);
                glVertex2f(xpos, ypos);
            glEnd();
            //particle's tail
            /* optimize later to only render
            new parts of tail */
            glBegin(GL_LINE_STRIP);
                for(n = past.begin();
                    n != past.end(); ++n)
                    glVertex2f(n->at(0), n->at(1));
            glEnd();
        }
    }
    //computes gravitation from other particles.
    void compForces(vector<Particle> zoo) {
        float fx = 0.0f, fy = 0.0f,
              f, slope, angle, r, pol;
        vector<Particle>::iterator zit;
        for(zit = zoo.begin();
            zit != zoo.end(); ++zit) {
            if(zit->pid == pid) continue;
            //calculate magnitude
            r = sqrt(pow(zit->xpos-xpos, 2)+pow(zit->ypos-ypos, 2));
            f = (G*zit->mass*mass)/pow(r, 2);
            //calculate slope
            slope = (zit->ypos-ypos)/(zit->xpos-xpos);
            angle = atan(slope);
            fx += (xpos>zit->xpos?-1:1)*abs(f*cos(angle));
            fy += (ypos>zit->ypos?-1:1)*abs(f*sin(angle));
        }
        applyForce(fx, fy);
    }
    //where x and y are measured in newtons
    void applyForce(float x, float y) {
        xvel += x/mass;
        yvel += y/mass;
    }
    void mov(void) {
        xpos += xvel;
        ypos += yvel;
        past.push_back(vector<float>(2,0));
        past.back().at(0) = xpos;
        past.back().at(1) = ypos;
    }
    void changeColor(float r, float g, float b) {
        color[0] = r;
        color[1] = g;
        color[2] = b;
    }
    float xvel, yvel,
          xpos, ypos, mass, color[3];
    vector<vector<float> > past; //previous coords
    unsigned int pid; //Particle ID
    static unsigned int pid_counter;
}; unsigned int Particle::pid_counter = 0;
namespace Command {
    vector<string> log(1, "");
    //appends to command string and carries out the commands
    void handle(unsigned char key, int x, int y) {
        if(key == 0x08 && log.back().size() > 0) {
            log.back() = log.back().substr(0, log.back().size()-1);
        }
        else if(key == 0x0D) {
            /* run command here: */
            log.push_back("");
        }
        else log.back() += key;
    }
    void render(void) {
        float tracker;
        unsigned short int i, u;
        string temp;
        glColor3f(1.0f, 1.0f, 1.0f);
        for(tracker = 0.95f, i = 0; i < log.size(); ++i, tracker -= 0.02f) {
            glRasterPos3f(-0.45f, tracker, 0.0f);
            temp = ">"+log.at(i);
            for(u = 0; u < temp.size(); ++u) {
                glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, temp.at(u));
            }
        }
    }
}
void clocked(void) {
    usleep(1000);
    glutPostRedisplay();
}
void render(void) {
    glClear(GL_COLOR_BUFFER_BIT);
    vector<Particle>::iterator zit;
    for(zit = zoo.begin();
        zit != zoo.end(); ++zit) {
        zit->render();
        zit->compForces(zoo);
    }
    for(zit = zoo.begin();
        zit != zoo.end(); ++zit)
        zit->mov();
    //divides command area from particles(visually)
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_LINES);
        glVertex2f(0.0f, 0.0f);
        glVertex2f(0.0f, 1.0f);
    glEnd();
    Command::render();
    glFlush();
}
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitWindowSize(600, 500);
    glutCreateWindow("Gravity");
    glPointSize(2.0f);
    glMatrixMode(GL_PROJECTION);
    glOrtho(-0.5, 1.0, 0, 1.0, -1, 1);
    glutDisplayFunc(&render);
    glutIdleFunc(&clocked);
    glutKeyboardFunc(&Command::handle);

    zoo.push_back(Particle(0.5f, 0.45f, 10000.0f));
    zoo.push_back(Particle(0.2f, 0.6f, 1.0f));
    zoo.at(0).changeColor(1.0f, 0.0f, 0.0f);
    zoo.at(1).changeColor(0.0f, 1.0f, 0.0f);
    zoo.at(1).xvel = 0.001f;

    glutMainLoop();
    return 0;
}
