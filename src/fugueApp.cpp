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

const int BS = 32;
const double X = 10;
const double Y = 10;
const int S = X * Y;
const int TRANSPOSE = -5;

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
    // beats per second
    double bpm;
    // seconds per beat
    double spb;
    double secs;
    bool paused;
	float mMaxFreq;
private:
    std::vector<int> notes;
};

void fugueApp::setup() {
    notes.resize(S);
    mMaxFreq = 2000.0f;
	mFreqTarget = 0.0f;
	mPhase = 0.0f;
    bpm = 200.0f;
	mPhaseAdjust = 0.0f;
    paused = false;
	audio::Output::play(audio::createCallback(this, &fugueApp::sineWave));
    std::fill(notes.begin(), notes.end(), 0);
}

void fugueApp::sineWave( uint64_t inSampleOffset, uint32_t ioSampleCount, audio::Buffer32f *ioBuffer ) {
    mPhaseAdjust = mFreqTarget / 44100.0f;
	for (int i = 0; i < ioSampleCount; i++) {
		mPhase += mPhaseAdjust;
		mPhase = mPhase - math<float>::floor(mPhase);
		float val = math<float>::sin(mPhase * 2.0f * M_PI);
		
		ioBuffer->mData[i*ioBuffer->mNumberChannels] = val;
		ioBuffer->mData[i*ioBuffer->mNumberChannels + 1] = val;
	}
}

void fugueApp::touchesBegan(TouchEvent event) {
    double ty = event.getTouches()[0].getY();
    double tx = event.getTouches()[0].getX();
    if (ty > (BS * Y)) {
        if (tx < BS) {
            paused = !paused;
        }
    } else {
        int idx = (std::floor(tx / BS) * X) +
            std::floor(ty / BS);
        if (notes[idx] == 0) {
            notes[idx] = 1;
        } else {
            notes[idx] = 0;
        }
    }
}

void fugueApp::draw() {
    spb = 60.0f / bpm;
    if (!paused) {
        secs = getElapsedSeconds();
    }
    int hlrow = std::fmod(std::floor(secs / spb), Y);
    bool blankrow = true;

    gl::setMatricesWindow(getWindowSize());
    gl::clear();

    gl::color(Color(1, 1, 1));
    
    // Draw the moving cursor
    gl::color(Color(1, 0.5, 1));
    gl::drawSolidRect(Rectf(fmod(secs / spb, Y) * BS, 0, fmod(secs / spb, Y) * (BS) + 1, BS * X));
    gl::color(Color(1, 1, 1));
    
    // Draw grid lines
    for (int x = 0; x <= X; x++) {
        gl::drawSolidRect(Rectf(0, x * BS, 320.0f, x * BS + 1));
    }
    for (int y = 0; y < Y; y++) {
        gl::drawSolidRect(Rectf(y * BS, 0, y * BS + 1, BS * X));
    }
    gl::drawSolidRect(Rectf(Y * BS - 1, 0, Y * BS, BS * X));
    
    // Draw a play button
    gl::color(Color(0.5, 1, 0.5));
    gl::drawSolidRect(Rectf(0, Y * BS, BS, (Y + 1) * BS));
    
    gl::color(Color(1, 1, 1));
    // Find and play the note
    for (int x = hlrow * X; x < (hlrow * X) + X; x++) {
        if (notes[x] == 1) {
            mFreqTarget = 440 * std::pow(2.0, ((x % (int) X) - TRANSPOSE) / 12.0);
            blankrow = false;
        }
    }
    if (blankrow) mFreqTarget = 0;
    
    // Draw notes
    for (int n = 0; n < S; n++) {
        if (notes[n] == 1) {
            if (std::floor(n / X) == hlrow) {
                double height = secs - std::floor(secs);
                std::clog << height << std::endl;
                std::clog << secs << std::endl;
                gl::color(Color(1, 0.5, 1));
                gl::drawSolidRect(Rectf(
                    std::floor(n / X) * BS + 1.0f,
                    std::floor(n % (int) X) * BS + 1.0f,
                    std::floor(n / X) * BS + (BS * height),
                    std::floor(n % (int) X) * BS + BS));
                gl::color(Color(1, 1, 1));
            } else {
                gl::drawSolidRect(Rectf(
                std::floor(n / X) * BS + 1.0f,
                std::floor(n % (int) X) * BS + 1.0f,
                std::floor(n / X) * BS + BS,
                std::floor(n % (int) X) * BS + BS));
            }
        }
    }
}

void fugueApp::resize(ResizeEvent event) { }

void fugueApp::update() { }

CINDER_APP_COCOA_TOUCH(fugueApp, RendererGl)