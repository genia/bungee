// Copyright (C) 2020-2025 Parabola Research Limited
// SPDX-License-Identifier: MPL-2.0

#pragma once

namespace Bungee::Internal {

struct Instrumentation
{
    static thread_local Instrumentation *threadLocal;
	bool enabled = false;
	int expected = 0;

	static void log(const char *format, ...);

    struct Call
    {
        Call(Instrumentation &instrumentation, int sequence);
        ~Call();
    };
};

}
