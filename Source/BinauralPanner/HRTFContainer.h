#pragma once
#include <array>
#include <map>
#include <atomic>
#include "../JuceLibraryCode/JuceHeader.h"

#include "triangle++/include/del_interface.hpp"
struct HRIRBuffer
{
	static const auto HRIR_SIZE = 200u;
	using ImpulseResponse = std::array<float, HRIR_SIZE>;

	ImpulseResponse leftEarIR;
	ImpulseResponse rightEarIR;
};

class HRTFContainer
{
public:
	HRTFContainer();
	~HRTFContainer();

	void updateHRIR(double azimuth, double elevation);
	const HRIRBuffer& hrir() const;

	void loadHrir();
private:
	static int getElvIndex(int elv);

	std::map<int, std::array<HRIRBuffer, 52>> hrirDict_;

#ifdef TPP
  ScopedPointer<tpp::Delaunay> triangulation_;
#else
  ScopedPointer<class Delaunay> triangulation_;
#endif

	HRIRBuffer hrir_[2];
	std::atomic_int hrirReadIndex;
};
