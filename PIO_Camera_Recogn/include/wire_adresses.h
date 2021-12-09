#ifndef WIRE_ADRESSES_H
#define WIRE_ADRESSES_H

int addr_cam_green = 0x00;  // len =          / 0x53   tl:f|tr:f|dl:f|dr:f
int addr_cam_green_len = 19;

int addr_cam_line  = addr_cam_green + addr_cam_green_len;  // len =          / 0x56   +00 /XX \XX |XX L00 R00 T00 L|0 R|0
int addr_cam_line_len = 5;

int addr_addr = 0x40;

#endif
