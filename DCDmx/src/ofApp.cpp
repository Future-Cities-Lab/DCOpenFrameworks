#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {
    ofEnableSmoothing();
    ofEnableAlphaBlending();
    ofSetFrameRate(22);
    
    barColor = ofColor(255.0,255.0,0.0);
    
    for (int i = 0; i < numBars; i++) {
        barBrightness[i] = 255.0;
    }
    
    //TODO:(COLLIN) Get Real Length Values OR Make better simulation
    float r = 0.4f;
    for (int i = 0; i < numBars; i++) {
        barHeights[i] = r*(ofGetHeight()/2);
        r *= 1.06f;
    }
    
    //zero our DMX value array
    memset( dmxData_, 0, DMX_DATA_LENGTH );
    
    //open the device
    dmxInterface_ = ofxGenericDmx::createDevice(DmxDevice::DMX_DEVICE_RAW);
    bool opened = dmxInterface_->open();
    if ( dmxInterface_ == 0 || !opened ) {
        printf( "No FTDI Device Found\n" );
    } else {
        printf( "isOpen: %i\n", dmxInterface_->isOpen() );
    }
    
    printf("ofxGenericDmx addon version: %s.%s\n", ofxGenericDmx::VERSION_MAJOR, ofxGenericDmx::VERSION_MINOR);

    gui.setup(); // most of the time you don't need a name
    gui.add(autoMode.setup("Automate", false));
    gui.add(color.setup("color",ofColor(100,100,140),ofColor(0,0),ofColor(255,255), 20.0));

    curWidth = 1024;
    curHeight = 768;
    
    decayRate = 0.99f;
    growthRate = 1.0001f;
    aGrowthRate = 3.0f;
    bAccel = 0.99;
    
    pulseGrowth = 19.0;
    pulseDecay = 0.5f;
    pulseHeightLeft = 1.01f;
    pulseHeightRight = 1.01f;
    pulseLeftGrowing = false;
    pulseRightGrowing = false;
    trainGrowing = false;;
    trainGrowthRate = 1.009f;
    trainDecay = 0.98;
    ambientLevel = 51.0;
}

//--------------------------------------------------------------
void ofApp::update() {
    
    if (autoMode) {
        if (ofRandom(1.0) > 0.99) {
            if (ofRandom(1.0) > 0.5) {
                int diff = 1024/numBars;
                Wave wave;
                wave.a = 1.1f;
                wave.b = diff + 3*(diff);
                wave.c = 100.0;
                wave.bVel = 10.0;
                if (ofRandom(1.0)>0.5) {
                    wave.bVel*=-1.0;
                }
                wave.growing = true;
                waves.push_back(wave);
                pulseLeftGrowing = true;
            } else {
                int diff = 1024/numBars;
                Wave wave;
                wave.a = 1.1f;
                wave.b = diff + 10*(diff);
                wave.c = 100.0;
                wave.bVel = 10.0;
                if (ofRandom(1.0)>0.5) {
                    wave.bVel*=-1.0;
                }
                wave.growing = true;
                waves.push_back(wave);
                pulseRightGrowing = true;
            }
        }
    }
    
    for (int i = 0; i < waves.size(); i++) {
        for (int x = 0; x < 1024; x++) {
            float top = pow(x-waves[i].b,2);
            float bottom = 2*pow(waves[i].c,2);
            waves[i].curve[x] = waves[i].a*exp(-(top/bottom));
        }
        waves[i].b += waves[i].bVel;
        if (waves[i].b >= ofGetWidth() || waves[i].b <= 0) {
            waves[i].bVel*=-1;
        }
        if (waves[i].a < (curHeight/4.0) - 6.0 && waves[i].growing) {
            waves[i].a*=aGrowthRate;
        } else {
            waves[i].growing = false;
            waves[i].a*=decayRate;
        }
        if (!waves[i].growing) {
            waves[i].bVel*=bAccel;
        }
        waves[i].c*=growthRate;

    }

    vector<int> toErase;
    for (int i = 0; i < waves.size(); i++) {
        if (waves[i].a < 1.0) {
            toErase.push_back(i);
        }
    }
    for (int i = 0; i < toErase.size(); i++) {
        int pos = toErase[i];
        waves.erase(waves.begin() + pos);
    }
    
    if (trainGrowing) {
        ambientLevel *= trainGrowthRate;
        if (ambientLevel >= 255.0) {
            ambientLevel = 255.0;
            trainGrowing = false;
        }
    } else if (!trainGrowing && ambientLevel > 51.0) {
        ambientLevel *= trainDecay;
        if (ambientLevel <= 51.0) {
            ambientLevel = 51.0;
        }
    }
    
    for (int x = 0; x < 1024; x++) {
        float newVal = 0.0;
        for (int i = 0; i < waves.size(); i++) {
            newVal += waves[i].curve[x];
        }
        gaussian[x] = ofMap(ofClamp(newVal,0.0,255.0),0.0,255.0,ambientLevel,255.0);
    }
   
    int diff = 1024/numBars;
    for (int x = diff/2; x < 1024 + (diff/2); x+=diff) {
        ofColor c = color;
        barBrightness[x/diff] = gaussian[x];
    }
    
    ofColor colorToSend = color;
    colorToSend.setBrightness(barBrightness[12]);
    dmxData_[1] = int(colorToSend.r);
    dmxData_[2] = int(colorToSend.g);
    dmxData_[3] = int(colorToSend.b);
    colorToSend.setBrightness(barBrightness[11]);
    dmxData_[4] = int(colorToSend.r);
    dmxData_[5] = int(colorToSend.g);
    dmxData_[6] = int(colorToSend.b);
    colorToSend.setBrightness(barBrightness[10]);
    dmxData_[7] = int(colorToSend.r);
    dmxData_[8] = int(colorToSend.g);
    dmxData_[9] = int(colorToSend.b);
    colorToSend.setBrightness(barBrightness[9]);
    dmxData_[10] = int(colorToSend.r);
    dmxData_[11] = int(colorToSend.g);
    dmxData_[12] = int(colorToSend.b);
    
    //force first byte to zero (it is not a channel but DMX type info - start code)
    dmxData_[0] = 0;
    
    if ( ! dmxInterface_ || ! dmxInterface_->isOpen() ) {
        printf( "Not updating, enttec device is not open.\n");
    } else{
        dmxInterface_->writeDmx( dmxData_, DMX_DATA_LENGTH );
    }
    
    
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofBackgroundGradient(ofColor(25.0,25.0,25.0), ofColor::black);

    int diff = curWidth/numBars;
    
    ofSetColor(185.0);
    ofFill();
    
    ofDrawBitmapString("K", diff + 3*(diff)-4, curHeight/2 + 20);
    ofDrawBitmapString("L", diff + 10*(diff)-2, curHeight/2 + 20);
    ofDrawBitmapString("Press 'T' for train simulation", 0.0, ofGetHeight()-100.0);

    ofEllipse(diff + 3*(diff), curHeight/2, 10, 10);
    ofEllipse(diff + 10*(diff), curHeight/2, 10, 10);
    
    ofSetColor(31.875);
    ofFill();
    ofRect(diff + 8*(diff)-1, (curHeight/4.0), 3, 2.0*(curHeight/4.0));
    ofRect(diff + 12*(diff)-1, (curHeight/4.0), 3, 2.0*(curHeight/4.0));

    
    // Draw the sections
    ofSetColor(31.875);
    ofFill();
    ofRect(0.0, curHeight/4.0, curWidth, 4.0);
    ofRect(0.0, 3.0*(curHeight/4.0), curWidth, 4.0);
    
    // Display the sound curve
    for (int i = 0; i < curWidth; i++) {
        ofColor c = color;
        c.setBrightness(gaussian[i]);
        ofSetColor(c);
        ofFill();
        ofEllipse(i, curHeight/4.0 - ofMap(gaussian[i], 0.0, 255.0, 0.0, 180.0), 2, 2);
    }
    
    ofSetColor(255.0);
    ofFill();
    
    vector<ofPoint> pts;
    
    pts.push_back(ofPoint(diff + 3*(diff) - 60,ofGetHeight()));
    pts.push_back(ofPoint(diff + 3*(diff), ofGetHeight()-pulseHeightLeft));
    pts.push_back(ofPoint(diff + 3*(diff) + 60,ofGetHeight()));
    ofPolyline line(pts);
    line.draw();
    
    if (pulseHeightLeft < 140.0 && pulseLeftGrowing) {
        pulseHeightLeft = ofClamp(pulseHeightLeft*pulseGrowth, 0.0, 140.0);
    } else {
        //TODO(COLLIN):HACK, DO THIS INTELLIGENTLY
        pulseHeightLeft = ofClamp(pulseHeightLeft*pulseDecay, 1.0, 140.0);
        pulseLeftGrowing = false;
    }
    
    
    pts.clear();
    pts.push_back(ofPoint(diff + 10*(diff) - 60,ofGetHeight()));
    pts.push_back(ofPoint(diff + 10*(diff), ofGetHeight()-pulseHeightRight));
    pts.push_back(ofPoint(diff + 10*(diff) + 60,ofGetHeight()));
    ofPolyline line2(pts);
    line2.draw();

    if (pulseHeightRight < 140.0 && pulseRightGrowing) {
        pulseHeightRight = ofClamp(pulseHeightRight*pulseGrowth, 0.0, 140.0);
    } else {
        //TODO(COLLIN):HACK, DO THIS INTELLIGENTLY
        pulseHeightRight = ofClamp(pulseHeightRight*pulseDecay, 1.0, 140.0);
        pulseRightGrowing = false;
    }
    
    ofSetColor(185.0);
    ofFill();
    ofDrawBitmapString("%:", 0, 3.0*(curHeight/4.0)+16.0);
    ofSetColor(31.875);
    ofFill();
    ofRect(0.0, 3.0*(ofGetHeight()/4.0)+20.0, curWidth, 4.0);
    ofSetColor(185.0);
    ofFill();
    ofDrawBitmapString("Lux:", 0, 3.0*(curHeight/4.0)+38.0);
    ofSetColor(31.875);
    ofFill();
    ofRect(0.0, 3.0*(ofGetHeight()/4.0)+40.0, curWidth, 4.0);

    for (int x = diff/2; x < curWidth + (diff/2); x+=diff) {
        ofColor thisBarsColor = color;
        float brightness = barBrightness[(x/diff)];
        ofSetColor(185.0);
        ofFill();
        ofDrawBitmapString(ofToString((int)ofMap(brightness, 0.0, 255.0, 0.0, 100.0)), x, 3.0*(curHeight/4.0)+16.0);
        ofDrawBitmapString(ofToString((int)ofMap(brightness, 0.0, 255.0, 0.0, 70.0)), x, 3.0*(curHeight/4.0)+38.0);
        thisBarsColor.setBrightness(brightness);
        ofSetColor(thisBarsColor);
        ofFill();
        float startY = 2.0*(curHeight/4.0);
        float y = (startY - barHeights[x/diff])/2.0;
        ofRect(x, (curHeight/4.0) + y, barWidth, barHeights[x/diff]);
    }
    gui.draw();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
    
    if (key == 107) {
        int diff = 1024/numBars;
        Wave wave;
        wave.a = 1.1f;
        wave.b = diff + 3*(diff);
        wave.c = 100.0;
        wave.bVel = 10.0;
        if (ofRandom(1.0)>0.5) {
            wave.bVel*=-1.0;
        }
        wave.growing = true;
        waves.push_back(wave);
        pulseLeftGrowing = true;
        
    } else if (key == 108) {
        int diff = 1024/numBars;
        Wave wave;
        wave.a = 1.1f;
        wave.b = diff + 10*(diff);
        wave.c = 100.0;
        wave.bVel = 10.0;
        if (ofRandom(1.0)>0.5) {
            wave.bVel*=-1.0;
        }
        wave.growing = true;
        waves.push_back(wave);
        pulseRightGrowing = true;

    } else if (key == 116) {
        trainGrowing = true;
    }

}

//--------------------------------------------------------------
void ofApp::exit() {
    if ( dmxInterface_ && dmxInterface_->isOpen() ) {
        // send all zeros (black) to every dmx channel and close!
        for ( int i = 0; i <= DMX_DATA_LENGTH; i++ ) dmxData_[i] = 0;
        dmxInterface_->writeDmx( dmxData_, DMX_DATA_LENGTH );
        dmxInterface_->close();
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ) {

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {
    curWidth = w;
    curHeight = h;
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}