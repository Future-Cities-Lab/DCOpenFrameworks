#pragma once

#include "ofMain.h"
#include "ofxGenericDmx.h"
#include "ofxJSON.h"
#include "ofxAssimpModelLoader.h"
#include "ofxNetwork.h"
#include "ofxOpenCv.h"
#include "ofxGui.h"

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

    
        char message[256];
        char formName[128];
        int  formNum;
        ofxPanel gui;


    private:
        ofCamera cam;

        ofxColorSlider ambientColor;
        ofxColorSlider swarmColor;
        ofxColorSlider addressableColor;

        float backgroundLevel = 0.0;
        bool backgroundGrowing = true;
        ofVec2f swarmPosition = ofVec2f(310, 45);
        ofVec2f swarmVector = ofVec2f(1.0, 0.0);

        int camWidth;
        int camHeight;

        /* OLD CODE */
    
        enum AnimationState { ACTIVATED, DEACTIVATED };
        AnimationState ANIMATION_STATE = DEACTIVATED;
        ofxUDPManager udpConnectionBroadcast;
        ofxUDPManager moduleConnections[3];
    
        float micLevelsTopNew[12];
    
//        ofVec2f absColumnPositionTop[11] = {
//            ofVec2f(300, 720),
//            ofVec2f(370, 720),
//            ofVec2f(440, 720),
//            ofVec2f(510, 720),
//            ofVec2f(580, 720),
//            ofVec2f(650, 720),
//            ofVec2f(720, 720),
//            ofVec2f(790, 720),
//            ofVec2f(860, 720),
//            ofVec2f(930, 720),
//            ofVec2f(1000, 720)};
    
        ofVec2f cameraPositionsTop[12] = {
            ofVec2f(265, 720),
            ofVec2f(335, 720),
            ofVec2f(405, 720),
            ofVec2f(475, 720),
            ofVec2f(545, 720),
            ofVec2f(615, 720),
            ofVec2f(685, 720),
            ofVec2f(755, 720),
            ofVec2f(825, 720),
            ofVec2f(895, 720),
            ofVec2f(965, 720),
            ofVec2f(1035, 720)};
    
        float gaussianBottom[1280] = {0.0};
    
    
        string verbalInstructions[10] = {"'w' to move forward, 's' to move backward",
                                    "'Up Arrow' to move up, 'Down Arrow' to move down",
                                    "'Left Arrow' to move leeft, 'Right Arrow' to move right",
                                    "'a' to toggle swarm",
                                    "Red is office column",
                                    "Blue are office cameras",
                                    "'spacebar' resets camera",
                                    "'m' to toggle simulation",
                                    "'i' for camera information"};
    
    
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
        float colorHue2;
    
        ofColor materialColor2;
    
        ofMaterial material;
        ofMaterial peopleMaterial;
        ofMaterial columnMaterial;

    
        bool cameraInfoIsOn = false;
        bool simulationIsOn = true;

    
    
        ofxJSONElement result;
        ofxJSONElement columnGeometry;
        ofxJSONElement faceGeometry;
    
        void sendToDMX();
        void newDrawRegion(float gaussLevels[1280], int start, int end, bool isEvent);
    
        void setupUDP();
};
