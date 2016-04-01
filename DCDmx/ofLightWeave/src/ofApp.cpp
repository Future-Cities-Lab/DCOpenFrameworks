#include "ofApp.h"

void ofApp::setup(){
//    ofEnableSmoothing();
//    ofEnableAlphaBlending();
//    ofSetVerticalSync(true);
//    ofSetFrameRate(60);
    
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
    
    std::string file = "Lightweave_loops.json";
    std::string columnsFile = "Lightweave_columns.json";
    
    bool parsingSuccessful = result.open(file);
    
    bool parsingSuccessfulColumn = columnGeometry.open(columnsFile);
    
    //ofArduino.connect();
    
    for (int region = 0; region < 6; region++) {
        string blah = "region" + ofToString(region);
        for (int rings = 0; rings < result[blah].size(); rings++) {
            string ring = "ring" + ofToString(rings);
            for (int pointPos = 0; pointPos < 3; pointPos++) {
                string point = "point" + ofToString(pointPos);
            }
        }
    }
    
    backgroundImage.loadImage("unnamed.jpg");
    
    //setupUDP();
    
    for (int i = 0; i < 10; i++) {
        Car carToAdd;
        carToAdd.pos = ofVec2f(ofRandom(1280), 427);
        carToAdd.vel = ofVec2f(-carSpeed, 0);
        carToAdd.counter = ofRandom(100);
        cars.push_back(carToAdd);
        
        Car carToAdd2;
        carToAdd2.pos = ofVec2f(ofRandom(1280), 493);
        carToAdd2.vel = ofVec2f(carSpeed, 0);
        carToAdd2.counter = ofRandom(100);
        cars.push_back(carToAdd2);
    }
    
    
    for (int i = 0; i < 11; i++) {
        micNoiseTop[i] = i*10;
    }
    
    for (int i = 0; i < 12; i++) {
        micNoiseBottom[i] = i*10;
    }
    
    camWidth = 320;
    camHeight = 240;
    
    vector<ofVideoDevice> devices = vidGrabber.listDevices();
    
    for(int i = 0; i < devices.size(); i++){
        if(devices[i].bAvailable){
            ofLogNotice() << devices[i].id << ": " << devices[i].deviceName;
        }else{
            ofLogNotice() << devices[i].id << ": " << devices[i].deviceName << " - unavailable ";
        }
    }
    
    vidGrabber.setDeviceID(0);
    vidGrabber.setDesiredFrameRate(60);
    vidGrabber.initGrabber(camWidth, camHeight);
    colorImg.allocate(320,240);
    grayImage.allocate(320,240);
    grayBg.allocate(320,240);
    grayDiff.allocate(320,240);
    
    bLearnBackground = true;
    threshold = 80;
    
    //videoTexture.allocate(videoInverted);
    
    bottomSwarm.a = 1.1f;
    bottomSwarm.b = (curWidth/4.0);
    bottomSwarm.c = 100.0;
    bottomSwarm.bVel = 1.0;
}

void ofApp::update() {
    
    // CV
    bool bNewFrame = false;
    vidGrabber.update();
    bNewFrame = vidGrabber.isFrameNew();
    if (bNewFrame) {
        colorImg.setFromPixels(vidGrabber.getPixels());
        grayImage = colorImg;
        if (bLearnBackground == true){
            grayBg = grayImage;
            bLearnBackground = false;
        }
        grayDiff.absDiff(grayBg, grayImage);
        grayDiff.threshold(threshold);
        contourFinder.findContours(grayDiff, 20, (340*240)/3, 10, true);
    }
    
    switch (ANIMATION_STATE) {
            
        case ACTIVATED: {
            
            for (int i = 0; i < 11; i++) {
                micNoiseTop[i] += 0.001;
                micLevelsTop[i] = ofMap(ofNoise(micNoiseTop[i], 1.0), 0.0, 1.0, 0.0, 255.0);
            }
            
            for (int i = 0; i < 12; i++) {
                if (i != 7) {
                    micNoiseBottom[i] += 0.001;
                    micLevelsBottom[i] = ofMap(ofNoise(micNoiseBottom[i], 1.0), 0.0, 1.0, -3.0, 1.0);
                }

            }
            
            int max_pos = 0;
            int max_element = -1000;
            for (int i = 0; i < 12; i++) {
                if (micLevelsBottom[i] > max_element) {
                    max_pos = i;
                    max_element = micLevelsBottom[i];
                }
            }
        
            for (int x = 0; x < 1280; x++) {
                float top = pow(x-bottomSwarm.b,2);
                float bottom = 2*pow(bottomSwarm.c,2);
                bottomSwarm.curve[x] = bottomSwarm.a*exp(-(top/bottom));
            }
            
            //bottomSwarm.b += bottomSwarm.bVel;
            ofVec2f norm = swarmPosition;
            bottomSwarm.b = norm.normalize().x*1280-160;
            
//            if (bottomSwarm.b >= ofGetWidth() || bottomSwarm.b <= 0) {
//                bottomSwarm.bVel*=-1.0;
//            }
//            bottomSwarm.c*=growthRate;
//            
            
            ofVec2f btm = absColumnPositionsBottom[max_pos];
            ofVec2f desired =  btm - swarmPosition;
            desired.normalize();
            desired *= 1.0;
            swarmPosition += desired;
            
            /* UPDATE WAVES */
            for (int x = 0; x < 1280; x++) {
                gaussianBottom[x] = ofMap(bottomSwarm.curve[x], 0.0, 1.1, ambientLevel, 255.0);
            }
            break;
        }
        case DEACTIVATED: {
            
            for (int x = 0; x < 1280; x++) {
                float top = pow(x-bottomSwarm.b,2);
                float bottom = 2*pow(bottomSwarm.c,2);
                bottomSwarm.curve[x] = bottomSwarm.a*exp(-(top/bottom));
            }
            
            swarmPosition += swarmVector;
            if (swarmPosition.x >= 300+770 || swarmPosition.x <= 300) {
                swarmVector *= -1.0;
            }
            
            ofVec2f norm = swarmPosition;
            bottomSwarm.b = norm.normalize().x*1280.0-160;
            
            for (int x = 0; x < 1280; x++) {
                gaussianBottom[x] = ofMap(bottomSwarm.curve[x], 0.0, 1.1, ambientLevel, 255.0);
            }
            break;
        }
        case EVENT: {
            break;
        }
    }

}

//--------------------------------------------------------------

void ofApp::draw() {
    
    ofSetColor(255.0, 255.0, 255.0);
    ofFill();
    
    /* Road Map */

    backgroundImage.draw(0, 0);
    
    /* Draw Columns */
    
    ofSetColor(100.0,100.0,100.0);
    ofFill();
    for (int region = 0; region < 6; region++) {
        string reg = "region" + ofToString(region);
        for (int pointPos = 0; pointPos < result[reg].size(); pointPos++) {
            string point = "point" + ofToString(pointPos);
            ofEllipse(columnGeometry[reg][point][0].asFloat(), 900-columnGeometry[reg][point][1].asFloat(), 10, 10);
        }
    }

    /* Office Location */

    ofSetColor(255.0,255.0,255.0);
    ofFill();
    ofDrawLine(752, 586, 752, 616);
    ofDrawLine(782, 586, 782, 616);
    ofDrawBitmapString("Office", 742, 586);
    
    ofSetColor(255);
    ofNoFill();
    ofDrawRectangle(300, 720, 770, 50);
    
    int cnt = 0;
    for (int i = 300; i <= 300+770; i+=70) {
        ofDrawCircle(i, 745, 3);
        ofDrawBitmapString((int)micLevelsBottom[cnt], i, 760);
        cnt++;
    }
    switch (ANIMATION_STATE) {
        case ACTIVATED: {
            //newDrawRegion(gaussian, 0, 3, false);
            newDrawRegion(gaussianBottom, 3, 6, false);
            ofSetColor(255);
            ofFill();
            ofDrawRectangle(swarmPosition.x, swarmPosition.y, 10, 50);
            break;
        }
        case DEACTIVATED: {
            //newDrawRegion(gaussian, 0, 3, false);
            newDrawRegion(gaussianBottom, 3, 6, false);
            //sendToDMX();
            ofSetColor(255);
            ofFill();
            ofDrawRectangle(swarmPosition.x, swarmPosition.y, 10, 50);
            break;
        }
        case EVENT: {

            break;
        }
    }
    sendToDMX();

    // draw the incoming, the grayscale, the bg and the thresholded difference
    ofSetHexColor(0xffffff);
//    colorImg.draw(20,20);
//    grayImage.draw(360,20);
//    grayBg.draw(20,280);
//    grayDiff.draw(360,280);
    
    // then draw the contours:
    
    ofFill();
    ofSetHexColor(0x333333);
    ofDrawRectangle(360,0,320,240);
    ofSetHexColor(0xffffff);
    
    // we could draw the whole contour finder
    contourFinder.draw(360,0);
    micLevelsBottom[7] = contourFinder.nBlobs;
    for (int i = 0; i < contourFinder.nBlobs; i++){
        contourFinder.blobs[i].draw(360, 0);
    }
    
    // finally, a report:
    ofSetHexColor(0xffffff);
    stringstream reportStr;
    reportStr << "bg subtraction and blob detection" << endl
    << "press ' ' to capture bg" << endl
    << "threshold " << threshold << " (press: +/-)" << endl
    << "num blobs found " << contourFinder.nBlobs << ", fps: " << ofGetFrameRate();
    ofDrawBitmapString(reportStr.str(), 20, 600);
}

//--------------------------------------------------------------

/* HELPERS */

//void ofApp::addWave(vector<Wave> *wavesToAddTo, float velocity) {
//    Wave newWave;
//    newWave.a = 1.1f;
//    if (velocity == -1.0) {
//        newWave.b = (3.0*curWidth/4.0);
//    } else {
//        newWave.b = (curWidth/4.0);
//    }
//    newWave.c = 100.0;
//    newWave.bVel = 10.0;
//    newWave.bVel *= velocity;
//    newWave.growing = true;
//    (*wavesToAddTo).push_back(newWave);
//}

//void ofApp::removeWaves(vector<Wave> *wavesToRemove, float velocity) {
//    vector<int> toErase;
//    for (int i = 0; i < (*wavesToRemove).size(); i++) {
//        if ((*wavesToRemove)[i].a < 1.0) {
//            toErase.push_back(i);
//        }
//    }
//    for (int i = 0; i < toErase.size(); i++) {
//        int pos = toErase[i];
//        (*wavesToRemove).erase((*wavesToRemove).begin() + pos);
//    }
//    for (int i = 0; i < toErase.size(); i++) {
//        addWave(wavesToRemove, velocity);
//    }
//}

//void ofApp::updateWaves(vector<Wave> *wavesToUpate) {
//    for (int i = 0; i < (*wavesToUpate).size(); i++) {
//        for (int x = 0; x < 1280; x++) {
//            float top = pow(x-(*wavesToUpate)[i].b,2);
//            float bottom = 2*pow((*wavesToUpate)[i].c,2);
//            (*wavesToUpate)[i].curve[x] = (*wavesToUpate)[i].a*exp(-(top/bottom));
//        }
//        (*wavesToUpate)[i].b += (*wavesToUpate)[i].bVel;
//        if ((*wavesToUpate)[i].b >= ofGetWidth() || (*wavesToUpate)[i].b <= 0) {
//            (*wavesToUpate)[i].bVel*=-1.0;
//        }
//        if ((*wavesToUpate)[i].a < (curHeight/4.0) - 6.0 && (*wavesToUpate)[i].growing) {
//            (*wavesToUpate)[i].a*=aGrowthRate;
//        } else {
//            (*wavesToUpate)[i].growing = false;
//            (*wavesToUpate)[i].a*=decayRate;
//        }
//        if (!(*wavesToUpate)[i].growing) {
//            (*wavesToUpate)[i].bVel*=bAccel;
//        }
//        (*wavesToUpate)[i].c*=growthRate;
//    }
//}

void ofApp::sendToDMX() {
//    ofColor curLeft = gui->lColor;
//    ofColor curRight = gui->rColor;
//    ofColor curBack = gui->bColor;
    
    ofColor c1;
    ofColor c2;
    ofColor c3;
    ofColor c4;
    
    float top_r = ofMap(backgroundLevel, 0.0, 255.0, bRed, 255.0);
    float top_g = ofMap(backgroundLevel, 0.0, 255.0, bGreen, 255.0);
    float top_b = ofMap(backgroundLevel, 0.0, 255.0, bBlue, 255.0);

//    float top_r_1 = ofMap(result["region0"]["ring0"]["point0"][0].asFloat(), 0.0, 1280.0, curLeft.r, curRight.r);
//    float top_g_1 = ofMap(result["region0"]["ring0"]["point0"][0].asFloat(), 0.0, 1280.0, curLeft.g, curRight.g);
//    float top_b_1 = ofMap(result["region0"]["ring0"]["point0"][0].asFloat(), 0.0, 1280.0, curLeft.b, curRight.b);
//
//    float top_r_2 = ofMap(result["region0"]["ring1"]["point0"][0].asFloat(), 0.0, 1280.0, curLeft.r, curRight.r);
//    float top_g_2 = ofMap(result["region0"]["ring1"]["point0"][0].asFloat(), 0.0, 1280.0, curLeft.g, curRight.g);
//    float top_b_2 = ofMap(result["region0"]["ring1"]["point0"][0].asFloat(), 0.0, 1280.0, curLeft.b, curRight.b);
//
//    float top_r_3 = ofMap(result["region0"]["ring2"]["point0"][0].asFloat(), 0.0, 1280.0, curLeft.r, curRight.r);
//    float top_g_3 = ofMap(result["region0"]["ring2"]["point0"][0].asFloat(), 0.0, 1280.0, curLeft.g, curRight.g);
//    float top_b_3 = ofMap(result["region0"]["ring2"]["point0"][0].asFloat(), 0.0, 1280.0, curLeft.b, curRight.b);
//
//    float top_r_4 = ofMap(result["region0"]["ring3"]["point0"][0].asFloat(), 0.0, 1280.0, curLeft.r, curRight.r);
//    float top_g_4 = ofMap(result["region0"]["ring3"]["point0"][0].asFloat(), 0.0, 1280.0, curLeft.g, curRight.g);
//    float top_b_4 = ofMap(result["region0"]["ring3"]["point0"][0].asFloat(), 0.0, 1280.0, curLeft.b, curRight.b);
    
    //cout << gaussianBottom[result["region4"]["ring0"]["point0"][0].asInt()] << endl;

    c1.r = ofMap(gaussianBottom[result["region4"]["ring0"]["point0"][0].asInt()], 51.0, 255.0, top_r, 255.0);
    c1.g = ofMap(gaussianBottom[result["region4"]["ring0"]["point0"][0].asInt()], 51.0, 255.0, top_g, 0.0);
    c1.b = ofMap(gaussianBottom[result["region4"]["ring0"]["point0"][0].asInt()], 51.0, 255.0, top_b, 0.0);
    
//    cout << ofMap(gaussianBottom[result["region4"]["ring0"]["point0"][0].asInt()], 51.0, 255.0, top_r, 255.0) << endl;
//    cout << ofMap(gaussianBottom[result["region4"]["ring0"]["point0"][0].asInt()], 51.0, 255.0, top_g, 0.0) << endl;
//    cout << ofMap(gaussianBottom[result["region4"]["ring0"]["point0"][0].asInt()], 51.0, 255.0, top_b, 0.0) << endl;
//    cout << "" << endl;


    c2.r = ofMap(gaussianBottom[result["region4"]["ring0"]["point0"][0].asInt()], 51.0, 255.0, top_r, 255.0);
    c2.g = ofMap(gaussianBottom[result["region4"]["ring0"]["point0"][0].asInt()], 51.0, 255.0, top_g, 0.0);
    c2.b = ofMap(gaussianBottom[result["region4"]["ring0"]["point0"][0].asInt()], 51.0, 255.0, top_b, 0.0);

    c3.r = ofMap(gaussianBottom[result["region4"]["ring0"]["point0"][0].asInt()], 51.0, 255.0, top_r, 255.0);
    c3.g = ofMap(gaussianBottom[result["region4"]["ring0"]["point0"][0].asInt()], 51.0, 255.0, top_g, 0.0);
    c3.b = ofMap(gaussianBottom[result["region4"]["ring0"]["point0"][0].asInt()], 51.0, 255.0, top_b, 0.0);

    c4.r = ofMap(gaussianBottom[result["region4"]["ring0"]["point0"][0].asInt()], 51.0, 255.0, top_r, 255.0);
    c4.g = ofMap(gaussianBottom[result["region4"]["ring0"]["point0"][0].asInt()], 51.0, 255.0, top_g, 0.0);
    c4.b = ofMap(gaussianBottom[result["region4"]["ring0"]["point0"][0].asInt()], 51.0, 255.0, top_b, 0.0);
    


    dmxData_[1] = int(c1.r);
    dmxData_[2] = int(c1.g);
    dmxData_[3] = int(c1.b);

    dmxData_[4] = int(c2.r);
    dmxData_[5] = int(c2.g);
    dmxData_[6] = int(c2.b);

    dmxData_[7] = int(c3.r);
    dmxData_[8] = int(c3.g);
    dmxData_[9] = int(c3.b);

    dmxData_[10] = int(c4.r);
    dmxData_[11] = int(c4.g);
    dmxData_[12] = int(c4.b);

    dmxData_[0] = 0;

    if (!dmxInterface_ || !dmxInterface_->isOpen()) {
        printf( "Not updating, enttec device is not open.\n");
    } else {
        dmxInterface_->writeDmx( dmxData_, DMX_DATA_LENGTH );
    }
}

void ofApp::drawCars() {
    ofSetColor(255.0);
    ofFill();
    for (auto const & carToDraw: cars) {
        ofEllipse(carToDraw.pos, 10, 10);
    }
}

void ofApp::drawSounds() {
    for (auto const & soundToUpdate: sounds) {
        ofSetColor(255,255,255, ofMap(soundToUpdate.val, 100, 0, 30, 0));
        ofFill();
        ofCircle(soundToUpdate.pos, ofMap(soundToUpdate.val, 100, 0, 30, 160));
    }
}

void ofApp::newDrawRegion(float gaussLevels[1280], int start, int end, bool isEvent) {
    for (int region = start; region < end; region++) {
        ofSetColor(185.0);
        ofFill();
        string reg = "region" + ofToString(region);
        for (int rings = 0; rings < result[reg].size(); rings++) {
            string ring = "ring" + ofToString(rings);
            float in = result[reg][ring]["point0"][0].asFloat();
            int inInt = (int) in;
            int gauss = gaussLevels[inInt];
            ofPolyline line;
            for (int pointPos = 0; pointPos < 3; pointPos++) {
                string point = "point" + ofToString(pointPos);
                ofColor c;
                
                float top_r = ofMap(backgroundLevel, 0.0, 255.0, bRed, 255.0);
                float top_g = ofMap(backgroundLevel, 0.0, 255.0, bGreen, 255.0);
                float top_b = ofMap(backgroundLevel, 0.0, 255.0, bBlue, 255.0);
                
                c.r = ofMap(gauss, 51.0, 255.0, top_r, 255.0);
                c.g = ofMap(gauss, 51.0, 255.0, top_g, 0.0);
                c.b = ofMap(gauss, 51.0, 255.0, top_b, 0.0);
                
                ofSetColor(c);
                ofFill();
                line.addVertex(result[reg][ring][point][0].asFloat(), 900-result[reg][ring][point][1].asFloat());
            }
            line.addVertex(result[reg][ring]["point0"][0].asFloat(), 900-result[reg][ring]["point0"][1].asFloat());
            line.draw();
        }
    }
}

void ofApp::drawRegion(float gaussLevels[1280], int start, int end, bool isEvent) {
    ofColor curLeft = gui->lColor;
    ofColor curRight = gui->rColor;
    ofColor curBack = gui->bColor;
    for (int region = start; region < end; region++) {
        ofSetColor(185.0);
        ofFill();
        string reg = "region" + ofToString(region);
        for (int rings = 0; rings < result[reg].size(); rings++) {
            string ring = "ring" + ofToString(rings);
            float in = result[reg][ring]["point0"][0].asFloat();
            int inInt = (int) in;
            int gauss = gaussLevels[inInt];
            ofPolyline line;
            for (int pointPos = 0; pointPos < 3; pointPos++) {
                string point = "point" + ofToString(pointPos);
                ofColor c;
                if (isEvent) {
                    c.r = ofMap(gauss, 51.0, 255.0, curBack.r, eventColor.r);
                    c.g = ofMap(gauss, 51.0, 255.0, curBack.g, eventColor.g);
                    c.b = ofMap(gauss, 51.0, 255.0, curBack.b, eventColor.b);
                } else {
                    float top_r = ofMap(result[reg][ring][point][0].asFloat(), 0.0, 1280.0, curRight.r, curLeft.r);
                    float top_g = ofMap(result[reg][ring][point][0].asFloat(), 0.0, 1280.0, curRight.g, curLeft.g);
                    float top_b = ofMap(result[reg][ring][point][0].asFloat(), 0.0, 1280.0, curRight.b, curLeft.b);
                    c.r = ofMap(gauss, 51.0, 255.0, curBack.r, top_r);
                    c.g = ofMap(gauss, 51.0, 255.0, curBack.g, top_g);
                    c.b = ofMap(gauss, 51.0, 255.0, curBack.b, top_b);
                }
                ofSetColor(c);
                ofFill();
                line.addVertex(result[reg][ring][point][0].asFloat(), 900-result[reg][ring][point][1].asFloat());
            }
            line.addVertex(result[reg][ring]["point0"][0].asFloat(), 900-result[reg][ring]["point0"][1].asFloat());
            line.draw();
        }
    }
}

void ofApp::checkMics() {
    std::string nextMessage = "&&&";
    for (int i = 0; i < 3; i++) {
        moduleConnections[i].Send(nextMessage.c_str(), nextMessage.size());
        ofSleepMillis(100);
        char udpMessage[1000];
        auto ret = udpConnectionBroadcast.Receive(udpMessage, 1000);
        while (udpMessage[0] != '\0') {
            //int msg = udpMessage[0];
            cout << (int) udpMessage[0] << endl;
            std::fill_n(udpMessage, 1000, 0);
            auto ret = udpConnectionBroadcast.Receive(udpMessage, 1000);
        }
    }
}

void ofApp::setupUDP() {
    udpConnectionBroadcast.Create();
    udpConnectionBroadcast.SetNonBlocking(true);
    udpConnectionBroadcast.Bind(6000);
    udpConnectionBroadcast.Connect("192.168.2.255", 8888);
    udpConnectionBroadcast.SetEnableBroadcast(true);

    std::string messageSent = "**";
    udpConnectionBroadcast.Send(messageSent.c_str(), messageSent.size());


    // WAIT
    ofSleepMillis(100);

    char udpMessage[1000];
    auto ret = udpConnectionBroadcast.Receive(udpMessage, 1000);
    while (udpMessage[0] != '\0') {

        ofxUDPManager udpConnectionRx;
        moduleConnections[((int)udpMessage[0]) - 100] = udpConnectionRx;
        moduleConnections[((int)udpMessage[0]) - 100].Create();
        moduleConnections[((int)udpMessage[0]) - 100].SetNonBlocking(true);
        moduleConnections[((int)udpMessage[0]) - 100].Bind(6000);

        string ip = "192.168.2." + ofToString((int)udpMessage[0]);
        cout << ip << endl;
        moduleConnections[((int)udpMessage[0]) - 100].Connect(ip.c_str(), 8888);
        moduleConnections[((int)udpMessage[0]) - 100].SetEnableBroadcast(false);
        //moduleConnections[((int)udpMessage[0]) - 100] = udpConnectionRx;

        std::fill_n(udpMessage, 1000, 0);
        auto ret = udpConnectionBroadcast.Receive(udpMessage, 1000);
        
    }
    
    ofSleepMillis(1000);
}

//--------------------------------------------------------------

/* OF SYSTEM */

void ofApp::keyPressed(int key) {
    cout << endl << endl;
    if (key == 'a') {
        
        ANIMATION_STATE = ACTIVATED;
//        waves2.clear();
//        waves.clear();
    } else if (key == 'd') {
        ANIMATION_STATE = DEACTIVATED;
//        waves2.clear();
//        waves.clear();
    } else if (key == 'e') {
        if (ANIMATION_STATE != EVENT) {
            ANIMATION_STATE = EVENT;
//            ofColor curLeft = gui->lColor;
//            ofColor curRight = gui->rColor;
//            float spot = ofRandom(10.0);
//            float newR = ofMap(spot, 0.0, 10.0, curLeft.r, curRight.r);
//            float newG = ofMap(spot, 0.0, 10.0, curLeft.g, curRight.g);
//            float newB = ofMap(spot, 0.0, 10.0, curLeft.b, curRight.b);
//            eventColor = ofColor(newR, newG, newB);
//            eventPosition = (int)ofRandom(23);
//            eventPos = eventPosition;
//            eventLevel = 255.0;
//            if (eventPosition > 12) {
//                waves2.clear();
//                for (int i = 0; i < 1280; i++) {
//                    gaussianBottom[i] = 255.0;
//                }
//            } else {
//                waves.clear();
//                for (int i = 0; i < 1280; i++) {
//                    gaussian[i] = 255.0;
//                }
//            }
        }
    } else if (key == ' '){
        bLearnBackground = true;
    }
}

void ofApp::exit() {
    if ( dmxInterface_ && dmxInterface_->isOpen() ) {
        for ( int i = 0; i <= DMX_DATA_LENGTH; i++ ) dmxData_[i] = 0;
        dmxInterface_->writeDmx( dmxData_, DMX_DATA_LENGTH );
        dmxInterface_->close();
    }
    ofSoundStreamStop();
    ofSoundStreamClose();
}

//--------------------------------------------------------------

/* UNUSED */

void ofApp::windowResized(int w, int h){}
void ofApp::gotMessage(ofMessage msg){}
void ofApp::dragEvent(ofDragInfo dragInfo){}
void ofApp::keyReleased(int key) {}
void ofApp::mouseMoved(int x, int y ){}
void ofApp::mouseDragged(int x, int y, int button){}
void ofApp::mousePressed(int x, int y, int button) {
    cout << x << endl;
    cout << y << endl;
    cout << "" << endl;
}
void ofApp::mouseReleased(int x, int y, int button){}
void ofApp::mouseEntered(int x, int y){}
void ofApp::mouseExited(int x, int y){}