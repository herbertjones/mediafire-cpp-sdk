//
// di_debug_tmp.hpp
// DebugTest
//
// Created by Zach Marquez on 2/12/14.
// Copyright (c) 2014 MediaFire, LLC. All rights reserved.
//
// The purpose of this file is to temporarily define some macros for debug
// output until the correct macros are defined. Then this header will be
// removed and the macros replaced with the correct version.
//
#pragma once
#include <iostream>

#define DiLogOutput(x) std::cout << "[" #x "] "

#define DiCritical() DiLogOutput(Critical)
#define DiError() DiLogOutput(Error)
#define DiInfo() DiLogOutput(Info)
#define DiVerbose() DiLogOutput(Verbose)
#define DiVeryVerbose() DiLogOutput(VeryVerbose)
