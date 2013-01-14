//
//  calibratorModeMerge.h
//  kinectMerger
//
//  Created by miguel valero espada on 11/18/12.
//
//

#ifndef kinectMerger_calibratorModeMerge_h
#define kinectMerger_calibratorModeMerge_h

//
//  calibratorModeCalibrate.h
//  kinectCalibrator
//
//  Created by miguel valero espada on 11/16/12.
//
//

#include "ofMain.h"


class calibratorModeMerge : public calibratorMode {
private:
    ofxOscReceiver receiver;
    int current_msg_string;
    string msg_strings[NUM_MSG_STRINGS];
    float timers[NUM_MSG_STRINGS];
    
    
public:
    void setup(){
        cout << "listening for osc messages on port " << PORT << "\n";
        receiver.setup(PORT);
        while(receiver.hasWaitingMessages()){
            ofxOscMessage m;
            receiver.getNextMessage(&m);
        }

    }
    
    void update(){
        
        while(receiver.hasWaitingMessages()){
            ofxOscMessage m;
            receiver.getNextMessage(&m);
            
            if(m.getAddress() == "/com"){
                int kinectID =  m.getArgAsInt32(0);
                string s = m.getArgAsString(1);
                cout << s << endl;
            }
        }

    }
    void draw(){
               
    }
    ofVec3f getCentroid(){
    }
    void stop(){
        
    }
    void continousKeyPress(bool keys[255]){
       
        
    }
  
    void keyPressed(int key){
              
    }
    void getStatus(char *str){
        sprintf(str, "MERGER");
    }
    
};





#endif
