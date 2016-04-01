#include "ofMain.h"
#include "ofApp.h"
#include "ofGuiApp.h"

//========================================================================
int main( ){
//	ofSetupOpenGL(1280,900,OF_WINDOW);
//	ofRunApp(new ofApp());
    
    ofGLWindowSettings settings;
    settings.width = 1280;
    settings.height = 900;
    settings.setPosition(ofVec2f(300,0));
    shared_ptr<ofAppBaseWindow> mainWindow = ofCreateWindow(settings);
    
    settings.width = 300;
    settings.height = 500;
    settings.setPosition(ofVec2f(0,0));
    shared_ptr<ofAppBaseWindow> guiWindow = ofCreateWindow(settings);
    
    shared_ptr<ofApp> mainApp(new ofApp);
    shared_ptr<ofGuiApp> guiApp(new ofGuiApp);

    mainApp->gui = guiApp;

    ofRunApp(guiWindow, guiApp);
    ofRunApp(mainWindow, mainApp);
    ofRunMainLoop();
}
