//
//  ofxFlyCamera.cpp
//  ofLightWeave
//
//  Created by Collin Schupman on 4/8/16.
//
//

#include "ofxFlyCamera.hpp"

ofxFlyCamera::ofxFlyCamera() {
    movementSpeed = 1.0;
    rollSpeed = 0.005;
    dragToLook = false;
    autoForward = false;
    mouseStatus = 0;
    moveState["up"] = 0;
    moveState["down"] = 0;
    moveState["left"] = 0;
    moveState["right"] = 0;
    moveState["forward"] = 0;
    moveState["back"] = 0;
    moveState["pitchUp"] = 0;
    moveState["pitchDown"] = 0;
    moveState["yawLeft"] = 0;
    moveState["yawRight"] = 0;
    moveState["rollLeft"] = 0;
    moveState["rollRight"] = 0;
    updateMovementVector();
    updateRotationVector();
}

void ofxFlyCamera::update(float delta) {
    float moveMult = delta * movementSpeed;
    float rotMult = delta * rollSpeed;
    moveVector *= moveMult;
    move(moveVector);
    rotationVector *= rotMult;
    tmpQuaternion.set(rotationVector.x, rotationVector.y, rotationVector.z, 1 );
    tmpQuaternion.normalize();
    rotate(tmpQuaternion);
}

void ofxFlyCamera::updateMovementVector() {
    bool forward = ( moveState["forward"] || ( autoForward && ! moveState["back"] ) ) ? 1 : 0;
    moveVector.x = ( - moveState["left"]  + moveState["right"] );
    moveVector.y = ( - moveState["down"]  + moveState["up"] );
    moveVector.z = ( - forward            + moveState["back"] );
};

void ofxFlyCamera::updateRotationVector() {
    rotationVector.x = ( - moveState["pitchDown"] + moveState["pitchUp"]);
    rotationVector.y = ( - moveState["yawRight"]  + moveState["yawLeft"]);
    rotationVector.z = ( - moveState["rollRight"] + moveState["rollLeft"]);
    
};


void ofxFlyCamera::keydown(int key) {
    if ( key == 1234 ) {
        return;
    }
    switch ( key ) {
        // shift
        //case 16: movementSpeedMultiplier = .1; break;
            
        case 87: /*W*/ moveState["forward"] = 1; break;
        case 83: /*S*/ moveState["back"] = 1; break;
            
        case 65: /*A*/ moveState["left"] = 1; break;
        case 68: /*D*/ moveState["right"] = 1; break;
            
        case 82: /*R*/ moveState["up"] = 1; break;
        case 70: /*F*/ moveState["down"] = 1; break;
            
        case 38: /*up*/ moveState["pitchUp"] = 1; break;
        case 40: /*down*/ moveState["pitchDown"] = 1; break;
            
        case 37: /*left*/ moveState["yawLeft"] = 1; break;
        case 39: /*right*/ moveState["yawRight"] = 1; break;
            
        case 81: /*Q*/ moveState["rollLeft"] = 1; break;
        case 69: /*E*/ moveState["rollRight"] = 1; break;
            
    }
    updateMovementVector();
    updateRotationVector();
};

void ofxFlyCamera::keyup(int key) {
    
    switch ( key ) {
            
        // shift
        //case 16:  movementSpeedMultiplier = 1; break;
            
        case 87: /*W*/ moveState["forward"] = 0; break;
        case 83: /*S*/ moveState["back"] = 0; break;
            
        case 65: /*A*/ moveState["left"] = 0; break;
        case 68: /*D*/ moveState["right"] = 0; break;
            
        case 82: /*R*/ moveState["up"] = 0; break;
        case 70: /*F*/ moveState["down"] = 0; break;
            
        case 38: /*up*/ moveState["pitchUp"] = 0; break;
        case 40: /*down*/ moveState["pitchDown"] = 0; break;
            
        case 37: /*left*/ moveState["yawLeft"] = 0; break;
        case 39: /*right*/ moveState["yawRight"] = 0; break;
            
        case 81: /*Q*/ moveState["rollLeft"] = 0; break;
        case 69: /*E*/ moveState["rollRight"] = 0; break;
            
    }
    
    updateMovementVector();
    updateRotationVector();
    
};

void ofxFlyCamera::mousedown(int button) {
    if (dragToLook) {
        mouseStatus++;
    } else {
        switch (button) {
            case 0: moveState["forward"] = 1; break;
            case 2: moveState["back"] = 1; break;
                
        }
        updateMovementVector();
    }
};

void ofxFlyCamera::mousemove(int x, int y, int button) {
    
    if (!dragToLook || mouseStatus > 0) {
        
        float halfWidth  = ofGetWidth()/2.0;
        float halfHeight = ofGetHeight()/2.0;
        
        moveState["yawLeft"]   = - (x - halfWidth) / halfWidth;
        moveState["pitchDown"] = (y - halfHeight) / halfHeight;
        updateRotationVector();
    }
};

void ofxFlyCamera::mouseup(int button) {
    if (dragToLook) {
        mouseStatus--;
        moveState["yawLeft"] = moveState["pitchDown"] = 0;
    } else {
        switch (button) {
            case 0: moveState["forward"]= 0; break;
            case 2: moveState["back"] = 0; break;
        }
        updateMovementVector();
        
    }
    updateRotationVector();
};
