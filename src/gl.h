/*
 * Copyright (C) 2018 Björn Spindel
 *
 * This file is part of libstrangle.
 *
 * libstrangle is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libstrangle is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libstrangle.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GL_H_
#define GL_H_

#define GL_NEAREST                        0x2600
#define GL_LINEAR                         0x2601

#define GL_NEAREST_MIPMAP_NEAREST         0x2700
#define GL_LINEAR_MIPMAP_NEAREST          0x2701
#define GL_NEAREST_MIPMAP_LINEAR          0x2702
#define GL_LINEAR_MIPMAP_LINEAR           0x2703

#define GL_TEXTURE_MAG_FILTER             0x2800
#define GL_TEXTURE_MIN_FILTER             0x2801

#define GL_MAX_TEXTURE_LOD_BIAS           0x84FD
#define GL_TEXTURE_LOD_BIAS               0x8501

#define TEXTURE_MAX_ANISOTROPY_EXT        0x84FE
#define TEXTURE_MAX_ANISOTROPY            0x84FE

#define GL_TEXTURE_ENV_MODE               0x2200
#define GL_TEXTURE_FILTER_CONTROL         0x8500

#define GL_TEXTURE_2D                     0x0DE1

#endif
