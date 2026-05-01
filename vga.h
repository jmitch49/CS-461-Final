/* VGA driver header for xv6 */
#ifndef _VGA_H_
#define _VGA_H_

// Switch to graphics mode (320x200 256-color)
void vgaMode13(void);

// Switch back to text mode
void vgaMode3(void);

// Drawing functions
void vgaPutPixel(int x, int y, uchar color);
void vgaFillRect(int x, int y, int w, int h, uchar color);
void vgaClear(uchar color);
void vgaDrawCircle(int cx, int cy, int r, uchar color);

// Screen dimensions
#define VGA_WIDTH 320
#define VGA_HEIGHT 200

#endif