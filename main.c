#include <GL/glut.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

/*  Kod preuzet sa http://poincare.matf.bg.ac.rs/~ivan/files/rg/vezbe/07/32_texture.tar.bz2 */
#include "image.h"

/* Konstanta za visinu skoka igraca */
const static float PLAYER_JUMP_HEIGHT = 1.4f;
/* Konstanta za maksimalnu visinu igraca na ekranu */
const static float PLAYER_MAX_YPOS = 7.5f;
/* Konstanta za minimalnu visinu igraca na ekranu  */
const static float PLAYER_MIN_YPOS = -7.5f;
/* Konstanta za poziciju na Y osi gornjih prepreka */
const static float OBSTACLE_UPPER_YPOS = 9.0f;
/* Konstanta za poziciju na Y osi donjih prepreka */
const static float OBSTACLE_LOWER_YPOS = -9.0f;
/* Konstanta za visinu prepreka */
const static float OBSTACLE_HEIGHT = 5.0f;
/* Konstanta za razdaljinu izmedju prepreka */
const static float OBSTACLE_DISTANCE = 12.0f;
/* Konstanta PI */
const static float pi = 3.141592653589793;


/* Promenljive koje cuvaju sirinu i visinu prozora */
static int window_width, window_height;

/* Indikator da li je animacija pokrenuta */
static int animation_running = 0;

/* Parametar animacije krila */
static float wing_anim_param;
/* Indikator u kom smeru se menja ugao krila */
static int ind = 1;

/* Pozicija igraca na Y osi */
static float ypos = 0;
/* Bafer za povecanje pozicije igraca na Y osi prilikom skoka radi gladje animacije  */
static float yinc = 0;

/* Identifikator teksture */
static GLuint texture_id;

static float obstacle_zpos = -9.5;

/* Prototipi funkcija koje se prosledjuju glut-u */
static void on_reshape(int width, int height);
static void on_display(void);
static void on_timer(int value);
static void on_keyboard(unsigned char key, int x, int y);

/* Prototip funkcije koja vrsi OpenGL inicijalizaciju */
static void initialize(void);
/* Prototip funkcije za crtanje igraca */
static void draw_player(void);
/* Prototip funkcije za crtanje jedne prepreke */
static void draw_obstacle(int orientation, int height);
/* Prototip funkcije za crtanje para prepreka */
static void draw_obstacle_pair(float zpos, int random_parameter);
/* Prototip funkcije za crtanje pozadine */
static void draw_background_image(void);

int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);

    glutInitWindowSize(800, 600);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("FlappyBird3D");

    glutKeyboardFunc(on_keyboard);
    glutReshapeFunc(on_reshape);
    glutDisplayFunc(on_display);

    glClearColor(0, 0, 0, 0);
    glEnable(GL_DEPTH_TEST);

    initialize();

    glutMainLoop();

    return 0;
}

static void on_reshape(int width, int height)
{
    /* Prilikom promene velicine prozora pamte se sirina i visina prozora */
    window_width = width;
    window_height = height;
}

static void initialize(void)
{
    /* Podesava se osvetljenje */
    GLfloat light_position[] = { 1, 1, 1, 0 };
    GLfloat light_ambient[] = { 0.3, 0.3, 0.3, 1 };
    GLfloat light_diffuse[] = { 0.7, 0.7, 0.7, 1 };
    GLfloat light_specular[] = { 0.9, 0.9, 0.9, 1 };

    /* Pali se osvetljenje */
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    
    /* Postavlja se seed za random koristeci trenutno vreme */
    srand(time(NULL));

    /* Ukljucuju se teksture */
    glEnable(GL_TEXTURE_2D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    /* Inicijalizuje se objekat Image i ucitava se tekstura */
    Image * image = image_init(0, 0);
    image_read(image, "sky.bmp");

    /* Generise se identifikator teksture */
    glGenTextures(1, &texture_id);

    /* Bajnduje se tekstura i postavljaju se njeni parametri */
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image->width, image->height, 0, GL_RGB, GL_UNSIGNED_BYTE, image->pixels);

    /* Unbajnduje aktivnu teksturu i iskljucuje se rad sa teksturama*/
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);

    /* Cisti se Image objekat za ucitavanje slika */
    image_done(image);
}

static void on_display(void)
{
    /* Cisti se prethodni sadrzaj prozora */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    draw_background_image();

    /* Cisti se samo dubinski bafer. Color bafer se ne cisti da bi ostala pozadinska slika */
    glClear(GL_DEPTH_BUFFER_BIT);

    /* Podesava se viewport */
    glViewport(0, 0, window_width, window_height);

    /* Podesava se projekciona matrica */
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60, window_width/(float)window_height, 1, 50);

    /* Podesava se tacka pogleda */
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();    
    gluLookAt(15, 0, 0, 0, 0, 0, 0, 1, 0);

    glEnable(GL_COLOR_MATERIAL);

    /* Crta se igrac */
    draw_player();

    draw_obstacle_pair(obstacle_zpos, 0);

    draw_obstacle_pair(obstacle_zpos - OBSTACLE_DISTANCE, -2);

    draw_obstacle_pair(obstacle_zpos - 2 * OBSTACLE_DISTANCE, 3);

    glDisable(GL_COLOR_MATERIAL);

    /* Salje se nova slika na ekran */
    glutSwapBuffers();
}

static void on_timer(int value)
{
    /* Proverava se prosledjena value promenljiva da se utvrdi da li callback dolazi od pravog tajmera */
    if (value != 0)
        return;

    if(ypos >= PLAYER_MAX_YPOS || ypos <= PLAYER_MIN_YPOS)
        animation_running = 0;

    if(animation_running)
    {
        /* Azurira se parametar animacije krila u odgovarajucem smeru */
        wing_anim_param += ind * 6;

        /* Azurira se Y pozicija igraca tako sto konstantno pada za 0.3 a uvecava se za vrednost trenutnog bafera */
        ypos = ypos - 0.3f + yinc;
        if(yinc > 0)
        	yinc -= 0.2f;
        else
        	yinc = 0;

        obstacle_zpos += 0.2f;

        /* Forsira se ponovno iscrtavanje prozora */
        glutPostRedisplay();

        /* Opet se poziva tajmer */
        glutTimerFunc(40, on_timer, 0);
    }
}

static void on_keyboard(unsigned char key, int x, int y)
{
    switch (key) {
    case 27:
        /* Na ESC se zavrsava program */
        exit(0);
        break;
    case ' ':
    	/* Na SPACE se skace povecavanjem bafera za visinu skoka */
        if(animation_running == 1)
            yinc += PLAYER_JUMP_HEIGHT;
        break;
    case 13:
        /* Na ENTER postavljamo sve parametre na pocetnu vrednost i pokrecemo igru */
        if(!animation_running)
        {
            animation_running = 1;
            ypos = 0;
            yinc = 0;
            wing_anim_param = 0.0f;
            glutTimerFunc(40, on_timer, 0);
        }
        break;
    }
}

static void draw_player(void)
{
    /* Crta se zuto telo ptice */
    glColor3f(1.000, 0.843, 0.000);
    glPushMatrix();
    	glTranslatef(0, ypos, 0);
    	glutSolidSphere(1, 50, 50);
    glPopMatrix();

    /* Crta se levo oko */
	glColor3f(1, 1, 1);
    glPushMatrix();
    	glTranslatef(-0.6, ypos + 0.2, -0.6);
    	glutSolidSphere(0.3, 8, 8);
    glPopMatrix();

    glColor3f(0, 0, 0);
    glPushMatrix();
    	glTranslatef(-0.9, ypos + 0.4, -0.6);
    	glutSolidSphere(0.1, 6, 6);
    glPopMatrix();

    /* Crta se desno oko */
	glColor3f(1, 1, 1);
    glPushMatrix();
    	glTranslatef(0.6, ypos + 0.3, -0.6);
    	glutSolidSphere(0.3, 12, 12);
    glPopMatrix();

    glColor3f(0, 0, 0);
    glPushMatrix();
    	glTranslatef(0.9, ypos + 0.4, -0.6);
    	glutSolidSphere(0.1, 6, 6);
    glPopMatrix();

    /* Crta se kljun */
    glColor3f(1.000, 0.388, 0.278);
    glPushMatrix();
    	glTranslatef(0, ypos - 0.2, -0.9);
    	glRotatef(180, 1, 0, 0);
    	glutSolidCone(0.3, 1, 12, 1);
    glPopMatrix();
    
    /* Crtanje belih animiranih krila: */
    glColor3f(1, 1, 1);
    /* Obrce se smer promene paramera animacije krila kada se dodje do odgovarajuceg gornjeg ili donjeg ugla */
    if(wing_anim_param >= 30 || wing_anim_param <= -30)
    	ind = -ind;

    /* Crta se desno krilo */
    glPushMatrix();
    	glTranslatef(0.5, ypos, 0.2);
        glRotatef(wing_anim_param, 0, 0, 1);
        glTranslatef(0.75,0,0);
        glScalef(0.75, 0.3, 1);
        glutSolidCube(1);
    glPopMatrix();

    /* Crta se levo krilo */
    glPushMatrix();
    	glTranslatef(-0.5, ypos, 0.2);
        glRotatef(-wing_anim_param, 0, 0, 1);
        glTranslatef(-0.75,0,0);
        glScalef(0.75, 0.3, 1);
        glutSolidCube(1);
    glPopMatrix();
}

static void draw_obstacle(int orientation, int height)
{
    int CYLINDER_SEGMENTS = 16;
    float radius = 1.5f;

    int i = 0;
    float angle = 0;

    /* Crta se prepreka kao zeleni pravougaonik iz dva dela */
    glPushMatrix();
        /* Svetliji deo pravougaonika */
        glColor3f(0.235, 0.702, 0.443);
        glBegin(GL_TRIANGLE_STRIP);
            for (i = 0; i <= CYLINDER_SEGMENTS; i++) {
                angle = i * (2 * pi / CYLINDER_SEGMENTS);

                /* Definisemo normalu povrsi */
                glNormal3f(cos(angle), 0, sin(angle));

                /* Definisemo koordinate tacaka */
                glVertex3f(radius * cos(angle), 0, radius * sin(angle));
                glVertex3f(radius * cos(angle), height - 1, radius * sin(angle));
            }
        glEnd();

        /* Tamniji deo pravougaonika */
        glTranslatef(0, height - 1, 0);
        glColor3f(0.180, 0.545, 0.341);
        glBegin(GL_TRIANGLE_STRIP);
            for (i = 0; i <= CYLINDER_SEGMENTS; i++) {
                angle = i * (2 * pi / CYLINDER_SEGMENTS);

                /* Definisemo normalu povrsi */
                glNormal3f(cos(angle), 0, sin(angle));

                /* Definisemo koordinate tacaka */
                glVertex3f(radius * cos(angle), 0, radius * sin(angle));
                glVertex3f(radius * cos(angle), 1, radius * sin(angle));
            }
        glEnd();

        /* Poklopac gornjeg pravougaonika */
        glTranslatef(0, 1, 0);
        glBegin(GL_TRIANGLE_FAN);
            glVertex3f(0, 0, 0);
            for (i = 0; i <= CYLINDER_SEGMENTS; i++) {
                angle = i * (2 * pi / CYLINDER_SEGMENTS);

                /* Definisemo normalu povrsi, Y normalu izracunavamo kao znak parametra orientation */
                glNormal3f(cos(angle), (orientation > 0) - (orientation < 0), sin(angle));

                /* Definisemo koordinate tacaka */
                glVertex3f(radius * cos(angle), 0, radius * sin(angle));
            }
        glEnd();
    glPopMatrix();
}

static void draw_obstacle_pair(float zpos, int random_parameter)
{
    glPushMatrix();
        glTranslatef(0, OBSTACLE_LOWER_YPOS, zpos);
        draw_obstacle(1, OBSTACLE_HEIGHT - random_parameter);
    glPopMatrix();
    glPushMatrix();
        glTranslatef(0, OBSTACLE_UPPER_YPOS, zpos);
        glRotatef(180, 1, 0, 0);
        draw_obstacle(-1, OBSTACLE_HEIGHT + random_parameter);
    glPopMatrix();
}

static void draw_background_image(void)
{
    /* Podesava se ortografska projekcija da bi se iscrtao poligon sa pozadinskom teksturom */
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(0, 1, 1, 0, -1, 1);

    /* Gasi se osvetljenje da ne utice na pozadinsku sliku */
    glDisable(GL_LIGHTING);

    /* Ukljucuju se teksture */
    glEnable(GL_TEXTURE_2D);

    /* Bajnduje se tekstura za pozadinsku sliku */
    glBindTexture(GL_TEXTURE_2D, texture_id);

    /* Crta se poligon preko celog ekrana */
    glColor3f(0, 0, 0);
    glBegin(GL_TRIANGLE_FAN);
        glTexCoord2f(1, 1);
        glVertex2f(0, 0);

        glTexCoord2f(0, 1);
        glVertex2f(1, 0);

        glTexCoord2f(0, 0);
        glVertex2f(1, 1);

        glTexCoord2f(1, 0);
        glVertex2f(0, 1);
    glEnd();

    /* Posle crtanja pozadinske slike, pali se osvetljenje i unbajnduje se tekstura */
    glBindTexture(GL_TEXTURE_2D, 0);
    glEnable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
}
