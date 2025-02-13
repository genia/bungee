// Copyright (C) 2020-2025 Parabola Research Limited
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "Fourier.h"
#include "Instrumentation.h"
#include "Output.h"
#include "Partials.h"
#include "Phase.h"
#include "Stretch.h"
#include "Window.h"

#include "bungee/../src/log2.h"
#include "bungee/Bungee.h"

#include <Eigen/Core>

#include <array>
#include <complex>
#include <memory>
#include <numbers>
#include <random>
#include <sstream>

namespace Bungee {

struct Grain
{
	struct Analysis
	{
		double positionError;
		double hopIdeal;
		double speed;
		int hop; // rounded
	};

	int log2TransformLength;
	Request request;

	double requestHop{};
	bool continuous{};
	int passthrough{};
	int validBinCount{};
	int muteFrameCountHead{};
	int muteFrameCountTail{};

	Resample::Operations resampleOperations{};

	InputChunk inputChunk{};
	Analysis analysis{};

	Eigen::ArrayXXcf transformed;
	Eigen::ArrayX<Phase::Type> phase;
	Eigen::ArrayXf energy;
	Eigen::ArrayX<Phase::Type> rotation;
	Eigen::ArrayX<Phase::Type> delta;
	std::vector<Partials::Partial> partials;
	Resample::Padded inputResampled;
	Eigen::ArrayXXf inputCopy;

	Output::Segment segment;

	Grain(int log2SynthesisHop, int channelCount);

	InputChunk specify(const Request &request, Grain &previous, SampleRates sampleRates, int log2SynthesisHop, double bufferStartPosition);

	bool reverse() const
	{
		return analysis.hop < 0;
	}

	bool valid() const
	{
		return !std::isnan(request.position);
	}

	void applyEnvelope();

	auto inputChunkMap(const float *data, std::ptrdiff_t stride, int &muteFrameCountHead, int &muteFrameCountTail, const Grain &previous)
	{
		if (!data && muteFrameCountHead + muteFrameCountTail < inputChunk.end - inputChunk.begin)
		{
			Internal::Instrumentation::log("FATAL: data==nullptr but we need valid audio input for this frame");
			std::abort();
		}

		typedef Eigen::OuterStride<Eigen::Dynamic> Stride;
		typedef Eigen::Map<Eigen::ArrayXXf, 0, Stride> Map;

		const auto frameCount = inputChunk.end - inputChunk.begin;

		muteFrameCountHead = std::clamp<int>(muteFrameCountHead, 0, frameCount);
		muteFrameCountTail = std::clamp<int>(muteFrameCountTail, 0, frameCount);

		Map m((float *)data, frameCount, transformed.cols(), Stride(stride));
		BUNGEE_ASSERT2(!m.middleRows(muteFrameCountHead, m.rows() - muteFrameCountHead - muteFrameCountTail).hasNaN());

		if (Internal::Instrumentation::threadLocal->enabled)
			overlapCheck(m, muteFrameCountHead, muteFrameCountTail, previous);

		return m;
	}

	void overlapCheck(Eigen::Ref<Eigen::ArrayXXf> input, int muteFrameCountHead, int muteFrameCountTail, const Grain &previous);

	Eigen::Ref<Eigen::ArrayXXf> resampleInput(Eigen::Ref<Eigen::ArrayXXf> input, int log2WindowLength, int &muteFrameCountHead, int &muteFrameCountTail);
};

} // namespace Bungee
