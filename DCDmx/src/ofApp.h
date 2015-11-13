#pragma once

#include "ofMain.h"
#include "ofxGenericDmx.h"
#include "ofxGui.h"
#include "ofxJSON.h"

#define DMX_DATA_LENGTH 494

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();
        void exit();
		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
        void carPressed();
        void personPressed();
        void bikePressed();
        void trainPressed();
        void addCars(int num);
        void addTrains();
        void addPeople(int num);
        void addBikes(int num);
private:
    
        float gaussianSensor1[1280];
        float gaussianSensor2[1280];
        float gaussian[1280];
    
        ofPolyline lineGraph;
    
        DmxDevice* dmxInterface_;
        unsigned char dmxData_[DMX_DATA_LENGTH];
        float red, green, blue;
    
        ofxColorSlider bColor;
        ofxColorSlider lColor;
        ofxColorSlider rColor;
        ofxButton addCar;
        ofxButton addPerson;
        ofxButton addBike;
        ofxButton addTrain;

        ofxFloatSlider yourMamma;
        ofxToggle autoMode;
        ofxToggle region1Info;
        ofxToggle region2Info;
        ofxToggle region3Info;
        ofxToggle region4Info;
        ofxToggle region5Info;
        ofxToggle region6Info;

        ofxPanel gui;
        float trigPos = 0.0;
    
        int curWidth;
        int curHeight;
    
        float decayRate;
        float growthRate;
        float aGrowthRate;
        float bAccel;
    
        bool pulseLeftGrowing;
        bool pulseRightGrowing;
        float pulseHeightLeft;
        float pulseHeightRight;
        float pulseGrowth;
        float pulseDecay;
    
        bool trainGrowing;
        float trainGrowthRate;
        float trainDecay;
        float ambientLevel;

        struct Wave {
            float a;
            float b;
            float c;
            float bVel;
            float curve[1280];
            bool growing;
        };
        struct Mover {
            ofVec2f pos;
            ofVec2f vel;
            ofVec2f accel;
        };
        void drawMovers(vector<Mover> movers, int size);
        vector<Mover> updateMovers(vector<Mover> movers);
        float trainSpeed = 0.5;
        float carSpeed = 3.0;
        float bikeSpeed = 1.0;
        float personSpeed = 0.3;
        vector<Mover> trains;
        vector<Mover> cars;
        vector<Mover> people;
        vector<Mover> bikes;

        vector<Wave> waves;
        vector<Wave> waves2;
        ofxJSONElement result;
        ofxJSONElement columnGeometry;

        ofImage backgroundImage;
};
