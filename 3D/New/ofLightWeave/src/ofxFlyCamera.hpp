//
//  ofxFlyCamera.hpp
//  ofLightWeave
//
//  Created by Collin Schupman on 4/8/16.
//
//

#ifndef ofxFlyCamera_hpp
#define ofxFlyCamera_hpp

#include <stdio.h>
#include "ofMain.h"


class ofxFlyCamera : public ofCamera
{
public:
        ofxFlyCamera();
    
        float movementSpeed = 1.0;
        float rollSpeed = 0.005;
        bool dragToLook = false;
        bool autoForward = false;
        ofQuaternion tmpQuaternion;
        int mouseStatus = 0;
        map<string, int> moveState;
        ofVec3f moveVector;
        ofVec3f rotationVector;
    
        void keydown(int key);
        void keyup(int key);
        void mousedown(int button);
        void mousemove(int x, int y, int button);
        void mouseup(int button);
    
        void update(float delta);
        void updateMovementVector();;
        void updateRotationVector();
};

#endif /* ofxFlyCamera_hpp */
