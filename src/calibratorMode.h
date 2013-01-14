//
//  calibratorMode.h
//  kinectCalibrator
//
//  Created by miguel valero espada on 11/16/12.
//
//
#include "ofMain.h"

#ifndef kinectCalibrator_calibratorMode_h
#define kinectCalibrator_calibratorMode_h

class calibratorMode{
public:
    virtual void setup() = 0;
    virtual void update() = 0;
    virtual void draw() = 0;
    virtual ofVec3f getCentroid(){return ofVec3f(0, 0, 0);};
    virtual void stop() = 0;
    virtual void keyPressed(int key) {};
    virtual void getStatus(char *str) {sprintf(str, "NO MODE");}
    virtual void continousKeyPress(bool keys[]) {};
};
#endif