#pragma once

#include "ofMain.h"
#include "ofxGenericDmx.h"
#include "ofxGui.h"
#include "ofxJSON.h"

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
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
    private:
    
        float gaussianSensor1[1280];
        float gaussianSensor2[1280];
        float gaussian[1280];
    
        ofPolyline lineGraph;
    
        DmxDevice* dmxInterface_;
        unsigned char dmxData_[DMX_DATA_LENGTH];
        float red, green, blue;
    
        ofxColorSlider color;
        ofxFloatSlider yourMamma;
        ofxToggle autoMode;
        ofxPanel gui;
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
        vector<Wave> waves;
        vector<Wave> waves2;
        ofxJSONElement result;
};
