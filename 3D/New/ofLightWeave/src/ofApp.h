#pragma once

#include "ofMain.h"
#include "ofxGenericDmx.h"
#include "ofxJSON.h"
#include "ofxAssimpModelLoader.h"
#include "ofxNetwork.h"
#include "ofxThread.h"
#include "ofxMailUtils.h"
#include "ofArduino.h"
#include "ofxOpenCv.h"
#include "IPVideoGrabber.h"
#include "ofxFlyCamera.hpp"


#define DMX_DATA_LENGTH 494

class ofApp : public ofBaseApp{

	public:
        void setup();
        void update();
        void draw();
        void exit();
        void keyPressed(int key);
        void keyReleased(int key);
        void mouseMoved(int x, int y );
        void mouseDragged(int x, int y, int button);
        void mousePressed(int x, int y, int button);
        void mouseReleased(int x, int y, int button);
        void mouseEntered(int x, int y);
        void mouseExited(int x, int y);
        void windowResized(int w, int h);
        void dragEvent(ofDragInfo dragInfo);
        void gotMessage(ofMessage msg);


        //shared_ptr<ofGuiApp> gui;
    
        char message[256];
        char formName[128];
        int  formNum;
    

    private:
        ofCamera cam;

        /* VISION CODE */
        bool bLearnBackground;
        int 				threshold;

        ofVideoGrabber vidGrabber;
        ofxCvColorImage colorImg;
        ofxCvGrayscaleImage grayImage, grayBg, grayDiff;
        ofxCvContourFinder contourFinder;
    
        /* SWARM CODE */
        float bRed = 135.0;
        float bGreen = 206.0;
        float bBlue = 250.0;
        ofColor backgroundColor = ofColor(135.0, 206.0, 250.0);
        float backgroundLevel = 0.0;
        bool backgroundGrowing = true;
        ofVec2f swarmPosition = ofVec2f(310, 45);
        ofVec2f swarmVector = ofVec2f(1.0, 0.0);

        int camWidth;
        int camHeight;

        /* OLD CODE */
    
        enum AnimationState { ACTIVATED, DEACTIVATED, EVENT };
        AnimationState ANIMATION_STATE = DEACTIVATED;
        float soundDecay = 0.989;
        ofxUDPManager udpConnectionBroadcast;
        ofxUDPManager moduleConnections[3];
        int eventPosition;
        float micLevelsTop[11];

    
        ofVec2f absColumnPositionTop[11] = {
            ofVec2f(300, 720),
            ofVec2f(370, 720),
            ofVec2f(440, 720),
            ofVec2f(510, 720),
            ofVec2f(580, 720),
            ofVec2f(650, 720),
            ofVec2f(720, 720),
            ofVec2f(790, 720),
            ofVec2f(860, 720),
            ofVec2f(930, 720),
            ofVec2f(1000, 720)};
    
        float gaussianBottom[1280] = {0.0};
    
        ofColor eventColor = ofColor(255.0,255.0,255.0);
        ofPolyline lineGraph;
        DmxDevice* dmxInterface_;
        unsigned char dmxData_[DMX_DATA_LENGTH];
        float red, green, blue;
        float trigPos = 0.0;
    
        int curWidth = 1280;
        int curHeight = 900;
        float decayRate = 0.99f;
        float growthRate = 1.0001f;
        float aGrowthRate = 3.0f;
        float bAccel = 0.99;
        float pulseHeightLeft = 1.01f;
        float pulseHeightRight = 1.01f;
        float ppulseGrowth = 19.0;
        float pulseDecay = 0.5f;
        bool ambientGrowing = true;
        float trainGrowthRate = 1.051f;
        float trainDecay = 0.95;
        float ambientLevel = 51.0;
        int eventPos = 0;
        float eventLevel = 255.0;
    
    
        struct Swarm {
            float a;
            float b;
            float c;
            float bVel;
            float curve[1280];
        };
    
        Swarm bottomSwarm;

        ofLight pointLight;
        ofColor lightColor;
        float colorHue;
        ofColor materialColor;
        ofMaterial material;
    
    
        ofxJSONElement result;
        ofxJSONElement columnGeometry;
        ofxJSONElement faceGeometry;
        ofImage backgroundImage;
        void sendToDMX();
        void newDrawRegion(float gaussLevels[1280], int start, int end, bool isEvent);
        void drawCars();
        void drawSounds();
        void checkMics();
    
        void setupUDP();
};
