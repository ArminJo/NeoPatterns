/*
 * DebugLevel.h
 * Include to propagate debug levels
 *
 *  Copyright (C) 2016-2020  Armin Joachimsmeyer
 *  Email: armin.joachimsmeyer@gmail.com
 *
 *  This file is part of Arduino-Utils https://github.com/ArminJo/Arduino-Utils.
 *
 *  Arduino-Utils is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/gpl.html>.
 *
 */

#ifndef DEBUGLEVEL_H_
#define DEBUGLEVEL_H_

// Propagate debug level
#ifdef TRACE
#  ifndef DEBUG
#define DEBUG
#  endif
#endif
#ifdef DEBUG
#  ifndef INFO
#define INFO
#  endif
#endif
#ifdef INFO
#  ifndef WARN
#define WARN
#  endif
#endif
#ifdef WARN
#  ifndef ERROR
#define ERROR
#  endif
#endif

#endif /* DEBUGLEVEL_H_ */

#pragma once
