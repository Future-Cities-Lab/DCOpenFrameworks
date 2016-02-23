//
//  ofGuiApp.cpp
//  ofLightWeave
//
//  Created by Collin Schupman on 11/18/15.
//
//

#include "ofGuiApp.h"

void ofGuiApp::setup(){
    ofBackground(0.0);
    ofSetVerticalSync(false);
    gui.setup();
    gui.add(bColor.setup("Background Color",ofColor(100),ofColor(0,0),ofColor(255,255), 20.0));
    gui.add(lColor.setup("Left Color",ofColor(255,255,0),ofColor(0,0),ofColor(255,255), 20.0));
    gui.add(rColor.setup("Right Color",ofColor(0,0,250),ofColor(0,0),ofColor(255,255), 20.0));
    gui.add(autoMode.setup("Automate Traffic", false));
    gui.add(addCar.setup("Add Car : press 'c' "));
    gui.add(addPerson.setup("Add Person : press 'p'"));
    gui.add(addBike.setup("Add Bike : press 'b'"));
    gui.add(addTrain.setup("Add Train : press 't'"));
    
    int nOutputs = 2;
    int nInputs = 2;
    //onset.setup();
    
    
    ofSoundStreamSetup(nOutputs, nInputs, this);
    
    gui.add(gotOnset.setup( "onset", 0, 0, 250));
    gui.add(onsetThreshold.setup( "threshold", 0, 0, 2));
    gui.add(onsetNovelty.setup( "onset novelty", 0, 0, 10000));
    gui.add(onsetThresholdedNovelty.setup( "thr. novelty", 0, -1000, 1000));
    //onsetThreshold = onset.threshold;
    
}

void ofGuiApp::update(){
//    onset.setThreshold(onsetThreshold);
//    if (onset.received()) {
//        gotOnset = 255;
//    } else {
//        gotOnset = 0;
//    }
//    onsetNovelty = onset.novelty;
//    onsetThresholdedNovelty = onset.thresholdedNovelty;
}

void ofGuiApp::draw(){
    gui.draw();
}

void ofGuiApp::audioIn(float * input, int bufferSize, int nChannels){
    //onset.audioIn(input, bufferSize, nChannels);
}

void ofGuiApp::audioOut(){
}
