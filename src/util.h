/*
 * Copyright (C) 2020 Björn Spindel
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

#pragma once

#include <stdbool.h>

enum PowerSupplyType {
	PowerSupplyType_Unknown,
	PowerSupplyType_Battery,
	PowerSupplyType_UPS,
	PowerSupplyType_Mains,
	PowerSupplyType_USB
};

enum PowerSupplyStatus {
	PowerSupplyStatus_Unknown,
	PowerSupplyStatus_Offline,
	PowerSupplyStatus_OnlineFixed,
	PowerSupplyStatus_OnlineProgrammable
};

typedef struct {
	char* path;
	enum PowerSupplyType type;
	enum PowerSupplyStatus status;
} PowerSupply;

int getPowerSupplies(PowerSupply**);
bool isRunningOnBattery();

