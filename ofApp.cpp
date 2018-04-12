#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup()
{
	video.setDeviceID(0);
    video.setDesiredFrameRate(60);
    video.initGrabber(640, 480);
    calculatedFlow = false;
    ofSetFrameRate(60);
    phase = 0;
}

//--------------------------------------------------------------
void ofApp::update(){
    
    //Decode the new frame if needed
	video.update();

	if ( video.isFrameNew() ){

		if ( gray1.bAllocated ) {
			gray2 = gray1;
			calculatedFlow = true;
		}

        //Convert to ofxOpenCv images
        ofPixels & pixels = video.getPixels();
        currentColor.setFromPixels( pixels );

        //Decimate images to 25% (makes calculations faster + works like a blurr too)
        float decimate = 0.25;
        ofxCvColorImage imageDecimated1;
        imageDecimated1.allocate( currentColor.width * decimate, currentColor.height * decimate );
        
        //High-quality resize
        imageDecimated1.scaleIntoMe( currentColor, CV_INTER_AREA );
        gray1 = imageDecimated1;

		if ( gray2.bAllocated ) {
            
            //Create OpenCV images
            Mat img1( gray1.getCvImage() );
            Mat img2( gray2.getCvImage() );
            //Image for flow
            Mat flow;                        
            
            //Computing optical flow (visit https://goo.gl/jm1Vfr for explanation of parameters)
            calcOpticalFlowFarneback( img1, img2, flow, 0.7, 3, 11, 5, 5, 1.1, 0 );
            
            //Split flow into separate images
            vector<Mat> flowPlanes;
            split( flow, flowPlanes );

            //Copy float planes to ofxCv images flowX and flowY
            //we call this to convert back from native openCV to ofxOpenCV data types
            IplImage iplX( flowPlanes[0] );
            flowX = &iplX;
            IplImage iplY( flowPlanes[1] );
            flowY = &iplY;

		}
	}
    phase += (avgX*10);
}

//--------------------------------------------------------------
void ofApp::draw(){
	ofBackground(0);	//Set the background color

    sumX = 0;
    sumY = 0;
    numOfEntries = 0;

    if (calculatedFlow){

        ofSetColor( 255, 255, 255 );
        
        // draw video feed to window
        // video.draw( 0, 0);

        int w = gray1.width;
        int h = gray1.height;

        //1. Input images + optical flow
        ofPushMatrix();
        ofScale( 4, 4 );

        //Optical flow
        float *flowXPixels = flowX.getPixelsAsFloats();
        float *flowYPixels = flowY.getPixelsAsFloats();

        ofSetColor( 0, 0, 255 );
        for (int y=0; y<h; y+=5) {
            for (int x=0; x<w; x+=5) {
                float fx = flowXPixels[ x + w * y ];
                float fy = flowYPixels[ x + w * y ];
                sumX += fx;
                sumY += fy;
                numOfEntries++;

                //Draw only long vectors
                
                // if ( fabs( fx ) + fabs( fy ) > 1 ) {
                // ofDrawRectangle( x-0.5, y-0.5, 1, 1 );
                // ofDrawLine( x, y, x + fx, y + fy );
                // }
            }
        }
        ofPopMatrix();
    }

    // if entries exist, calculate averages for all entries
    if(numOfEntries>0){
       avgX = ofClamp(sumX / numOfEntries,-5,5);
       avgY = sumY / numOfEntries;
    }

    //if(avgX > 0.075){
    //cout << "avgX: " << endl;
    //cout << avgX << endl;
    //}
    //
    // draw average vector for feed's optical flow
    ofPushMatrix();
    ofPushStyle();
        ofTranslate(ofGetWidth()/2,ofGetHeight()/2);
        ofSetColor(255,0,0);
        ofDrawLine(0,0,avgX*-100,avgY*-100);
    ofPopStyle();
    ofPopMatrix();

    // visual elements: 4 doors
    int numOfDoors = 4;
    for (int i=0; i<numOfDoors; i++){
        door(phase + i * 180/numOfDoors);
    }
}

//--------------------------------------------------------------
void ofApp::door(float p){

    if(avgX < 0.075){
    //if(!ofInRange(p,-20,20)){
    cout << "p: " << endl;
    cout << p << endl;
    }

    ofPushMatrix();
    ofPushStyle();
    ofSetRectMode(OF_RECTMODE_CENTER);
    ofTranslate(ofGetWidth()/2, ofGetHeight()/2);
    if(ofInRange(avgX,-0.075,0.075)){
    // rotate X, Y, Z via phase
    //ofRotateZ(p);
    ofScale(0.5);
    }
    ofRotateY(p);
    float s = abs(sin(ofDegToRad(p))) + 0.3;
    ofScale(s,s,s);
    ofNoFill();
    ofSetColor(255);
    ofSetLineWidth(3);
    ofDrawEllipse(0, 0, 300,300);
    ofPopStyle();
    ofPopMatrix();

}
