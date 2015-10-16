#pragma once

#include "ofMain.h"
#include "ofxGenericDmx.h"
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
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
    private:
        const unsigned int numBars = 16;
        ofColor barColor;
        unsigned int barBrightness[16];
        float barHeights[16];
        const unsigned int barWidth = 4;
        float attractorXPos = 0.0;
    
        float gaussianSensor1[1024];
        float gaussianSensor2[1024];
        float gaussian[1024];
    
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

        struct Wave {
            float a;
            float b;
            float c;
            float bVel;
            float curve[1024];
            bool growing;
        };
        vector<Wave> waves;
};
