//
//  ofGuiApp.h
//  ofLightWeave
//
//  Created by Collin Schupman on 11/18/15.
//
//

#ifndef __ofLightWeave__ofGuiApp__
#define __ofLightWeave__ofGuiApp__

#include "ofMain.h"
#include <stdio.h>
#include "ofxGui.h"
//#include "ofxAubio.h"

class ofGuiApp : public ofBaseApp {
    public:
        void setup();
        void update();
        void draw();
    
        //ofxAubioOnset onset;
        ofxFloatSlider gotOnset;
        ofxFloatSlider onsetThreshold;
        ofxFloatSlider onsetNovelty;
        ofxFloatSlider onsetThresholdedNovelty;
    
        ofxColorSlider bColor;
        ofxColorSlider lColor;
        ofxColorSlider rColor;
        ofxButton addCar;
        ofxButton addPerson;
        ofxButton addBike;
        ofxButton addTrain;
        
        ofxFloatSlider yourMamma;
        ofxToggle autoMode;
        ofxToggle region1Info;
        ofxToggle region2Info;
        ofxToggle region3Info;
        ofxToggle region4Info;
        ofxToggle region5Info;
        ofxToggle region6Info;
        
        ofxPanel gui;

        void audioIn(float * input, int bufferSize, int nChannels);
        void audioOut();
};

#endif /* defined(__ofLightWeave__ofGuiApp__) */
