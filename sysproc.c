#include "types.h"
#include "x86.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return proc->pid;
}

addr_t
sys_sbrk(void)
{
  addr_t addr;
  addr_t n;

  argaddr(0, &n);
  addr = proc->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(proc->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

// Return and clear the last key pressed (non-blocking)
addr_t sys_lastkey(void)
{
  extern volatile int last_key;
  int k = last_key;
  last_key = -1;
  return k;
}

// VGA syscalls - these run in kernel mode and can access VGA memory

// Switch to VGA graphics mode (mode 13h: 320x200 256-color)
addr_t sys_vgamode(void)
{
  extern void vgaMode13(void);
  vgaMode13();
  return 0;
}

// Test if VGA memory is accessible
addr_t sys_vgaptest(void)
{
  extern int vgaMemTest(void);
  return vgaMemTest();
}

// Fill a rectangle in VGA memory
addr_t sys_vgafill(void)
{
  int x, y, w, h, color;
  extern void vgaKernelFillRect(int x, int y, int w, int h, uchar color);
  
  if(argint(0, &x) < 0) return -1;
  if(argint(1, &y) < 0) return -1;
  if(argint(2, &w) < 0) return -1;
  if(argint(3, &h) < 0) return -1;
  if(argint(4, &color) < 0) return -1;
  
  vgaKernelFillRect(x, y, w, h, (uchar)color);
  return 0;
}

// Draw a circle in VGA memory
addr_t sys_vgacircle(void)
{
  int cx, cy, r, color;
  extern void vgaKernelDrawCircle(int cx, int cy, int r, uchar color);
  
  if(argint(0, &cx) < 0) return -1;
  if(argint(1, &cy) < 0) return -1;
  if(argint(2, &r) < 0) return -1;
  if(argint(3, &color) < 0) return -1;
  
  vgaKernelDrawCircle(cx, cy, r, (uchar)color);
  return 0;
}
