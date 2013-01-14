//
//  kinectData.h
//  kinectCalibrator
//
//  Created by miguel valero espada on 11/17/12.
//
//
#include "ofMain.h"
#include "constants.h"
#include "tracker.h"

#ifndef kinectCalibrator_kinectData_h
#define kinectCalibrator_kinectData_h

class cloud{
    ofMesh points;
    int id;
    ofColor c;
public:
    cloud(int _id){
        id = _id;
            switch(id % 5){
                case 0:
                    c = ofColor::fromHex(0xE0D0AA);
                    break;
                case 1:
                    c = ofColor::fromHex(0x8DA893);
                    break;
                case 2:
                    c = ofColor::fromHex(0x1DA813);
                    break;
                case 3:
                    c = ofColor::fromHex(0x0DFF192);
                    break;
                case 4:
                    c = ofColor::fromHex(0x0DCF102);
                    break;
                default:
                    c = ofColor::fromHex(0xFFFFFF);
                    break;
            }
    
    
    }
    
    void addPoint(ofVec3f p){
        points.addVertex(p);
    }
    void draw(){
        ofPushStyle();
        ofSetColor(c);
        points.drawVertices();
        ofPopStyle();
    }
    int getNumVertices(){
        return points.getNumVertices();
    }
    int getId(){
        return id;
    }
    
};

class kinectData{
    ofMesh points;
    vector<ofVec3f> coms;
    vector<cloud> meshes;
    ofVec2f center;
    ofVec2f ref;
    
    ofMatrix4x4 M;
    ofColor c;
    int lastData = 0;
    int pFrame = -1;
    
    ofxOscSender *sender;
    string ip;
    int port = -1;
    
public:
    bool bOld = true;
    kinectData(){
        port = -1;
        sender = NULL;
        ip = "";
    }
    void setHost(string _ip, int _port){
        if(port == -1){
            sender = new ofxOscSender();
            ip = _ip;
            port = _port;
            sender->setup(ip, port);
        }
    }
    void setCenter(ofVec2f _c, ofVec2f _r){
        ref = _r;
        center = _c;
    }
    ~kinectData(){
    }
    void sendSaving(int frame){
        if(port != -1){
            ofxOscMessage m;
            m.setAddress("/save");
            m.addIntArg(frame);
            sender->sendMessage(m);
        }
    }
    void sendReset(){
        if(port != -1){
            ofxOscMessage m;
            m.setAddress("/reset");
            sender->sendMessage(m);
        }
    }
    
    void setColor(ofColor _c){
        c = _c;
    }
    void setMatrix(ofMatrix4x4 _M){
        M = _M;
    }
    void addPoint(ofVec3f p, int id){
        p = M.postMult(p);
        bool added = false;
        for (int i = 0; i < meshes.size(); i++){
            if (meshes[i].getId() == id) {
                meshes[i].addPoint(p);
                added = true;
                break;
            }
        }
        if (!added) {
            cloud c(id);
            c.addPoint(p);
            meshes.push_back(c);
        }
    }
    void draw(){
        ofPushStyle();
        ofSetColor(c);
   
        for (vector<cloud>::iterator it = meshes.begin(); it!=meshes.end(); ++it)
            (*it).draw();
        
        
        for (vector<ofVec3f>::iterator it = coms.begin(); it!=coms.end(); ++it){
            ofPushMatrix();
            ofTranslate((*it));
            ofSetColor(255);
            ofSphere(10);
            ofPopMatrix();
        }
        ofPopStyle();
        
    }
    void markAsOld(){
        bOld = true;
    }
    void markAsNew(){
        bOld = false;
    }
    void clearCOM(){
        coms.clear();
    }
    void clearMesh(int frame){
        if(frame != pFrame){
            meshes.clear();
            pFrame = frame;
        }
    }
    void addCOM(ofVec3f c){
        c = M.postMult(c);
        c.y = 0;
        if(coms.size() == 0){
            coms.push_back(c);
            return;
        }
        else{
            for (vector<ofVec3f>::iterator it = coms.begin(); it!=coms.end(); ++it){
                if(c.z < (*it).z){
                    coms.insert(it, c);
                    return;
                }
            }
        }
        coms.push_back(c);
    }
    int getCOMsize() {
        return coms.size();
    }
    ofVec3f getCOM(int i){
        return coms[i];
    }
    void getStatus(char *str, int i){
        char comPos[50];
        char comPosAll[400];
        strcpy(comPosAll, "");
        for(int i = 0; i < N; i++){
            
            ofVec2f pp;
            pp.x = coms[i].x - center.x;
            pp.y = coms[i].z - center.y;            
            if(i < coms.size()){
                sprintf(comPos, "(%4.f, %4.f, %4.f) [%4.f, %4.f]", coms[i].x, coms[i].y, coms[i].z, pp.angle(-ref), pp.length());
                strcat(comPosAll, comPos);
            }
            strcat(comPosAll, "\n");
        }
        int n = 0;
        for (vector<cloud>::iterator it = meshes.begin(); it!=meshes.end(); ++it)
            n += (*it).getNumVertices();
                
        sprintf(str, "[KINECT %1d]: %4d point data\nip: %s port %5d\n%s\n"
                , i, n, ip.c_str(), port, comPosAll);
        
    }
    
   
};
#endif
