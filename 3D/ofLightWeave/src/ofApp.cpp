#include "ofApp.h"

int xPos;
int yPos;
int zPos;


//vector<ofVideoGrabber> videoGrabbers;
//vector<ofxCvGrayscaleImage> grayImages, grayBgs, grayDiffs;
//vector<ofxCvContourFinder> contourFinders;
//vector<ofxCvColorImage> colorImgs;

void ofApp::setup(){
    ofEnableSmoothing();
    ofEnableAlphaBlending();
    ofSetFrameRate(60);
    ofSetVerticalSync(true);
    ofEnableDepthTest();
    ofEnableAntiAliasing();
    
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
   
/* NEW CODE: MULTIPLE CAMERAS */
//    for (int i = 1; i < devices.size(); i++) {
//        ofVideoGrabber grab;
//        grab.setDeviceID(i);
//        grab.setDesiredFrameRate(60);
//        grab.initGrabber(camWidth, camHeight);
//        videoGrabbers.push_back(grab);
//
//        ofxCvGrayscaleImage grayImage;
//        ofxCvGrayscaleImage grayBg;
//        ofxCvGrayscaleImage grayDiff;
//        ofxCvColorImage colorImg;
//        grayImage.allocate(320,240);
//        grayBg.allocate(320,240);
//        grayDiff.allocate(320,240);
//        colorImg.allocate(320,240);
//        grayImages.push_back(grayImage);
//        grayBgs.push_back(grayBg);
//        grayDiffs.push_back(grayImage);
//        ofxCvContourFinder *findeR = new ofxCvContourFinder;
//        contourFinders.push_back(*findeR);
//        colorImgs.push_back(colorImg);
//    }

/* OLD CODE: ONE CAMERAS */
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
    
    xPos = 0;
    yPos = 0;
    zPos = 0;
    
    cam.setPosition(result["region0"]["ring0"]["point0"][0].asFloat(),result["region0"]["ring0"]["point0"][1].asFloat(),result["region0"]["ring0"]["point0"][2].asFloat());
    cam.lookAt(ofVec3f(result["region0"]["ring1"]["point0"][0].asFloat(),result["region0"]["ring1"]["point0"][1].asFloat(),result["region0"]["ring1"]["point0"][2].asFloat()));
    cam.rotate(ofRadToDeg(PI/2), 1.0, 0.0, 0.0);
    cam.setFov(32.0);
    
    
}

void ofApp::update() {
    //cam.setPosition(ofMap(mouseX, 0, ofGetWidth(), 2477.176854, -4000.0), 1000, ofMap(mouseY, 0, ofGetHeight(), 1600, -1600));

    // CV
    bool bNewFrame = false;
    
    /* OLD CODE: ONE CAMERA */
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

/* NEW CODE: MULTIPLE CAMERAS */
//    for (int i = 0; i < videoGrabbers.size(); i++) {
//        videoGrabbers[i].update();
//        bNewFrame = videoGrabbers[i].isFrameNew();
//        if (bNewFrame) {
//            colorImgs[i].setFromPixels(videoGrabbers[i].getPixels());
//            grayImages[i] = colorImgs[i];
////            if (bLearnBackground == true){
////                grayBgs[i] = grayImages[i];
////                bLearnBackground = false;
////            }
//            grayDiffs[i].absDiff(grayBgs[i], grayImages[i]);
//            grayDiffs[i].threshold(threshold);
//            //contourFinders[i].findContours(grayDiffs[i], 20, (340*240)/3, 10, true);
//        }
//   }
    
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
            
            ofVec2f norm = swarmPosition;
            bottomSwarm.b = norm.normalize().x*1280-160;
            ofVec2f btm = absColumnPositionsBottom[max_pos];
            ofVec2f desired =  btm - swarmPosition;
            float d = sqrt((desired.x*desired.x) + (desired.y+desired.y));
            desired.normalize();
            if (d < 100) {
                float m = ofMap(d, 0.0, 100.0, 0.0, 4.0);
                desired *= m;
            } else {
                desired *= 4.0;
            }
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
    ofBackground(0.0, 0.0, 0.0);
    
    ofSetColor(255.0, 255.0, 255.0);
    ofFill();
    
    /* Road Map */

    //backgroundImage.draw(0, 0);
    
    /* Draw Columns */
    
//    ofSetColor(100.0,100.0,100.0);
//    ofFill();
//    for (int region = 0; region < 6; region++) {
//        string reg = "region" + ofToString(region);
//        for (int pointPos = 0; pointPos < result[reg].size(); pointPos++) {
//            string point = "point" + ofToString(pointPos);
//            ofEllipse(columnGeometry[reg][point][0].asFloat(), 900-columnGeometry[reg][point][1].asFloat(), 10, 10);
//        }
//    }

    /* Office Location */

//    ofSetColor(255.0,255.0,255.0);
//    ofFill();
//    ofDrawLine(752, 586, 752, 616);
//    ofDrawLine(782, 586, 782, 616);
//    ofDrawBitmapString("Office", 742, 586);
//    
//    ofSetColor(255);
//    ofNoFill();
//    ofDrawRectangle(300, 720, 770, 50);
    
    int cnt = 0;
    for (int i = 300; i <= 300+770; i+=70) {
        ofDrawCircle(i, 745, 3);
        ofDrawBitmapString((int)micLevelsBottom[cnt], i, 760);
        cnt++;
    }
    ofPushMatrix();

    cam.begin();

//    ofRotateX(ofRadToDeg(ofMap(mouseX, 0, ofGetWidth(), -1.0, 1.0)));
//    ofRotateZ(ofRadToDeg(ofMap(mouseY, 0, ofGetHeight(), -1.0, 1.0)));
//    ofPushMatrix();
//    ofTranslate(0,0,20);
    newDrawRegion(gaussianBottom, 3, 6, false);
//    ofPopMatrix();
    cam.end();

    ofPopMatrix();
    ofSetColor(255);
    ofFill();
    ofDrawRectangle(swarmPosition.x, swarmPosition.y, 10, 50);

    sendToDMX();

//    draw the incoming, the grayscale, the bg and the thresholded difference
//    ofSetHexColor(0xffffff);
//    colorImg.draw(20,20);
//    grayImage.draw(360,20);
//    grayBg.draw(20,280);
//    grayDiff.draw(360,280);
    ofSetHexColor(0x333333);
    ofFill();

    micLevelsBottom[7] = contourFinder.nBlobs;
    for (int i = 0; i < contourFinder.nBlobs; i++){
        contourFinder.blobs[i].draw(0, 0);
    }
    ofDrawRectangle(0, 0, 320, 240);
    ofSetHexColor(0xffffff);
    ofFill();
/* NEW CODE: MULTIPLE CAMERAS */
//    ofFill();
//    ofSetHexColor(0x333333);
//    for (int i = 0; i < videoGrabbers.size(); i++) {
//        ofDrawRectangle(360*i,0,320,240);
//
//        contourFinders[i].draw(360*i,0);
//        micLevelsBottom[7] = contourFinders[i].nBlobs;
//        for (int j = 0; j < contourFinders[i].nBlobs; j++){
//            contourFinders[i].blobs[j].draw(360, 0);
//        }
//    }
//    ofSetHexColor(0xffffff);


// finally, a report:
//    ofSetHexColor(0xffffff);
//    stringstream reportStr;
//    reportStr << "bg subtraction and blob detection" << endl
//    << "press ' ' to capture bg" << endl
//    << "threshold " << threshold << " (press: +/-)" << endl
//    << "num blobs found " << contourFinder.nBlobs << ", fps: " << ofGetFrameRate();
//    ofDrawBitmapString(reportStr.str(), 20, 600);
}

//--------------------------------------------------------------

/* HELPERS */

void ofApp::sendToDMX() {
    
    ofColor c1;
    ofColor c2;
    ofColor c3;
    ofColor c4;
    
    float top_r = ofMap(backgroundLevel, 0.0, 255.0, bRed, 255.0);
    float top_g = ofMap(backgroundLevel, 0.0, 255.0, bGreen, 255.0);
    float top_b = ofMap(backgroundLevel, 0.0, 255.0, bBlue, 255.0);

    c1.r = ofMap(gaussianBottom[result["region4"]["ring0"]["point0"][0].asInt()], 51.0, 255.0, top_r, 255.0);
    c1.g = ofMap(gaussianBottom[result["region4"]["ring0"]["point0"][0].asInt()], 51.0, 255.0, top_g, 0.0);
    c1.b = ofMap(gaussianBottom[result["region4"]["ring0"]["point0"][0].asInt()], 51.0, 255.0, top_b, 0.0);

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
        ofDrawEllipse(carToDraw.pos, 10, 10);
    }
}

void ofApp::drawSounds() {
    for (auto const & soundToUpdate: sounds) {
        ofSetColor(255,255,255, ofMap(soundToUpdate.val, 100, 0, 30, 0));
        ofFill();
        ofDrawCircle(soundToUpdate.pos, ofMap(soundToUpdate.val, 100, 0, 30, 160));
    }
}

void ofApp::newDrawRegion(float gaussLevels[1280], int start, int end, bool isEvent) {
    ofSetLineWidth(2.5);
    for (int region = start; region < end; region++) {
        ofSetColor(185.0);
        ofFill();
        string reg = "region" + ofToString(region);
        for (int rings = 0; rings < result[reg].size(); rings++) {
            string ring = "ring" + ofToString(rings);
            float in = result[reg][ring]["point0"][0].asFloat();
            int inInt = (int) in;
            int gauss = gaussLevels[inInt];
            
            // Set Color
            ofColor c;

            float top_r = ofMap(backgroundLevel, 0.0, 255.0, bRed, 255.0);
            float top_g = ofMap(backgroundLevel, 0.0, 255.0, bGreen, 255.0);
            float top_b = ofMap(backgroundLevel, 0.0, 255.0, bBlue, 255.0);

            int max_pos = 0;
            int max_element = -1000;
            for (int i = 0; i < 12; i++) {
                if (micLevelsBottom[i] > max_element) {
                    max_pos = i;
                    max_element = micLevelsBottom[i];
                }
            }

            ofVec2f btm = absColumnPositionsBottom[max_pos];
            ofVec2f desired =  btm - swarmPosition;
            float d = sqrt((desired.x*desired.x) + (desired.y+desired.y));
            float r = ofMap(ofClamp(d, 0.0, 700.0), 0.0, 700.0, 25.0, 76.5);

            ofColor bleh = ofColor::fromHsb(r, 255, 255);

            c.r = ofMap(gauss, 51.0, 255.0, top_r, bleh.r);
            c.g = ofMap(gauss, 51.0, 255.0, top_g, bleh.g);
            c.b = ofMap(gauss, 51.0, 255.0, top_b, bleh.b);
            
            ofSetColor(c);
            ofFill();
            
            // LINE DRAWING
            ofPolyline line;
            for (int pointPos = 0; pointPos < 3; pointPos++) {
                string point = "point" + ofToString(pointPos);
                line.addVertex(result[reg][ring][point][0].asFloat(), 900-result[reg][ring][point][1].asFloat(), result[reg][ring][point][2].asFloat());
            }
            line.addVertex(result[reg][ring]["point0"][0].asFloat(), 900-result[reg][ring]["point0"][1].asFloat(),result[reg][ring]["point0"][2].asFloat());
            line.draw();
        }
    }
}

void ofApp::drawRegion(float gaussLevels[1280], int start, int end, bool isEvent) {
//    ofColor curLeft = gui->lColor;
//    ofColor curRight = gui->rColor;
//    ofColor curBack = gui->bColor;
//    for (int region = start; region < end; region++) {
//        ofSetColor(185.0);
//        ofFill();
//        string reg = "region" + ofToString(region);
//        for (int rings = 0; rings < result[reg].size(); rings++) {
//            string ring = "ring" + ofToString(rings);
//            float in = result[reg][ring]["point0"][0].asFloat();
//            int inInt = (int) in;
//            int gauss = gaussLevels[inInt];
//            ofPolyline line;
//            for (int pointPos = 0; pointPos < 3; pointPos++) {
//                string point = "point" + ofToString(pointPos);
//                ofColor c;
//                if (isEvent) {
//                    c.r = ofMap(gauss, 51.0, 255.0, curBack.r, eventColor.r);
//                    c.g = ofMap(gauss, 51.0, 255.0, curBack.g, eventColor.g);
//                    c.b = ofMap(gauss, 51.0, 255.0, curBack.b, eventColor.b);
//                } else {
//                    float top_r = ofMap(result[reg][ring][point][0].asFloat(), 0.0, 1280.0, curRight.r, curLeft.r);
//                    float top_g = ofMap(result[reg][ring][point][0].asFloat(), 0.0, 1280.0, curRight.g, curLeft.g);
//                    float top_b = ofMap(result[reg][ring][point][0].asFloat(), 0.0, 1280.0, curRight.b, curLeft.b);
//                    c.r = ofMap(gauss, 51.0, 255.0, curBack.r, top_r);
//                    c.g = ofMap(gauss, 51.0, 255.0, curBack.g, top_g);
//                    c.b = ofMap(gauss, 51.0, 255.0, curBack.b, top_b);
//                }
//                ofSetColor(c);
//                ofFill();
//                line.addVertex(result[reg][ring][point][0].asFloat(), 900-result[reg][ring][point][1].asFloat());
//            }
//            line.addVertex(result[reg][ring]["point0"][0].asFloat(), 900-result[reg][ring]["point0"][1].asFloat());
//            line.draw();
//        }
//    }
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
    if (key == 'a') {
        ANIMATION_STATE = ACTIVATED;
        bottomSwarm.bVel = 4.0;
        swarmVector.x = 14.0;
    } else if (key == 'd') {
        ANIMATION_STATE = DEACTIVATED;
        bottomSwarm.bVel = 1.0;
        swarmVector.x = 1.0;
    } else if (key == 'e') {
        if (ANIMATION_STATE != EVENT) {
            ANIMATION_STATE = EVENT;
        }
    } else if (key == ' '){
        bLearnBackground = true;
    } else if (key == 357) {
        cam.setPosition(cam.getX(), cam.getY(), cam.getZ() + 10.0);
    } else if (key == 359) {
        cam.setPosition(cam.getX(), cam.getY(), cam.getZ() - 10.0);

    } else if (key == 356) {
        cam.setPosition(cam.getX(), cam.getY() + 10.0, cam.getZ());

    } else if (key == 358) {
        cam.setPosition(cam.getX(), cam.getY() - 10.0, cam.getZ());

    } else if (key == 'w') {
        cam.setPosition(cam.getX() + 10.0, cam.getY(), cam.getZ());

    } else if (key == 's') {
        cam.setPosition(cam.getX() - 10.0, cam.getY(), cam.getZ());

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
void ofApp::windowResized(int w, int h) {}
void ofApp::gotMessage(ofMessage msg) {}
void ofApp::dragEvent(ofDragInfo dragInfo) {}
void ofApp::keyReleased(int key) {}
void ofApp::mouseMoved(int x, int y ) {}
void ofApp::mouseDragged(int x, int y, int button) {}
void ofApp::mousePressed(int x, int y, int button) {}
void ofApp::mouseReleased(int x, int y, int button) {}
void ofApp::mouseEntered(int x, int y) {}
void ofApp::mouseExited(int x, int y) {}