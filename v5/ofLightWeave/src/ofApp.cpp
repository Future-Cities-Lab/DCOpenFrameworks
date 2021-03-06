#include "ofApp.h"

int xPos;
int yPos;
int zPos;

float sphereXPos;
float sphereZPos;

float spheresXPos[20];
float spheresZPos[20];

ofColor cameraColor1;
ofColor cameraColor2;
ofColor columnColor;
ofColor activeColor;

float springing = .0009;
float damping = .98;


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
    
    std::string file = "Lightweave_loops2.json";
    std::string columnsFile = "Lightweave_columns2.json";
    std::string facesFile = "Lightweave_faces2.json";
    
    bool parsingSuccessful = result.open(file);
    
    bool parsingSuccessfulColumn = columnGeometry.open(columnsFile);

    bool parsingSuccessfulFaces = faceGeometry.open(facesFile);

    
    for (int region = 0; region < 6; region++) {
        string blah = "region" + ofToString(region);
        for (int rings = 0; rings < result[blah].size(); rings++) {
            string ring = "ring" + ofToString(rings);
            for (int pointPos = 0; pointPos < 3; pointPos++) {
                string point = "point" + ofToString(pointPos);
            }
        }
    }
        
    //setupUDP();
    
    camWidth = 320;
    camHeight = 240;

    vector<ofVideoDevice> devices = vidGrabber.listDevices();
    for (int i = 0; i < devices.size(); i++) {
        if (devices[i].bAvailable) {
            ofLogNotice() << devices[i].id << ": " << devices[i].deviceName;
        } else {
            ofLogNotice() << devices[i].id << ": " << devices[i].deviceName << " - unavailable ";
        }
    }
    
    for (int i = 0; i < devices.size(); i++) {
        vidGrabbers[i].setDeviceID(devices[i].id);
        vidGrabbers[i].setDesiredFrameRate(20);
        vidGrabbers[i].initGrabber(camWidth, camHeight);
    }
    
    colorImg.allocate(320,240);
    grayImage.allocate(320,240);
    grayBg.allocate(320,240);
    grayDiff.allocate(320,240);
    
    bLearnBackground = true;
    threshold = 80;
        
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
    
    sphereZPos = 25.9297;
    sphereXPos = 364.928;
    
    for (int i = 0; i < 20; i++) {
        spheresXPos[i] = ofRandom(result["region0"]["ring0"]["point0"][0].asFloat()-500, result["region0"]["ring0"]["point0"][0].asFloat()+500.0);
        spheresZPos[i] = ofRandom(result["region0"]["ring0"]["point0"][2].asFloat()-100.0, result["region0"]["ring0"]["point0"][2].asFloat()+100.0);
    }
    
    /* LIGHTING */
    ofSetSmoothLighting(true);
    

    pointLight.setDiffuseColor( ofColor(0.f, 255.f, 0.f));
    
    pointLight.setSpecularColor( ofColor(255.f, 255.f, 255.f));
    pointLight.setPosition(result["region0"]["ring0"]["point0"][0].asFloat(),result["region0"]["ring0"]["point0"][1].asFloat(),result["region0"]["ring0"]["point0"][2].asFloat());
    
    material.setShininess( 64 );
    
    colorHue = ofRandom(0, 250);
    colorHue2 = ofRandom(0, 250);

    
    lightColor.setBrightness( 180.f );
    lightColor.set(250,250,210);
    
    materialColor.setBrightness(250.f);
    materialColor.set(100,100,100);
    
    lightColor.setHue(colorHue);
    pointLight.setDiffuseColor(lightColor);
    
    materialColor.setHue(colorHue);
    material.setSpecularColor(materialColor);
    
    
    materialColor.set(255.0,0.0,0.0);
    columnMaterial.setSpecularColor(materialColor);
    
    materialColor.set(55.0,55.0,55.0);
    peopleMaterial.setSpecularColor(materialColor);
    
    
    cameraColor1.set(0.0, 0.0, 255.0);
    cameraColor2.set(0.0, 0.0, 255.0);
    columnColor.set(255, 0, 0);
    activeColor.set(0.0,0.0,255.0);

}

void ofApp::update() {
    
    for (int i = 0; i < vidGrabber.listDevices().size(); i++) {
        vidGrabbers[i].update();
    }
    
    for (int i = 0; i < vidGrabber.listDevices().size(); i++) {
        if (vidGrabbers[i].isFrameNew()) {
            colorImg.setFromPixels(vidGrabbers[i].getPixels());
            grayImage = colorImg;
            if (bLearnBackground == true) {
                grayBg = grayImage;
                bLearnBackground = false;
            }
            grayDiff.absDiff(grayBg, grayImage);
            grayDiff.threshold(30);
            contourFinder[i].findContours(grayDiff, 5, (340*240)/4, 4, false, true);
        }
    }
    
    micLevelsTopNew[4] = contourFinder[0].nBlobs;
    micLevelsTopNew[5] = contourFinder[1].nBlobs;
    
    switch (ANIMATION_STATE) {
        case ACTIVATED: {
            int max_pos = 0;
            int max_element = -1000;
            for (int i = 0; i < 12; i++) {
                if (micLevelsTopNew[i] > max_element) {
                    max_pos = i;
                    max_element = micLevelsTopNew[i];
                }
            }
        
            for (int x = 0; x < 1280; x++) {
                float top = pow(x-bottomSwarm.b,2);
                float bottom = 2*pow(bottomSwarm.c,2);
                bottomSwarm.curve[x] = bottomSwarm.a*exp(-(top/bottom));
            }
            
            ofVec2f norm = swarmPosition;
            bottomSwarm.b = norm.normalize().x*1280-160;
            ofVec2f btm = absColumnPositionTop[max_pos];
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
    }
    int cnt = 0;
    for (int i = 0; i < 12; i++) {
        if (i == 4 || i == 5) {
        } else {
            micLevelsTopNew[i] = 0.0;
        }
    }
    if (simulationIsOn) {
        int cntN = 0;
        for (int region = 3; region < 6; region++) {
            string reg = "region" + ofToString(region);
            int numCols;
            if (region == 4) {
                numCols = 3;
            } else {
                numCols = 4;
            }
            /* TODO: Did this get fucked up? */
            for (int pointPos = 0; pointPos < numCols; pointPos++) {
                string point = "point" + ofToString(pointPos);
                float colX = columnGeometry[reg][point][0].asFloat();
                for (int i = 0; i < 20; i++) {
                    if (i == 4 || i == 5) {
                    } else {
                        if (abs(spheresXPos[i]-columnGeometry[reg][point][0].asFloat()) < 100) {
                            micLevelsTopNew[cntN]++;
                        }
                    }
                }
                cntN++;
            }
        }
    }
    

}

//--------------------------------------------------------------

void ofApp::draw() {
    ofBackground(0.0, 0.0, 0.0);
    
    ofSetColor(255.0,255.0,255.0);
    ofFill();
    for (int i = 0; i < 10; i++) {
        ofDrawBitmapString(verbalInstructions[i], 10, (i*20)+600);
    }
    //ofDrawBitmapString("'I' for camera information", 10, 760);
    
    if (cameraInfoIsOn) {
        ofSetColor(255.0, 255.0, 255.0);
        ofFill();
        
        /* drawing cameras */
        ofSetColor(255.0,0.0,0.0);
        ofDrawBitmapString("Active column", 10, 30+10);
        ofSetColor(0.0,0.0,255.0);
        ofDrawBitmapString("Active camera #", 10, 50+10);
        ofSetColor(255,105,180);
        ofDrawBitmapString("# of Pedestrians", 10, 60+20);
        ofSetColor(255.0, 255.0, 255.0);
        int cnt = 0;
        for (int i = 300-70; i <= 300+700; i+=70) {
            if (cnt == 4 || cnt == 5 ) {
                ofSetColor(cameraColor1);
                ofDrawBitmapString("["+ofToString(cnt+1)+"]", i+22, 50+10);
                ofSetColor(255,105,180);
                ofDrawBitmapString((int)micLevelsTopNew[cnt], i+35, 60+10);
                ofSetColor(255.0, 255.0, 255.0);
            } else {
                if (simulationIsOn) {
                    ofSetColor(255,105,180);
                } else {
                    ofSetColor(100,100,100);
                }
                ofDrawBitmapString((int)micLevelsTopNew[cnt], i+35, 60+10);
                ofSetColor(255.0, 255.0, 255.0);
                ofDrawBitmapString("["+ofToString(cnt+1)+"]", i+22, 50+10);
            }
            cnt++;
        }
        
        /* Draw Columns */
        cnt = 0;
        ofFill();
        ofSetColor(100.0, 100.0, 100.0);
        for (int i = 300; i <= 300+700; i+=70) {
            if (cnt == 4) {
                ofSetColor(columnColor);
                ofDrawCircle(i, 45, 7);
                ofSetColor(100.0, 100.0, 100.0);
            } else {
                ofDrawCircle(i, 45, 7);
            }
            cnt++;
        }
        int max_pos = 0;
        int max_element = -1000;
        for (int i = 0; i < 12; i++) {
            if (micLevelsTopNew[i] > max_element) {
                max_pos = i;
                max_element = micLevelsTopNew[i];
            }
        }
        ofVec2f btm = absColumnPositionTop[max_pos];
        ofVec2f desired =  btm - swarmPosition;
        float d = sqrt((desired.x*desired.x) + (desired.y+desired.y));
        float r = ofMap(ofClamp(d, 0.0, 700.0), 0.0, 700.0, 25.0, 76.5);
        ofColor swarmColor = ofColor::fromHsb(r, 255, 255);
        ofSetColor(swarmColor);
        ofDrawRectangle(swarmPosition.x, 45, 10, 50);
        ofSetColor(columnColor);
        ofFill();
        ofDrawCircle(578, 270, 14);
        ofNoFill();
        ofSetColor(cameraColor1);
        ofDrawBitmapString("Camera 5", 238, 140);
        ofDrawRectangle(238, 150, 340, 240);
        ofSetColor(cameraColor2);
        ofDrawBitmapString("Camera 6", 578, 140);
        ofDrawRectangle(578, 150, 340, 240);
        for (int j = 0; j < contourFinder[0].nBlobs; j++){
            contourFinder[0].blobs[j].draw(238, 150);
        }
        for (int j = 0; j < contourFinder[1].nBlobs; j++){
            contourFinder[1].blobs[j].draw(578, 150);
        }
    }
    
    ofEnableLighting();
    pointLight.enable();
    material.begin();
    ofPushMatrix();
    cam.begin();
    peopleMaterial.begin();
    if (simulationIsOn) {
        for (int i = 0; i < 20; i++) {
            ofSpherePrimitive cyl;
            cyl.setPosition(spheresXPos[i]+=ofRandom(1.5), spheresZPos[i], 10.9297);
            cyl.set(10, 10);
            cyl.draw();
            if (spheresXPos[i] >= 0.0) {
                spheresXPos[i] = ofRandom(result["region0"]["ring0"]["point0"][0].asFloat()-500, result["region0"]["ring0"]["point0"][0].asFloat()+500.0);
            }
        }
    }
    peopleMaterial.end();
    ofSetColor(100.0);
    ofFill();
    
    int ct = 0;
    for (int region = 0; region < 3; region++) {
        string reg = "region" + ofToString(region);
        for (int pointPos = 0; pointPos < 4; pointPos++) {
            if (region == 1 && pointPos == 3) {
            } else {
                string point = "point" + ofToString(pointPos);
                ofCylinderPrimitive cyl;
                cyl.setPosition(columnGeometry[reg][point][0].asFloat(), columnGeometry[reg][point][1].asFloat(), columnGeometry[reg][point][2].asFloat()-90);
                cyl.set(2.0, 130.0);
                cyl.rotate(90, ofVec3f(1.0, 0.0, 0.0));
                if (ct == 4) {
                    columnMaterial.begin();
                    cyl.draw();
                    columnMaterial.end();
                } else {
                    cyl.draw();
                }
                ct++;
            }
        }
    }
    material.end();
    ofDisableLighting();


    newDrawRegion(gaussianBottom, 0, 3, false);
    
    ofSetColor(155.0, 155.0, 155.0);
    ofFill();
    for (int face = 0; face < 5; face++) {
        string fac = "face" + ofToString(face);
        ofPoint p1;
        ofPoint p2;
        ofPoint p3;
        ofPoint p4;
        p1.set(ofVec3f(faceGeometry[fac]["point0"][0].asFloat(),faceGeometry[fac]["point0"][1].asFloat(),faceGeometry[fac]["point0"][2].asFloat()));
        p2.set(ofVec3f(faceGeometry[fac]["point1"][0].asFloat(),faceGeometry[fac]["point1"][1].asFloat(),faceGeometry[fac]["point1"][2].asFloat()));
        p3.set(ofVec3f(faceGeometry[fac]["point2"][0].asFloat(),faceGeometry[fac]["point2"][1].asFloat(),faceGeometry[fac]["point2"][2].asFloat()));
        p4.set(ofVec3f(faceGeometry[fac]["point3"][0].asFloat(),faceGeometry[fac]["point3"][1].asFloat(),faceGeometry[fac]["point3"][2].asFloat()));
        ofDrawLine(p1, p2);
        ofDrawLine(p2, p3);
        ofDrawLine(p3, p4);
        ofDrawLine(p4, p1);
    }
    cam.end();
    ofPopMatrix();

    sendToDMX();
}

//--------------------------------------------------------------

/* HELPERS */

void ofApp::sendToDMX() {
    
    float inRes = result["region1"]["ring0"]["point0"][0].asFloat();
    float in = ofMap(inRes, -2000.0, -40.0, 0.0, 1280.0);
    int inInt = (int) in;
    int gauss = gaussianBottom[inInt];
    
    ofColor c;
    
    float top_r = ofMap(backgroundLevel, 0.0, 255.0, bRed, 255.0);
    float top_g = ofMap(backgroundLevel, 0.0, 255.0, bGreen, 255.0);
    float top_b = ofMap(backgroundLevel, 0.0, 255.0, bBlue, 255.0);
    
    int max_pos = 0;
    int max_element = -1000;
    for (int i = 0; i < 12; i++) {
        if (micLevelsTopNew[i] > max_element) {
            max_pos = i;
            max_element = micLevelsTopNew[i];
        }
    }
    
    ofVec2f btm = absColumnPositionTop[max_pos];
    ofVec2f desired =  btm - swarmPosition;
    float d = sqrt((desired.x*desired.x) + (desired.y+desired.y));
    
    float r = ofMap(ofClamp(d, 0.0, 700.0), 0.0, 700.0, 25.0, 76.5);
    
    ofColor bleh = ofColor::fromHsb(r, 255, 255);
    
    c.r = ofMap(gauss, 51.0, 255.0, top_r, bleh.r);
    c.g = ofMap(gauss, 51.0, 255.0, top_g, bleh.g);
    c.b = ofMap(gauss, 51.0, 255.0, top_b, bleh.b);
    
    dmxData_[1] = int(c.r);
    dmxData_[2] = int(c.g);
    dmxData_[3] = int(c.b);

    dmxData_[4] = int(c.r);
    dmxData_[5] = int(c.g);
    dmxData_[6] = int(c.b);

    dmxData_[7] = int(c.r);
    dmxData_[8] = int(c.g);
    dmxData_[9] = int(c.b);

    dmxData_[10] = int(c.r);
    dmxData_[11] = int(c.g);
    dmxData_[12] = int(c.b);
    dmxData_[0] = 0;

    if (!dmxInterface_ || !dmxInterface_->isOpen()) {
        printf( "Not updating, enttec device is not open.\n");
    } else {
        dmxInterface_->writeDmx( dmxData_, DMX_DATA_LENGTH );
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
            float inRes = result[reg][ring]["point0"][0].asFloat();
            float in = ofMap(inRes, -2000.0, -40.0, 0.0, 1280.0);
            
            int inInt = (int) in;
            int gauss = gaussLevels[inInt];

            ofColor c;

            float top_r = ofMap(backgroundLevel, 0.0, 255.0, bRed, 255.0);
            float top_g = ofMap(backgroundLevel, 0.0, 255.0, bGreen, 255.0);
            float top_b = ofMap(backgroundLevel, 0.0, 255.0, bBlue, 255.0);

            int max_pos = 0;
            int max_element = -1000;
            for (int i = 0; i < 12; i++) {
                if (micLevelsTopNew[i] > max_element) {
                    max_pos = i;
                    max_element = micLevelsTopNew[i];
                }
            }

            ofVec2f btm = absColumnPositionTop[max_pos];
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
                line.addVertex(result[reg][ring][point][0].asFloat(), result[reg][ring][point][1].asFloat(), result[reg][ring][point][2].asFloat());
            }
            line.addVertex(result[reg][ring]["point0"][0].asFloat(), result[reg][ring]["point0"][1].asFloat(),result[reg][ring]["point0"][2].asFloat());
            line.draw();
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
    } else if (key == 'i') {
        if (cameraInfoIsOn) {
            cameraInfoIsOn = false;
        } else {
            cameraInfoIsOn = true;
        }
    } else if (key == 'm') {
        if (simulationIsOn) {
            simulationIsOn = false;
        } else {
            simulationIsOn = true;
        }
    }
}

void ofApp::exit() {
//    if ( dmxInterface_ && dmxInterface_->isOpen() ) {
//        for ( int i = 0; i <= DMX_DATA_LENGTH; i++ ) dmxData_[i] = 0;
//        dmxInterface_->writeDmx( dmxData_, DMX_DATA_LENGTH );
//        dmxInterface_->close();
//    }
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