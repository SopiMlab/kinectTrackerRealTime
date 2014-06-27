//
//  mlp.h
//  kinectMergerRealTime
//
//  Created by miguel valero espada on 11/24/12.
//
// Load mesh Lab Project

#include "constants.h"

#ifndef kinectMergerRealTime_mlp_h
#define kinectMergerRealTime_mlp_h

class mlp{
private:
    ofxXmlSettings MLP;
    ofxXmlSettings XML;
    
    string matrixFile, path;
    string savingMsg;
    ofMatrix4x4 M[K];
    string kFile[K];
    
    
    ofMatrix4x4 parseMatrix(string s){
        std::vector<float> v;
        std::istringstream iss(s);
        std::copy(std::istream_iterator<float>(iss),
                  std::istream_iterator<float>(),
                  std::back_inserter(v));
        
        return ofMatrix4x4(v[0], v[1], v[2], v[3],
                           v[4], v[5], v[6], v[7],
                           v[8], v[9], v[10], v[11],
                           v[12], v[13], v[14], v[15]);
        
    }
    void openFile(const char* msg, string &s, string &path){
        ofFileDialogResult result = ofSystemLoadDialog(msg);
        string aux = result.getName();
        s = result.getPath();
        path = result.getPath();
        path.erase(path.size() - aux.size(), path.size() - 1);
    }
    
    public:
    void setup(){
        cout << "loading " << "xmlSettings.xml" << endl;
        
        if( XML.loadFile(ofToDataPath("xmlSettings.xml")) ){
            cout << "settings loaded!" << endl;
        }else{
            cout << "unable to load " << "xmlSettings.xml" << " check data/ folder" << endl;
        }
        matrixFile= XML.getValue("MATRIX_FILE", "");
        path= XML.getValue("MATRIX_PATH", "");
        openMeshLabProject();
    }
    
    void getMatrixFile(){
        string oldMatrixFile = matrixFile;
        string oldPath = path;
        openFile("MeshLab mlp file", matrixFile, path);
        if(matrixFile.size() > 0){
            XML.setValue("MATRIX_FILE", matrixFile);
            XML.setValue("MATRIX_PATH", path);
            XML.saveFile("xmlSettings.xml");
        }
        else{
            matrixFile = oldMatrixFile;
            oldPath = path;
        }
    }
    void openMeshLabProject(){
        if (!matrixFile.compare("")) return;
        
        if( MLP.loadFile(ofToDataPath(matrixFile)) ){
            cout << "aligment loaded!" << endl;
        }else{
            cout << "unable to load " << matrixFile << " check data/ folder" << endl;
            return;
        }
        
        MLP.pushTag("MeshLabProject");
        MLP.pushTag("MeshGroup");
        for(int i = 0; i < K; i++){
            kFile[i] =  MLP.getAttribute("MLMesh", "filename", "", i);
            cout << kFile[i] << endl;
            string s = MLP.getValue("MLMesh:MLMatrix44", "", i);
            if(s.size() == 0 ){
                cout << "Incorrect matrix file format " << endl;
                return;
            }
            M[i] = parseMatrix(s);
            
            cout << "Loading matrix... " <<  MLP.getAttribute("MLMesh","label","", i)
            << "\n" << M[i] << endl;
        }
    }

    void openMLP(){
        getMatrixFile();
        openMeshLabProject();
    }
    void getStatus(char *str){
        string s;
        if(matrixFile.size() > 30){
            s = "...";
            s+= matrixFile.substr(matrixFile.size() - 30, matrixFile.size() - 1);
        }
        else
            s = matrixFile;
        sprintf(str, "[ALIGMENT MATRIX]:\n%s\n", s.c_str());
    }
    ofMatrix4x4 getMatrix(int i){
        return M[i];
    }

};

#endif
