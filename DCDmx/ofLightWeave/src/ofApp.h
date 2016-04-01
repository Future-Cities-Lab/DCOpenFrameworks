#pragma once

#include "ofMain.h"
#include "ofxGenericDmx.h"
#include "ofxJSON.h"
#include "ofGuiApp.h"
#include "ofxAssimpModelLoader.h"
#include "ofxNetwork.h"
#include "ofxThread.h"
#include "ofxMailUtils.h"
#include "ofArduino.h"
#include "ofxOpenCv.h"
#include "IPVideoGrabber.h"


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


        shared_ptr<ofGuiApp> gui;
    
        char message[256];
        char formName[128];
        int  formNum;
    

    private:
    
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
        ofVec2f swarmPosition = ofVec2f(310, 720);
        ofVec2f swarmVector = ofVec2f(1.0, 0.0);

        ofPixels videoInverted;
        ofTexture videoTexture;
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
        float micNoiseTop[11];

        float micLevelsBottom[12];
        float micNoiseBottom[12];

        ofVec2f columnPositions[23] = {ofVec2f(396,302),ofVec2f(460,298),
            ofVec2f(548,301),ofVec2f(612,310),
            ofVec2f(761,373),ofVec2f(825,381),
            ofVec2f(932,307),ofVec2f(997,302),
            ofVec2f(1081,294),ofVec2f(1147,299),
            ofVec2f(890,373),ofVec2f(159,670),
            ofVec2f(223,668),ofVec2f(311,666),
            ofVec2f(393,658),ofVec2f(456,655),
            ofVec2f(614,596),ofVec2f(683,590),
            ofVec2f(742,590),ofVec2f(914,662),
            ofVec2f(979,668),ofVec2f(1064,663),
            ofVec2f(1127,658)};
    
        ofVec2f absColumnPositionsBottom[12] = {
            ofVec2f(300,720),
            ofVec2f(370,720),
            ofVec2f(440,720),
            ofVec2f(510,720),
            ofVec2f(580,720),
            ofVec2f(650,720),
            ofVec2f(720,720),
            ofVec2f(790,720),
            ofVec2f(860,720),
            ofVec2f(930,720),
            ofVec2f(1000,720),
            ofVec2f(1070,720)};
    
        //float gaussian[1280];
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
    
//        struct Wave {
//            float a;
//            float b;
//            float c;
//            float bVel;
//            float curve[1280];
//            bool growing;
//        };
    
        struct Swarm {
            float a;
            float b;
            float c;
            float bVel;
            float curve[1280];
        };
    
        Swarm bottomSwarm;
    
        struct Car {
            ofVec2f pos;
            ofVec2f vel;
            int counter;
        };
        struct Sound {
            ofVec2f pos;
            ofVec2f vel;
            float val;
        };
        int prevBestPosition = 0;
        vector<Car> cars;
        vector<Sound> sounds;
        float trainSpeed = 0.5;
        float carSpeed = 1.0;
        float bikeSpeed = 1.0;
        float personSpeed = 0.3;
    
//        vector<Wave> waves;
//        vector<Wave> waves2;
    
        ofxJSONElement result;
        ofxJSONElement columnGeometry;
        ofImage backgroundImage;
        void sendToDMX();
        void newDrawRegion(float gaussLevels[1280], int start, int end, bool isEvent);
        void drawRegion(float gaussLevels[1280], int start, int end, bool isEvent);
        void drawCars();
        void drawSounds();
        void checkMics();
    
//        void updateWaves(vector<Wave> *wavesToUpate);
//        void removeWaves(vector<Wave> *wavesToRemove, float velocity);
//        void addWave(vector<Wave> *wavesToAddTo, float velocity);
    
        void setupUDP();
};
