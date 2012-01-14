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

const int BS = 64;
const double X = 10;
const double Y = 14;
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
    // beats per second
    double bpm;
    // seconds per beat
    double spb;
    double secs;
    bool paused;
	float mMaxFreq;
private:
    std::vector<int> chord;
    std::vector<int> notes;
    std::vector<float> phases;
};

void fugueApp::setup() {
    notes.resize(S);
    mMaxFreq = 2000.0f;
	mFreqTarget = 0.0f;
	mPhase = 0.0f;
    bpm = 300.0f;
    paused = false;
	audio::Output::play(audio::createCallback(this, &fugueApp::sineWave));
    std::fill(notes.begin(), notes.end(), 0);
    gl::setMatricesWindow(getWindowSize());
}

void fugueApp::sineWave( uint64_t inSampleOffset, uint32_t ioSampleCount, audio::Buffer32f *ioBuffer ) {
    phases.empty();
    for (int i = 0; i < chord.size(); i++) {
        phases.push_back(440 * std::pow(2.0, ((chord.at(i) % (int) X) - TRANSPOSE) / 12.0) / 44100.0f);
    }
    float val = 0;
	for (int i = 0; i < ioSampleCount; i++) {
        val = 0;
        for (int j = 0; j < phases.size(); j++) {
            mPhase += phases[j];
            mPhase = mPhase - math<float>::floor(mPhase);
            val += math<float>::sin(mPhase * 2.0f * M_PI);
        }
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
            if (paused) mFreqTarget = 0;
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
    int hlrow = std::fmod(std::floor(secs / spb), X);
    bool blankrow = true;

    gl::clear();
    gl::color(Color(1, 1, 1));
    
    // Draw the moving cursor
    gl::color(Color(1, 0.5, 1));
    gl::drawSolidRect(Rectf(fmod(secs / spb, X) * BS, 0, fmod(secs / spb, X) * (BS) + 1, BS * Y));
    
    // Draw grid lines
    gl::color(Color(0.4, 0.4, 0.4));
    for (int y = 0; y <= Y; y++) {
        gl::drawSolidRect(Rectf(0, y * BS, 640.0f, y * BS + 1));
    }
    for (int x = 0; x < X; x++) {
        gl::drawSolidRect(Rectf(x * BS, 0, x * BS + 1, BS * Y));
    }
    gl::drawSolidRect(Rectf(X * BS - 1, 0, X * BS, BS * Y));
    
    // Draw a play button
    if (paused) {
        gl::color(Color(0.5, 1, 0.5));
    } else {
        gl::color(Color(0.5, 0.5, 0.5));
    }
    gl::drawSolidRect(Rectf(0, Y * BS, BS + 1, (Y + 1) * BS + 1));
    gl::color(Color(0, 0, 0));
    gl::drawSolidCircle(Vec2f(BS / 2, Y * BS + (BS / 2)), BS / 4, 3);

    gl::color(Color(1, 1, 1));
    // Find and play the note
    chord.empty();
    for (int x = hlrow * X; x < (hlrow * X) + X; x++) {
        if (notes[x] == 1) {
            blankrow = false;
            chord.push_back(x);
        }
    }
    if (blankrow || paused) mFreqTarget = 0;
    
    // Draw notes
    for (int n = 0; n < S; n++) {
        if (notes[n] == 1) {
            if (std::floor(n / X) == hlrow) {
                double width = (secs / spb) - std::floor(secs / spb);
                gl::color(Color(1, 0.5, 1));
                gl::drawSolidRect(Rectf(
                    std::floor(n / X) * BS + 1.0f,
                    std::floor(n % (int) X) * BS + 1.0f,
                    std::floor(n / X) * BS + (BS * width),
                    std::floor(n % (int) X) * BS + BS));
                gl::color(Color(1, 1, 1));
            } else {
                if (std::floor(n / X) > hlrow) {
                    gl::color(1, 0.2, 1);
                } else {
                    gl::color(1, 1, 1);
                }
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