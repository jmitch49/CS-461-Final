// Pac-Man for xv6 - VGA Graphics Version (syscall-based)

#include "types.h"
#include "user.h"
#include "fcntl.h"
// Use syscall-based VGA (no direct hardware access)

// Game constants
#define WIDTH 320
#define HEIGHT 200
#define MAX_GHOSTS 4

// Game symbols (VGA colors)
#define PACMAN_COLOR  0x90  // Yellow
#define GHOST1_COLOR  0x04  // Red
#define GHOST2_COLOR  0x02  // Green  
#define GHOST3_COLOR  0x01  // Blue
#define DOT_COLOR    0xAC  // Light yellow
#define WALL_COLOR  0x18  // Dark blue
#define EMPTY_COLOR 0x00  // Black
#define POWER_COLOR 0xA8  // Orange

// Directions
#define UP 0
#define DOWN 1
#define LEFT 2
#define RIGHT 3

// Game state
struct game {
  int px, py; //pacman position
  int pdir; //pacman direction
  int score;
  int lives;
  int ghosts[MAX_GHOSTS][3]; //x,y positions
  int num_ghosts;
  int game_over;
  int win;
};

// Simple maze layout (scaled for VGA)
static int maze[20][28] = {
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1},
  {1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1},
  {1,0,1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1},
  {1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1},
  {1,0,1,1,1,1,0,1,0,1,0,1,1,1,1,1,0,1,0,1,0,1,1,1,1,1,1,1},
  {1,0,1,1,1,1,0,1,0,1,0,1,1,1,1,1,0,1,0,1,0,1,1,1,1,1,1,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1},
  {1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1},
  {1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,1,1,1,1,0,1,0,1,1,1,0,1,1,1,1,1,0,1,1,1,0,1,1,1,1,1},
  {1,0,1,1,1,1,0,1,0,1,1,1,0,1,1,1,1,1,0,1,1,1,0,1,1,1,1,1},
  {1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1},
  {1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

void
init_game(struct game *g)
{
  g->score = 0;
  g->lives = 3;
  g->game_over = 0;
  g->win = 0;
  g->num_ghosts = 3;
  
  g->px = 14;
  g->py = 13;
  g->pdir = RIGHT;
  
  g->ghosts[0][0] = 12; g->ghosts[0][1] = 8; g->ghosts[0][2] = LEFT;
  g->ghosts[1][0] = 14; g->ghosts[1][1] = 8; g->ghosts[1][2] = RIGHT;
  g->ghosts[2][0] = 13; g->ghosts[2][1] = 9; g->ghosts[2][2] = UP;
}

int
can_move(struct game *g, int x, int y)
{
  if(x < 0 || x >= 28 || y < 0 || y >= 20)
    return 0;
  return maze[y][x] != 1;
}

void
move_pacman(struct game *g)
{
  int nx = g->px, ny = g->py;
  
  switch(g->pdir){
    case UP:    ny--; break;
    case DOWN:  ny++; break;
    case LEFT:  nx--; break;
    case RIGHT: nx++; break;
  }
  
  if(can_move(g, nx, ny)){
    g->px = nx;
    g->py = ny;
    // g->score += 10;

    //Eat pellet
    if(maze[ny][nx] == 0){
      maze[ny][nx] = 2;   //mark as eaten
      g->score += 10;
    }
  }
}

// Simple pseudo-random number generator
static int seed = 12345;

int
myrand(void)
{
  seed = seed * 1103515245 + 12345;
  return (seed >> 16) & 0x7FFF;
}

void
move_ghost(struct game *g, int idx)
{
  int *gh = g->ghosts[idx];
  int dirs[] = {0, 1, 2, 3};
  int i, newdir;
  
  for(i = 0; i < 4; i++){
    newdir = dirs[myrand() % 4];
    int nx = gh[0], ny = gh[1];
    
    switch(newdir){
      case UP:    ny--; break;
      case DOWN:  ny++; break;
      case LEFT:  nx--; break;
      case RIGHT: nx++; break;
    }
    
    if(can_move(g, nx, ny)){
      gh[0] = nx;
      gh[1] = ny;
      gh[2] = newdir;
      break;
    }
  }
}

int
check_collision(struct game *g)
{
  int i;
  for(i = 0; i < g->num_ghosts; i++){
    if(g->px == g->ghosts[i][0] && g->py == g->ghosts[i][1]){
      return 1;
    }
  }
  return 0;
}

int
check_win(struct game *g)
{
  int i, j;
  for(i = 0; i < 19; i++){
    for(j = 0; j < 28; j++){
      if(maze[i][j] == 0)
        return 0;
    }
  }
  return 1;
}

void
draw_game(struct game *g)
{
  int x, y;
  int cellW = WIDTH / 28;
  int cellH = HEIGHT / 19;
  
  // Clear screen via syscall
  vgafill(0, 0, 320, 200, EMPTY_COLOR);
  
  // Draw maze
  for(y = 0; y < 19; y++){
    for(x = 0; x < 28; x++){
      if(maze[y][x] == 1){
        vgafill(x * cellW, y * cellH, cellW, cellH, WALL_COLOR);
      } else if(maze[y][x] == 0){// Draw dot if not eaten
        vgafill(x * cellW + cellW/2 - 1, y * cellH + cellH/2 - 1, 2, 2, DOT_COLOR);
      }
      else {
        // Do nothing for other cell types
      }
    }
  }
  
  // Draw Pac-Man (circle)
  int pcx = g->px * cellW + cellW/2;
  int pcy = g->py * cellH + cellH/2;
  int radius = (cellW < cellH ? cellW : cellH) / 2 - 2;
  vgacircle(pcx, pcy, radius, PACMAN_COLOR);
  
  // Draw ghosts
  int i;
  for(i = 0; i < g->num_ghosts; i++){
    int gx = g->ghosts[i][0] * cellW + cellW/2;
    int gy = g->ghosts[i][1] * cellH + cellH/2;
    uchar color = (i == 0) ? GHOST1_COLOR : (i == 1) ? GHOST2_COLOR : GHOST3_COLOR;
    vgacircle(gx, gy, radius - 1, color);
  }

  for(int i = 0; i < g->score / 10 && i < 100; i++){
    vgafill(i * 3, 0, 2, 5, PACMAN_COLOR);
}
}

int
poll_key(void)
{
  return lastkey();
}

void
game_loop(struct game *g)
{
  int running = 1;
  int key;
  
  // Switch to graphics mode via syscall
  vgamode();
  
  while(running){
    draw_game(g);
    
    if(check_win(g)){
      g->win = 1;
      g->game_over = 1;
      break;
    }
    
    // Wait for key
    key = -1;
    while(key == -1){
      key = poll_key();
      sleep(1);
    }
    
    if(key == 'q' || key == 'Q'){
      running = 0;
      break;
    }
    
    switch(key){
      case 'w': case 'W':
        g->pdir = UP;
        break;
      case 's': case 'S':
        g->pdir = DOWN;
        break;
      case 'a': case 'A':
        g->pdir = LEFT;
        break;
      case 'd': case 'D':
        g->pdir = RIGHT;
        break;
      default:
        continue;
    }
    
    // move_pacman(g);
    // move_ghost(g, 0);
    // move_ghost(g, 1);
    // move_ghost(g, 2);

    int hit  = 0;
    move_pacman(g);

    //check before pacman moves
    if(check_collision(g)){
      hit = 1;
    }
    else {
      move_ghost(g, 0);
      move_ghost(g, 1);
      move_ghost(g, 2);

      // Check collision AFTER ghosts move
      if(check_collision(g)){
        hit = 1;
      }
    }

    if(hit){
      g->lives--;

      //handel life lost once
      if(g->lives <= 0){
        g->game_over = 1;
        break;
      }

      //reset positions after life lost
      g->px = 14;
      g->py = 13;
      
      g->ghosts[0][0] = 12; g->ghosts[0][1] = 8;
      g->ghosts[1][0] = 14; g->ghosts[1][1] = 8;
      g->ghosts[2][0] = 13; g->ghosts[2][1] = 9;

    }


    
    // if(check_collision(g)){
     
    //   g->lives--;
      
    //   if(g->lives <= 0){
    //      g->game_over = 1;
    //     break;
    //   }

    //   g->px = 14;
    //   g->py = 13;
    //   g->ghosts[0][0] = 12; g->ghosts[0][1] = 8;
    //   g->ghosts[1][0] = 14; g->ghosts[1][1] = 8;
    //   g->ghosts[2][0] = 13; g->ghosts[2][1] = 9;
    // }
    
  }
  
  // Switch back to text mode (vgamode() does this internally when exiting)
  // For now, just exit - the mode switch happens on next console output
}

int
main(int argc, char *argv[])
{
  struct game g;
  
  printf(1, "Starting Pac-Man (VGA Graphics)...\n");
  printf(1, "Use w/a/s/d to move, q to quit\n");
  printf(1, "You have 3 lives. Good luck!\n");
  printf(1, "Press Enter to start...\n");
  
  char buf[16];
  gets(buf, 16);
  
  init_game(&g);
  game_loop(&g);
  
  if(g.game_over){
    printf(1, "GAME OVER! Final Score: %d\n", g.score);
  } else if(g.win){
    printf(1, "YOU WIN! Final Score: %d\n", g.score);
  }
  
  printf(1, "Thanks for playing!\n");
  exit();
}