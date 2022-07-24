#include "FreeRTOS.h"
#include "task.h"
#include "TUM_Draw.h"
#include "TUM_Ball.h"
#include "objects.h"


#define mainGENERIC_STACK_SIZE ((unsigned short)2560)
#define mainGENERIC_PRIORITY (tskIDLE_PRIORITY)

#define FPS 30
#define FRAMETIME 1000/FPS

#define MIDX (int)SCREEN_WIDTH/2
#define MIDY (int)SCREEN_HEIGHT/2

#define BORDER 100
#define HEADERHEIGHT BORDER
#define FOOTERHEIGHT BORDER
#define LEFT_BORDER 10
#define RIGHT_BORDER 10
#define UPPER_BORDER 100
#define LOWER_BORDER FOOTERHEIGHT + BORDER
#define GROUND_THICKNESS 3
#define CEILING_THICKNESS 5
#define PLAYERHEIGHT 35
#define PROJECTILESPEED 500
#define PROJECTILESIZE 5
#define PROJECTILECOLOR Green
#define INACTIVE -200
#define ENEMYSIZE 40
#define ENEMYVSPACING 70
#define ENEMYHSPACING 60
#define ENEMYSTARTY 200
#define EASYENEMYSCORE 10
#define MEDIUMENEMYSCORE 20
#define HARDENEMYSCORE 30
#define UFOSCORE 100
#define BASEGAMETICK 1000

static TaskHandle_t vRenderHandle;
static TaskHandle_t vBallOnCollisionHandle;
static TaskHandle_t vGameTickRenderHandle;
static TaskHandle_t vProjectileHandlerHandle;
static TaskHandle_t vRandomProjectilesHandle;


static image_handle_t logo_image;
static image_handle_t exit_image;
static image_handle_t startbutton_active_image;
static image_handle_t startbutton_inactive_image;
static image_handle_t hsbutton_active_image;
static image_handle_t hsbutton_inactive_image;
static image_handle_t cheatsbutton_active_image;
static image_handle_t cheatsbutton_inactive_image;
static image_handle_t arrow_keys_image;
static image_handle_t spacebar_image;
static image_handle_t enter_key_image;
static image_handle_t cheats_header_image;
static image_handle_t cheats1_active_image;
static image_handle_t cheats2_active_image;
static image_handle_t cheats3_active_image;
static image_handle_t cheats1_inactive_image;
static image_handle_t cheats2_inactive_image;
static image_handle_t cheats3_inactive_image;
static image_handle_t highscores_header_image;

static image_handle_t spritesheet_image;
static spritesheet_handle_t spritesheet;

extern struct game game;
extern struct playerhitbox playerhitbox;
extern gameparammutex parameters;
extern hs highscores;

extern int enemyid[ENEMYROW][ENEMYCOL][2];
extern int projectileid[MAXPROJECTILES][2];


int initGraphics(char* bin_folder_path);

void vRender();
void vGameTickRender();
void vProjectileHandler();
void vRandomProjectiles();
void ballCollisionCallbackInContext();

void startGame();
void stopGame();

void queueRedCircle(); //test function
void renderStartScreen(int selectedmenuentry);
void renderWelcomeScreen(int score1, int hiscore);
void renderCheatsScreen(int menuentry, struct gameparameters param);
void renderHighscoreScreen(int* highscores);
void renderPauseScreen(int menuentry);

void renderHeader();
void renderFooter(int lives, int level, int inf_lives);
void renderPlayer(int offset);
void addProjectile(int x, int y, int direction, unsigned int color, int radius, 
    int speed, int maxspeed);
void addPlayerProjectile(int x, int y, int direction, unsigned int color, int radius, 
    int speed, int maxspeed);
void handleProjectiles();
void ballOnCollision(int* id);
void enemyOnCollision(int* id);
void wallOnCollision();
void playerOnCollision();
void renderEnemies(char animationstate);

void onPlayerProjectileCollision();

void clearScreen();

void clearImages();
void exitGraphics();