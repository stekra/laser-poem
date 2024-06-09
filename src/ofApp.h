#pragma once

#define USE_ETHERDREAM

#include "ofMain.h"
#include "ofxLaserManager.h"
#include "ofxLaserGraphic.h"
#include "ofxSvg.h"

class ofApp : public ofBaseApp {
public:
    void setup();
    void update();
    void draw();
    
    void keyPressed(int key);
        
    ofParameter<int>    currentSVG;
    ofParameter<string> currentSVGFilename;
    ofParameter<float>  scale;
    ofParameter<bool>   rotate3D;
    ofParameter<int>    renderProfileIndex;
    ofParameter<string> renderProfileLabel;
    
    vector<string> fileNames;
    vector<ofxLaser::Graphic> laserGraphics;
    ofPolyline completeLine;
    vector<ofColor> completeLineColors;
    ofPolyline animatedLine;
    vector<ofColor> animatedLineColors;
    ofParameter<ofColor> color;
    ofParameter<bool> playAnimation;
    ofParameter<float> t;
    ofParameter<float> animationTime;
    ofParameter<float> fadeTime;
    ofParameter<float> holdTime;
    ofParameter<bool> resample;
    ofParameter<int> resolution;
    
    enum animationState { drawing, holding, fading };
    animationState animationState;
    
    ofxLaser::Manager laserManager;
    
    int laserWidth;
    int laserHeight;
    
    bool autoArm;
};
