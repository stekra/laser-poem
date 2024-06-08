#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main(int argc, char *argv[]){
	ofSetupOpenGL(1500, 750, OF_WINDOW);
    
    ofApp *app = new ofApp();
            
    for (int i = 0; i < argc; i++) {
        if (ofToString(argv[i]) == "--autoArm") {
            app->autoArm = true;
        }
    }
    
	ofRunApp(app);
}
