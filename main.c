#include <GL/glut.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <string.h>

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
/* Konstanta za broj stranica ciling_indra za prepreke */
const static int OBSTACLE_CYLINDER_SEGMENTS = 16;
/* Konstanta za poluprecnik cilindra za prepreke */
const static float OBSTACLE_RADIUS = 1.5f;
/* Konstanta PI */
const static float pi = 3.141592653589793;


/* Promenljive koje cuvaju sirinu i visinu prozora */
static int window_width, window_height;

/* Indikator da li je animacija pokrenuta */
static int animation_running = 0;

/* Parametar animacije krila */
static float player_wing_anim_param;
/* Indikator u kom smeru se menja ugao krila */
static int player_wing_ind = 1;

/* Pozicija igraca na Y osi */
static float player_ypos = 0;
/* Bafer za povecanje pozicije igraca na Y osi prilikom skoka radi gladje animacije  */
static float player_yinc = 0;

/* Identifikator teksture */
static GLuint texture_id[3];

/* Promenljiva koja cuva poziciju prvog para prepreki */
static float obstacle_zpos = -9.5f;
/* Niz koji cuva nasumicno generisane parametre gde se otvor na paru prepreki nalazi */
static int obstacle_gaps[3];
/* Niz koji cuva dodatni nasumicni parametar za svaku prepreku tako da sirina otvora ne bude uvek ista */
static int obstacle_gaps_random_param[3];
/* Parametar koji odredjuje da li je prva prepreka predjena */
static int first_obstacle_passed_ind = 0;
/* Parametri pretposlednjeg predjenog para prepreki koji se crta da ne bi nestajao prvi par prepreki pre nego sto izadje sa ekrana na widescreen rezolucijama prozora */
static int old_obstacle_gap;
static int old_obstacle_gap_random_param;

/* Promenljiva koja cuva proteklo vreme od pocetka prikazivanja poruke o komandama */
static int message_time = 0;
/* Indikator da li treba da se prikaze pocetni ekran */
static int show_start_screen = 1;
/* Indikator da li treba da se prikaze zavrsni ekran */
static int show_end_screen = 0;

/* Promenljiva koja cuva rezultat */
static int score = 0;

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
static void draw_obstacle_pair(float zpos, int random_placement_param, int additional_random_param);
/* Prototip funkcije za crtanje pozadine */
static void draw_background_image(void);
/* Prototip funkcije za ispisivanje teksta na odredjenoj lokaciji */
static void draw_text(char* text, float posx, float posy, float R, float G, float B);

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
    /* Prilikom promene velicine prozora pamte se nova sirina i visina prozora */
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

    /* Pali se bojenje neteksturisanih modela preko glColor */
    glEnable(GL_COLOR_MATERIAL);
    
    /* Postavlja se seed za random koristeci trenutno vreme */
    srand(time(NULL));

    /* Ukljucuju se teksture */
    glEnable(GL_TEXTURE_2D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    /* Inicijalizuje se objekat Image i ucitava se tekstura */
    Image * image = image_init(0, 0);
    image_read(image, "resources/sky.bmp");

    /* Generise se identifikator teksture */
    glGenTextures(3, texture_id);

    /* Bajnduje se tekstura za pozadinu i postavljaju se njeni parametri */
    glBindTexture(GL_TEXTURE_2D, texture_id[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image->width, image->height, 0, GL_RGB, GL_UNSIGNED_BYTE, image->pixels);

    /* Ucitava se tekstura za pocetni ekran */
    image_read(image, "resources/startscreen.bmp");

    /* Bajnduje se tekstura za pocetni ekran i postavljaju se njeni parametri */
    glBindTexture(GL_TEXTURE_2D, texture_id[1]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image->width, image->height, 0, GL_RGB, GL_UNSIGNED_BYTE, image->pixels);

    /* Ucitava se tekstura za zavrsni ekran */
    image_read(image, "resources/endscreen.bmp");

    /* Bajnduje se tekstura za pocetni ekran i postavljaju se njeni parametri */
    glBindTexture(GL_TEXTURE_2D, texture_id[2]);
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

    /* Podesava se viewport */
    glViewport(0, 0, window_width, window_height);

    /* Crta se odgovarajuca pozadina */
    draw_background_image();

    /* Cisti se samo dubinski bafer. Color bafer se ne cisti da bi ostala pozadinska slika */
    glClear(GL_DEPTH_BUFFER_BIT);
    
    /* Podesava se projekciona matrica */
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60, window_width/(float)window_height, 1, 50);

    /* Podesava se tacka pogleda */
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();    
    gluLookAt(15, 0, 0, 0, 0, 0, 0, 1, 0);

    /* Crta se igra ukoliko se ne prikazuje zavrsni ekran */
    if(!show_end_screen)
    {
        /* Crta se igrac */
        draw_player();

        /* Crtaju sve tri prepreke na odgovarajucim razdaljinama */
        int i;
        for(i = 0; i < 3; ++i)
            draw_obstacle_pair(obstacle_zpos - i * OBSTACLE_DISTANCE, obstacle_gaps[i], obstacle_gaps_random_param[i]);

        /* Crta se stari sacuvani par prepreka */
        if(first_obstacle_passed_ind && score > 1)
            draw_obstacle_pair(obstacle_zpos + OBSTACLE_DISTANCE, old_obstacle_gap, old_obstacle_gap_random_param);

        /* Crta se tekst ako je nije vise na startnom ekranu */
        if(!show_start_screen)
        {
            /* Cisti se bafer dubine da bi se tekst iscrtao preko svega */
            glClear(GL_DEPTH_BUFFER_BIT);

            /* Priprema se string i ispisuje se dosadasnji rezultat */
            char buffer[50];
            sprintf(buffer, "SCORE: %d", score);
            draw_text(buffer, -1, -8.5, 1, 1, 1);

            /* Priprema se i ispisuje poruka o komanda ako treba */
            if(message_time < 1000)
            {
                sprintf(buffer, "PRESS [SPACE] TO JUMP");
                draw_text(buffer, -3.5, -4, 1, 1, 1);
                sprintf(buffer, "PRESS [ESC] TO QUIT");
                draw_text(buffer, -3.1, -5, 1, 1, 1);
            }
        }
    }
    /* Ako se prikazuje zavrsni ekran, onda ispisujemo ostvareni rezultat */
    else
    {
        /* Priprema se string i ispisuje se konacni rezultat */
        char buffer[50];
        sprintf(buffer, "%d POINTS", score);
        draw_text(buffer, -1.25, -3, 1, 1, 1);
    }

    /* Salje se nova slika na ekran */
    glutSwapBuffers();
}

static void on_timer(int value)
{
    /* Proverava se prosledjena value promenljiva da se utvrdi da li callback dolazi od pravog tajmera */
    if (value != 0)
        return;

    /* Proverava se da li je igrac udario u donju ili gornju ivicu prozora */
    if(player_ypos >= PLAYER_MAX_YPOS || player_ypos <= PLAYER_MIN_YPOS)
    {
        animation_running = 0;
        show_end_screen = 1;
        glutPostRedisplay();
    }

    /* Postavlja se indikator da li je predjena prva prepreka */
    first_obstacle_passed_ind = (obstacle_zpos >= 0);

    /* Proverava se kolizija sa centralnim od tri para prepreka */
    if(((player_ypos >= OBSTACLE_UPPER_YPOS - OBSTACLE_HEIGHT - obstacle_gaps[first_obstacle_passed_ind] - 1) 
        || (player_ypos <= OBSTACLE_LOWER_YPOS + OBSTACLE_HEIGHT - obstacle_gaps[first_obstacle_passed_ind] - obstacle_gaps_random_param[first_obstacle_passed_ind] + 1)) && (obstacle_zpos - first_obstacle_passed_ind * OBSTACLE_DISTANCE - 1 <= 1.5 && obstacle_zpos - first_obstacle_passed_ind * OBSTACLE_DISTANCE + 1 >= -1.5))
        {
            animation_running = 0;
            show_end_screen = 1;
            glutPostRedisplay();
        }


    if(animation_running)
    {
        /* Azurira se parametar animacije krila u odgovarajucem smeru */
        player_wing_anim_param += player_wing_ind * 24;

        /* Azurira se Y pozicija igraca tako sto konstantno pada za 0.3 a uvecava se za vrednost trenutnog bafera */
        player_ypos = player_ypos - 0.3f + player_yinc;
        if(player_yinc > 0)
        	player_yinc -= 0.2f;
        else
        	player_yinc = 0;

        /* Azurira se pozicija prvog para prepreka na Z osi */
        obstacle_zpos += 0.5f;

        /* Uvecava se rezultat kada igrac predje prvi par prepreki */
        if(obstacle_zpos == 1.5f)
            score++;

        /* Kada prvi u nizu parova prepreki izadje sa ekrana, dodaje se novi par na kraj */
        if(obstacle_zpos >= OBSTACLE_DISTANCE + OBSTACLE_RADIUS)
        {
            /* Uvecava se rezultat */
            score++;

            old_obstacle_gap = obstacle_gaps[0];
            old_obstacle_gap_random_param = obstacle_gaps_random_param[0];

            /* Pamte se parametri poslednja dva para prepreka na mesto prva dva */
            obstacle_gaps[0] = obstacle_gaps[1];
            obstacle_gaps_random_param[0] = obstacle_gaps_random_param[1];
            obstacle_gaps[1] = obstacle_gaps[2];
            obstacle_gaps_random_param[1] = obstacle_gaps_random_param[2];

            /* Generise se novi par prepreka na poslednjem mestu */
            obstacle_gaps[2] = rand() % 4;
            obstacle_gaps_random_param[2] = rand() % 3;
            if(rand() % 2) 
                obstacle_gaps[2] = -obstacle_gaps[2];
            if(rand() % 2)
                obstacle_gaps_random_param[2] = -obstacle_gaps_random_param[2];

            /* Umanjuje se pozicija na Z osi prvog para prepreki za razdaljinu izmedju prepreki jer sada srednji par postaje prvi */
            obstacle_zpos -= OBSTACLE_DISTANCE;
        }

        /* Povecava se vreme proteklo od pokazivanja poruke */
        if(message_time < 1000)
            message_time += 35;

        /* Forsira se ponovno iscrtavanje prozora */
        glutPostRedisplay();

        /* Opet se poziva tajmer */
        glutTimerFunc(35, on_timer, 0);
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
            player_yinc += PLAYER_JUMP_HEIGHT;
        break;
    case 13:
        /* Na ENTER se postavljaju svi parametri na pocetnu vrednost i pokrece se igra ako nije vec pokrenuta */
        if(!animation_running)
        {
            /* Pokrece se igra */
            animation_running = 1;

            /* Iskljucuje se pocetni ekran */
            show_start_screen = 0;

            /* Iskljucuje se zavrsni ekran */
            show_end_screen = 0;

            /* Postavljaju se parametri igre na pocetne vrednost */
            player_ypos = 0;
            player_yinc = 0;
            player_wing_anim_param = 0.0f;
            obstacle_zpos = -9.5;
            first_obstacle_passed_ind = 0;
            int i;
            for(i = 1; i < 3; ++i)
            {
                obstacle_gaps[i] = rand() % 4;
                obstacle_gaps_random_param[i] = rand() % 3;
                if(rand() % 2) 
                    obstacle_gaps[i] = -obstacle_gaps[i];
                if(rand() % 2)
                    obstacle_gaps_random_param[i] = -obstacle_gaps_random_param[i];
            }
            score = 0;
            message_time = 0;

            /* Postavljaju se parametri prvog para prepreka na 0 tako da se ocuva kontinuitet sa pocetnim ekranom */
            obstacle_gaps[0] = 0;
            obstacle_gaps_random_param[0] = 0;

            /* Pokrece se tajmer */
            glutTimerFunc(35, on_timer, 0);
        }
        break;
    }
}

static void draw_player(void)
{
    /* Crta se zuto telo ptice */
    glColor3f(1.000, 0.843, 0.000);
    glPushMatrix();
    	glTranslatef(0, player_ypos, 0);
    	glutSolidSphere(1, 50, 50);
    glPopMatrix();

    /* Crta se levo oko */
	glColor3f(1, 1, 1);
    glPushMatrix();
    	glTranslatef(-0.6, player_ypos + 0.2, -0.6);
    	glutSolidSphere(0.3, 8, 8);
    glPopMatrix();

    glColor3f(0, 0, 0);
    glPushMatrix();
    	glTranslatef(-0.9, player_ypos + 0.4, -0.6);
    	glutSolidSphere(0.1, 6, 6);
    glPopMatrix();

    /* Crta se desno oko */
	glColor3f(1, 1, 1);
    glPushMatrix();
    	glTranslatef(0.6, player_ypos + 0.3, -0.6);
    	glutSolidSphere(0.3, 12, 12);
    glPopMatrix();

    glColor3f(0, 0, 0);
    glPushMatrix();
    	glTranslatef(0.9, player_ypos + 0.4, -0.6);
    	glutSolidSphere(0.1, 6, 6);
    glPopMatrix();

    /* Crta se kljun */
    glColor3f(1.000, 0.388, 0.278);
    glPushMatrix();
    	glTranslatef(0, player_ypos - 0.2, -0.9);
    	glRotatef(180, 1, 0, 0);
    	glutSolidCone(0.3, 1, 12, 1);
    glPopMatrix();
    
    /* Crtanje belih animiranih krila: */
    glColor3f(1, 1, 1);
    /* Obrce se smer promene paramera animacije krila kada se dodje do odgovarajuceg gornjeg ili donjeg ugla */
    if(player_wing_anim_param >= 30 || player_wing_anim_param <= -30)
    	player_wing_ind = -player_wing_ind;

    /* Crta se desno krilo */
    glPushMatrix();
    	glTranslatef(0.5, player_ypos, 0.2);
        glRotatef(player_wing_anim_param, 0, 0, 1);
        glTranslatef(0.75,0,0);
        glScalef(0.75, 0.3, 1);
        glutSolidCube(1);
    glPopMatrix();

    /* Crta se levo krilo */
    glPushMatrix();
    	glTranslatef(-0.5, player_ypos, 0.2);
        glRotatef(-player_wing_anim_param, 0, 0, 1);
        glTranslatef(-0.75,0,0);
        glScalef(0.75, 0.3, 1);
        glutSolidCube(1);
    glPopMatrix();
}

static void draw_obstacle(int orientation, int height)
{
    int i = 0;
    float angle = 0;

    /* Crta se prepreka kao zeleni pravougaonik iz dva dela */
    glPushMatrix();
        /* Svetliji deo pravougaonika */
        glColor3f(0.235, 0.702, 0.443);
        glBegin(GL_TRIANGLE_STRIP);
            for (i = 0; i <= OBSTACLE_CYLINDER_SEGMENTS; i++) {
                angle = i * (2 * pi / OBSTACLE_CYLINDER_SEGMENTS);

                /* Definisemo normalu povrsi */
                glNormal3f(cos(angle), 0, sin(angle));

                /* Definisemo koordinate tacaka */
                glVertex3f(OBSTACLE_RADIUS * cos(angle), 0, OBSTACLE_RADIUS * sin(angle));
                glVertex3f(OBSTACLE_RADIUS * cos(angle), height - 1, OBSTACLE_RADIUS * sin(angle));
            }
        glEnd();

        /* Tamniji deo pravougaonika */
        glTranslatef(0, height - 1, 0);
        glColor3f(0.180, 0.545, 0.341);
        glBegin(GL_TRIANGLE_STRIP);
            for (i = 0; i <= OBSTACLE_CYLINDER_SEGMENTS; i++) {
                angle = i * (2 * pi / OBSTACLE_CYLINDER_SEGMENTS);

                /* Definisemo normalu povrsi */
                glNormal3f(cos(angle), 0, sin(angle));

                /* Definisemo koordinate tacaka */
                glVertex3f(OBSTACLE_RADIUS * cos(angle), 0, OBSTACLE_RADIUS * sin(angle));
                glVertex3f(OBSTACLE_RADIUS * cos(angle), 1, OBSTACLE_RADIUS * sin(angle));
            }
        glEnd();

        /* Poklopac gornjeg pravougaonika */
        glTranslatef(0, 1, 0);
        glBegin(GL_TRIANGLE_FAN);
            glVertex3f(0, 0, 0);
            for (i = 0; i <= OBSTACLE_CYLINDER_SEGMENTS; i++) {
                angle = i * (2 * pi / OBSTACLE_CYLINDER_SEGMENTS);

                /* Definisemo normalu povrsi, Y normalu izracunavamo kao znak parametra orientation */
                glNormal3f(cos(angle), (orientation > 0) - (orientation < 0), sin(angle));

                /* Definisemo koordinate tacaka */
                glVertex3f(OBSTACLE_RADIUS * cos(angle), 0, OBSTACLE_RADIUS * sin(angle));
            }
        glEnd();
    glPopMatrix();
}

static void draw_obstacle_pair(float zpos, int random_placement_param, int additional_random_param)
{
    glPushMatrix();
        glTranslatef(0, OBSTACLE_LOWER_YPOS, zpos);
        draw_obstacle(1, OBSTACLE_HEIGHT - random_placement_param - additional_random_param);
    glPopMatrix();
    glPushMatrix();
        glTranslatef(0, OBSTACLE_UPPER_YPOS, zpos);
        glRotatef(180, 1, 0, 0);
        draw_obstacle(-1, OBSTACLE_HEIGHT + random_placement_param);
    glPopMatrix();
}

static void draw_background_image(void)
{
    /* Podesava se ortografska projekcija da bi se iscrtao poligon sa pozadinskom teksturom */
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(1, 0, 1, 0, -1, 1);

    /* Gasi se osvetljenje da ne utice na pozadinsku sliku */
    glDisable(GL_LIGHTING);

    /* Ukljucuju se teksture */
    glEnable(GL_TEXTURE_2D);

    /* Bajnduje se tekstura za pozadinu ili za pocetni ekran */
    if(show_start_screen)
        glBindTexture(GL_TEXTURE_2D, texture_id[1]);
    else if(show_end_screen)
        glBindTexture(GL_TEXTURE_2D, texture_id[2]);
    else
        glBindTexture(GL_TEXTURE_2D, texture_id[0]);

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

static void draw_text(char* text, float posx, float posy, float R, float G, float B)
{
    int i;
    glDisable(GL_LIGHTING);
    glColor3f(R, G, B);
    glRasterPos3f(0, posy, -posx);
    for (i = 0; i < strlen(text); i++)
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, text[i]);
    glEnable(GL_LIGHTING);
}
