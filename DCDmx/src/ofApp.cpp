#include "ofApp.h"

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
    
    addCar.addListener(this, &ofApp::carPressed);
    addPerson.addListener(this,&ofApp::personPressed);
    addBike.addListener(this,&ofApp::bikePressed);
    addTrain.addListener(this,&ofApp::trainPressed);
    
    gui.setup(); // most of the time you don't need a name
    gui.add(bColor.setup("Background Color",ofColor(100),ofColor(0,0),ofColor(255,255), 20.0));
    gui.add(lColor.setup("Left Color",ofColor(255,255,0),ofColor(0,0),ofColor(255,255), 20.0));
    gui.add(rColor.setup("Right Color",ofColor(0,0,250),ofColor(0,0),ofColor(255,255), 20.0));
    gui.add(autoMode.setup("Automate Traffic", false));
    gui.add(addCar.setup("Add Car"));
    gui.add(addPerson.setup("Add Person"));
    gui.add(addBike.setup("Add Bike"));
    gui.add(addTrain.setup("Add Train"));
//    gui.add(region1Info.setup("Region1 Info", false));
//    gui.add(region2Info.setup("Region2 Info", false));
//    gui.add(region3Info.setup("Region3 Info", false));
//    gui.add(region4Info.setup("Region4 Info", false));
//    gui.add(region5Info.setup("Region5 Info", false));
//    gui.add(region6Info.setup("Region6 Info", false));

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
    std::string columnsFile = "Lightweave_columns.json";
    
    // Now parse the JSON
    bool parsingSuccessful = result.open(file);
    
    bool parsingSuccessfulColumn = columnGeometry.open(columnsFile);
    
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
    
    if (parsingSuccessfulColumn) {
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
    backgroundImage.loadImage("unnamed.jpg");
    addCars(4);
    addTrains();
    addPeople(20);
    addBikes(4);
}

//--------------------------------------------------------------
void ofApp::update() {
    
    for (int i = 0; i < trains.size(); i++) {
        if (trains[i].pos.x > 1047/2 && trains[i].pos.x < 1047) {
            if (trains[i].pos.y + trains[i].vel.y >= 304 && trains[i].pos.y < 304) {
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
            } else if (trains[i].pos.y + trains[i].vel.y >= 373 && trains[i].pos.y < 373) {
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
            } else if (trains[i].pos.y + trains[i].vel.y >= 595 && trains[i].pos.y < 595) {
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
            } else if (trains[i].pos.y + trains[i].vel.y >= 659 && trains[i].pos.y < 659) {
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
        } else {

            if (trains[i].pos.y + trains[i].vel.y <= 292 && trains[i].pos.y >= 292) {
                cout << i << endl;

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
            } else if (trains[i].pos.y + trains[i].vel.y <= 647 && trains[i].pos.y >= 647) {
                cout << i << endl;

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
            }

    
        }
    }
    
    
//    if (autoMode) {
//        if (ofRandom(1.0)>0.99) {
//            float newProb = ofRandom(4.0);
//            if (newProb >= 0.0 && newProb <= 1.0) {
//                addCars(4);
//            } else if (newProb >= 1.0 && newProb <= 2.0) {
//                addTrains();
//            } else if (newProb >= 2.0 && newProb <= 3.0) {
//                addPeople(20);
//            } else {
//                addBikes(4);
//            }
//        }
//        if (ofRandom(1.0) > 0.99) {
//            if (ofRandom(1.0) > 0.5) {
//                Wave wave;
//                wave.a = 1.1f;
//                wave.b = (curWidth/4.0);
//                wave.c = 100.0;
//                wave.bVel = 10.0;
//                if (ofRandom(1.0)>0.5) {
//                    wave.bVel*=-1.0;
//                }
//                wave.growing = true;
//                waves.push_back(wave);
//                pulseLeftGrowing = true;
//            } else {
//                Wave wave;
//                wave.a = 1.1f;
//                wave.b = 3.0*(curWidth/4.0);
//                wave.c = 100.0;
//                wave.bVel = 10.0;
//                if (ofRandom(1.0)>0.5) {
//                    wave.bVel*=-1.0;
//                }
//                wave.growing = true;
//                waves.push_back(wave);
//                pulseRightGrowing = true;
//            }
//        }
//    }
    
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
    cars = updateMovers(cars);
    trains = updateMovers(trains);
    people = updateMovers(people);
    bikes = updateMovers(bikes);
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofBackground(0.0);
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

    
    /* Current Office location */
    //ofRect(diff + 8*(diff)-1, (curHeight/4.0), 3, 2.0*(curHeight/4.0));
    //ofRect(diff + 12*(diff)-1, (curHeight/4.0), 3, 2.0*(curHeight/4.0));
    
    /* Sound Curve */
//    for (int i = 0; i < curWidth; i++) {
//        ofColor c = color;
//        c.setBrightness(gaussian[i]);
//        ofSetColor(c);
//        ofFill();
//        ofEllipse(i, ((minY-10)+(maxY+10))/2 - ofMap(gaussian[i], 0.0, 255.0, 0.0, 180.0), 2, 2);
//    }
//    ofSetColor(255.0);
//    ofFill();
    
    /* Train Simulation Update */
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
    
    /* DMX Communication */
    
        ofColor curLeft = lColor;
        ofColor curRight = rColor;
        ofColor curBack = bColor;
        ofColor c;

        float top_r = ofMap(result["region0"]["ring0"]["point0"][0].asFloat(), 0.0, 1280.0, curLeft.r, curRight.r);
        float top_g = ofMap(result["region0"]["ring0"]["point0"][0].asFloat(), 0.0, 1280.0, curLeft.g, curRight.g);
        float top_b = ofMap(result["region0"]["ring0"]["point0"][0].asFloat(), 0.0, 1280.0, curLeft.b, curRight.b);
        c.r = ofMap(gaussian[result["region0"]["ring0"]["point0"][0].asInt()], 51.0, 255.0, curBack.r, top_r);
        c.g = ofMap(gaussian[result["region0"]["ring0"]["point0"][0].asInt()], 51.0, 255.0, curBack.g, top_g);
        c.b = ofMap(gaussian[result["region0"]["ring0"]["point0"][0].asInt()], 51.0, 255.0, curBack.b, top_b);
    
        //c.setBrightness(gaussian[result["region0"]["ring0"]["point0"][0].asInt()]);
        dmxData_[1] = int(c.r);
        dmxData_[2] = int(c.g);
        dmxData_[3] = int(c.b);
        //c.setBrightness(gaussian[result["region0"]["ring0"]["point0"][0].asInt()]);
        dmxData_[4] = int(c.r);
        dmxData_[5] = int(c.g);
        dmxData_[6] = int(c.b);
        //c.setBrightness(gaussian[result["region0"]["ring0"]["point0"][0].asInt()]);
        dmxData_[7] = int(c.r);
        dmxData_[8] = int(c.g);
        dmxData_[9] = int(c.b);
        //c.setBrightness(gaussian[result["region0"]["ring0"]["point0"][0].asInt()]);
        dmxData_[10] = int(c.r);
        dmxData_[11] = int(c.g);
        dmxData_[12] = int(c.b);
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

        for (int rings = 0; rings < result[reg].size(); rings++) {
            string ring = "ring" + ofToString(rings);
            float in = result[reg][ring]["point0"][0].asFloat();
            int inInt = (int) in;

            int gauss = gaussian[inInt];
            ofPolyline line;
           

            for (int pointPos = 0; pointPos < 3; pointPos++) {
                string point = "point" + ofToString(pointPos);

                ofColor c;

                float top_r = ofMap(result[reg][ring][point][0].asFloat(), 0.0, 1280.0, curLeft.r, curRight.r);
                float top_g = ofMap(result[reg][ring][point][0].asFloat(), 0.0, 1280.0, curLeft.g, curRight.g);
                float top_b = ofMap(result[reg][ring][point][0].asFloat(), 0.0, 1280.0, curLeft.b, curRight.b);
                c.r = ofMap(gauss, 51.0, 255.0, curBack.r, top_r);
                c.g = ofMap(gauss, 51.0, 255.0, curBack.g, top_g);
                c.b = ofMap(gauss, 51.0, 255.0, curBack.b, top_b);
                ofSetColor(c);
                ofFill();
                line.addVertex(result[reg][ring][point][0].asFloat(), 900-result[reg][ring][point][1].asFloat());
            }
            line.addVertex(result[reg][ring]["point0"][0].asFloat(), 900-result[reg][ring]["point0"][1].asFloat());
            line.draw();
        }
    }
    
    /* Draw Cars and Trains */
    ofSetColor(255.0,255.0,255.0);
    ofFill();
    drawMovers(cars, 10);
    drawMovers(trains, 20);
    drawMovers(people, 2);
    drawMovers(bikes, 6);

    ofSetColor(255.0);
    ofFill();
    
    gui.draw();
}


vector<ofApp::Mover> ofApp::updateMovers(vector<Mover> movers) {
    for (int i = 0; i < movers.size(); i++) {
        movers[i].vel += movers[i].accel;
        movers[i].pos += movers[i].vel;
    }
    vector<int> toErase;
    for (int i = 0; i < movers.size(); i++) {
        if (movers[i].pos.x > 1280 || movers[i].pos.x < 0) {
            toErase.push_back(i);
        }
    }
    for (int i = 0; i < toErase.size(); i++) {
        int pos = toErase[i];
        movers.erase(movers.begin() + pos);
    }
    return movers;
}

void ofApp::drawMovers(vector<Mover> movers, int size) {
    for (int i = 0; i < movers.size(); i++) {
        ofEllipse(movers[i].pos, size, size);
    }
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
        for ( int i = 0; i <= DMX_DATA_LENGTH; i++ ) dmxData_[i] = 0;
        dmxInterface_->writeDmx( dmxData_, DMX_DATA_LENGTH );
        dmxInterface_->close();
    }
}

void ofApp::addCars(int num) {
    for (int i = 0; i < num/2; i++) {
        Mover car;
        car.pos = ofVec2f(ofRandom(1280), 427);
        car.vel = ofVec2f(-carSpeed, 0);
        car.accel = ofVec2f(0, 0);
        cars.push_back(car);
    }
    for (int i = 0; i < num/2; i++) {
        Mover car;
        car.pos = ofVec2f(ofRandom(1280), 493);
        car.vel = ofVec2f(carSpeed, 0);
        car.accel = ofVec2f(0, 0);
        cars.push_back(car);
    }
}

void ofApp::addTrains() {
    if (ofRandom(1.0) > 0.5) {
        Mover train;
        train.pos = ofVec2f(1047, 0);
        train.vel = ofVec2f(-0.75, 2.5);
        train.accel = ofVec2f(0, 0);
        trains.push_back(train);
    } else {
    
        Mover train2;
        train2.pos = ofVec2f(227, 900);
        train2.vel = ofVec2f(0.75, -2.5);
        train2.accel = ofVec2f(0, 0);
        trains.push_back(train2);
    }
}

void ofApp::addPeople(int num) {
    for (int i = 0; i < num; i++) {
        Mover person;
        ofVec2f pos;
        if (ofRandom(1.0) > 0.5){
            pos.y = ofRandom(528,662);
        } else {
            pos.y = ofRandom(238,368);
        }
        pos.x = ofRandom(1280);
        person.pos = pos;
        person.vel = ofVec2f(-personSpeed, 0.0);
        if (ofRandom(1.0) > 0.5){
            person.vel*=-1.0;
        }
        person.accel = ofVec2f(0, 0);
        people.push_back(person);
        
    }
}
void ofApp::addBikes(int num) {
    for (int i = 0; i < num; i++) {
        Mover bike;
        ofVec2f pos;
        if (ofRandom(1.0) > 0.5){
            pos.y = ofRandom(528,662);
        } else {
            pos.y = ofRandom(238,368);
        }
        pos.x = ofRandom(1280);
        bike.pos = pos;
        bike.vel = ofVec2f(-bikeSpeed, 0.0);
        if (ofRandom(1.0) > 0.5){
            bike.vel*=-1.0;
        }
        bike.accel = ofVec2f(0, 0);
        bikes.push_back(bike);
    }
}

//--------------------------------------------------------------
void ofApp::carPressed(){
    addCars(2);
}

//--------------------------------------------------------------
void ofApp::personPressed(){
    addPeople(20);
}

//--------------------------------------------------------------
void ofApp::bikePressed(){
    addBikes(4);
}

//--------------------------------------------------------------
void ofApp::trainPressed(){
    addTrains();
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