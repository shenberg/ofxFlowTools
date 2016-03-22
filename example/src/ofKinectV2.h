#pragma once

#include <libfreenect2/libfreenect2.hpp>
#include <libfreenect2/frame_listener_impl.h>
#include <ofMain.h>

#define KINECT_LOG "KinectV2"
class ofKinectV2 {
public:
	ofKinectV2() : listener(libfreenect2::Frame::Color | libfreenect2::Frame::Depth),
		device(nullptr), isNewFrame(false), pipeline(nullptr)
	{

	}
	int setup(int w, int h) {
		videoTexture.allocate(w, h, GL_RGB); //TODO: not sure

		if (freenect2.enumerateDevices() == 0)
		{
			ofLogError(KINECT_LOG) << "no device connected!";
			return -1;
		}

		serial = freenect2.getDefaultDeviceSerialNumber();
		pipeline = new libfreenect2::CpuPacketPipeline();
		device = freenect2.openDevice(serial, pipeline);


		if (device == 0)
		{
			ofLogError(KINECT_LOG) << "failure opening device!";
			return -1;
		}
		/*
		int types = 0;
		if (true) //(enable_rgb)
		types |= libfreenect2::Frame::Color;
		if (true) //(enable_depth)
		types |= libfreenect2::Frame::Ir | libfreenect2::Frame::Depth;
		libfreenect2::SyncMultiFrameListener listener(types);
		*/

		device->setColorFrameListener(&listener);
		device->setIrAndDepthFrameListener(&listener);
		/// [listeners]

		/// [start]

		if (!device->start())
			return -1;

		ofLogNotice(KINECT_LOG) << "device serial: " << device->getSerialNumber();
		ofLogNotice(KINECT_LOG) << "device firmware: " << device->getFirmwareVersion();

		return 0;
	}

	bool isFrameNew() const {
		return isNewFrame;
	}

	void update() {
		isNewFrame = false;

		if (device == nullptr) {
			return;
		}
		if (listener.hasNewFrame()) {
			isNewFrame = true;
			listener.waitForNewFrame(frames);
			libfreenect2::Frame *rgb = frames[libfreenect2::Frame::Color];
			libfreenect2::Frame *ir = frames[libfreenect2::Frame::Ir];
			libfreenect2::Frame *depth = frames[libfreenect2::Frame::Depth];

			//TODO see how to get the data
			videoTexture.loadData(rgb->data, rgb->width, rgb->height, GL_RGB);


			listener.release(frames);
		}
	}

	void draw(int x, int y, int w, int h) {
		videoTexture.draw(x, y, w, h);
	}

	virtual ~ofKinectV2() {
		if (device != nullptr) {
			device->stop();
			device->close();
			delete device;
			if (pipeline != nullptr) {
				delete pipeline;
			}
		}
	}

private:
	libfreenect2::Freenect2Device *device;
	libfreenect2::Freenect2 freenect2;
	std::string serial;
	libfreenect2::FrameMap frames;
	libfreenect2::SyncMultiFrameListener listener;
	libfreenect2::CpuPacketPipeline *pipeline;
	ofTexture videoTexture;
	bool isNewFrame;
};