#include "ofApp.h"


vector<vector<vector<int> > > geometry;
float minZ = 10000.0f;
float maxZ = -10000.0f;
float minY = 10000.0f;
float maxY = -10000.0f;


//--------------------------------------------------------------
void ofApp::setup() {
    ofEnableSmoothing();
    ofEnableAlphaBlending();
    ofSetFrameRate(22);

    float r = 0.4f;
    
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

    curWidth = 1280;
    curHeight = 900;
    
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
    
    std::string file = "Lightweave_loops.json";
    
    // Now parse the JSON
    bool parsingSuccessful = result.open(file);
    
    if (parsingSuccessful) {
        ofLogNotice("ofApp::setup") << result.getRawString();
        if (!result.save("example_output_pretty.json", true)) {
            ofLogNotice("ofApp::setup") << "example_output_pretty.json written unsuccessfully.";
        } else {
            ofLogNotice("ofApp::setup") << "example_output_pretty.json written successfully.";
        }
        // now write without pretty print
        if (!result.save("example_output_fast.json", false)) {
            ofLogNotice("ofApp::setup") << "example_output_pretty.json written unsuccessfully.";
        } else {
            ofLogNotice("ofApp::setup") << "example_output_pretty.json written successfully.";
        }
    } else {
        ofLogError("ofApp::setup")  << "Failed to parse JSON" << endl;
    }
    for (int region = 0; region < 6; region++) {
        string blah = "region" + ofToString(region);
        for (int rings = 0; rings < result[blah].size(); rings++) {
            string ring = "ring" + ofToString(rings);
            for (int pointPos = 0; pointPos < 3; pointPos++) {
                string point = "point" + ofToString(pointPos);
                if (result[blah][ring][point][2].asFloat() > maxZ) {
                    maxZ = result[blah][ring][point][2].asFloat();
                }
                if (result[blah][ring][point][2].asFloat() < minZ && result[blah][ring][point][2].asFloat() != 0.0) {
                    minZ = result[blah][ring][point][2].asFloat();
                }
                if (result[blah][ring][point][1].asFloat() > maxY) {
                    maxY = result[blah][ring][point][1].asFloat();
                }
                if (result[blah][ring][point][1].asFloat() < minY && result[blah][ring][point][1].asFloat() != 0.0) {
                    minY = result[blah][ring][point][1].asFloat();
                }
            }
        }
    }
    maxZ*=(1280.0/1920.0);
    maxY*=(900.0/1080.0);
}

//--------------------------------------------------------------
void ofApp::update() {

    
    if (autoMode) {
        if (ofRandom(1.0) > 0.99) {
            if (ofRandom(1.0) > 0.5) {
                Wave wave;
                wave.a = 1.1f;
                wave.b = (curWidth/4.0);
                wave.c = 100.0;
                wave.bVel = 10.0;
                if (ofRandom(1.0)>0.5) {
                    wave.bVel*=-1.0;
                }
                wave.growing = true;
                waves.push_back(wave);
                pulseLeftGrowing = true;
            } else {
                Wave wave;
                wave.a = 1.1f;
                wave.b = 3.0*(curWidth/4.0);
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
        for (int x = 0; x < 1280; x++) {
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
    
    for (int x = 0; x < 1280; x++) {
        float newVal = 0.0;
        for (int i = 0; i < waves.size(); i++) {
            newVal += waves[i].curve[x];
        }
        gaussian[x] = ofMap(ofClamp(newVal,0.0,255.0), 0.0, 255.0, ambientLevel, 255.0);
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofBackground(0.0);
    
    ofSetColor(255.0);
    ofFill();
    ofDrawBitmapString("Press 'T' for train simulation", 0.0, ofGetHeight()-100.0);
    
    /* Sensor Points */
    ofSetColor(31.875);
    ofFill();
    ofEllipse((1280.0/4.0), ((maxY+10)-(minY-10))/2, 10, 10);
    ofEllipse(3*(1280.0/4.0), ((maxY+10)-(minY-10))/2, 10, 10);
    ofEllipse((1280.0/4.0), ((maxY+10)-(minY-10)), 10, 10);
    ofEllipse(3*(1280.0/4.0), ((maxY+10)-(minY-10)), 10, 10);
    
    ofSetColor(255.0);
    ofFill();
    ofDrawBitmapString("K", (1280.0/4.0) - 4, (((maxY+10)-(minY-10))/2) + 16);
    ofDrawBitmapString("L", 3*(1280.0/4.0) - 4, (((maxY+10)-(minY-10))/2) + 16);

    /* Current Office location */
    //ofRect(diff + 8*(diff)-1, (curHeight/4.0), 3, 2.0*(curHeight/4.0));
    //ofRect(diff + 12*(diff)-1, (curHeight/4.0), 3, 2.0*(curHeight/4.0));
    
    // Draw boundaries
    ofRect(0.0, ((minY-10)+(maxY+10))/2, curWidth, 4.0);
    ofRect(0.0, minY-10, curWidth, 4.0);
    
    //display the sound curve
    for (int i = 0; i < curWidth; i++) {
        ofColor c = color;
        c.setBrightness(gaussian[i]);
        ofSetColor(c);
        ofFill();
        ofEllipse(i, ((minY-10)+(maxY+10))/2 - ofMap(gaussian[i], 0.0, 255.0, 0.0, 180.0), 2, 2);
    }

    // update the train
    if (pulseHeightLeft < 140.0 && pulseLeftGrowing) {
        pulseHeightLeft = ofClamp(pulseHeightLeft*pulseGrowth, 0.0, 140.0);
    } else {
        //TODO(COLLIN):HACK, DO THIS INTELLIGENTLY
        pulseHeightLeft = ofClamp(pulseHeightLeft*pulseDecay, 1.0, 140.0);
        pulseLeftGrowing = false;
    }


    if (pulseHeightRight < 140.0 && pulseRightGrowing) {
        pulseHeightRight = ofClamp(pulseHeightRight*pulseGrowth, 0.0, 140.0);
    } else {
        //TODO(COLLIN):HACK, DO THIS INTELLIGENTLY
        pulseHeightRight = ofClamp(pulseHeightRight*pulseDecay, 1.0, 140.0);
        pulseRightGrowing = false;
    }
    
    ofColor colorToSend = color;
    colorToSend.setBrightness(gaussian[result["region0"]["ring0"]["point0"][0].asInt()]);
    dmxData_[1] = int(colorToSend.r);
    dmxData_[2] = int(colorToSend.g);
    dmxData_[3] = int(colorToSend.b);
    colorToSend.setBrightness(gaussian[result["region0"]["ring0"]["point0"][0].asInt()]);
    dmxData_[4] = int(colorToSend.r);
    dmxData_[5] = int(colorToSend.g);
    dmxData_[6] = int(colorToSend.b);
    colorToSend.setBrightness(gaussian[result["region0"]["ring0"]["point0"][0].asInt()]);
    dmxData_[7] = int(colorToSend.r);
    dmxData_[8] = int(colorToSend.g);
    dmxData_[9] = int(colorToSend.b);
    colorToSend.setBrightness(gaussian[result["region0"]["ring0"]["point0"][0].asInt()]);
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
    
    for (int region = 0; region < 6; region++) {
        ofSetColor(185.0);
        ofFill();
        string reg = "region" + ofToString(region);
        ofDrawBitmapString(reg + " %:", 215, (((minY-10)/6)*region) + (minY-10)/12);
        ofDrawBitmapString(reg + " Lux:", 215, (((minY-10)/6)*region) + (minY-10)/12 + 12.0);
        for (int rings = 0; rings < result[reg].size(); rings++) {
            ofSetColor(185.0);
            ofFill();
            string ring = "ring" + ofToString(rings);
            float in = (1280.0/1920.0)*result[reg][ring]["point0"][0].asFloat();
            int inInt = (int) in;
            int gauss = gaussian[inInt];
            ofDrawBitmapString(ofToString((int)ofMap(gauss, 0, 255.0, 0.0,100.0)), 310 + 27*rings, (((minY-10)/6)*region) + (minY-10)/12);
            ofDrawBitmapString(ofToString((int)ofMap(gauss, 0, 255.0, 7.0, 51.0)), 310 + 27*rings, (((minY-10)/6)*region) + (minY-10)/12 + 12.0);
            ofPolyline line;
            ofColor c = color;
            c.setBrightness(gauss);
            ofSetColor(c);
            ofFill();
            for (int pointPos = 0; pointPos < 3; pointPos++) {
                string point = "point" + ofToString(pointPos);
                if (result[reg][ring][point][0].asFloat() < 1920) {
                    line.addVertex((1280.0/1920.0)*result[reg][ring][point][0].asFloat(), (900.0/1080.0)*(1080-result[reg][ring][point][1].asFloat()));
                }
            }
            if (result[reg][ring]["point0"][0].asFloat() < 1920) {
                line.addVertex((1280.0/1920.0)*result[reg][ring]["point0"][0].asFloat(), (900.0/1080.0)*(1080-result[reg][ring]["point0"][1].asFloat()));
            }
            line.draw();
        }
    }
    

    gui.draw();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
    if (key == 107) {
        float vel = ofRandom(7.0,30.0);
        Wave wave;
        wave.a = 1.1f;
        wave.b = (1280.0/4.0);
        wave.c = 100.0;
        wave.bVel = vel;
        wave.growing = true;
        waves.push_back(wave);
        Wave wave2;
        wave2.a = 1.1f;
        wave2.b = (1280.0/4.0);
        wave2.c = 100.0;
        wave2.bVel = -vel;
        wave2.growing = true;
        waves.push_back(wave2);
        pulseLeftGrowing = true;
    } else if (key == 108) {
        float vel = ofRandom(7.0,30.0);
        Wave wave;
        wave.a = 1.1f;
        wave.b = 3.0*(1280.0/4.0);
        wave.c = 100.0;
        wave.bVel = vel;
        wave.growing = true;
        waves.push_back(wave);
        Wave wave2;
        wave2.a = 1.1f;
        wave2.b = 3.0*(1280.0/4.0);
        wave2.c = 100.0;
        wave2.bVel = -vel;
        wave2.growing = true;
        waves.push_back(wave2);
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