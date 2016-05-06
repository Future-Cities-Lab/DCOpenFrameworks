#include "ofApp.h"

int xPos;
int yPos;
int zPos;

float sphereXPos;
float sphereZPos;

float spheresXPos[20];
float spheresZPos[20];

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
    
    backgroundImage.loadImage("unnamed.jpg");
    
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
   
    vidGrabber.setDeviceID(0);
    vidGrabber.setDesiredFrameRate(60);
    vidGrabber.initGrabber(camWidth, camHeight);
    
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
    lightColor.setSaturation( 150.f );
    
    materialColor.setBrightness(250.f);
    materialColor.setSaturation(200);
    
    lightColor.setHue(colorHue);
    pointLight.setDiffuseColor(lightColor);
    materialColor.setHue(colorHue);
    material.setSpecularColor(materialColor);
}

void ofApp::update() {
    vidGrabber.update();
    //do we have a new frame?
    if (vidGrabber.isFrameNew()){
        colorImg.setFromPixels(vidGrabber.getPixels());
        grayImage = colorImg; // convert our color image to a grayscale image
        if (bLearnBackground == true) {
            grayBg = grayImage; // update the background image
            bLearnBackground = false;
        }
        grayDiff.absDiff(grayBg, grayImage);
        grayDiff.threshold(30);
        contourFinder.findContours(grayDiff, 5, (340*240)/4, 4, false, true);
    }
    
    switch (ANIMATION_STATE) {
        case ACTIVATED: {
            int max_pos = 0;
            int max_element = -1000;
            for (int i = 0; i < 11; i++) {
                if (micLevelsTop[i] > max_element) {
                    max_pos = i;
                    max_element = micLevelsTop[i];
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
        case EVENT: {
            break;
        }
    }
    int cnt = 0;
    for (int i = 0; i < 11; i++) {
        if (i != 4) {
            micLevelsTop[i] = 0.0;
        }
    }
    int cntN = 0;
    for (int region = 3; region < 6; region++) {
        string reg = "region" + ofToString(region);
        int numCols;
        if (region == 4) {
            numCols = 3;
        } else {
            numCols = 4;
        }
        for (int pointPos = 0; pointPos < numCols; pointPos++) {
            string point = "point" + ofToString(pointPos);
            float colX = columnGeometry[reg][point][0].asFloat();
            for (int i = 0; i < 20; i++) {
                if (i != 4) {
                    if (abs(spheresXPos[i]-columnGeometry[reg][point][0].asFloat()) < 100) {
                        micLevelsTop[cntN]++;
                    }
                }
            }
            cntN++;
        }
    }
    

}

//--------------------------------------------------------------

void ofApp::draw() {
    ofBackground(0.0, 0.0, 0.0);
    
    ofEnableLighting();
    pointLight.enable();
    material.begin();
    ofPushMatrix();
    cam.begin();

    ofSetColor(255);
    ofFill();

    for (int i = 0; i < 20; i++) {
        ofCylinderPrimitive cyl;
        cyl.setPosition(spheresXPos[i]+=ofRandom(0.5), spheresZPos[i], 28.9297);
        cyl.set(2.0, 55.0);
        cyl.rotate(90, ofVec3f(1.0, 0.0, 0.0));
        cyl.draw();
        if (spheresXPos[i] >= 0.0) {
            spheresXPos[i] = ofRandom(result["region0"]["ring0"]["point0"][0].asFloat()-500, result["region0"]["ring0"]["point0"][0].asFloat()+500.0);
        }
    }
    
    ofSetColor(100.0);
    ofFill();
    int ct = 0;
    for (int region = 0; region < 3; region++) {
        string reg = "region" + ofToString(region);
        for (int pointPos = 0; pointPos < 4; pointPos++) {
            lightColor.setHue(colorHue);
            pointLight.setDiffuseColor(lightColor);
            if (region == 1 && pointPos == 3) {
                
            } else {
                string point = "point" + ofToString(pointPos);
                ofCylinderPrimitive cyl;
                cyl.setPosition(columnGeometry[reg][point][0].asFloat(), columnGeometry[reg][point][1].asFloat(), columnGeometry[reg][point][2].asFloat()-90);
                cyl.set(2.0, 130.0);
                cyl.rotate(90, ofVec3f(1.0, 0.0, 0.0));
                if (ct == 4) {
//                    ofSetColor(255.0, 0.0, 255.0);
//                    ofFill();
                    lightColor.setHue(colorHue2);
                    pointLight.setDiffuseColor(lightColor);
                    cyl.set(8.0, 130.0);

                    cyl.draw();
                    ofSetColor(100.0);
                    ofFill();
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

    ofSetColor(255.0,255.0,255.0);
    ofFill();
    for (int i = 0; i < 8; i++) {
        ofDrawBitmapString(verbalInstructions[i], 10, (i*10)+700);
    }
    ofDrawBitmapString("Press 'I' for camera information", 10, 800);

    if (cameraInfoIsOn) {
        ofSetColor(255.0, 255.0, 255.0);
        ofFill();
        int cnt = 0;
        for (int i = 300; i <= 300+700; i+=70) {
            if (cnt == 4) {
                ofSetColor(255.0, 0.0, 255.0);
                ofFill();
                ofDrawCircle(i, 45, 3);
                ofDrawBitmapString((int)micLevelsTop[cnt], i, 60);
                ofSetColor(255.0, 255.0, 255.0);
                ofFill();
            } else {
                ofDrawCircle(i, 45, 3);
                ofDrawBitmapString((int)micLevelsTop[cnt], i, 60);
            }
            cnt++;
        }

        ofDrawRectangle(swarmPosition.x, 45, 10, 50);
        
        ofNoFill();
        ofDrawRectangle(560, 150, 340, 240);
        for (int j = 0; j < contourFinder.nBlobs; j++){
            contourFinder.blobs[j].draw(560, 150);
        }
        ofSetColor(255.0,255.0,255.0);
        ofDrawRectangle(560-340, 150, 340, 240);
        ofDrawRectangle(560+340, 150, 340, 240);
    }

    micLevelsTop[4] = contourFinder.nBlobs;

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
    for (int i = 0; i < 11; i++) {
        if (micLevelsTop[i] > max_element) {
            max_pos = i;
            max_element = micLevelsTop[i];
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
            for (int i = 0; i < 11; i++) {
                if (micLevelsTop[i] > max_element) {
                    max_pos = i;
                    max_element = micLevelsTop[i];
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
    } else if (key == 'i') {
        if (cameraInfoIsOn) {
            cameraInfoIsOn = false;
        } else {
            cameraInfoIsOn = true;
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