// Pac-Man for xv6 - ASCII console version

#include "types.h"
#include "user.h"
#include "fcntl.h"

// Game constants
#define WIDTH 40
#define HEIGHT 20
#define MAX_GHOSTS 4

// Game symbols
#define PACMAN 'C'
#define GHOST 'G'
#define DOT '.'
#define WALL '#'
#define EMPTY ' '
#define POWER 'O'

// Directions
#define UP 0
#define DOWN 1
#define LEFT 2
#define RIGHT 3

// Game state
struct game {
  char board[HEIGHT][WIDTH];
  int px, py;        // Pac-Man position
  int pdir;          // Pac-Man direction
  int score;
  int lives;
  int ghosts[MAX_GHOSTS][3];  // x, y, direction
  int num_ghosts;
  int game_over;
  int win;
};

// Simple map - 1 = wall, 0 = dot
char level1[HEIGHT][WIDTH] = {
  "########################################",
  "#......................................#",
  "#......................................#",
  "#..####..#####....#####....####..####..#",
  "#..#..................................#",
  "#..#....#####....#####....#####..#....#",
  "#......#....#..#....#....#....#..#....#",
  "######.#....#..#....#....#....#.......#",
  "#......#....#..#....#....#....#.......#",
  "#..####....#..#....#....#....#..####..#",
  "#..........#............#.............#",
  "#..####....#..########..#..####..####..#",
  "#..#..................................#",
  "#..#....#####....#####....#####..#....#",
  "#......#....#..#....#....#....#..#....#",
  "######.#....#..#....#....#....#.......#",
  "#......................................#",
  "#......................................#",
  "########################################"
};

void
clear_screen(void)
{
  printf(1, "\033[2J");
  printf(1, "\033[H");
}

void
init_game(struct game *g)
{
  int i, j;
  
  g->score = 0;
  g->lives = 3;
  g->game_over = 0;
  g->win = 0;
  g->num_ghosts = 3;
  
  // Initialize board from level
  for(i = 0; i < HEIGHT; i++){
    for(j = 0; j < WIDTH; j++){
      g->board[i][j] = level1[i][j];
    }
  }
  
  // Pac-Man starting position
  g->px = 20;
  g->py = 15;
  g->pdir = RIGHT;
  
  // Ghosts starting positions
  g->ghosts[0][0] = 19; g->ghosts[0][1] = 10; g->ghosts[0][2] = LEFT;
  g->ghosts[1][0] = 20; g->ghosts[1][1] = 10; g->ghosts[1][2] = RIGHT;
  g->ghosts[2][0] = 19; g->ghosts[2][1] = 11; g->ghosts[2][2] = UP;
}

int
can_move(struct game *g, int x, int y)
{
  if(x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT)
    return 0;
  return g->board[y][x] != WALL;
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
    
    // Eat dot
    if(g->board[ny][nx] == DOT){
      g->board[ny][nx] = EMPTY;
      g->score += 10;
    } else if(g->board[ny][nx] == POWER){
      g->board[ny][nx] = EMPTY;
      g->score += 50;
    }
  }
}

// Simple pseudo-random number generator
static int seed = 12345;

int
rand(void)
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
  
  // Simple random movement
  for(i = 0; i < 4; i++){
    newdir = dirs[rand() % 4];
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
  
  for(i = 0; i < HEIGHT; i++){
    for(j = 0; j < WIDTH; j++){
      if(g->board[i][j] == DOT || g->board[i][j] == POWER)
        return 0;
    }
  }
  return 1;
}

void
draw_game(struct game *g)
{
  int i, j;
  char display[HEIGHT][WIDTH];
  
  // Copy board to display
  for(i = 0; i < HEIGHT; i++){
    for(j = 0; j < WIDTH; j++){
      display[i][j] = g->board[i][j];
    }
  }
  
  // Draw Pac-Man
  display[g->py][g->px] = PACMAN;
  
  // Draw ghosts
  for(i = 0; i < g->num_ghosts; i++){
    display[g->ghosts[i][1]][g->ghosts[i][0]] = GHOST;
  }
  
  // Clear and draw
  printf(1, "\033[2J");
  printf(1, "\033[H");
  printf(1, "=== PAC-MAN xv6 ===\n");
  printf(1, "Score: %d  Lives: %d\n", g->score, g->lives);
  printf(1, "Controls: w=up, s=down, a=left, d=right, q=quit\n\n");
  
  for(i = 0; i < HEIGHT; i++){
    for(j = 0; j < WIDTH; j++){
      printf(1, "%c", display[i][j]);
    }
    printf(1, "\n");
  }
  
  if(g->game_over){
    printf(1, "\nGAME OVER! Final Score: %d\n", g->score);
  } else if(g->win){
    printf(1, "\nYOU WIN! Final Score: %d\n", g->score);
  }
}

int
read_key(void)
{
  char c;
  int n;
  
  n = read(0, &c, 1);
  if(n > 0){
    return c;
  }
  return -1;
}

void
game_loop(struct game *g)
{
  int running = 1;
  int key;
  
  while(running){
    // Draw the game
    draw_game(g);
    
    // Check win condition
    if(check_win(g)){
      g->win = 1;
      g->game_over = 1;
      draw_game(g);
      break;
    }
    
    // Check collision with ghosts
    if(check_collision(g)){
      g->lives--;
      if(g->lives <= 0){
        g->game_over = 1;
        draw_game(g);
        break;
      }
      // Reset positions
      g->px = 20;
      g->py = 15;
      g->ghosts[0][0] = 19; g->ghosts[0][1] = 10;
      g->ghosts[1][0] = 20; g->ghosts[1][1] = 10;
      g->ghosts[2][0] = 19; g->ghosts[2][1] = 11;
    }
    
    // Read keyboard input (blocking)
    printf(1, "\nMove (w/a/s/d or q): ");
    key = read_key();
    
    if(key == 'q' || key == 'Q'){
      running = 0;
      break;
    }
    
    // Update direction based on input
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
    }
    
    // Move Pac-Man
    move_pacman(g);
    
    // Move ghosts
    move_ghost(g, 0);
    move_ghost(g, 1);
    move_ghost(g, 2);
    
    // Small delay to make game playable
    sleep(1);
  }
}

int
main(int argc, char *argv[])
{
  struct game g;
  
  printf(1, "Starting Pac-Man...\n");
  printf(1, "Use w/a/s/d to move, q to quit\n");
  printf(1, "Press Enter to start...\n");
  
  // Wait for enter
  char buf[16];
  gets(buf, 16);
  
  init_game(&g);
  game_loop(&g);
  
  printf(1, "Thanks for playing!\n");
  exit();
}