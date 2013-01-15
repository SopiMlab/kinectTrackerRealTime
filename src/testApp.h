#pragma once

#include "ofMain.h"
#include "ofxOsc.h"
#include "merger.h"
#include "mlp.h"
#include "tracker.h"
#include "constants.h"
#include "matcher.h"


class testApp : public ofBaseApp{
  public:
    void setup();
    void update();
    void draw();
    
    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y);
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    void processOSC();
    
    ofCamera cam;
    merger merger;
    mlp matrixData;
    
    ofxOscReceiver receiver;
    ofxOscSender sender;

    
    kinectData kinects[K];
    tracker trackers[N];
    matcher match;
    bool bTracking, bCalibrated, bSaving, pbSaving, bReset, bTop;
    bool bGhost0, bGhost1;
    
     
    void setLineColor(int i);
    void drawAxes(ofVec3f centroid, ofVec3f refVector);
    
    void pivot(ofVec3f center, float aX, float aY, float aZ);
    void sendPing();
    void sendDistances();
    void sendPositions();
    void sendAzimuts();
    void sendSaving(int frame);
    void sendReset();
    void writeLog();
    
    // For calculating the azimuts
    ofVec3f center, refPoint, refVector;
    
    
    // Positions of ghosts users
    ofVec3f ghost0, ghost1;
};
