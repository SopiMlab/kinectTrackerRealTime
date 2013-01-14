//
//  tracker.h
//  kinectMergerRealTime
//
//  Created by miguel valero espada on 11/24/12.
//
//

#include "constants.h"

#ifndef kinectMergerRealTime_tracker_h
#define kinectMergerRealTime_tracker_h

class tracker{
public:
    vector<ofVec3f> candidates;
    ofVec3f pos;
    ofVec3f lerpedPos;
    ofVec3f pLerpedPos;
    float smoothFactor=0.2;
    ofVec3f v;
   
    void resetCandidates(){
        candidates.clear();
    }
    
    void lerp() {
        pLerpedPos = lerpedPos;
        lerpedPos.interpolate(pos, smoothFactor);
        v = lerpedPos - pLerpedPos;
    }
    
    void draw(){
        ofPushMatrix();
        ofSetColor(255, 0, 255);
        ofTranslate(pos);
        ofSphere(10);
        ofPopMatrix();
    }
    
    void addCandidate(ofVec3f c){
        candidates.push_back(c);
        
    }
    bool match() {
        ofVec3f *closest = NULL;
        
        for (vector<ofVec3f>::iterator it = candidates.begin(); it!=candidates.end(); ++it){
            if (closest == NULL) closest = &(*it);
            else if (pos.distance(*it) < pos.distance(*closest))
                closest = &(*it);
        }
        if(closest == NULL) return;
        
        pos.x = (*closest).x;
        pos.y = (*closest).y;
        pos.z = (*closest).z;
        lerp();
    }
    
    
    
};

#endif
