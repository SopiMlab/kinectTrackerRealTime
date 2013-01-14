//
//  calibratorModeCalibrateFilm.h
//  kinectCalibrator
//
//  Created by miguel valero espada on 11/16/12.
//
//

//
//  calibratorModeCalibrate.h
//  kinectCalibrator
//
//  Created by miguel valero espada on 11/16/12.
//
//
#include <iterator>
#include <iostream>
#include "testApp.h"
#include "ofxObjLoader.h"
#include "ofxXmlSettings.h"
#include "calibratorMode.h"
#include "ofxFileDialogOSX.h"

class calibratorModeCalibrate : public calibratorMode {
private:
    
public:
    void setup(){
    }
    void update(){
    }
    void draw(){
        
    }
    ofVec3f getCentroid(){
    }
    void stop(){
        
    }
    void keyPressed(int key){
        
    }
    void getStatus(char *str){
        sprintf(str, "Sequence:");
    }
    
};



