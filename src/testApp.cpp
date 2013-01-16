#include "testApp.h"
#include "ofxSimpleGuiToo.h"


bool keys[255];
char oscStatus[255];

float camZoom, camPosX, camPosY, camRotX, camRotY;
ofVec3f centroid;
int pMouseX, pMouseY;

ofxSimpleGuiTitle *status;
ofxSimpleGuiToggle *calibratedButton;
bool bLoadMLP;
int frame;

ofFile logFile;
int state;

bool bSetCenter, bSetRefPoint, bSetGhost0, bSetGhost1;


ofxXmlSettings XML;
//--------------------------------------------------------------
void testApp::setup(){
    
    ofBackground(0);
    ofSetFrameRate(60);
    
    int PORT = 12000;
    int REMOTE_PORT = 12000;
    string REMOTE_HOST = "169.254.0.1";
    
	receiver.setup(PORT);
    sender.setup(REMOTE_HOST, REMOTE_PORT);
    
    sprintf(oscStatus, "[LOCALPORT] %d\n[REMOTE PORT] (%s, %5d)\n", PORT, REMOTE_HOST.c_str(), REMOTE_PORT);
    
    gui.setup();
	gui.addTitle("TRACKER \n[i] hide controls");
    gui.addToggle("TRACK", bTracking).setSize(200, 20);
    calibratedButton = &gui.addToggle("CALIBRATED", bCalibrated);
    calibratedButton->setSize(200, 20);
    gui.addToggle("SAVE", bSaving).setSize(200, 20);
    gui.addButton("Load MeshLab File", bLoadMLP).setSize(200, 20);
    gui.addButton("Reset Server Counter", bReset).setSize(200, 20);

    gui.addSlider("Zoom", camZoom, -5000, 5000).setSmoothing(0.9);
    gui.addSlider("camPosX", camPosX, -200, 200).setSmoothing(0.9);
    gui.addSlider("camPosY", camPosY, -200, 200).setSmoothing(0.9);
    gui.addSlider("camRotX", camRotX, 0, 360).setSmoothing(0.9);
    gui.addSlider("camRotY", camRotY, 0, 360).setSmoothing(0.9);
    gui.addToggle("Lock top view", bTop).setSize(200, 20);
    gui.addToggle("Enable ghost 1", bGhost0).setSize(200, 20);
    gui.addToggle("Enable ghost 2", bGhost1).setSize(200, 20);
    
    
    gui.addButton("Set Center", bSetCenter).setSize(200, 20);
    gui.addButton("Set Ref Point", bSetRefPoint).setSize(200, 20);
    gui.addButton("Set Ghost 1", bSetGhost0).setSize(200, 20);
    gui.addButton("Set Ghost 2", bSetGhost1).setSize(200, 20);
    status = &gui.addTitle("STATUS");
    status->setNewColumn(true);
    
	gui.loadFromXML();
    gui.show();
    
    matrixData.setup();
    
    // LOAD space references
    
    XML.loadFile(ofToDataPath("xmlSettings.xml"));
    int cX = XML.getValue("CENTER_X", 0);
    int cZ = XML.getValue("CENTER_Z", 0);
    int rX = XML.getValue("REF_X", 0);
    int rZ = XML.getValue("REF_Z", 0);
    
    center = ofVec3f(cX, 0, cZ);
    refPoint = ofVec3f(rX, 0,  rZ);
    refVector = center - refPoint;

    for(int i = 0; i < K; i++){
        kinects[i].setMatrix(matrixData.getMatrix(i));
        kinects[i].setCenter(center, refVector);
    }
    
    // LOAD GHOSTS
    int gX, gZ;
    
    gX = XML.getValue("GHOST_0_X", 0);
    gZ = XML.getValue("GHOST_0_Z", 0);
    ghost0 = ofVec3f(gX, 0, gZ);
    
    gX = XML.getValue("GHOST_1_X", 0);
    gZ = XML.getValue("GHOST_1_Z", 0);
    ghost1 = ofVec3f(gX, 0, gZ);
    
    cout << "GHOSTs positions... " << endl;
    cout << ghost0 << endl;
    cout << ghost1 << endl;
    //-----
    
    
    bTracking = false;
    bCalibrated = false;
    bSaving = false;
    pbSaving = false;
    bReset = false;
    bTop = false;
    
    
    // Purge old osc messages
    while(receiver.hasWaitingMessages()){
		ofxOscMessage m;
		receiver.getNextMessage(&m);
    }

  
    logFile.open("log.txt", ofFile::WriteOnly, false);
}

//--------------------------------------------------------------
void testApp::update(){
    for(int i = 0; i < K; i++)
        kinects[i].markAsOld();
    
    processOSC();

    //-------------------------
    
    if(bLoadMLP){
        bLoadMLP = false;
        matrixData.openMLP();
        for(int i = 0; i < K; i++)
            kinects[i].setMatrix(matrixData.getMatrix(i));
    }
    
    //-------------------------
    
    if(bTracking){
        if(!bCalibrated) {
            bCalibrated = match.startTracking(kinects, trackers);
        }
        else{
            for(int i = 0; i < N; i++)
                trackers[i].resetCandidates();
            
            match.matchCandidates(kinects, trackers);
            
            for(int i = 0; i < N; i++)
                trackers[i].match();
        }
    }
    else{
        bCalibrated = false;
    }
    
 
    //----
    sendDistances();
    sendPositions();
    sendAzimuts();
    
    if(bReset){
        sendReset();
        bReset = false;
    }
    if(bSaving){
        if(!pbSaving){
            frame = 0;
            pbSaving = true;
        }
        sendSaving(frame);
        frame += 1;
    }
    else{
        pbSaving = false;
    }
        
        
    if(ofGetFrameNum() % 30 == 0)
        sendPing();
    
    
    //----- SCREEN INFO
    

    char msg[2048];
    
    matrixData.getStatus(msg);
    strcat(msg, oscStatus);
    
    char kinectMsg[500];
    for(int i = 0; i < K; i++){
        kinects[i].getStatus(kinectMsg, i);
        strcat(msg, kinectMsg);
    }
    char other[500];
    sprintf(other, "\n[CENTER] %4.f, %4.f\n[REF POINT] %4.f, %4.f ", center.x, center.z, refPoint.x, refPoint.z);
    strcat(msg, other);
    
    sprintf(other, "\n[GHOST 1] %4.f, %4.f\n[GHOST 2] %4.f, %4.f ", ghost0.x, ghost0.z, ghost1.x, ghost1.z);
    strcat(msg, other);
    
    float delta[3];
    int n = 0;

     if(bTracking && bCalibrated){
        for(int i = 0; i < N; i++){
            sprintf(other, "\n[TRACKER %d] %4.f %4.f %4.f", i, trackers[i].pos.x,
                    trackers[i].pos.y, trackers[i].pos.z);
            strcat(msg, other);
            
        }
    }
    sprintf(other, "\n[STATE] %1.d", state);
    strcat(msg, other);
    
    strcat(msg, "\n\nPRESS AND DRAG:\n'1' cam zoom '2' cam x/y '2' cam rot\n");
    strcat(msg, "PRESS:\n' ' to start tracking\n");

    status->setName(msg);
    status->setSize(300, 400);
    calibratedButton ->setValue(bCalibrated);
    
    //----- 
    // ...
    //writeLog();
    
       
}

//--------------------------------------------------------------
void testApp::draw(){
    centroid = center;
    cam.setPosition(ofVec3f(0, 0, -centroid.z));
    
    cam.lookAt(centroid, ofVec3f(0,1,0));
    cam.setFarClip(50000);
    
    cam.begin();
    
    ofPushMatrix();
    ofTranslate(camPosX, camPosY, camZoom);
    
    if(bTop){
        pivot(centroid, 110, 0, 0);
    }
    else{
        pivot(centroid, camRotX, camRotY, 0);
    }
    
    ofScale(-1.0, 1.0, 1.0);
    drawAxes(centroid, refPoint);
    
    ofPushStyle();
    
    //-------------------------
    for(int i = 0; i < K; i++)
        kinects[i].draw();
    
    
    if(bCalibrated && bTracking){
        for(int i = 0; i < N; i++)
            trackers[i].draw();
        
        for(int i = 0; i < N - 1; i++)
            for(int j = 1; j < N; j++){
                setLineColor(i + j);
                ofLine(trackers[i].lerpedPos, trackers[j].lerpedPos);
            }
        
        //-------------------------
        ofEnableAlphaBlending();
        ofSetColor(255, 0, 0, 50);
        ofFill();
        ofBeginShape();
        for(int i = 0; i < N; i++)
            ofVertex(trackers[i].lerpedPos);
        ofEndShape();
        ofDisableAlphaBlending();
        //-------------------------
    }
    
    ofPopStyle();
   

    ofPopMatrix();

    cam.end();
    
    gui.draw();    
}

void testApp::keyPressed(int key){
    
    switch (key ) {
        case 'i':
            gui.toggleDraw();
            break;
        case ' ':
            bTracking = !bTracking;
            gui.toggleDraw();
            break;
            
       
       /*case 'z':
            match.swap(trackers, 0, 1);
            break;
        case 'x':
            match.swap(trackers, 0, 2);
            break;
        case 'c':
            match.swap(trackers, 1, 2);
            break;*/
     
        default:
            break;
    }
    if(key > 0 && key <255)
        keys[key] = true;
    
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){
   
    if(key > 0 && key <255)
        keys[key] = false;
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
    if(keys['1']) camZoom -=  (mouseX - pMouseX) * 3;
    if(keys['2']) {
        camPosX -=  (mouseX - pMouseX) *2;
        camPosY -=  (mouseY - pMouseY) *2;
    }
    if(keys['3']) {
        camRotY -=  (mouseX - pMouseX) / 10.;
        camRotX -=  (mouseY - pMouseY) / 10.;
    }
    
    pMouseX = mouseX;
    pMouseY = mouseY;

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
    pMouseX = mouseX;
    pMouseY = mouseY;

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 

}

void testApp::processOSC(){
   
	while(receiver.hasWaitingMessages()){
        
        char *data;
		ofxOscMessage m;
        receiver.getNextMessage(&m);
        
        if(m.getAddress() == "/ping"){
            int _k = m.getArgAsInt32(0);
            int port = m.getArgAsInt32(1);
            kinects[_k].setHost(m.getRemoteIp(), port);
        }
		else if(m.getAddress() == "/pc"){
            int _k = m.getArgAsInt32(0);
            int f = m.getArgAsInt32(4);
            kinects[_k].clearMesh(f);
            if(_k < K){
                unsigned long l;
                int id = m.getArgAsInt32(1);
                data = m.getArgAsBlob(2, l);
                int d = m.getArgAsInt32(3);
                for(int i = 0; i < d * 6; i += 6){
                    ofVec3f p;
                    p.x = ((short)data[i + 1] << 8) | ((short)data[i] & 0xFF);
                    p.y = ((short)data[i + 3] << 8) | ((short)data[i + 2] & 0xFF);
                    p.z = ((short)data[i + 5] << 8) | ((short)data[i + 4] & 0xFF);
                    kinects[_k].addPoint(p, id);
                }
            }
            
		}
        
		if(m.getAddress() == "/com"){
            int _k = m.getArgAsInt32(0);
            
            kinects[_k].clearCOM();
            kinects[_k].markAsNew();
            
            string s = m.getArgAsString(1);
            vector<string> tokens =ofSplitString(s, ",");
            
            for (vector<string>::iterator it = tokens.begin(); it!=tokens.end(); ++it) {
                vector<string>comData = ofSplitString(*it, " ");
                if(comData.size() == 5){
                    ofVec3f pos;
                    
                    pos.x = ofToFloat(comData[2]);
                    pos.y = ofToFloat(comData[3]);
                    pos.z = ofToFloat(comData[4]);
                    kinects[_k].addCOM(pos);
                }
            }
            
            // Set references COM0 from Kinect0
            if(kinects[0].getCOMsize() > 0){
                if(bSetCenter) {
                    center = kinects[0].getCOM(0);
                    XML.setValue("CENTER_X", center.x);
                    XML.setValue("CENTER_Z", center.z);
                    XML.saveFile("xmlSettings.xml");
                }
                if(bSetRefPoint) {
                    refPoint = kinects[0].getCOM(0);
                    XML.setValue("REF_X", refPoint.x);
                    XML.setValue("REF_Z", refPoint.z);
                    XML.saveFile("xmlSettings.xml");
                }
                if(bSetGhost0) {
                    ghost0 = kinects[0].getCOM(0);
                    XML.setValue("GHOST_0_X", ghost0.x);
                    XML.setValue("GHOST_0_Z", ghost0.z);
                    XML.saveFile("xmlSettings.xml");
                }
                if(bSetGhost1) {
                    ghost1 = kinects[0].getCOM(0);
                    XML.setValue("GHOST_1_X", ghost1.x);
                    XML.setValue("GHOST_1_Z", ghost1.z);
                    XML.saveFile("xmlSettings.xml");
                }
            }

            
            if(_k == 0){ //Add ghost test users
                if(bGhost0) kinects[0].addCOM(ghost0);
                if(bGhost1) kinects[0].addCOM(ghost1);
                
            }

            

        }
	}
    
}

void testApp::setLineColor(int i){
    switch(i){
        case 1:
            ofSetHexColor(0xFF0000);
            break;
        case 2:
            ofSetHexColor(0x00FF00);
            break;
        case 3:
            ofSetHexColor(0x0000FF);
            break;
        default:
            ofSetHexColor(0xFFFFFF);
            break;
    }
}
void testApp::drawAxes(ofVec3f centroid, ofVec3f ref){
    ofPushMatrix();
    ofTranslate(centroid);
    ofPushStyle();
    ofSetColor(255, 0, 0);
    ofLine(0, 0, 0, 200, 0, 0);
    
    ofSetColor(0, 255, 0);
    ofLine(0, 0, 0, 0, 200, 0);
    
    ofSetColor(0, 0, 255);
    ofLine(0, 0, 0, 0, 0, 200);
    
    ofSetColor(255, 255, 255);
    ofLine(0, 0, ref.x, ref.z);
    ofPopStyle();
    ofPopMatrix();
}

void testApp::pivot(ofVec3f center, float aX, float aY, float aZ){
    ofTranslate(center);
    ofRotateX(aX);
    ofRotateY(aY);
    ofRotateZ(aZ);
    ofTranslate(-center);
}
void testApp::sendPing(){
    ofxOscMessage m;
    m.setAddress("/ping");
    sender.sendMessage(m);
}

void testApp::sendPositions(){
    
    ofxOscMessage m;
    m.setAddress("/speed");
    for(int i = 0; i < N; i++){
        ofVec3f delta = trackers[i].lerpedPos - trackers[i].pLerpedPos;
        m.addFloatArg(abs(delta.length()));
    }
    
    sender.sendMessage(m);
}



void testApp::sendAzimuts() {
    
    ofxOscMessage m;
    m.setAddress("/azimuts");
    for(int i = 0; i < N; i++){
        ofVec3f v(trackers[i].lerpedPos.x - center.x, 0, trackers[i].lerpedPos.z - center.z);
        float angle  = v.angle(-refVector);
        m.addFloatArg(angle);
        m.addFloatArg(v.length());
    }
    sender.sendMessage(m);
    
}

void testApp::sendDistances() {
    
        float delta[N];
        int n = 0;
        for(int i = 0; i < N - 1; i++)
            for(int j = i + 1; j < N; j++){
                delta[n] = trackers[i].lerpedPos.distance(trackers[j].lerpedPos) -
                   trackers[i].pLerpedPos.distance(trackers[j].pLerpedPos) ;
                n ++;
            }
        
    int b;
    
    ofxOscMessage m;
        m.setAddress("/behaviour");
        bool cohesion = true;
        bool separation = true;
        for(int i = 0; i < n; i++){
            cohesion = cohesion && (delta[i] <= -2);
            separation = separation && (delta[i] >= 2);
        }
        if(cohesion) b = 1;
        else if(separation) b = -1;
        else b = 0;
        m.addIntArg(b);
        sender.sendMessage(m);
    
    m.clear();
    m.setAddress("/distances");
    for(int i = 0; i < N - 1; i++)
        for(int j = i + 1; j < N; j++){
            float d = trackers[i].lerpedPos.distance(trackers[j].lerpedPos);
            m.addFloatArg(d);
        }
    sender.sendMessage(m);
}
void testApp::sendSaving(int frame){
    for(int i = 0; i < K; i++)
        kinects[i].sendSaving(frame);
    
}
void testApp::sendReset(){
    for(int i = 0; i < K; i++)
        kinects[i].sendReset();
    
}

void testApp::writeLog(){
    //-------------------------
    // Writing machine learning data
    if(bTracking && bCalibrated){
        state = 0; // state is used for machine learning mark-up
        if(keys['1']) state = 1;
        if(keys['2']) state = 2;
       // if(keys['3']) state = 3;
       
        
       logFile << trackers[0].lerpedPos.x << " " << trackers[0].lerpedPos.z << " "
        << trackers[1].lerpedPos.x << " " << trackers[1].lerpedPos.z << " "
        << trackers[2].lerpedPos.x << " " << trackers[2].lerpedPos.z << " " << state << endl;
        
        cout << trackers[0].lerpedPos.x << " " << trackers[0].lerpedPos.z << " "
        << trackers[1].lerpedPos.x << " " << trackers[1].lerpedPos.z << " "
        << trackers[2].lerpedPos.x << " " << trackers[2].lerpedPos.z << " " << state << endl;
        logFile.flush();
    }
}