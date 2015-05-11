/*
 * Copyright (C) 2015 Fondazione Istituto Italiano di Tecnologia
 * Authors: Silvio Traversaro
 * CopyPolicy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT
 *
 */

#include "Utils.h"
#include <iostream>
#include <cassert>

namespace iDynTree
{
    int UNKNOWN = -1;

    bool checkEqualOrUnknown(const int op1, const int op2)
    {
        return (op1 == op2) ||
               (op1  <   0) ||
               (op2  <   0);
    }

    bool checkEqualAndNotUnknown(const int op1, const int op2)
    {
        return (op1 == op2) &&
               (op1 >=   0) &&
               (op2 >=   0);
    }

    void reportError(const char* className, const char* methodName, const char* errorMessage)
    {
        std::cerr << "[ERROR] " << className << " :: " << methodName << " : " << errorMessage <<  "\n";
        assert(false);
    }

}