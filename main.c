#include <GL/glut.h>

/* Promenljive koje cuvaju sirinu i visinu prozora */
static int window_width, window_height;

/* Parametar animacije krila */
static float wing_anim_param;
/* Indikator u kom smeru se menja ugao krila */
static int ind = 1;

/* Pozicija igraca na Y osi */
static float ypos = 0;
/* Bafer za povecanje pozicije igraca na Y osi prilikom skoka radi gladje animacije  */
static float yinc = 0;

/* Prototipi funkcija koje se prosledjuju glut-u */
static void on_reshape(int width, int height);
static void on_display(void);
static void on_timer(int value);
static void on_keyboard(unsigned char key, int x, int y);

/* Prototip funkcije za crtanje igraca */
static void draw_player();

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

    glutTimerFunc(50, on_timer, 0);

    glutMainLoop();

    return 0;
}

static void on_reshape(int width, int height)
{
    /* Prilikom promene velicine prozora pamte se sirina i visina prozora */
    window_width = width;
    window_height = height;
}

static void on_display(void)
{
	/* Podesava se osvetljenje */
	GLfloat light_position[] = { 0.5, 1, 1, 0 };
    GLfloat light_ambient[] = { 0.3, 0.3, 0.3, 1 };
    GLfloat light_diffuse[] = { 0.7, 0.7, 0.7, 1 };
    GLfloat light_specular[] = { 0.9, 0.9, 0.9, 1 };

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    
    /* Pali se osvetljenje */
    glEnable(GL_COLOR_MATERIAL);

    /* Cisti se prethodni sadrzaj prozora */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /* Podesava se viewport */
    glViewport(0, 0, window_width, window_height);

    /* Podesava se projekciona matrica */
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60, window_width/(float)window_height, 1, 50);

    /* Podesava se tacka pogleda */
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();    
    gluLookAt(
       	15, 0, 0,
        0, 0, 0,
        0, 1, 0
    );
    
    /* Crta se igrac */
    draw_player();

    /* Salje se nova slika na ekran */
    glutSwapBuffers();
}

static void on_timer(int value)
{
    /* Proverava se prosledjena value promenljiva da se utvrdi da li callback dolazi od pravog tajmera */
    if (value != 0)
        return;

    /* Azurira se parametar animacije krila u odgovarajucem smeru */
    wing_anim_param += ind * 6;

    /* Azurira se Y pozicija igraca tako sto konstantno pada za 0.3 a uvecava se za vrednost trenutnog bafera */
    ypos = ypos - 0.3f + yinc;
    if(yinc > 0)
    	yinc -= 0.2f;
    else
    	yinc = 0;

    /* Forsira se ponovno iscrtavanje prozora */
    glutPostRedisplay();

    /* Opet se poziva tajmer */
    glutTimerFunc(50, on_timer, 0);
}

static void on_keyboard(unsigned char key, int x, int y)
{
    switch (key) {
    case 27:
        /* Na ESC se zavrsava program */
        exit(0);
        break;
    case ' ':
    	/* Na SPACE se skace povecavanjem bafera za 1 */
        yinc += 1.0f;
        break;
    }
}

static void draw_player()
{
    /* Crta se zuto telo ptice */
    glColor3f(1, 0.9, 0);
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
    glColor3f(1, 0.5, 0);
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