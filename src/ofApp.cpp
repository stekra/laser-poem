#include "ofApp.h"

//--------------------------------------------------------------

void ofApp::setup() {
    laserWidth = 1000;
    laserHeight = 300;
    laserManager.setCanvasSize(laserWidth, laserHeight);
    
    // Get SVGs from data/svgs
    string path = "svgs/";
    ofDirectory dir(path);
    dir.allowExt("svg");
    dir.listDir();
    dir.sort();
    
    const vector<ofFile>& files = dir.getFiles();
    
    dir.close();
    
    if (files.size() == 0) {
        ofSystemAlertDialog("No SVGs found! Looking at data/svgs/");
        ofExit();
    }
    
    for(int i = 0; i < files.size(); i++) {
        const ofFile & file = files.at(i);
        
        laserGraphics.emplace_back();
        laserGraphics.back().addSvgFromFile(file.getAbsolutePath(), false, false);
        fileNames.push_back(file.getFileName());
    }
        
    laserManager.addCustomParameter(currentSVG.set("Current SVG", 0, 0, static_cast<int>(laserGraphics.size() - 1)));
    laserManager.addCustomParameter(scale.set("SVG scale", 1.0, 0.1,6));
    laserManager.addCustomParameter(renderProfileLabel.set("Render Profile name",""));
    laserManager.addCustomParameter(renderProfileIndex.set("Render Profile", 2, 0, 2));
    laserManager.addCustomParameter(color.set("Color", ofColor(0, 255, 0)));
    laserManager.addCustomParameter(playAnimation.set("Play", true));
    laserManager.addCustomParameter(t.set("t", 0, 0, 1));
    laserManager.addCustomParameter(animationTime.set("Animation Time", 10, 1, 20));
    laserManager.addCustomParameter(holdTime.set("Hold Time", 0, 0, 5));
    laserManager.addCustomParameter(resample.set("Resample", true));
    laserManager.addCustomParameter(resolution.set("downres", 20, 2, 100));
    
    animationState = drawing;
    currentSVG = 0;
    t = 0;
}

//--------------------------------------------------------------

void ofApp::update() {
    laserManager.update();
    
    // Animation timer
    if (playAnimation) {
        float td = ofGetLastFrameTime() / animationTime;
        if (animationState == holding) {
            if (holdTime == 0) {
                td = 1 - t;
            }
            else {
                td = ofGetLastFrameTime() / holdTime;
            }
        }
        t = t + td;
        
        if (t >= 1) {
            t = fmod(t, 1);
            switch (animationState) {
                case drawing:
                    animationState = holding;
                    break;
                case holding:
                    animationState = fading;
                    break;
                case fading:
                    currentSVG = ofWrap(currentSVG + 1, 0, laserGraphics.size());
                    animationState = drawing;
                    break;
            }
            
            // Hack for auto dac assign
            ofxLaser::Laser *l1 = laserManager.getLasers()[0];
            ofxLaser::Laser *l2 = laserManager.getLasers()[1];
            
            laserManager.dacAssigner.updateDacList();
            if (!l1->hasDac() && l1->getDacConnectedState() != 0) laserManager.dacAssigner.assignToLaser("Etherdream 124DAC510BC1", *l1);
            if (!l1->hasDac() && l2->getDacConnectedState() != 0) laserManager.dacAssigner.assignToLaser("Etherdream 9EAAFDDF841E", *l2);
        }
    }
    
    // Build polyline from SVG
    auto svg = laserGraphics[currentSVG];
    auto polylines = svg.polylines;
    
    completeLine.clear();
    completeLineColors.clear();
    
    for (int j = 0; j < polylines.size(); j++) {
        ofPolyline line = *polylines[j];
        if (resample) line = polylines[j]->getResampledBySpacing(resolution);
        
        for (int i = 0; i < line.size(); i++) {
            completeLine.addVertex(line.getVertices()[i]);
            completeLineColors.push_back(color);
        }
        // Add black line between paths
        if (j + 1 < polylines.size()) {
            completeLine.addVertex(polylines[j]->getVertices().back());
            completeLine.addVertex(polylines[j+1]->getVertices().front());
            completeLineColors.push_back(ofColor(0));
            completeLineColors.push_back(ofColor(0));
        }
    }
    
    switch (animationState) {
        case drawing: {
            // Build current line state
            animatedLine.clear();
            animatedLineColors.clear();
            
            for (int i = 0; i < t * completeLine.size(); i++) {
                animatedLine.addVertex(completeLine[i]);
                animatedLineColors.push_back(completeLineColors[i]);
            }
            
            // Add one last interpolated vertex
            float findex = t * completeLine.size();
            animatedLine.addVertex(completeLine.getPointAtIndexInterpolated(findex));
            animatedLineColors.push_back(completeLineColors[floor(findex)]);
            break;
        }
        case holding: {
            animatedLine = completeLine;
            animatedLineColors = completeLineColors;
            break;
        }
        case fading: {
//            // Full lines
//            animatedLine = completeLine;
//            animatedLineColors = completeLineColors;
//            
//            // Fade
//            for (int i = 0; i < animatedLineColors.size(); i++) {
//                if (animatedLineColors[i] != ofColor(0)) {
//                    float pt = t - (i / (float)animatedLineColors.size()) * (1 - fadeDuration);
//                    pt = ofClamp(pt / fadeDuration, 0, 1);
//                    animatedLineColors[i].setBrightness(255 * (1 - pt));
//                }
//            }
//            break;
            
            animatedLine.clear();
            animatedLineColors.clear();
            
            // Add one first interpolated vertex
            float findex = t * completeLine.size();
            animatedLine.addVertex(completeLine.getPointAtIndexInterpolated(findex));
            animatedLineColors.push_back(completeLineColors[floor(findex)]);
            
            for (int i = ceil(findex); i < completeLine.size(); i++) {
                animatedLine.addVertex(completeLine[i]);
                animatedLineColors.push_back(completeLineColors[i]);
            }
            break;
        }
    }
    
    if (autoArm) {
        laserManager.armAllLasersListener();
    }
}

//--------------------------------------------------------------

void ofApp::draw() {
    ofBackground(20, 20, 25);
    
    string renderProfile;
    switch (renderProfileIndex) {
        case 0 :
            renderProfile = OFXLASER_PROFILE_DETAIL;
            break;
        case 1 :
            renderProfile = OFXLASER_PROFILE_DEFAULT;
            break;
        case 2 :
            renderProfile = OFXLASER_PROFILE_FAST;
            break;
    }
    renderProfileLabel = "Render Profile: OFXLASER_PROFILE_" + renderProfile;
    
    laserManager.beginDraw();
    
    ofPushMatrix();
    ofTranslate(0, 50);
    ofScale(scale, scale);

    laserManager.drawPoly(animatedLine, animatedLineColors, renderProfile);
    
    ofPopMatrix();
    
    laserManager.endDraw();
    laserManager.send();
    
    laserManager.drawUI();
    
    // Wall separation Line
    ofPushStyle();
    ofSetColor(100, 40, 40);
    ofDrawLine(400, 8, 400, laserHeight + 8);
    ofPopStyle();
    
    // Keybind Info
    float w = ofGetWidth() / 2;
    ofDrawBitmapStringHighlight("F              Fullscreen      ", w, ofGetHeight() - 140, ofGetKeyPressed('f') ? ofColor::gray : ofColor::black);
    ofDrawBitmapStringHighlight("Shift+Tab/Tab  Prev/Next Laser ", w, ofGetHeight() - 120, ofGetKeyPressed(OF_KEY_TAB) ? ofColor::gray : ofColor::black);
    ofDrawBitmapStringHighlight("Left/Right     Prev/Next SVG   ", w, ofGetHeight() - 100, ofGetKeyPressed(OF_KEY_LEFT) || ofGetKeyPressed(OF_KEY_RIGHT) ? ofColor::gray : ofColor::black);
    ofDrawBitmapStringHighlight("Space          Play/Pause      ", w, ofGetHeight() -  80, ofGetKeyPressed(' ') ? ofColor::gray : ofColor::black);
    ofDrawBitmapStringHighlight("R              Reset           ", w, ofGetHeight() -  60, ofGetKeyPressed('r') ? ofColor::gray : ofColor::black);
    ofDrawBitmapStringHighlight("S              Save Settings   ", w, ofGetHeight() -  40, ofGetKeyPressed('s') ? ofColor::gray : ofColor::black);
    ofDrawBitmapStringHighlight("Esc            Quit Immediately", w, ofGetHeight() -  20, ofGetKeyPressed(OF_KEY_ESC) ? ofColor::gray : ofColor::black);
}

//--------------------------------------------------------------

void ofApp::keyPressed(int key) {
    if (key == 'f')          { ofToggleFullscreen(); }
    if (key == OF_KEY_TAB)   { ofGetKeyPressed(OF_KEY_SHIFT) ? laserManager.selectPreviousLaser() : laserManager.selectNextLaser(); }
    if (key == OF_KEY_LEFT)  { currentSVG = ofWrap(currentSVG - 1, 0, laserGraphics.size()); }
    if (key == OF_KEY_RIGHT) { currentSVG = ofWrap(currentSVG + 1, 0, laserGraphics.size()); }
    if (key == ' ')          { playAnimation = !playAnimation; }
    if (key == 'r')          { animationState = drawing; currentSVG = 0; t = 0; }
    if (key == 's')          { laserManager.saveSettings(); }
}
