#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main( ){
//
//    ofGLWindowSettings settings;
//    settings.width = 1280;
//    settings.height = 900;
//    settings.setPosition(ofVec2f(300,0));
//    shared_ptr<ofAppBaseWindow> mainWindow = ofCreateWindow(settings);
//
//    
//    shared_ptr<ofApp> mainApp(new ofApp);
//    
//    ofRunApp(mainWindow, mainApp);
    ofSetupOpenGL(1280, 900, OF_WINDOW);
    ofRunApp(new ofApp());

}
