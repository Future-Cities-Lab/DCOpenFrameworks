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

float springing =      .0009;
float damping   =      .98;

/* 
 NEW CAMERA CODE:

 */
ofVideoGrabber 		    vidGrabber;
ofVideoGrabber 		    vidGrabber1;
vector<int> pcCams;


ofxCvColorImage			colorImg;
ofxCvGrayscaleImage 	grayImage;
ofxCvGrayscaleImage 	grayBg;
ofxCvGrayscaleImage 	grayDiff;
ofxCvContourFinder 	    contourFinder;


ofxCvColorImage			colorImg1;
ofxCvGrayscaleImage 	grayImage1;
ofxCvGrayscaleImage 	grayBg1;
ofxCvGrayscaleImage 	grayDiff1;
ofxCvContourFinder 	    contourFinder1;

int 				    threshold;
bool				    bLearnBackground;
bool				    bLearnBackground1;

bool                    drawOne;

int timeToReset = 20000;
float lastTime = 0.0;

float camera1BackgroundLevel = 0.0;
float camera2BackgroundLevel = 0.0;

int numBlobsNeeded = 3;
float minBlobArea = 800.0;
int previousBlobCount1 = 0;
int previousBlobCount2 = 0;

float avgYPosOfBlobs = 0.0;

//float slidePosition = 0.0;
ofVec2f horizontalPosition = ofVec2f(0.0, 0.0);

int sideSection = 1;
float sideLevel = 0.0;


/* NEW CODE: SECTIONS OF 3 */
float leftLevel = 0.0;
float centerLevel = 0.0;
float rightLevel = 0.0;

float frontLevel = 0.0;
float backLevel = 0.0;


void ofApp::setup() {
    
    ofEnableSmoothing();
    ofEnableAlphaBlending();
    ofSetFrameRate(60);
    ofSetVerticalSync(true);
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
        if (!devices[i].deviceName.find("USB")) {
            cout << devices[i].id << endl;
            pcCams.push_back(devices[i].id);
        }
    }
    

    vidGrabber.setDeviceID(pcCams[0]);
//    vidGrabber.setDeviceID(0);

    vidGrabber.initGrabber(320,240);
    
    vidGrabber1.setDeviceID(pcCams[1]);
//    vidGrabber1.setDeviceID(0);

    vidGrabber1.initGrabber(320,240);
    
    colorImg1.allocate(320,240);
    grayImage1.allocate(320,240);
    grayBg1.allocate(320,240);
    grayDiff1.allocate(320,240);
    
    colorImg.allocate(320,240);
    grayImage.allocate(320,240);
    grayBg.allocate(320,240);
    grayDiff.allocate(320,240);
    
    bLearnBackground = true;
    bLearnBackground1 = true;
    threshold = 80;
    drawOne = false;
    
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

    gui.setup();
    gui.add(ambientColor.setup("ambientColor", ofColor(100, 100, 140), ofColor(0, 0), ofColor(255, 255)));
    gui.add(swarmColor.setup("swarmColor", ofColor(100, 100, 140), ofColor(0, 0), ofColor(255, 255)));
    gui.add(addressableColor.setup("addressableColor", ofColor(100, 100, 140), ofColor(0, 0), ofColor(255, 255)));
    gui.loadFromFile("settings.xml");

    
}

void ofApp::update() {
    
    if (ofGetElapsedTimeMillis() - lastTime >= timeToReset) {
        lastTime = ofGetElapsedTimeMillis();
        bLearnBackground = true;
        bLearnBackground1 = true;
    }
    
    
    micLevelsTopNew[4] = contourFinder.nBlobs;
    micLevelsTopNew[5] = contourFinder1.nBlobs;
    
    
    /* NEW CAMERA CODE */
    
    bool bNewFrame = false;
    
    bool bNewFrame1 = false;
    
    vidGrabber.update();
    bNewFrame = vidGrabber.isFrameNew();
    
    vidGrabber1.update();
    bNewFrame1 = vidGrabber.isFrameNew();
    
    if (bNewFrame) {
        
        colorImg.setFromPixels(vidGrabber.getPixels(), 320,240);
        
        grayImage = colorImg;
        if (bLearnBackground == true){
            grayBg = grayImage;		// the = sign copys the pixels from grayImage into grayBg (operator overloading)
            bLearnBackground = false;
        }
        grayDiff.absDiff(grayBg, grayImage);
        grayDiff.threshold(threshold);
        contourFinder.findContours(grayDiff, 20, (340*240)/3, 10, true);
        
        bool leftHasIt = false;
        bool centerHasIt = false;
        bool rightHasIt = false;
        
        for (int i = 0; i < contourFinder.nBlobs; i++) {
            if (contourFinder.blobs[i].centroid.y >= 0.0 && contourFinder.blobs[i].centroid.y <= (240.0/3.0)) {
                leftHasIt = true;
            } else if (contourFinder.blobs[i].centroid.y > (240.0/3.0) && contourFinder.blobs[i].centroid.y <= 2.0*(240.0/3.0)) {
                centerHasIt = true;
            } else {
                rightHasIt = true;
            }
        }
        
        float inc = 20.0;

        if (leftHasIt) {
            if (leftLevel < 255.0) {
                leftLevel += inc;
            }
        } else {
            if (leftLevel > 0.0) {
                leftLevel -= inc;
            }

        }
        
        if (centerHasIt) {
            if (centerLevel < 255.0) {
                centerLevel += inc;
            }
        } else {
            if (centerLevel > 0.0) {
                centerLevel -= inc;
            }
            
        }
        
        if (rightHasIt) {
            if (rightLevel < 255.0) {
                rightLevel += inc;
            }
        } else {
            if (rightLevel > 0.0) {
                rightLevel -= inc;
            }
        }

        // CHECK AREA
        bool hasLargeEnoughArea = false;
        for (int i = 0; i < contourFinder.nBlobs; i++) {
            if (contourFinder.blobs[i].area >= minBlobArea) {
                hasLargeEnoughArea = true;
                break;
            }
        }
        
        if (hasLargeEnoughArea) {
            // CHECK BLOB #
            if (contourFinder.nBlobs >= numBlobsNeeded) {
                camera1BackgroundLevel += 15.0;
                camera1BackgroundLevel = ofClamp(camera1BackgroundLevel, 0.0, 250.0);
            }
        }
        if (contourFinder.nBlobs == 0) {
            camera1BackgroundLevel -= 15.0;
            camera1BackgroundLevel = ofClamp(camera1BackgroundLevel, 0.0, 250.0);
        }
    }
    
    if (bNewFrame1){
        
        colorImg1.setFromPixels(vidGrabber1.getPixels(), 320,240);
        
        grayImage1 = colorImg1;
        if (bLearnBackground1 == true){
            grayBg1 = grayImage1;
            bLearnBackground1 = false;
        }
        grayDiff1.absDiff(grayBg1, grayImage1);
        grayDiff1.threshold(threshold);
        contourFinder1.findContours(grayDiff1, 20, (340*240)/3, 10, true);
        
        bool hasLargeEnoughArea = false;
        for (int i = 0; i < contourFinder1.nBlobs; i++) {
            if (contourFinder1.blobs[i].area >= minBlobArea) {
                hasLargeEnoughArea = true;
                break;
            }
        }
        
        if (hasLargeEnoughArea) {
            if (contourFinder1.nBlobs >= numBlobsNeeded) {
                camera2BackgroundLevel+=15.0;
                camera2BackgroundLevel = ofClamp(camera2BackgroundLevel, 0, 250);
            }
        }
        if (contourFinder1.nBlobs == 0) {
            camera2BackgroundLevel-=15.0;
            camera2BackgroundLevel = ofClamp(camera2BackgroundLevel, 0, 250);
        }
        
    }
    
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
//            ofVec2f btm = absColumnPositionTop[max_pos];
            ofVec2f btm = cameraPositionsTop[max_pos];
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
    
    gui.draw();
    ofEnableDepthTest();
    
    ofSetColor(255.0,255.0,255.0);
    ofFill();
    for (int i = 0; i < 10; i++) {
        ofDrawBitmapString(verbalInstructions[i], 10, (i*20)+500);
    }
    
    if (cameraInfoIsOn) {
        ofSetColor(255.0, 255.0, 255.0);
        ofFill();
        
        /* drawing cameras */
        ofSetColor(255.0,0.0,0.0);
        ofDrawBitmapString("Active column", 10, 350+20);
        ofSetColor(0.0,0.0,255.0);
        ofDrawBitmapString("Active camera #", 10, 360+20);
        ofSetColor(255,105,180);
        ofDrawBitmapString("# of Pedestrians", 10, 370+20);
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
        //cout << "" << endl;
        
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
        //ofVec2f btm = absColumnPositionTop[max_pos];
        ofVec2f btm = cameraPositionsTop[max_pos];
        ofVec2f desired =  btm - swarmPosition;
        float d = sqrt((desired.x*desired.x) + (desired.y+desired.y));
        float r = ofMap(ofClamp(d, 0.0, 700.0), 0.0, 700.0, 25.0, 76.5);
        ofColor swarmColor = ofColor(255.0, 0.0, 255.0);
        ofSetColor(swarmColor);
        ofDrawRectangle(swarmPosition.x, 45, 10, 50);
        ofSetColor(columnColor);
        ofFill();
        ofDrawCircle(578, 270, 14);
    
        ofSetColor(cameraColor2);
        ofSetColor(cameraColor2, 100.0);
        ofDrawLine(238+(340/2), 150, 238+(340/2), 150+240);
        ofDrawLine(238, 150+(240/3), 238+340, 150+(240/3));
        ofDrawLine(238, 150+(2*(240/3)), 238+340, 150+(2*(240/3)));

        
        ofDrawLine(578+(340/2), 150, 578+(340/2), 150+240);
        ofDrawLine(578, 150+(240/3), 578+340, 150+(240/3));
        ofDrawLine(578, 150+(2*(240/3)), 578+340, 150+(2*(240/3)));

        
        for (int i = 0; i < contourFinder.nBlobs; i++) {
            if (contourFinder.blobs[i].area >= minBlobArea) {
                if (contourFinder.blobs[i].centroid.x < 60 || contourFinder.blobs[i].centroid.x > 120) {
                    contourFinder.blobs[i].draw(238, 150);
                }
            }
        }
        
        ofNoFill();
        ofSetColor(cameraColor1);
        ofDrawBitmapString("Camera 5", 238, 140);
        ofDrawRectangle(238, 150, 340, 240);
        ofSetColor(255,255,255,50.0);
        colorImg.draw(238, 150);

        
        //avgYPosOfBlobs = 0.0;
        for (int i = 0; i < contourFinder1.nBlobs; i++) {
            avgYPosOfBlobs += contourFinder1.blobs[i].centroid.y;
            
            if (contourFinder1.blobs[i].centroid.x < 60 || contourFinder1.blobs[i].centroid.x > 120) {
                contourFinder1.blobs[i].draw(578, 150);
            }
        }

        
        avgYPosOfBlobs = 0.0;
        int biggestBlobPos = 0;
        float largestBlobArea = -1000.0;
        for (int i = 0; i < contourFinder.nBlobs; i++) {
            if (contourFinder.blobs[i].area > largestBlobArea) {
                largestBlobArea = contourFinder.blobs[i].area;
                biggestBlobPos = i;
            }
        }
        if (contourFinder.nBlobs == 0) {
            avgYPosOfBlobs = 0;
        } else {
            avgYPosOfBlobs = contourFinder.blobs[biggestBlobPos].centroid.y;
        }
        
        if (avgYPosOfBlobs == 0) {
            sideSection = 0;
        } else if (avgYPosOfBlobs > 0 && avgYPosOfBlobs <= 238.0/3.0) {
            sideSection = 1;
        } else if (avgYPosOfBlobs > 238.0/3.0 && avgYPosOfBlobs <= (2.0*238.0)/3.0) {
            sideSection = 2;
        } else {
            sideSection = 3;
        }

        ofNoFill();
        ofSetColor(cameraColor2);
        ofDrawBitmapString("Camera 6", 578, 140);
        ofDrawRectangle(578, 150, 340, 240);
        ofSetColor(255,255,255,50.0);
        colorImg1.draw(578, 150);
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
    
    vector<ofVideoDevice> devices = vidGrabber.listDevices();

    if (ANIMATION_STATE == ACTIVATED) {
        ofDrawBitmapString("Swarm is active", 10, 420);
    } else {
        ofDrawBitmapString("Swarm is inactive", 10, 420);
    }
    if (simulationIsOn) {
        ofDrawBitmapString("Pedestrian simulation is active", 10, 440);
    } else {
        ofDrawBitmapString("Pedestrian simulation is inactive", 10, 440);
    }
    ofDisableDepthTest();
}

//--------------------------------------------------------------

/* HELPERS */

void ofApp::sendToDMX() {
    
    float level1 = backgroundLevel;
    float level2 = backgroundLevel;
    
    float inRes = result["region1"]["ring10"]["point0"][0].asFloat();
    float in = ofMap(inRes, -2000.0, -40.0, 0.0, 1280.0);
    int inInt = (int) in;
    int gauss = gaussianBottom[inInt];
    
    float inRes2 = result["region1"]["ring9"]["point0"][0].asFloat();
    float in2 = ofMap(inRes2, -2000.0, -40.0, 0.0, 1280.0);
    int inInt2 = (int) in2;
    int gauss2 = gaussianBottom[inInt2];
    
    float inRes3 = result["region1"]["ring8"]["point0"][0].asFloat();
    float in3 = ofMap(inRes3, -2000.0, -40.0, 0.0, 1280.0);
    int inInt3 = (int) in3;
    int gauss3 = gaussianBottom[inInt3];
    
    float inRes4 = result["region1"]["ring7"]["point0"][0].asFloat();
    float in4 = ofMap(inRes4, -2000.0, -40.0, 0.0, 1280.0);
    int inInt4 = (int) in4;
    int gauss4 = gaussianBottom[inInt4];

    ofColor ambColor = ambientColor;

    float top_r = ofMap(camera1BackgroundLevel, 0.0, 255.0, ambColor.r, 255.0);
    float top_g = ofMap(camera1BackgroundLevel, 0.0, 255.0, ambColor.g, 255.0);
    float top_b = ofMap(camera1BackgroundLevel, 0.0, 255.0, ambColor.b, 255.0);
    
    float top2_r = ofMap(camera2BackgroundLevel, 0.0, 255.0, ambColor.r, 255.0);
    float top2_g = ofMap(camera2BackgroundLevel, 0.0, 255.0, ambColor.g, 255.0);
    float top2_b = ofMap(camera2BackgroundLevel, 0.0, 255.0, ambColor.b, 255.0);
    
    
    ofColor c;
    ofColor c2;
    ofColor c3;
    ofColor c4;

    
    float newGauss = ofClamp(gauss, 245.0, 255.0);
    float newGauss2 = ofClamp(gauss2, 245.0, 255.0);
    float newGauss3 = ofClamp(gauss3, 245.0, 255.0);
    float newGauss4 = ofClamp(gauss4, 245.0, 255.0);
    
    ofColor swmColor = swarmColor;

    
//    c.r = ofMap(newGauss, 245.0, 255.0, top2_r, swmColor.r);
//    c.g = ofMap(newGauss, 245.0, 255.0, top2_g, swmColor.g);
//    c.b = ofMap(newGauss, 245.0, 255.0, top2_b, swmColor.b);
//    
//    c2.r = ofMap(newGauss2, 245.0, 255.0, top2_r, swmColor.r);
//    c2.g = ofMap(newGauss2, 245.0, 255.0, top2_g, swmColor.g);
//    c2.b = ofMap(newGauss2, 245.0, 255.0, top2_b, swmColor.b);
//    
//    c3.r = ofMap(newGauss3, 245.0, 255.0, top_r, swmColor.r);
//    c3.g = ofMap(newGauss3, 245.0, 255.0, top_g, swmColor.g);
//    c3.b = ofMap(newGauss3, 245.0, 255.0, top_b, swmColor.b);
//    
//    c4.r = ofMap(newGauss4, 245.0, 255.0, top_r, swmColor.r);
//    c4.g = ofMap(newGauss4, 245.0, 255.0, top_g, swmColor.g);
//    c4.b = ofMap(newGauss4, 245.0, 255.0, top_b, swmColor.b);
    
    
    c.r = ofMap(newGauss, 245.0, 255.0, top2_r, swmColor.r);
    c.g = ofMap(newGauss, 245.0, 255.0, top2_g, swmColor.g);
    c.b = ofMap(newGauss, 245.0, 255.0, top2_b, swmColor.b);

    c2.r = ofMap(newGauss2, 245.0, 255.0, top2_r, swmColor.r);
    c2.g = ofMap(newGauss2, 245.0, 255.0, top2_g, swmColor.g);
    c2.b = ofMap(newGauss2, 245.0, 255.0, top2_b, swmColor.b);

    c3.r = ofMap(newGauss3, 245.0, 255.0, top_r, swmColor.r);
    c3.g = ofMap(newGauss3, 245.0, 255.0, top_g, swmColor.g);
    c3.b = ofMap(newGauss3, 245.0, 255.0, top_b, swmColor.b);

    c4.r = ofMap(newGauss4, 245.0, 255.0, top_r, swmColor.r);
    c4.g = ofMap(newGauss4, 245.0, 255.0, top_g, swmColor.g);
    c4.b = ofMap(newGauss4, 245.0, 255.0, top_b, swmColor.b);

    
    //ofColor newC = ofColor(255, 0, 0);
    
    dmxData_[1] = int(c.r);
    dmxData_[2] = int(c.g);
    dmxData_[3] = int(c.b);

    dmxData_[4] = int(c2.r);
    dmxData_[5] = int(c2.g);
    dmxData_[6] = int(c2.b);

    dmxData_[7] = int(c3.r);
    dmxData_[8] = int(c3.g);
    dmxData_[9] = int(c3.b);

    /* REPLICATE IDEA */
    for (int i = 10; i <= 112; i+=3) {
        dmxData_[i] = int(c4.r);
        dmxData_[i+1] = int(c4.g);
        dmxData_[i+2] = int(c4.b);
    }

    // UPDATING POSITION.......
//    slidePosition +=1.0;
//    if (slidePosition >= 100.0) {
//        slidePosition = 0.0;
//    }
    
    /* MINI SWARM */
//    ofVec2f btm = ofVec2f(0.0, 0.0);
//    if (sideSection == 0) {
//        
//    } else {
//        if (sideSection == 1) {
//            btm.x = 0.0;
//        } else if (sideSection == 2) {
//            btm.x = 8.0;
//        } else {
//            btm.x = 16.0;
//        }
//        ofVec2f desired =  btm - horizontalPosition;
//        desired.x /= 14.0;
//        horizontalPosition += desired;
//        
//        int channel = horizontalPosition.x;
//        int channel2 = 34 - channel;
//        
//        int channel1Behind = channel - 1;
//        int channel1Front = channel + 1;
//        
//        int channel2Behind = channel2 - 1;
//        int channel2Front = channel2 + 1;
//        
//        channel1Behind += 2;
//        channel1Behind %= 34;
//        channel += 2;
//        channel %= 34;
//        channel1Front += 2;
//        channel1Front %= 34;
//        
//        
//        channel2Behind += 2;
//        channel2Behind %= 34;
//        channel2 += 2;
//        channel2 %= 34;
//        channel2Front += 2;
//        channel2Front %= 34;
//        
//        
//        int channelPositionInDMX1Behind = 10 + (3*channel1Behind);
//        int channelPositionInDMX = 10 + (3*channel);
//        int channelPositionInDMX1Front = 10 + (3*channel1Front);
//        
//        int channelPositionInDMX2Behind = 10 + (3*channel2Behind);
//        int channelPositionInDMX2 = 10 + (3*channel2);
//        int channelPositionInDMX2Front = 10 + (3*channel2Front);
//        
//        
//        dmxData_[channelPositionInDMX1Behind+0] = int(255*.5);
//        dmxData_[channelPositionInDMX1Behind+1] = int(255*.5);
//        dmxData_[channelPositionInDMX1Behind+2] = int(0);
//        
//        dmxData_[channelPositionInDMX+0] = int(255);
//        dmxData_[channelPositionInDMX+1] = int(255);
//        dmxData_[channelPositionInDMX+2] = int(0);
//        
//        dmxData_[channelPositionInDMX1Front+0] = int(255*.5);
//        dmxData_[channelPositionInDMX1Front+1] = int(255*.5);
//        dmxData_[channelPositionInDMX1Front+2] = int(0);
//        
//        
//        dmxData_[channelPositionInDMX2Behind+0] = int(255*.5);
//        dmxData_[channelPositionInDMX2Behind+1] = int(255*.5);
//        dmxData_[channelPositionInDMX2Behind+2] = int(0);
//        
//        dmxData_[channelPositionInDMX2+0] = int(255);
//        dmxData_[channelPositionInDMX2+1] = int(255);
//        dmxData_[channelPositionInDMX2+2] = int(0);
//        
//        dmxData_[channelPositionInDMX2Front+0] = int(255*.5);
//        dmxData_[channelPositionInDMX2Front+1] = int(255*.5);
//        dmxData_[channelPositionInDMX2Front+2] = int(0);
//    }
    
    //IDEA 3
//    if (sideSection == 0) {
//    } else if (sideSection == 1) {
//        for (int i = 106; i <= 106+(2*3); i+=3) {
//            dmxData_[i+0] = int(sideLevel);
//            dmxData_[i+1] = int(sideLevel);
//            dmxData_[i+2] = int(sideLevel);
//        }
//        
//        for (int i = 10; i <= 10+(8*3); i+=3) {
//            dmxData_[i+0] = int(sideLevel);
//            dmxData_[i+1] = int(sideLevel);
//            dmxData_[i+2] = int(sideLevel);
//        }
//        if (sideLevel < 255.0) {
//            sideLevel += 5.0;
//        }
//        if (sideLevel >= 255.0) {
//            sideLevel = 0.0;
//            //sideSection = 2;
//        }
//    } else if (sideSection == 2) {
//        for (int i = 37; i <= 37+(6*3); i+=3) {
//            dmxData_[i+0] = int(sideLevel);
//            dmxData_[i+1] = int(sideLevel);
//            dmxData_[i+2] = int(sideLevel);
//        }
//        for (int i = 82; i <= 82+(7*3); i+=3) {
//            dmxData_[i+0] = int(sideLevel);
//            dmxData_[i+1] = int(sideLevel);
//            dmxData_[i+2] = int(sideLevel);
//        }
//        if (sideLevel < 255.0) {
//            sideLevel += 5.0;
//        }
//        if (sideLevel >= 255.0) {
//            sideLevel = 0.0;
//            //sideSection = 3;
//        }
//    } else {
//        for (int i = 58; i <= 58+(7*3); i+=3) {
//            dmxData_[i+0] = int(sideLevel);
//            dmxData_[i+1] = int(sideLevel);
//            dmxData_[i+2] = int(sideLevel);
//        }
//        if (sideLevel < 255.0) {
//            sideLevel += 5.0;
//        }
//        if (sideLevel >= 255.0) {
//            sideLevel = 0.0;
//            //sideSection = 1;
//        }
//    }
    
    ofColor adColor = addressableColor;

    
    // IDEA 3
    /* SECTION 1 */
    float l_r = ofMap(leftLevel, 0.0, 255.0, ambColor.r, adColor.r);
    float l_g = ofMap(leftLevel, 0.0, 255.0, ambColor.g, adColor.g);
    float l_b = ofMap(leftLevel, 0.0, 255.0, ambColor.b, adColor.b);

    for (int i = 106; i <= 106+(2*3); i+=3) {
        dmxData_[i+0] = int(l_r);
        dmxData_[i+1] = int(l_g);
        dmxData_[i+2] = int(l_b);
    }
    
    for (int i = 10; i <= 10+(8*3); i+=3) {
        dmxData_[i+0] = int(l_r);
        dmxData_[i+1] = int(l_g);
        dmxData_[i+2] = int(l_b);
    }
    
    /* SECTION 2 */
    float c_r = ofMap(centerLevel, 0.0, 255.0, ambColor.r, adColor.r);
    float c_g = ofMap(centerLevel, 0.0, 255.0, ambColor.g, adColor.g);
    float c_b = ofMap(centerLevel, 0.0, 255.0, ambColor.b, adColor.b);

    for (int i = 37; i <= 37+(6*3); i+=3) {
        dmxData_[i+0] = int(c_r);
        dmxData_[i+1] = int(c_g);
        dmxData_[i+2] = int(c_b);
    }
    for (int i = 82; i <= 82+(7*3); i+=3) {
        dmxData_[i+0] = int(c_r);
        dmxData_[i+1] = int(c_g);
        dmxData_[i+2] = int(c_b);
    }
    
    /* SECTION 3 */
    float r_r = ofMap(rightLevel, 0.0, 255.0, ambColor.r, adColor.r);
    float r_g = ofMap(rightLevel, 0.0, 255.0, ambColor.g, adColor.g);
    float r_b = ofMap(rightLevel, 0.0, 255.0, ambColor.b, adColor.b);

    for (int i = 58; i <= 58+(7*3); i+=3) {
        dmxData_[i+0] = int(r_r);
        dmxData_[i+1] = int(r_g);
        dmxData_[i+2] = int(r_b);
    }
    
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
            
            float backgroundLevelRef = backgroundLevel;
            if (reg == "region1") {
                if (ring == "ring10" || ring == "ring9") {
                    backgroundLevelRef = camera1BackgroundLevel;
                } else if (ring == "ring8" || ring == "ring7") {
                    backgroundLevelRef = camera2BackgroundLevel;
                }
            }

            ofColor c;
            ofColor ambColor = ambientColor;
            ofColor swmColor = swarmColor;


            float top_r = ofMap(backgroundLevelRef, 0.0, 255.0, ambColor.r, 255.0);
            float top_g = ofMap(backgroundLevelRef, 0.0, 255.0, ambColor.g, 255.0);
            float top_b = ofMap(backgroundLevelRef, 0.0, 255.0, ambColor.b, 255.0);
            

            //ofColor bleh = ofColor(255.0, 0.0, 255.0);
            
            float newGauss = ofClamp(gauss, 245.0, 255.0);


            c.r = ofMap(newGauss, 245.0, 255.0, top_r, swmColor.r);
            c.g = ofMap(newGauss, 245.0, 255.0, top_g, swmColor.g);
            c.b = ofMap(newGauss, 245.0, 255.0, top_b, swmColor.b);


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
        if (ANIMATION_STATE == ACTIVATED) {
            ANIMATION_STATE = DEACTIVATED;
            bottomSwarm.bVel = 1.0;
            swarmVector.x = 1.0;
        } else if (ANIMATION_STATE == DEACTIVATED) {
            ANIMATION_STATE = ACTIVATED;
            bottomSwarm.bVel = 4.0;
            swarmVector.x = 14.0;
        }
    } else if (key == ' ') {
        bLearnBackground = true;
        bLearnBackground1 = true;
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
    } else if(key == 'p'){
        gui.saveToFile("settings.xml");
    }
    else if(key == 'l'){
        gui.loadFromFile("settings.xml");
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
void ofApp::mouseMoved(int x, int y ) {
//    cout << x << endl;
//    cout << y << endl;
}
void ofApp::mouseDragged(int x, int y, int button) {}
void ofApp::mousePressed(int x, int y, int button) {}
void ofApp::mouseReleased(int x, int y, int button) {}
void ofApp::mouseEntered(int x, int y) {}
void ofApp::mouseExited(int x, int y) {}