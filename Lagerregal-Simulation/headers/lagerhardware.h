#ifndef LAGERHARDWARE_H_
#define LAGERHARDWARE_H_

typedef struct {
	signed long x,y,z;
	signed long pin_x,pin_y,pin_z;
	signed long dist_x,dist_yu,dist_yo,dist_z;
	signed long x_speed,x_speed_fast,y_speed,z_speed;
} regal_status;

#endif
