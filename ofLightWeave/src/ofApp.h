#pragma once

#include "ofMain.h"
#include "ofxGenericDmx.h"
#include "ofxJSON.h"
#include "ofGuiApp.h"
#include "ofxAssimpModelLoader.h"
#include "ofxNetwork.h"
#include "ofxThread.h"
#include "ofxMailUtils.h"

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
        void carPressed();
        void personPressed();
        void bikePressed();
        void trainPressed();
        void addCars(int num);
        void addTrains();
        void addPeople(int num);
        void addBikes(int num);
        shared_ptr<ofGuiApp> gui;
    
        char message[256];
        char formName[128];
        int  formNum;
        
        ofxPop3ClientUtils pop3;
        ofxSmtpClientUtils smtp;

    private:
    
    
        float gaussianSensor1[1280];
        float gaussianSensor2[1280];
        float gaussian[1280];
        
        ofPolyline lineGraph;
        
        DmxDevice* dmxInterface_;
        unsigned char dmxData_[DMX_DATA_LENGTH];
        float red, green, blue;
    
        float trigPos = 0.0;
        
        int curWidth;
        int curHeight;
        
        float decayRate;
        float growthRate;
        float aGrowthRate;
        float bAccel;
        
        bool pulseLeftGrowing;
        bool pulseRightGrowing;
        float pulseHeightLeft;
        float pulseHeightRight;
        float pulseGrowth;
        float pulseDecay;
        
        bool trainGrowing;
        float trainGrowthRate;
        float trainDecay;
        float ambientLevel;
        
        struct Wave {
            float a;
            float b;
            float c;
            float bVel;
            float curve[1280];
            bool growing;
        };
    
        struct Mover {
            ofVec2f pos;
            ofVec2f vel;
            ofVec2f accel;
        };
    
        void drawMovers(vector<Mover> movers, int size);
        vector<Mover> updateMovers(vector<Mover> movers);
        float trainSpeed = 0.5;
        float carSpeed = 3.0;
        float bikeSpeed = 1.0;
        float personSpeed = 0.3;
        vector<Mover> trains;
        vector<Mover> cars;
        vector<Mover> people;
        vector<Mover> bikes;
        
        vector<Wave> waves;
        vector<Wave> waves2;
        ofxJSONElement result;
        ofxJSONElement columnGeometry;
        
        ofImage backgroundImage;
};
