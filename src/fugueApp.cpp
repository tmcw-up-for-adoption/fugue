#include "cinder/audio/Output.h"
#include "cinder/audio/Callback.h"
#include "cinder/app/AppCocoaTouch.h"
#include "cinder/app/Renderer.h"
#include "cinder/Surface.h"
#include "cinder/gl/Texture.h"
#include "cinder/Camera.h"
#include <vector>

using namespace ci;
using namespace ci::app;

const int BS = 20;
const int X = 32;
const int Y = 16;
const int S = X * Y;

class fugueApp : public AppCocoaTouch {
  public:
	virtual void setup();
    virtual void sineWave(uint64_t inSampleOffset,
                          uint32_t ioSampleCount,
                          audio::Buffer32f *ioBuffer);
	virtual void resize(ResizeEvent event);
	virtual void update();
	virtual void draw();
	virtual void touchesBegan(TouchEvent event);
	float mFreqTarget;
	float mPhase;
	float mPhaseAdjust;
	float mMaxFreq;
  private:
    std::vector<int> notes;
};

void fugueApp::setup() {
    notes.resize(S);
    mMaxFreq = 2000.0f;
	mFreqTarget = 0.0f;
	mPhase = 0.0f;
	mPhaseAdjust = 0.0f;
	audio::Output::play(audio::createCallback(this, &fugueApp::sineWave));
    std::fill(notes.begin(), notes.end(), 0);
}

void fugueApp::sineWave( uint64_t inSampleOffset, uint32_t ioSampleCount, audio::Buffer32f *ioBuffer ) {
    mPhaseAdjust = mFreqTarget / 44100.0f;
	for(int i = 0; i < ioSampleCount; i++) {
		mPhase += mPhaseAdjust;
		mPhase = mPhase - math<float>::floor(mPhase);
		float val = math<float>::sin(mPhase * 2.0f * M_PI);
		
		ioBuffer->mData[i*ioBuffer->mNumberChannels] = val;
		ioBuffer->mData[i*ioBuffer->mNumberChannels + 1] = val;
	}
}

void fugueApp::resize(ResizeEvent event) {
}

void fugueApp::touchesBegan(TouchEvent event) {
    int idx = 
      (std::floor(event.getTouches()[0].getX() / BS) * X) +
    std::floor(event.getTouches()[0].getY() / BS);
    if (notes[idx] == 0) {
        notes[idx] = 1;
    } else {
        notes[idx] = 0;
    }
}

void fugueApp::update() {
}

void fugueApp::draw() {
    gl::setMatricesWindow(getWindowSize());
    gl::clear();
    gl::color(Color(1, 1, 1));
    
    int secs = getElapsedSeconds() * 5;
    int hlrow = secs % Y;
    
    // 16 40px blocks
    for (int x = 0; x < X; x++) {
        gl::drawSolidRect(Rectf(0, x * BS, 320.0f, x * BS + 1));
    }
    // 8 40px blocks
    for (int y = 0; y < Y; y++) {
        if (y == hlrow) gl::color(Color(1, 0.5, 1));
        gl::drawSolidRect(Rectf(y * BS, 0, y * BS + 1, 640.0f));
        if (y == hlrow) gl::color(Color(1, 1, 1));
    }
    gl::color(Color(1, 1, 1));
    for (int n = 0; n < S; n++) {
        if (notes[n] == 1) {
            if (std::floor(n / X) == hlrow) {
                gl::color(Color(1, 0.5, 1));
                mFreqTarget = 440 * std::pow(2.0, ((n % X)) / 12.0);
                std::clog << mFreqTarget << std::endl;
            }
            gl::drawSolidRect(Rectf(
                std::floor(n / X) * BS + 1.0f,
                std::floor(n % X) * BS + 1.0f,
                std::floor(n / X) * BS + BS,
                std::floor(n % X) * BS + BS));
            if (std::floor(n / X) == hlrow) gl::color(Color(1, 1, 1));
        }
    }
}

CINDER_APP_COCOA_TOUCH(fugueApp, RendererGl)