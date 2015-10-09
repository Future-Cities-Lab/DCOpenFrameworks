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
        r*=1.06f;
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
    gui.add(color.setup("color",ofColor(100,100,140),ofColor(0,0),ofColor(255,255)));
    gui.add(yourMamma.setup("AutoSpeed", 0.0, 0.0, 1.0));

    curWidth = 1024;
    curHeight = 768;
}

//--------------------------------------------------------------
void ofApp::update() {
    
    int functionLength = 500;
    
    
    int diff = 1024/numBars;
    
    float c1 = 0.0f;
    float c2 = 0.0f;
    
    if (autoMode) {
        c1 = ofMap(ofMap(sin(trigPos),-1.0,1.0,85,115),85,115,0.0,100.0);
        c2 = ofMap(ofMap(cos(trigPos),-1.0,1.0,85,115),85,115,0.0,100.0);
        trigPos += yourMamma;
    }

    float a = (curHeight/4.0) - 6.0;
    float b1 = diff + 3*(diff);
    float b2 = diff + 10*(diff);

    // TODO(COLLIN): FIX RESIZE HERE
    for (int x = 0; x < 1024; x++) {
        float top = pow(x-b1,2);
        float bottom = 2*pow(c1,2);
        gaussianSensor1[x] = a*exp(-(top/bottom));
    }
    
    for (int x = 0; x < 1024; x++) {
        float top = pow(x-b2,2);
        float bottom = 2*pow(c2,2);
        gaussianSensor2[x] = a*exp(-(top/bottom));
    }
    
    for (int x = 0; x < 1024; x++) {
        gaussian[x] = ofClamp(gaussianSensor1[x]+gaussianSensor2[x],0.0,255.0);
    }
   
    for (int x = diff/2; x < 1024 + (diff/2); x+=diff) {
        barBrightness[x/diff] = ofMap(gaussian[x],0.0, (curHeight/4.0) - 6.0, 31.875, 255.0);
    }
    
    ofColor colorToSend = color;
    colorToSend.setBrightness(barBrightness[9]);
    dmxData_[1] = int(colorToSend.r);
    dmxData_[2] = int(colorToSend.g);
    dmxData_[3] = int(colorToSend.b);
    colorToSend.setBrightness(barBrightness[10]);
    dmxData_[4] = int(colorToSend.r);
    dmxData_[5] = int(colorToSend.g);
    dmxData_[6] = int(colorToSend.b);
    colorToSend.setBrightness(barBrightness[11]);
    dmxData_[7] = int(colorToSend.r);
    dmxData_[8] = int(colorToSend.g);
    dmxData_[9] = int(colorToSend.b);
    colorToSend.setBrightness(barBrightness[12]);
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
    //ofBackground(0.0);
    ofBackgroundGradient(ofColor(25.0,25.0,25.0), ofColor::black);

    int diff = curWidth/numBars;
    
    ofSetColor(185.0);
    ofFill();
    
    //int val = sensor1Value;
    //ofDrawBitmapString("Sensor 1 :" + ofToString(val) + "dB", diff + 3*(diff) - 50.0, (curHeight/4.0)-2.0);
    //val = sensor2Value;
    //ofDrawBitmapString("Sensor 2 :" + ofToString(val) + "dB", diff + 10*(diff) - 50.0, (curHeight/4.0)-2.0);

    ofEllipse(diff + 3*(diff), curHeight/2, 10, 10);
    ofEllipse(diff + 10*(diff), curHeight/2, 10, 10);

    
    // Draw the sections
    ofSetColor(31.875);
    ofFill();
    ofRect(0.0, curHeight/4.0, curWidth, 4.0);
    ofRect(0.0, 3.0*(curHeight/4.0), curWidth, 4.0);
    
    // Display the sound curve
    for (int i = 0; i < curWidth; i++) {
        ofSetColor(185.0);
        ofFill();
        ofEllipse(i, curHeight/4.0 - gaussian[i], 2,2);
    }


    vector<ofPoint> pts;
    for (int x = diff/2; x < curWidth + (diff/2); x+=diff) {
        pts.push_back(ofPoint(x, ofMap(barBrightness[x/diff], 0.0, 255.0, 3.0*(curHeight/4.0 + 6.0)+40.0, curHeight)));
    }
    ofPolyline line(pts);
    ofSetColor(color);
    ofFill();
    line.draw();
    
    ofSetColor(185.0);
    ofFill();
    ofDrawBitmapString("Light %:", 0, 3.0*curHeight+16.0);
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
        //if (x != diff/2 && x !)
        ofColor thisBarsColor = color;
        float brightness = barBrightness[(x/diff)];
        ofSetColor(185.0);
        ofFill();
        ofDrawBitmapString(ofToString((int)ofMap(brightness, 0.0, 255.0, 0.0, 100.0)), x, 3.0*(curHeight/4.0)+16.0);
        ofDrawBitmapString(ofToString((int)ofMap(brightness, 0.0, 255.0, 20.0, 60.0)), x, 3.0*(curHeight/4.0)+38.0);
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
void ofApp::keyPressed(int key){

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
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

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